#ifndef MIGANGBOT_EVENT_EVENT_HANDLER_H_
#define MIGANGBOT_EVENT_EVENT_HANDLER_H_

#include <initializer_list>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <array>
#include <algorithm>

#include <co/co.h>
#include <nlohmann/json.hpp>
#include <jpcre2.hpp>

#include "event/trie.h"
#include "event/event_filter.h"
#include "event/regex_matcher.h"
#include "logger/logger.h"
#include "bot/onebot_11/api_bot.h"
#include "utility.h"
#include "permission/permission.h"
#include "event/type.h"

namespace white
{

// 存储命令的array的大小，取决于权限设置，Superuser总是为最大值
constexpr auto kCommandArraySize = permission::SUPERUSER + 1;

class EventHandler
{
public:
    static EventHandler &GetInstance() noexcept
    {
        static EventHandler event_handler;
        return event_handler;
    }

    void InitFilter(std::unique_ptr<EventFilter> &&filter)
    {
        filter_ = std::move(filter);
    }

public:
    template<typename F>
    bool RegisterCommand(const int command_type, const std::string &command, F &&func, int permission = permission::NORMAL, bool only_to_me = false);

    template<typename F>
    bool RegisterNotice(const std::string &notice_type, const std::string &sub_type, F &&func);

    template<typename F>
    bool RegisterRequest(const std::string &request_type, const std::string &sub_type, F &&func);

    template<typename F>
    bool RegisterRegex(const std::initializer_list<std::string> &patterns, F &&func, int permission = permission::NORMAL);

    bool Handle(Event &event, onebot11::ApiBot &bot) noexcept;

public:
    EventHandler(const EventHandler &) = delete;
    EventHandler &operator=(const EventHandler &) = delete;
    EventHandler(const EventHandler &&) = delete;
    EventHandler &operator=(const EventHandler &&) = delete;

private:
    EventHandler() {}
    ~EventHandler() {}

private:
    const plugin_func &MatchedHandler(Event &event, const std::string &message) const noexcept;
    const SearchResult MatchHelper(int permission, const std::string &message, bool only_to_me) const noexcept;

private:
    std::array<std::unordered_map<std::string, plugin_func>, kCommandArraySize> command_fullmatch_each_perm_;
    std::array<Trie, kCommandArraySize> command_prefix_each_perm_;
    std::array<Trie, kCommandArraySize> command_suffix_each_perm_;
    std::array<std::unordered_map<std::string, plugin_func>, kCommandArraySize> command_keyword_each_perm_;

    std::array<std::unordered_map<std::string, plugin_func>, kCommandArraySize> command_fullmatch_each_perm_to_me_;
    std::array<Trie, kCommandArraySize> command_prefix_each_perm_to_me_;
    std::array<Trie, kCommandArraySize> command_suffix_each_perm_to_me_;
    std::array<std::unordered_map<std::string, plugin_func>, kCommandArraySize> command_keyword_each_perm_to_me_;

    std::array<std::vector<RegexMatcher>, kCommandArraySize> command_regex_each_perm_;

    std::array<std::vector<plugin_func>, kCommandArraySize> all_msg_handler_each_perm_;

    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<plugin_func>>> notice_handler_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<plugin_func>>> request_handler_;
    plugin_func no_func_avaliable_;

    std::unique_ptr<EventFilter> filter_;
};



template<typename F>
inline bool EventHandler::RegisterCommand(const int command_type, const std::string &command, F &&func, int permission, bool only_to_me)
{
    switch(command_type)
    {
        case FULLMATCH:
            if(only_to_me)
                return command_fullmatch_each_perm_to_me_[permission].emplace(std::move(command), std::forward<F>(func)).second;
            return command_fullmatch_each_perm_[permission].emplace(std::move(command), std::forward<F>(func)).second;
            break;
        case PREFIX:
            if(only_to_me)
                return command_prefix_each_perm_to_me_[permission].Insert(std::move(command), std::forward<F>(func));
            return command_prefix_each_perm_[permission].Insert(std::move(command), std::forward<F>(func));
            break;
        case SUFFIX:
        {
            if(only_to_me)
                return command_suffix_each_perm_to_me_[permission].InsertFromBack(std::move(command), std::forward<F>(func));
            return command_suffix_each_perm_[permission].InsertFromBack(std::move(command), std::forward<F>(func));
            break;
        }
        case KEYWORD:
            if(only_to_me)
                return command_keyword_each_perm_to_me_[permission].emplace(std::move(command), std::forward<F>(func)).second;
            return command_fullmatch_each_perm_[permission].emplace(std::move(command), std::forward<F>(func)).second;
            break;
        case ALLMSG:
            all_msg_handler_each_perm_[permission].push_back(std::forward<F>(func));
            return true;
            break;
        default:
            return command_fullmatch_each_perm_[permission].emplace(std::move(command), std::forward<F>(func)).second;
    }
    return true;
}

template<typename F>
inline bool EventHandler::RegisterNotice(const std::string &notice_type, const std::string &sub_type, F &&func)
{
    notice_handler_[notice_type][sub_type].push_back(std::forward<F>(func));
    return true;
}

template<typename F>
inline bool EventHandler::RegisterRequest(const std::string &request_type, const std::string &sub_type, F &&func)
{
    request_handler_[request_type][sub_type].push_back(std::forward<F>(func));
    return true;
}

template<typename F>
inline bool EventHandler::RegisterRegex(const std::initializer_list<std::string> &patterns, F &&func, int permission)
{
    command_regex_each_perm_[permission].push_back(RegexMatcher(patterns, std::forward<F>(func)));
    return true;
}

inline bool EventHandler::Handle(Event &event, onebot11::ApiBot &bot) noexcept
{
    if(filter_ && !filter_->operator()(event))
        return false;
    if(event.contains("post_type"))
    {
        auto post_type = event["post_type"].get<std::string>();
        switch(post_type[3])
        {
            // message
            case 's':
            {
                if(event["message_type"].get<std::string>()[0] == 'p')
                {
                    if(event["sub_type"].get<std::string>()[0] == 'f')
                        LOG_INFO("Bot[{}]收到来自好友[{}({})]的消息: {}", 
                            event["self_id"].get<QId>(), 
                            event["sender"].value("nickname", ""),
                            event["sender"].value("user_id", 0),
                            event.value("message", "Unknown message")
                        );
                }else
                {
                    if(event["anonymous"] != nullptr)
                    {
                        LOG_INFO("Bot[{}]收到来自群[{}]的匿名消息: {}", 
                            event["self_id"].get<QId>(), 
                            event["group_id"].get<GId>(),
                            event.value("message", "Unknown message")
                        );
                    }else
                    {
                        LOG_INFO("Bot[{}]收到来自群[{}]成员[{}({})]的消息: {}", 
                            event["self_id"].get<QId>(), 
                            event["group_id"].get<GId>(),
                            event["sender"].value("nickname", ""),
                            event["sender"].value("user_id", 0),
                            event.value("message", "Unknown message")
                        );
                    }   
                }
                auto message = event["message"].get<std::string>();
                const auto &func = MatchedHandler(event, message);
                if(func)
                    go([&func, event, &bot]()
                    {
                        func(event, bot);
                    }); // 原对象会消失，event必须拷贝
                else
                {
                    // check_regex
                    // check all
                    switch(permission::GetUserPermission(event))
                    {
                        case permission::SUPERUSER:
                            for(auto &regex_matcher : command_regex_each_perm_[permission::SUPERUSER])
                                if(regex_matcher.Check(message))
                                    go([&regex_matcher, event, &bot]()
                                    {
                                        regex_matcher.GetFunc()(event, bot);
                                    });
                            for(const auto &func : all_msg_handler_each_perm_[permission::SUPERUSER])
                                go([&func, event, &bot]()
                                {
                                    func(event, bot);
                                });
                        case permission::WHITE_LIST:
                            for(auto &regex_matcher : command_regex_each_perm_[permission::WHITE_LIST])
                                if(regex_matcher.Check(message))
                                    go([&regex_matcher, event, &bot]()
                                    {
                                        regex_matcher.GetFunc()(event, bot);
                                    });
                            for(const auto &func : all_msg_handler_each_perm_[permission::WHITE_LIST])
                                go([&func, event, &bot]()
                                {
                                    func(event, bot);
                                });
                        case permission::GROUP_OWNER:
                            for(auto &regex_matcher : command_regex_each_perm_[permission::GROUP_OWNER])
                                if(regex_matcher.Check(message))
                                    go([&regex_matcher, event, &bot]()
                                    {
                                        regex_matcher.GetFunc()(event, bot);
                                    });
                            for(const auto &func : all_msg_handler_each_perm_[permission::GROUP_OWNER])
                                go([&func, event, &bot]()
                                {
                                    func(event, bot);
                                });
                        case permission::GROUP_ADMIN:
                            for(auto &regex_matcher : command_regex_each_perm_[permission::GROUP_ADMIN])
                                if(regex_matcher.Check(message))
                                    go([&regex_matcher, event, &bot]()
                                    {
                                        regex_matcher.GetFunc()(event, bot);
                                    });
                            for(const auto &func : all_msg_handler_each_perm_[permission::GROUP_ADMIN])
                                go([&func, event, &bot]()
                                {
                                    func(event, bot);
                                });
                        case permission::GROUP_MEMBER:
                            for(auto &regex_matcher : command_regex_each_perm_[permission::GROUP_MEMBER])
                                if(regex_matcher.Check(message))
                                    go([&regex_matcher, event, &bot]()
                                    {
                                        regex_matcher.GetFunc()(event, bot);
                                    });
                            for(const auto &func : all_msg_handler_each_perm_[permission::GROUP_MEMBER])
                                go([&func, event, &bot]()
                                {
                                    func(event, bot);
                                });
                        case permission::PRIVATE:
                            for(auto &regex_matcher : command_regex_each_perm_[permission::PRIVATE])
                                if(regex_matcher.Check(message))
                                    go([&regex_matcher, event, &bot]()
                                    {
                                        regex_matcher.GetFunc()(event, bot);
                                    });
                            for(const auto &func : all_msg_handler_each_perm_[permission::PRIVATE])
                                go([&func, event, &bot]()
                                {
                                    func(event, bot);
                                });
                        case permission::NORMAL:
                            for(auto &regex_matcher : command_regex_each_perm_[permission::NORMAL])
                                if(regex_matcher.Check(message))
                                    go([&regex_matcher, event, &bot]()
                                    {
                                        regex_matcher.GetFunc()(event, bot);
                                    });
                            for(const auto &func : all_msg_handler_each_perm_[permission::NORMAL])
                                go([&func, event, &bot]()
                                {
                                    func(event, bot);
                                });
                        case permission::BLACK:
                            for(auto &regex_matcher : command_regex_each_perm_[permission::BLACK])
                                if(regex_matcher.Check(message))
                                    go([&regex_matcher, event, &bot]()
                                    {
                                        regex_matcher.GetFunc()(event, bot);
                                    });
                            for(const auto &func : all_msg_handler_each_perm_[permission::BLACK])
                                go([&func, event, &bot]()
                                {
                                    func(event, bot);
                                });
                        default:
                            break;
                    } 
                }  
            }
            break;
            // notice
            case 'i':
            {
                auto notice_type = event.value("notice_type", "");
                auto sub_type = event.value("sub_type", "");
                LOG_INFO("Bot[{}] 收到一个通知事件: {}.{}", event["self_id"].get<QId>(), notice_type, sub_type);
                if(notice_handler_.count(notice_type) && notice_handler_.at(notice_type).count(sub_type))
                {
                    for(auto &func : notice_handler_.at(notice_type).at(sub_type))
                        go([&func, event, &bot]()
                        {
                            func(event, bot);
                        });
                }
            }
            break;
            // request
            case 'u':
            {
                auto request_type = event.value("request_type", "");
                auto sub_type = event.value("sub_type", "");
                LOG_INFO("Bot[{}] 收到一个请求事件: {}.{}", event["self_id"].get<QId>(), request_type, sub_type);
                if(request_handler_.count(request_type) && request_handler_.at(request_type).count(sub_type))
                {
                    for(auto &func : request_handler_.at(request_type).at(sub_type))
                        go([&func, event, &bot]()
                        {
                            func(event, bot);
                        });
                }
            }
            break;
            // meta_event
            case 'a':
            {
                auto meta_event_type = event["meta_event_type"].get<std::string>();
                if(meta_event_type[0] == 'l')
                {
                    auto sub_type = event["sub_type"].get<std::string>();
                    switch(sub_type[0])
                    {
                        case 'e':
                        case 'c':
                            LOG_INFO("Bot[{}]已成功建立连接", event["self_id"].get<QId>());
                            break;
                        case 'd':
                            LOG_INFO("Bot[{}]已断开连接", event["self_id"].get<QId>());
                            break;
                    }
                }else
                {
                    LOG_DEBUG("Bot[{}]与[{}]收到一次心跳连接", event["self_id"].get<QId>(), event["time"].get<int64_t>());
                }
            }
            break;
            default:
                break;
        }
    }
    return true;
}

inline const plugin_func &EventHandler::MatchedHandler(Event &event, const std::string &message) const noexcept
{
    if(message.empty())
        return no_func_avaliable_;
    if(event.is_null())
        return no_func_avaliable_;
    std::string_view msg(message);
    bool only_to_me = false;
    if(msg.starts_with("[CQ:at"))
    {
        auto at_id_start = msg.find_first_of('=') + 1;
        if(at_id_start != std::string_view::npos)
        {
            auto at_id_end = msg.find_first_of(']');
            if(at_id_end != std::string_view::npos)
            {
                auto at_id = msg.substr(at_id_start, at_id_end - at_id_start);
                auto self_id = std::to_string(event["self_id"].get<QId>());
                if(at_id == self_id)
                {
                    only_to_me = true;
                    msg = msg.substr(msg.find_first_of(']') + 1);
                    msg = msg.substr(msg.find_first_not_of(' '));
                    event["__to_me__"] = true;
                }
            }
        }
    }
    auto msg_str = std::string(msg);
    auto perm = permission::GetUserPermission(event);
    switch(perm)
    {
        case permission::SUPERUSER:
        {
            auto func_ret = MatchHelper(permission::SUPERUSER, msg_str, only_to_me);
            if (func_ret.func)
            {
                event["__command_size__"] = func_ret.command_size;
                return func_ret.func;
            }
        }
        case permission::WHITE_LIST:
        {
            auto func_ret = MatchHelper(permission::WHITE_LIST, msg_str, only_to_me);
            if (func_ret.func)
            {
                event["__command_size__"] = func_ret.command_size;
                return func_ret.func;
            }
        }       
        case permission::GROUP_OWNER:
        {
            auto func_ret = MatchHelper(permission::GROUP_OWNER, msg_str, only_to_me);
            if (func_ret.func)
            {
                event["__command_size__"] = func_ret.command_size;
                return func_ret.func;
            }
        }            
        case permission::GROUP_ADMIN:
        {
            auto func_ret = MatchHelper(permission::GROUP_ADMIN, msg_str, only_to_me);
            if (func_ret.func)
            {
                event["__command_size__"] = func_ret.command_size;
                return func_ret.func;
            }
        } 
        case permission::GROUP_MEMBER:
        {
            auto func_ret = MatchHelper(permission::GROUP_MEMBER, msg_str, only_to_me);
            if (func_ret.func)
            {
                event["__command_size__"] = func_ret.command_size;
                return func_ret.func;
            }
        }
        case permission::PRIVATE:
        {
            auto func_ret = MatchHelper(permission::PRIVATE, msg_str, only_to_me);
            if (func_ret.func)
            {
                event["__command_size__"] = func_ret.command_size;
                return func_ret.func;
            }
        }
        case permission::NORMAL:
        {
            auto func_ret = MatchHelper(permission::NORMAL, msg_str, only_to_me);
            if (func_ret.func)
            {
                event["__command_size__"] = func_ret.command_size;
                return func_ret.func;
            }
        }   
        case permission::BLACK:
        {
            auto func_ret = MatchHelper(permission::BLACK, msg_str, only_to_me);
            if (func_ret.func)
            {
                event["__command_size__"] = func_ret.command_size;
                return func_ret.func;
            }
        }
        default:
            return no_func_avaliable_;
    }   
    return no_func_avaliable_;
}

inline const SearchResult EventHandler::MatchHelper(int permission, const std::string &message, bool only_to_me) const noexcept
{
    if(only_to_me)
    {
        if (command_fullmatch_each_perm_to_me_[permission].count(message))
            return {command_fullmatch_each_perm_to_me_[permission].at(message), 0};
        auto &func_prefix_result = command_prefix_each_perm_to_me_[permission].Search(message);
        if(func_prefix_result.func)
            return func_prefix_result;
        auto &func_suffix_result = command_suffix_each_perm_to_me_[permission].SearchFromBack(message);
        if(func_suffix_result.func)
            return func_suffix_result;
    }else
    {
        if (command_fullmatch_each_perm_[permission].count(message))
            return {command_fullmatch_each_perm_[permission].at(message), 0};
        auto &func_prefix_result = command_prefix_each_perm_[permission].Search(message);
        if(func_prefix_result.func)
            return func_prefix_result;
        auto &func_suffix_result = command_suffix_each_perm_[permission].SearchFromBack(message);
        if(func_suffix_result.func)
            return func_suffix_result;
    }
    return {no_func_avaliable_, 0};
}

} // namespace white

#endif
#ifndef MIGANGBOTCPP_EVENT_HANDLER_EVENT_HANDLER_H_
#define MIGANGBOTCPP_EVENT_HANDLER_EVENT_HANDLER_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include "event/trie.h"
#include "event/event_filter.h"
#include "event/event.h"
#include <nlohmann/json.hpp>
#include <condition_variable>
#include <mutex>
#include <queue>
#include "pool/thread_pool.h"
#include "logger/logger.h"
#include "bot/api_bot.h"
#include "utility.h"

#include <iostream>

namespace white
{

using plugin_func = std::function<void(const Event &, onebot11::ApiBot &)>;

constexpr auto FULLMATCH = 0;
constexpr auto PREFIX = 1;
constexpr auto SUFFIX = 2;
constexpr auto KEYWORD = 3;
constexpr auto ALLMSG = 4;

class EventHandler
{
public:
    static EventHandler &GetInstance()
    {
        static EventHandler event_handler;
        return event_handler;
    }

    void Init(std::size_t thread_num)
    {
        pool_.reset(new ThreadPool(thread_num));
    }

    void InitFilter(std::unique_ptr<EventFilter> &&filter)
    {
        filter_ = std::move(filter);
    }

public:
    template<typename F>
    bool RegisterCommand(const int command_type, const std::string &command, F &&func);

    template<typename F>
    bool RegisterNotice(const std::string &notice_type, const std::string &sub_type, F &&func);

    template<typename F>
    bool RegisterRequest(const std::string &request_type, const std::string &sub_type, F &&func);

    bool Handle(const Event &event, onebot11::ApiBot &bot) const;


private:
    EventHandler() {}
    ~EventHandler() {}

    EventHandler(const EventHandler &) = delete;
    EventHandler &operator=(const EventHandler &) = delete;
    EventHandler(const EventHandler &&) = delete;
    EventHandler &operator=(const EventHandler &&) = delete;

private:
    const plugin_func &MatchedHandler(const Event &msg) const;
    const std::vector<plugin_func> &FreeHandler() const;

private:
    std::unordered_map<std::string, plugin_func> command_fullmatch_;
    Trie command_prefix_;
    Trie command_suffix_;
    std::unordered_map<std::string, plugin_func> command_keyword_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<plugin_func>>> notice_handler_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<plugin_func>>> request_handler_;
    std::vector<plugin_func> all_msg_handler_;
    plugin_func no_func_avaliable_;

    std::unique_ptr<EventFilter> filter_;
    std::unique_ptr<ThreadPool> pool_;
};

template<typename F>
inline bool EventHandler::RegisterCommand(const int command_type, const std::string &command, F &&func)
{
    switch(command_type)
    {
        case FULLMATCH:
            return command_fullmatch_.emplace(std::move(command), std::forward<F>(func)).second;
            break;
        case PREFIX:
            return command_prefix_.Insert(std::move(command), std::forward<F>(func));
            break;
        case SUFFIX:
        {
            auto w_command = str_to_wstr(command);
            std::reverse(w_command.begin(), w_command.end());
            return command_suffix_.Insert(std::move(wstr_to_str(w_command)), std::forward<F>(func));
            break;
        }
        case KEYWORD:
            return command_keyword_.emplace(std::move(command), std::forward<F>(func)).second;
            break;
        case ALLMSG:
            all_msg_handler_.push_back(std::forward<F>(func));
            return true;
            break;
        default:
            return command_fullmatch_.emplace(std::move(command), std::forward<F>(func)).second;
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

inline bool EventHandler::Handle(const Event &event, onebot11::ApiBot &bot) const
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
                LOG_INFO("Bot[{}] 收到一条消息: {}", event.value("self_id", 0), event.value("message", "Unknown message"));
                auto func = MatchedHandler(event);
                if(func)
                    pool_->AddTask(std::bind(func, event, std::ref(bot))); // 原对象会消失，event必须拷贝
                else
                {
                    auto &funcs = FreeHandler();
                    for(auto &func : funcs)
                        pool_->AddTask(std::bind(func, event, std::ref(bot)));
                }  
            }
            break;
            // notice
            case 'i':
            {
                auto notice_type = event.value("notice_type", "");
                auto sub_type = event.value("sub_type", "");
                if(notice_handler_.count(notice_type) && notice_handler_.at(notice_type).count(sub_type))
                {
                    for(auto &func : notice_handler_.at(notice_type).at(sub_type))
                        pool_->AddTask(std::bind(func, event, std::ref(bot)));
                }
            }
            break;
            // request
            case 'u':
            {
                auto request_type = event.value("request_type", "");
                auto sub_type = event.value("sub_type", "");
                if(request_handler_.count(request_type) && request_handler_.at(request_type).count(sub_type))
                {
                    for(auto &func : request_handler_.at(request_type).at(sub_type))
                        pool_->AddTask(std::bind(func, event, std::ref(bot)));
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

inline const plugin_func &EventHandler::MatchedHandler(const Event &event) const
{
    if(event.is_null())
        return no_func_avaliable_;
    std::string msg_str = event.value("message", "");
    if(msg_str.empty())
        return no_func_avaliable_;
    if (command_fullmatch_.count(msg_str))
        return command_fullmatch_.find(msg_str)->second;
    const plugin_func &func_prefix = command_prefix_.Search(msg_str);
    if(func_prefix)
        return func_prefix;
    std::string msg_str_reverse = msg_str;
    std::reverse(msg_str_reverse.begin(), msg_str_reverse.end());
    const plugin_func &func_suffix = command_suffix_.Search(msg_str_reverse);
    if(func_suffix)
        return func_suffix;
    return no_func_avaliable_;
}

inline const std::vector<plugin_func> &EventHandler::FreeHandler() const
{
    return all_msg_handler_;
}

} // namespace white

#endif
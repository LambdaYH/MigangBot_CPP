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

#include <iostream>

namespace white
{

using plugin_func = std::function<void(const Event &, ApiBot &)>;

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

    void InitFilter(std::unique_ptr<EventFilter> &&filter)
    {
        filter_ = std::move(filter);
    }

public:
    template<typename F>
    bool RegisterCommand(const int type, const std::string &command, F &&func);
    bool Handle(const Event &event, ApiBot &bot) const;

private:
    EventHandler() : pool_(new ThreadPool(8)) {}
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
    std::vector<plugin_func> all_msg_handler_;
    plugin_func no_func_avaliable_;

    std::unique_ptr<EventFilter> filter_;
    std::unique_ptr<ThreadPool> pool_;
};

template<typename F>
inline bool EventHandler::RegisterCommand(const int type, const std::string &command, F &&func)
{
    switch(type)
    {
        case FULLMATCH:
            return command_fullmatch_.emplace(std::move(command), std::forward<F>(func)).second;
            break;
        case PREFIX:
            return command_prefix_.Insert(std::move(command), std::forward<F>(func));
            break;
        case SUFFIX:
        {
            std::string command_reverse = command;
            std::reverse(command_reverse.begin(), command_reverse.end());
            return command_suffix_.Insert(std::move(command_reverse), std::forward<F>(func));
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
}

inline bool EventHandler::Handle(const Event &event, ApiBot &bot) const
{
    if(filter_ && !filter_->operator()(event))
        return false;
    LOG_INFO("Bot[{}] Got a Message: {}", event["self_id"].get<uint64_t>(), event["message"].get<std::string>());
    auto func = MatchedHandler(event);
    if(func)
        pool_->AddTask(std::bind(func, event, std::ref(bot))); // 原对象会消失，event必须拷贝
    else
    {
        auto &funcs = FreeHandler();
        for(auto &func : funcs)
            pool_->AddTask(std::bind(func, event, std::ref(bot)));
    }
    return true;
}

inline const plugin_func &EventHandler::MatchedHandler(const Event &event) const
{
    std::string msg_str = event["message"].get<std::string>();
    if(command_fullmatch_.count(msg_str))
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
#ifndef MIGANGBOT_EVENT_EVENT_HANDLER_H_
#define MIGANGBOT_EVENT_EVENT_HANDLER_H_

#include <algorithm>
#include <array>
#include <exception>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <co/co.h>
#include <jpcre2.hpp>
#include <nlohmann/json.hpp>

#include "bot/onebot_11/api_bot.h"
#include "event/event_filter.h"
#include "event/regex_matcher.h"
#include "event/trie.h"
#include "event/type.h"
#include "logger/logger.h"
#include "permission/permission.h"
#include "utility.h"
#include "service/service_manager.h"
#include "global_config.h"

namespace white {

class EventHandler {
 public:
  static EventHandler &GetInstance() noexcept {
    static EventHandler event_handler;
    return event_handler;
  }

 public:
  bool RegisterCommand(const int command_type, const std::string &command,
                       std::shared_ptr<TriggeredService> service);

  bool RegisterNotice(const std::string &notice_type,
                      const std::string &sub_type,
                      std::shared_ptr<TriggeredService> service);

  bool RegisterRequest(const std::string &request_type,
                       const std::string &sub_type,
                       std::shared_ptr<TriggeredService> service);

  bool RegisterRegex(const std::initializer_list<std::string> &patterns,
                     std::shared_ptr<TriggeredService> service);

  bool Handle(Event &event, onebot11::ApiBot &bot) noexcept;

 public:
  EventHandler(const EventHandler &) = delete;
  EventHandler &operator=(const EventHandler &) = delete;
  EventHandler(EventHandler &&) = delete;
  EventHandler &operator=(EventHandler &&) = delete;

 private:
  EventHandler() : filter_(std::make_unique<EventFilter>()) {}
  ~EventHandler() {}

 private:
  std::unordered_map<std::string, std::shared_ptr<TriggeredService>>
      command_fullmatch_;
  Trie command_prefix_;
  Trie command_suffix_;

  std::vector<RegexMatcher> command_regex_;

  std::vector<std::shared_ptr<TriggeredService>> all_msg_handler_;

  std::unordered_map<
      std::string,
      std::unordered_map<std::string,
                         std::vector<std::shared_ptr<TriggeredService>>>>
      notice_handler_;
  std::unordered_map<
      std::string,
      std::unordered_map<std::string,
                         std::vector<std::shared_ptr<TriggeredService>>>>
      request_handler_;

  std::unique_ptr<EventFilter> filter_;
};

inline bool EventHandler::RegisterCommand(
    const int command_type, const std::string &command,
    std::shared_ptr<TriggeredService> service) {
  switch (command_type) {
    case FULLMATCH:
      return command_fullmatch_.emplace(command, service).second;
      break;
    case PREFIX:
      return command_prefix_.Insert(command, service);
      break;
    case SUFFIX:
      return command_suffix_.InsertFromBack(command, service);
      break;
    case ALLMSG:
      all_msg_handler_.push_back(service);
      return true;
      break;
    default:
      return command_fullmatch_.emplace(command, service).second;
  }
  return true;
}

inline bool EventHandler::RegisterNotice(
    const std::string &notice_type, const std::string &sub_type,
    std::shared_ptr<TriggeredService> service) {
  notice_handler_[notice_type][sub_type].push_back(service);
  return true;
}

inline bool EventHandler::RegisterRequest(
    const std::string &request_type, const std::string &sub_type,
    std::shared_ptr<TriggeredService> service) {
  request_handler_[request_type][sub_type].push_back(service);
  return true;
}

inline bool EventHandler::RegisterRegex(
    const std::initializer_list<std::string> &patterns,
    std::shared_ptr<TriggeredService> service) {
  command_regex_.push_back(RegexMatcher(patterns, service));
  return true;
}

inline bool EventHandler::Handle(Event &event, onebot11::ApiBot &bot) noexcept {
  if (!filter_->Filter(event)) return false;
  if (event.contains("post_type")) {
    auto post_type = event["post_type"].get<std::string>();
    switch (post_type[3]) {
      // message
      case 's': {
        auto msg = event["message"].get<std::string_view>();
        auto message_type = event["message_type"].get<std::string>()[0];
        auto perm = permission::GetUserPermission(event);
        switch (message_type) {
          case 'p':
            if (event["sub_type"].get<std::string>()[0] == 'f')
              LOG_INFO("Bot[{}]收到来自好友[{}({})]的消息: {}",
                       event["self_id"].get<QId>(),
                       event["sender"].value("nickname", ""),
                       event["sender"].value("user_id", 0), msg);
            break;
          case 'g':
            if (event["anonymous"] != nullptr)
              LOG_INFO("Bot[{}]收到来自群[{}]的匿名消息: {}",
                       event["self_id"].get<QId>(),
                       event["group_id"].get<GId>(), msg);
            else
              LOG_INFO("Bot[{}]收到来自群[{}]成员[{}({})]的消息: {}",
                       event["self_id"].get<QId>(),
                       event["group_id"].get<GId>(),
                       event["sender"].value("nickname", ""),
                       event["sender"].value("user_id", 0), msg);
            break;
          default:
            break;
        }
        // pre_propose
        if (msg.starts_with("[CQ:at")) {
          // auto at_id_start = msg.find_first_of('=') + 1;
          auto at_id_start = std::min(static_cast<std::size_t>(10), msg.size());
          auto at_id_end = std::min(msg.find_first_of(']'), msg.size());
          auto at_id = msg.substr(at_id_start, at_id_end - at_id_start);
          if (at_id == std::to_string(event["self_id"].get<QId>())) {
            msg = msg.substr(msg.find_first_of(']') + 1);
            msg = msg.substr(msg.find_first_not_of(' '));
            event["__to_me__"] = true;
            event["message"] = msg;
          }
        } else if (msg.starts_with(config::BOT_NAME)) {
          event["__to_me__"] = true;
          msg.remove_prefix(config::BOT_NAME.size());
          event["message"] = msg;
        }
        std::string message{msg};
        switch (message_type) {
          case 'g': {
            auto group_id = event["group_id"].get<GId>();

            // command match
            if (command_fullmatch_.count(message)) {
              const auto &service = command_fullmatch_.at(message);
              if (service->CheckIsEnable(group_id) &&
                  service->CheckPerm(perm) &&
                  service->CheckToMe(event.contains("__to_me__")))
                go([&service, event, &bot] { service->Run(event, bot); });
            }
            {
              const auto &service =
                  command_prefix_.LongestPrefix(message, event);
              if (service && service->CheckIsEnable(group_id) &&
                  service->CheckPerm(perm) &&
                  service->CheckToMe(event.contains("__to_me__")))
                go([&service, event, &bot] { service->Run(event, bot); });
            }
            {
              const auto &service =
                  command_suffix_.LongestSuffix(message, event);
              if (service && service->CheckIsEnable(group_id) &&
                  service->CheckPerm(perm) &&
                  service->CheckToMe(event.contains("__to_me__")))
                go([&service, event, &bot] { service->Run(event, bot); });
            }

            // regex match
            for (auto &regex_matcher : command_regex_) {
              if (regex_matcher.Check(message)) {
                const auto &service = regex_matcher.GetService();
                if (service->CheckIsEnable(group_id) &&
                    service->CheckPerm(perm))
                  go([&service, event, &bot] { service->Run(event, bot); });
              }
            }

            // match all
            for (const auto &service : all_msg_handler_)
              if (service->CheckIsEnable(group_id) && service->CheckPerm(perm))
                go([&service, event, &bot] { service->Run(event, bot); });
          } break;
          case 'p': {
            // commmand match
            if (command_fullmatch_.count(message)) {
              const auto &service = command_fullmatch_.at(message);
              if (service->CheckPerm(perm))
                go([&service, event, &bot] { service->Run(event, bot); });
            }
            {
              const auto &service =
                  command_prefix_.LongestPrefix(message, event);
              if (service && service->CheckPerm(perm))
                go([&service, event, &bot] { service->Run(event, bot); });
            }
            {
              const auto &service =
                  command_suffix_.LongestSuffix(message, event);
              if (service && service->CheckPerm(perm))
                go([&service, event, &bot] { service->Run(event, bot); });
            }

            // regex_match
            for (auto &regex_matcher : command_regex_) {
              if (regex_matcher.Check(message)) {
                const auto &service = regex_matcher.GetService();
                if (service->CheckPerm(perm))
                  go([&service, event, &bot] { service->Run(event, bot); });
              }
            }

            // match all
            for (const auto &service : all_msg_handler_)
              if (service->CheckPerm(perm))
                go([&service, event, &bot] { service->Run(event, bot); });
          } break;
          default:
            break;
        }
      } break;
      // notice
      case 'i': {
        auto notice_type = event.value("notice_type", "");
        auto sub_type = event.value("sub_type", "");
        LOG_INFO("Bot[{}] 收到一个通知事件: {}.{}", event["self_id"].get<QId>(),
                 notice_type, sub_type);
        if (notice_handler_.count(notice_type) &&
            notice_handler_.at(notice_type).count(sub_type)) {
          for (const auto &service :
               notice_handler_.at(notice_type).at(sub_type))
            if (!event.contains("group_id") ||
                service->CheckIsEnable(event["group_id"].get<GId>()))
              go([&service, event, &bot]() { service->Run(event, bot); });
        }
        if (!sub_type.empty()) {
          if (notice_handler_.count(notice_type) &&
              notice_handler_.at(notice_type).count("")) {
            for (const auto &service : notice_handler_.at(notice_type).at(""))
              if (!event.contains("group_id") ||
                  service->CheckIsEnable(event["group_id"].get<GId>()))
                go([&service, event, &bot]() { service->Run(event, bot); });
          }
        }
      } break;
      // request
      case 'u': {
        auto request_type = event.value("request_type", "");
        auto sub_type = event.value("sub_type", "");
        LOG_INFO("Bot[{}] 收到一个请求事件: {}.{}", event["self_id"].get<QId>(),
                 request_type, sub_type);
        if (request_handler_.count(request_type) &&
            request_handler_.at(request_type).count(sub_type)) {
          for (const auto &service :
               request_handler_.at(request_type).at(sub_type))
            if (!event.contains("group_id") ||
                service->CheckIsEnable(event["group_id"].get<GId>()))
              go([&service, event, &bot]() { service->Run(event, bot); });
        }
      } break;
      // meta_event
      case 'a': {
        auto meta_event_type = event["meta_event_type"].get<std::string>();
        if (meta_event_type[0] == 'l') {
          auto sub_type = event["sub_type"].get<std::string>();
          switch (sub_type[0]) {
            case 'e':
            case 'c':
              LOG_INFO("Bot[{}]已成功建立连接", event["self_id"].get<QId>());
              break;
            case 'd':
              LOG_INFO("Bot[{}]已断开连接", event["self_id"].get<QId>());
              break;
          }
        } else {
          LOG_DEBUG("Bot[{}]与[{}]收到一次心跳连接",
                    event["self_id"].get<QId>(), event["time"].get<int64_t>());
        }
      } break;
      default:
        break;
    }
  }
  return true;
}
}  // namespace white

#endif
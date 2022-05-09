#ifndef MIGANGBOT_BOT_BOT_H_
#define MIGANGBOT_BOT_BOT_H_

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>

#include <nlohmann/json.hpp>
#include <hv/WebSocketServer.h>
#include <oneapi/tbb/concurrent_unordered_map.h>


#include "bot/onebot_11/api_bot.h"
#include "event/event.h"
#include "event/event_handler.h"
#include "global_config.h"
#include "version.h"

namespace white {

class Bot : public std::enable_shared_from_this<Bot> {
 public:
  Bot();
  ~Bot();

  void Run(const WebSocketChannelPtr &channel) noexcept;

  void OnRead(const std::string &msg) noexcept;

  const auto &GetBot() noexcept { return api_bot_; }

 private:
  void OnRun();

  void Process(const std::string &message) noexcept;

  void Notify(const std::string &msg);

  void SetEchoFunction(const std::time_t echo_code,
                       std::function<void(const Json &)> &&func);

  bool EventProcess(const Event &event) noexcept;

 private:
  WebSocketChannelPtr channel_;
  tbb::concurrent_unordered_map<std::time_t, std::function<void(const Json &)>>
      echo_function_;
  std::function<void(const std::string &)> notify_;
  std::function<void(const std::time_t, std::function<void(const Json &)> &&)>
      set_echo_function_;
  onebot11::ApiBot api_bot_;
};

inline Bot::Bot()
    : notify_(std::bind(&Bot::Notify, this, std::placeholders::_1)),
      set_echo_function_(std::bind(&Bot::SetEchoFunction, this,
                                   std::placeholders::_1,
                                   std::placeholders::_2)),
      api_bot_(notify_, set_echo_function_) {}

inline Bot::~Bot() {}

inline void Bot::Run(const WebSocketChannelPtr &channel) noexcept {
  channel_ = channel;
  OnRun();
}

inline void Bot::OnRun() {
  for (auto superuser : config::SUPERUSERS)
    api_bot_.send_private_msg(
        superuser, fmt::format("MigangBot已启动\n版本: {}", kMigangBotVersion));
}

inline void Bot::OnRead(const std::string &msg) noexcept { Process(msg); }

inline void Bot::Notify(const std::string &msg) {
  LOG_DEBUG("Msg To sent: {}", msg);
  channel_->send(msg);
}

inline void Bot::SetEchoFunction(const std::time_t echo_code,
                                 std::function<void(const Json &)> &&func) {
  echo_function_[echo_code] = std::move(func);
}

inline void Bot::Process(const std::string &message) noexcept {
  static auto event_handle = [this](auto &event) {
    EventHandler::GetInstance().Handle(event, api_bot_);
  };
  try {
    auto msg = Json::parse(message);
    if (EventProcess(msg)) event_handle(msg);
  } catch (Json::exception &e) {
    LOG_ERROR("Exception: {}", e.what());
  }
}

inline bool Bot::EventProcess(const Event &event) noexcept {
  if (event.contains("retcode")) {
    std::time_t echo_code = 0;
    if (event.contains("echo")) echo_code = event["echo"].get<std::time_t>();
    if (echo_function_.count(echo_code)) {
      echo_function_.at(echo_code)(event["data"]);
      echo_function_.unsafe_erase(echo_code);
    }
    return false;
  } else if (event.contains("message")) {
    QId user_id = event["user_id"].get<QId>();
    if (event.contains("group_id")) {
      GId group_id = event["group_id"].get<GId>();
      if (api_bot_.IsNeedMessage(group_id, user_id))
        api_bot_.FeedMessage(group_id, user_id,
                             event["message"].get<std::string>());
    } else {
      if (api_bot_.IsNeedMessage(0, user_id))
        api_bot_.FeedMessage(0, user_id, event["message"].get<std::string>());
    }
    if (api_bot_.IsSomeOneNeedMessage(user_id))
      api_bot_.FeedMessageTo(user_id, event["message"].get<std::string>());
  }
  return true;
}

}  // namespace white

#endif
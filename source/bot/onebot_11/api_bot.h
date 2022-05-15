#ifndef MIGANGBOT_BOT_ONEBOT_11_API_BOT_H_
#define MIGANGBOT_BOT_ONEBOT_11_API_BOT_H_

#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>

#include <co/co.h>

#include "api/onebot_11/api_impl.h"
#include "bot/onebot_11/future_wrapper.h"
#include "bot/onebot_11/desired_value.h"
#include "event/event.h"
#include "logger/logger.h"
#include "type.h"
#include "closure.h"

namespace white {
namespace onebot11 {

class ApiBot;

using ClosureForPlugin = Closure<const Event &, onebot11::ApiBot &>;

template <typename F>
class FunctionForPlugin : public ClosureForPlugin {
 public:
  FunctionForPlugin(F &&func) : func_(std::forward<F>(func)) {}
  virtual ~FunctionForPlugin() = default;

  virtual void Run(const Event &event, onebot11::ApiBot &bot) const {
    func_(event, bot);
  }

 private:
  std::remove_reference_t<F> func_;
};

class ApiBot {
 public:
  template <typename Str>
  CoFutureWrapper<MessageID> send_private_msg(const QId user_id, Str &&message,
                                              bool auto_escape = false);

  template <typename Str>
  CoFutureWrapper<MessageID> send_group_msg(const GId group_id, Str &&message,
                                            bool auto_escape = false);

  template <typename Type, typename Str, typename ID>
  CoFutureWrapper<MessageID> send_msg(Type &&type, Str &&message, ID &&id,
                                      bool auto_escape = false);

  template <typename Str>
  CoFutureWrapper<MessageID> send(const Event &event, Str &&message,
                                  bool at_sender = false,
                                  bool auto_escape = false);

  CoFutureWrapper<GroupInfo> get_group_info(const GId group_id,
                                            bool no_cache = false);

  CoFutureWrapper<std::vector<GroupInfo>> get_group_list(bool no_cache = false);

  CoFutureWrapper<UserInfo> get_stranger_info(const QId user_id,
                                              bool no_cache = false);

  void delete_msg(const MsgId msg_id);

  template <typename Str>
  void reject(const Event &event, Str &&reason = "");

  void approve(const Event &event);

  void set_group_leave(const GId group_id, bool is_dismiss = false);

 public:
  std::string WaitForNextMessage(const Event &event);

  std::string WaitForNextMessageFrom(QId person);

  bool IsSomeOneNeedMessage(QId user_id) const;

  bool IsNeedMessage(GId group_id, QId user_id) const;

  template <typename Str>
  void FeedMessageTo(QId user_id, Str &&message);

  template <typename Str>
  void FeedMessage(GId group_id, QId user_id, Str &&message);

 public:
  template<typename Notify, typename SetEcho>
  ApiBot(Notify &&notify, SetEcho &&set_echo_function)
      : notify_(new FunctionForNotify(std::forward<Notify>(notify))),
        set_echo_function_(new FunctionForSetEcho(std::forward<SetEcho>(set_echo_function))),
        u_(-10000, 10000) {}

  ~ApiBot() {
    delete notify_;
    delete set_echo_function_;
  }

 private:
  template <typename T>
  void EchoFunction(const std::weak_ptr<co_promise<T>> &weak_p,
                    const Json &value) {
    auto shared_p = weak_p.lock();
    if (!shared_p) return;
    try {
      if (value.is_null()) {
        shared_p->set_value(DefaultValue<T>());
        return;
      }
      shared_p->set_value(DesiredValue<T>(value));
    } catch (const std::exception &e) {
      LOG_ERROR("Exception In EchoFunction: {}", e.what());
    }
  }

  template <typename T>
  CoFutureWrapper<T> Echo(Json &msg) {
    static auto timestamp_micro = []() {
      return std::chrono::time_point_cast<std::chrono::microseconds>(
                 std::chrono::high_resolution_clock::now())
          .time_since_epoch()
          .count();
    };
    std::time_t echo_code =
        timestamp_micro() +
        u_(random_engine_) *
            10000;  // 用纳秒会导致接受的和发送的不一致，姑且用微秒吧
    msg["echo"] = echo_code;  // 多线程下降低冲突的可能性，加个随机数
    auto promise = std::make_shared<co_promise<T>>();
    set_echo_function_->Run(
        echo_code, [this, weak_p = std::weak_ptr(promise)](const Json &value) {
          EchoFunction(weak_p, value);
        });
    return CoFutureWrapper{std::move(promise)};
  }

 private:
  const ClosureNotify *const notify_;
  const ClosureSetEcho *const set_echo_function_;

  std::mt19937 random_engine_;
  std::uniform_int_distribution<std::time_t> u_;

  std::unordered_map<
      GId, std::unordered_map<
               QId, std::queue<std::weak_ptr<co_promise<std::string>>>>>
      someone_group_message_;
  std::unordered_map<GId, std::unordered_map<QId, std::mutex>>
      someone_group_message_mutex_;

  std::unordered_map<QId, std::queue<std::weak_ptr<co_promise<std::string>>>>
      someone_need_message_;
  std::unordered_map<QId, std::mutex> someone_need_message_mutex_;
};

inline std::string ApiBot::WaitForNextMessage(const Event &event) {
  std::shared_ptr<co_promise<std::string>> p =
      std::make_shared<co_promise<std::string>>();
  {
    QId user_id = event["user_id"].get<QId>();
    if (event.contains("group_id")) {
      GId group_id = event["group_id"].get<GId>();
      std::lock_guard<std::mutex> locker(
          someone_group_message_mutex_[group_id][user_id]);
      someone_group_message_[group_id][user_id].push(std::weak_ptr(p));
    } else {
      std::lock_guard<std::mutex> locker(
          someone_group_message_mutex_[0][user_id]);
      someone_group_message_[0][user_id].push(std::weak_ptr(p));
    }
  }

  return CoFutureWrapper(std::move(p)).Ret();
}

inline std::string ApiBot::WaitForNextMessageFrom(QId person) {
  std::shared_ptr<co_promise<std::string>> p =
      std::make_shared<co_promise<std::string>>();
  {
    std::lock_guard<std::mutex> locker(someone_need_message_mutex_[person]);
    someone_need_message_[person].push(std::weak_ptr(p));
  }
  return CoFutureWrapper(std::move(p)).Ret();
}

inline bool ApiBot::IsSomeOneNeedMessage(QId user_id) const {
  return someone_need_message_.count(user_id);
}

inline bool ApiBot::IsNeedMessage(GId group_id, QId user_id) const {
  return someone_group_message_.count(group_id) &&
         someone_group_message_.at(group_id).count(user_id);
}

template <typename Str>
inline void ApiBot::FeedMessageTo(QId user_id, Str &&message) {
  std::lock_guard<std::mutex> locker(someone_need_message_mutex_.at(user_id));
  while (!someone_need_message_.at(user_id).empty()) {
    auto weak_p = someone_need_message_.at(user_id).front();
    someone_need_message_.at(user_id).pop();
    if (auto shared_p = weak_p.lock()) {
      shared_p->set_value(std::forward<Str>(message));
      break;
    }
  }
  if (someone_need_message_.at(user_id).empty())
    someone_need_message_.erase(user_id);
}

template <typename Str>
inline void ApiBot::FeedMessage(GId group_id, QId user_id, Str &&message) {
  std::lock_guard<std::mutex> locker(
      someone_group_message_mutex_.at(group_id).at(user_id));
  while (!someone_group_message_.at(group_id).at(user_id).empty()) {
    auto weak_p = someone_group_message_.at(group_id).at(user_id).front();
    someone_group_message_.at(group_id).at(user_id).pop();
    if (auto shared_p = weak_p.lock()) {
      shared_p->set_value(std::forward<Str>(message));
      break;
    }
  }
  if (someone_group_message_.at(group_id).at(user_id).empty())
    someone_group_message_.at(group_id).erase(user_id);
  if (someone_group_message_.at(group_id).empty())
    someone_group_message_.erase(group_id);
}

template <typename Str>
inline CoFutureWrapper<MessageID> ApiBot::send_private_msg(
    const uint64_t user_id, Str &&message, bool auto_escape) {
  Json msg = api_impl::send_private_msg(user_id, std::forward<Str>(message),
                                        auto_escape);
  auto ret = Echo<MessageID>(msg);
  notify_->Run(msg.dump());
  return ret;
}

template <typename Str>
inline CoFutureWrapper<MessageID> ApiBot::send_group_msg(const GId group_id,
                                                         Str &&message,
                                                         bool auto_escape) {
  Json msg = api_impl::send_group_msg(group_id, std::forward<Str>(message),
                                      auto_escape);
  auto ret = Echo<MessageID>(msg);
  notify_->Run(msg.dump());
  return ret;
}

template <typename Type, typename Str, typename ID>
inline CoFutureWrapper<MessageID> ApiBot::send_msg(Type &&type, Str &&message,
                                                   ID &&id, bool auto_escape) {
  Json msg =
      api_impl::send_msg(std::forward<Type>(type), std::forward<Str>(message),
                         std::forward<ID>(id), auto_escape);
  auto ret = Echo<MessageID>(msg);
  notify_->Run(msg.dump());
  return ret;
}

template <typename Str>
inline CoFutureWrapper<MessageID> ApiBot::send(const Event &event,
                                               Str &&message, bool at_sender,
                                               bool auto_escape) {
  Json msg;
  if (event.value("message_type", "private") == "group") {
    if (at_sender)
      return send_group_msg(
          event["group_id"].get<GId>(),
          fmt::format("[CQ:at,qq={}] {}", event["user_id"].get<QId>(),
                      std::forward<Str>(message)),
          auto_escape);
    else
      return send_group_msg(event["group_id"].get<GId>(),
                            std::forward<Str>(message), auto_escape);
  }
  return send_private_msg(event["user_id"].get<QId>(),
                          std::forward<Str>(message));
}

inline CoFutureWrapper<GroupInfo> ApiBot::get_group_info(const GId group_id,
                                                         bool no_cache) {
  Json msg = api_impl::get_group_info(group_id, no_cache);
  auto ret = Echo<GroupInfo>(msg);
  notify_->Run(msg.dump());
  return ret;
}

inline CoFutureWrapper<std::vector<GroupInfo>> ApiBot::get_group_list(
    bool no_cache) {
  Json msg = api_impl::get_group_list(no_cache);
  auto ret = Echo<std::vector<GroupInfo>>(msg);
  notify_->Run(msg.dump());
  return ret;
}

inline CoFutureWrapper<UserInfo> ApiBot::get_stranger_info(const QId user_id,
                                                           bool no_cache) {
  auto msg = api_impl::get_stranger_info(user_id);
  auto ret = Echo<UserInfo>(msg);
  notify_->Run(msg.dump());
  return ret;
}

inline void ApiBot::delete_msg(MsgId msg_id) {
  Json msg = api_impl::delete_msg(msg_id);
  notify_->Run(msg.dump());
}

template <typename Str>
inline void ApiBot::reject(const Event &event, Str &&reason) {
  auto flag = event["flag"].get<std::string>();
  auto request_type = event["request_type"].get<std::string>();
  if (request_type == "friend") {
    auto msg = api_impl::set_friend_add_request(flag, false);
    notify_->Run(msg.dump());
  } else if (request_type == "group" &&
             event["sub_type"].get<std::string>() == "invite") {
    auto msg = api_impl::set_group_add_request(
        flag, "invite", std::forward<Str>(reason), false);
    notify_->Run(msg.dump());
  }
}

inline void ApiBot::approve(const Event &event) {
  auto flag = event["flag"].get<std::string>();
  auto request_type = event["request_type"].get<std::string>();
  if (request_type == "friend") {
    auto msg = api_impl::set_friend_add_request(flag, true);
    notify_->Run(msg.dump());
  } else if (request_type == "group" &&
             event["sub_type"].get<std::string>() == "invite") {
    auto msg = api_impl::set_group_add_request(flag, "invite", "", true);
    notify_->Run(msg.dump());
  }
}

inline void ApiBot::set_group_leave(const GId group_id, bool is_dismiss) {
  auto msg = api_impl::set_group_leave(group_id, is_dismiss);
  notify_->Run(msg.dump());
}

}  // namespace onebot11
}  // namespace white

#endif
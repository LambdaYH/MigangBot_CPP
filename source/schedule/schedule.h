#ifndef MIGANGBOT_SCHEDULE_SCHEDULE_H_
#define MIGANGBOT_SCHEDULE_SCHEDULE_H_

#include <mutex>
#include <list>

#include "schedule/Bosma/Scheduler.h"
#include "bot/onebot_11/api_bot.h"

namespace white {
  
using namespace std::chrono_literals;

inline Bosma::Scheduler &Schedule() {
  static Bosma::Scheduler scheduler;
  return scheduler;
}

class BotSet {
 public:
  static BotSet &GetInstance() {
    static BotSet botset;
    return botset;
  }

  auto AddBot(onebot11::ApiBot *bot) {
    std::lock_guard<std::mutex> locker(mutex_);
    bots_.push_front(bot);
    return bots_.cbegin();
  }

  void RemoveBot(std::list<onebot11::ApiBot *>::const_iterator &it) {
    std::lock_guard<std::mutex> locker(mutex_);
    bots_.erase(it);
  }

  // 需要改进
  // 当调用该bot的一瞬间，Bot析构了，将会导致访问空悬指针的问题
  // 其他情况下，析构后不会影响迭代器
  const auto &GetBots() { return bots_; }

 public:
  BotSet(const BotSet &b) = delete;
  BotSet(const BotSet &&b) = delete;
  BotSet &operator=(const BotSet &b) = delete;
  BotSet &operator=(const BotSet &&b) = delete;

 private:
  BotSet() {}
  ~BotSet() {}

 private:
  std::list<onebot11::ApiBot *> bots_;
  std::mutex mutex_;
};

inline const std::list<onebot11::ApiBot *> &Bots() {
  return BotSet::GetInstance().GetBots();
};

}  // namespace white

#endif
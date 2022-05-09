#ifndef MIGANGBOT_SCHEDULE_SCHEDULE_H_
#define MIGANGBOT_SCHEDULE_SCHEDULE_H_

#include <mutex>
#include <list>

#include "Scheduler.h"
#include "bot/onebot_11/api_bot.h"

namespace white {
inline Bosma::Scheduler &Schedule() {
  static Bosma::Scheduler scheduler(std::thread::hardware_concurrency());
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
    return bots_.begin();
  }

  void RemoveBot(const auto &it) {
    std::lock_guard<std::mutex> locker(mutex_);
    bots_.erase(it);
  }

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

}  // namespace white

#endif
#pragma once

#include <atomic>
#include <ctime>
#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "schedule/Bosma/croncpp.h"
#include "schedule/Bosma/InterruptableSleep.h"
#include "co/co.h"
#include "InterruptableSleep.h"

#include "logger/logger.h"

namespace Bosma {
using Clock = std::chrono::system_clock;

class Task {
 public:
  explicit Task(std::function<void()> &&f, const std::string &id,
                bool recur = false, bool interval = false)
      : f(std::move(f)),
        task_unique_id_(id),
        recur(recur),
        interval(interval) {}

  virtual Clock::time_point get_new_time() const = 0;

  std::function<void()> f;

  std::string task_unique_id_;
  bool recur;
  std::atomic_bool interval;
};

class InTask : public Task {
 public:
  explicit InTask(std::function<void()> &&f, const std::string &id)
      : Task(std::move(f), id) {}

  // dummy time_point because it's not used
  Clock::time_point get_new_time() const override {
    return Clock::time_point(Clock::duration(0));
  }
};

class EveryTask : public Task {
 public:
  EveryTask(Clock::duration time, std::function<void()> &&f,
            const std::string &id, bool interval = false)
      : Task(std::move(f), id, true, interval), time(time) {}

  Clock::time_point get_new_time() const override {
    return Clock::now() + time;
  };
  Clock::duration time;
};

class CronTask : public Task {
 public:
  CronTask(const std::string &expression, std::function<void()> &&f,
           const std::string &id)
      : Task(std::move(f), id, true), cron_(cron::make_cron(expression)) {}

  Clock::time_point get_new_time() const override {
    return cron::cron_next(cron_, Clock::now());
  };
  cron::cronexpr cron_;
};

inline bool try_parse(std::tm &tm, const std::string &expression,
                      const std::string &format) {
  std::stringstream ss(expression);
  return !(ss >> std::get_time(&tm, format.c_str())).fail();
}

class Scheduler {
 public:
  explicit Scheduler()
      : done(false), auto_increase_id_(0), manage_thread_(std::thread{[this]() {
          while (!done) {
            if (tasks.empty()) {
              sleeper.sleep();
            } else {
              auto time_of_first_task = (*tasks.begin()).first;
              sleeper.sleep_until(time_of_first_task);
            }
            manage_tasks();
          }
        }}) {}

  Scheduler(const Scheduler &) = delete;

  Scheduler(Scheduler &&) noexcept = delete;

  Scheduler &operator=(const Scheduler &) = delete;

  Scheduler &operator=(Scheduler &&) noexcept = delete;

  ~Scheduler() {
    done = true;
    sleeper.interrupt();
    manage_thread_.join();
  }

  bool remove(const std::string &id) {
    // id_to_task manage task's life cycle
    // run at most once after deletion
    std::lock_guard<std::mutex> locker(id_lock);
    if (!id_to_task.count(id)) return false;
    id_to_task.at(id)->interval = false;
    id_to_task.erase(id);
    return true;
  }

  template <typename _Callable, typename... _Args>
  void in(const std::string &id, const Clock::time_point time, _Callable &&f,
          _Args &&...args) {
    std::shared_ptr<Task> t = std::make_shared<InTask>(
        std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...),
        id);
    add_task(time, std::move(t));
  }

  template <typename _Callable, typename... _Args>
  void in(const Clock::time_point time, _Callable &&f, _Args &&...args) {
    in("", time, std::forward<_Callable>(f), std::forward<_Args>(args)...);
  }

  template <typename _Callable, typename... _Args>
  void in(const Clock::duration time, _Callable &&f, _Args &&...args) {
    in("", Clock::now() + time, std::forward<_Callable>(f),
       std::forward<_Args>(args)...);
  }

  template <typename _Callable, typename... _Args>
  void in(const std::string &id, const Clock::duration time, _Callable &&f,
          _Args &&...args) {
    in(id, Clock::now() + time, std::forward<_Callable>(f),
       std::forward<_Args>(args)...);
  }

  template <typename _Callable, typename... _Args>
  void at(const std::string &id, const std::string &time, _Callable &&f,
          _Args &&...args) {
    // get current time as a tm object
    auto time_now = Clock::to_time_t(Clock::now());
    std::tm tm = *std::localtime(&time_now);

    // our final time as a time_point
    Clock::time_point tp;

    if (try_parse(tm, time, "%H:%M:%S")) {
      // convert tm back to time_t, then to a time_point and assign to final
      tp = Clock::from_time_t(std::mktime(&tm));

      // if we've already passed this time, the user will mean next day, so add
      // a day.
      if (Clock::now() >= tp) tp += std::chrono::hours(24);
    } else if (try_parse(tm, time, "%Y-%m-%d %H:%M:%S")) {
      tp = Clock::from_time_t(std::mktime(&tm));
    } else if (try_parse(tm, time, "%Y/%m/%d %H:%M:%S")) {
      tp = Clock::from_time_t(std::mktime(&tm));
    } else {
      // could not parse time
      throw std::runtime_error("Cannot parse time string: " + time);
    }
    // determine if it is a time at past
    if (tp > Clock::now())
      in(id, tp, std::forward<_Callable>(f), std::forward<_Args>(args)...);
  }

  template <typename _Callable, typename... _Args>
  void at(const std::string &time, _Callable &&f, _Args &&...args) {
    at("", time, std::forward<_Callable>(f), std::forward<_Args>(args)...);
  }

  template <typename _Callable, typename... _Args>
  void every(const std::string &id, const Clock::duration time, _Callable &&f,
             _Args &&...args) {
    std::shared_ptr<Task> t = std::make_shared<EveryTask>(
        time,
        std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...),
        id);
    auto next_time = t->get_new_time();
    add_task(next_time, std::move(t));
  }

  template <typename _Callable, typename... _Args>
  void every(const Clock::duration time, _Callable &&f, _Args &&...args) {
    every("", time, std::forward<_Callable>(f), std::forward<_Args>(args)...);
  }

  // expression format:
  // from https://en.wikipedia.org/wiki/Cron#Overview
  //    ┌───────────── minute (0 - 59)
  //    │ ┌───────────── hour (0 - 23)
  //    │ │ ┌───────────── day of month (1 - 31)
  //    │ │ │ ┌───────────── month (1 - 12)
  //    │ │ │ │ ┌───────────── day of week (0 - 6) (Sunday to Saturday)
  //    │ │ │ │ │
  //    │ │ │ │ │
  //    * * * * *
  template <typename _Callable, typename... _Args>
  void cron(const std::string &id, const std::string &expression, _Callable &&f,
            _Args &&...args) {
    std::shared_ptr<Task> t = std::make_shared<CronTask>(
        expression,
        std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...),
        id);
    auto next_time = t->get_new_time();
    add_task(next_time, std::move(t));
  }

  template <typename _Callable, typename... _Args>
  void cron(const std::string &expression, _Callable &&f, _Args &&...args) {
    cron("", expression, std::forward<_Callable>(f),
         std::forward<_Args>(args)...);
  }

  template <typename _Callable, typename... _Args>
  void interval(const std::string &id, const Clock::duration time,
                _Callable &&f, _Args &&...args) {
    std::shared_ptr<Task> t = std::make_shared<EveryTask>(
        time,
        std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...), id,
        true);
    add_task(Clock::now(), std::move(t));
  }

  template <typename _Callable, typename... _Args>
  void interval(const Clock::duration time, _Callable &&f, _Args &&...args) {
    interval("", time, std::forward<_Callable>(f),
             std::forward<_Args>(args)...);
  }

 private:
  std::atomic<bool> done;

  Bosma::InterruptableSleep sleeper;

  std::multimap<Clock::time_point, std::weak_ptr<Task>> tasks;
  std::unordered_map<std::string, std::shared_ptr<Task>> id_to_task;
  std::mutex lock;
  std::mutex id_lock;
  std::atomic_size_t auto_increase_id_;

  std::thread manage_thread_;

  std::string get_random_id() {
    auto random_id = "__internal__" + std::to_string(auto_increase_id_++);
    while (id_to_task.count(random_id))
      random_id = "__internal__" + std::to_string(auto_increase_id_++);
    return random_id;
  }

  void add_task(const Clock::time_point time, std::shared_ptr<Task> t) {
    std::lock_guard<std::mutex> l(lock);
    tasks.emplace(time, std::weak_ptr<Task>(t));
    if (t->task_unique_id_.empty()) t->task_unique_id_ = get_random_id();
    id_to_task.emplace(t->task_unique_id_, std::move(t));
    sleeper.interrupt();
  }

  void manage_tasks() {
    std::lock_guard<std::mutex> l(lock);

    auto end_of_tasks_to_run = tasks.upper_bound(Clock::now());

    // if there are any tasks to be run and removed
    if (end_of_tasks_to_run != tasks.begin()) {
      // keep track of tasks that will be re-added
      decltype(tasks) recurred_tasks;

      // for all tasks that have been triggered
      for (auto i = tasks.begin(); i != end_of_tasks_to_run; ++i) {
        auto &task_weak = (*i).second;
        auto task = task_weak.lock();
        if (!task) continue;
        if (task->interval) {
          // if it's an interval task, only add the task back after f() is
          // completed
          go([this, task = std::move(task)]() {
            try {
              task->f();
            } catch (const std::exception &e) {
              white::LOG_ERROR("Exception in schedule task: {}", e.what());
            }
            // no risk of race-condition,
            // add_task() will wait for manage_tasks() to release lock
            if (task->interval) add_task(task->get_new_time(), std::move(task));
          });
        } else {
          go([task]() {
            try {
              task->f();
            } catch (const std::exception &e) {
              white::LOG_ERROR("Exception in schedule task: {}", e.what());
            }
          });
          // calculate time of next run and add the new task to the tasks to be
          // recurred

          if (task->recur)
            recurred_tasks.emplace(task->get_new_time(), std::move(task_weak));
        }
      }

      // remove the completed tasks
      tasks.erase(tasks.begin(), end_of_tasks_to_run);

      // re-add the tasks that are recurring
      for (auto &task : recurred_tasks)
        tasks.emplace(task.first, std::move(task.second));
    }
  }
};
}  // namespace Bosma

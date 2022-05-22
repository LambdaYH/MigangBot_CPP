#ifndef MIGANGBOT_ScheduleService_ScheduleService_H_
#define MIGANGBOT_ScheduleService_ScheduleService_H_

#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "bot/onebot_11/api_bot.h"
#include "co/co.h"
#include "event/type.h"
#include "global_config.h"
#include "permission/permission.h"
#include "type.h"
#include "service/service.h"

namespace white {

template <class InputIt, class OutputIt, class Pred, class Fct>
inline void transform_if(InputIt first, InputIt last, OutputIt dest, Pred pred,
                         Fct transform) {
  while (first != last) {
    if (pred(*first)) *dest++ = transform(*first);

    ++first;
  }
}

// everyschedule task must own its own service
class ScheduleService : public Service {
 public:
  ScheduleService(const std::string &service_name,
                  const std::string &description, const int manage_permission,
                  const bool enable_on_default = true)
      : Service(service_name, description, manage_permission,
                enable_on_default) {}

  ScheduleService(const std::string &service_name, const int manage_permission,
                  const bool enable_on_default = true)
      : ScheduleService(service_name, "", manage_permission,
                        enable_on_default) {}

  ScheduleService(const std::string &service_name,
                  const bool enable_on_default = true)
      : ScheduleService(service_name, "", permission::GROUP_ADMIN,
                        enable_on_default) {}

  ScheduleService(const std::string &service_name,
                  const std::string &description,
                  const bool enable_on_default = true)
      : ScheduleService(service_name, description, permission::GROUP_ADMIN,
                        enable_on_default) {}

 public:
  template <typename Str>
  void BroadCast(onebot11::ApiBot *bot, Str &&message,
                 const std::time_t interval_ms = 500) {
    auto group_to_send = GetEnableGroup(bot);
    for (auto group : group_to_send) {
      bot->send_group_msg(group, std::forward<Str>(message));
      co::sleep(interval_ms);
    }
  }

  template <class InputIt>
  void BroadCast(onebot11::ApiBot *bot, InputIt start, InputIt end,
                 const std::time_t interval_ms) {
    auto group_to_send = GetEnableGroup(bot);
    while (start != end) {
      for (auto group : group_to_send) {
        bot->send_group_msg(group, *start);
        co::sleep(interval_ms);
      }
      ++start;
    }
  }

 private:
  std::vector<GId> GetEnableGroup(onebot11::ApiBot *bot) {
    if (!enable_on_default_) {
      std::lock_guard<std::mutex> locker(mutex_);
      return std::vector<GId>(groups_.begin(), groups_.end());
    }
    auto groups = bot->get_group_list().get();
    std::vector<GId> ret;
    {
      std::lock_guard<std::mutex> locker(mutex_);
      transform_if(
          groups.begin(), groups.end(), std::back_inserter(ret),
          [this](const auto &group_info) {
            return !groups_.count(group_info.group_id);
          },
          [](const auto &group_info) { return group_info.group_id; });
    }
    return ret;
  }
};
}  // namespace white

#endif
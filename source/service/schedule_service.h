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
class ScheduleService {
 public:
  ScheduleService(const std::string &service_name, const int manage_permission,
                  const bool enable_on_default = true)
      : service_name_(service_name),
        manage_permission_(manage_permission),
        enable_on_default_(enable_on_default),
        config_path_(config::kServiceDir / (service_name_ + ".json")) {
    LoadConfig();
  }
  ScheduleService(const std::string &service_name,
                  const bool enable_on_default = true)
      : ScheduleService(service_name, permission::GROUP_ADMIN,
                        enable_on_default) {}

 public:
  const std::string &GetServiceName() const noexcept { return service_name_; }

  void BroadCast(onebot11::ApiBot *bot, const std::string &message,
                 const std::time_t interval) {
    auto group_to_send = GetEnableGroup(bot);
    for (auto group : group_to_send) {
      bot->send_group_msg(group, message);
      co::sleep(interval);
    }
  }

  bool GroupEnable(const GId group_id, const int permission) {
    if (permission < manage_permission_) return false;
    if (enable_on_default_) {
      std::lock_guard<std::mutex> locker(mutex_);
      if (groups_.count(group_id)) groups_.erase(group_id);
      SaveConfig();
      return true;
    }
    std::lock_guard<std::mutex> locker(mutex_);
    if (groups_.emplace(group_id).second) SaveConfig();
    return true;
  }

  bool GroupDisable(const GId group_id, const int permission) {
    if (permission < manage_permission_) return false;
    if (!enable_on_default_) {
      std::lock_guard<std::mutex> locker(mutex_);
      if (groups_.count(group_id)) groups_.erase(group_id);
      SaveConfig();
      return true;
    }
    std::lock_guard<std::mutex> locker(mutex_);
    if (groups_.emplace(group_id).second) SaveConfig();
    return true;
  }

 private:
  std::vector<GId> GetEnableGroup(onebot11::ApiBot *bot) {
    if (!enable_on_default_) {
      std::lock_guard<std::mutex> locker(mutex_);
      return std::vector<GId>(groups_.begin(), groups_.end());
    }
    auto groups = bot->get_group_list().Ret();
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

  void SaveConfig() {
    config_["groups"] = R"([])"_json;
    for (auto it : groups_) config_["groups"].push_back(it);
    std::ofstream(config_path_) << std::setw(4) << config_ << std::endl;
  }

  void LoadConfig() {
    if (!std::filesystem::exists(config_path_)) {
      config_["enable_on_default"] = enable_on_default_;
      config_["groups"] = R"([])"_json;
      std::ofstream(config_path_) << std::setw(4) << std::endl;
    } else {
      std::ifstream(config_path_) >> config_;
    }
    enable_on_default_ = config_["enable_on_default"].get<bool>();
    for (std::size_t i = 0; i < config_["groups"].size(); ++i)
      groups_.insert(config_["groups"].get<GId>());
  }

 private:
  const std::string service_name_;

  const int manage_permission_;
  bool enable_on_default_;

  std::unordered_set<GId> groups_;

  const std::string config_path_;

  Json config_;
  std::mutex mutex_;
};
}  // namespace white

#endif
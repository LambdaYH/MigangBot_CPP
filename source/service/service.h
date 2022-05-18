#ifndef MIGANGBOT_SERVICE_SERVICE_H_
#define MIGANGBOT_SERVICE_SERVICE_H_

#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "event/type.h"
#include "global_config.h"
#include "permission/permission.h"
#include "type.h"
#include "event/event.h"

namespace white {
class Service {
 public:
  Service(const std::string &service_name, const std::string &package_name,
          const std::string &description, const int manage_permission,
          const bool enable_on_default = true)
      : service_name_(service_name),
        package_name_(package_name),
        description_(description),
        manage_permission_(manage_permission),
        enable_on_default_(enable_on_default),
        config_path_(config::kServiceDir / (service_name_ + ".json")) {
    LoadConfig();
  }

  virtual ~Service() = default;

 public:
  const std::string &GetServiceName() const noexcept { return service_name_; }
  const std::string &GetPackageName() const noexcept { return package_name_; }
  const std::string &GetDescription() const noexcept { return description_; }
  const int Permission() const noexcept { return manage_permission_; };

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

  bool GroupStatus(const GId group_id) {
    if (enable_on_default_) return !groups_.count(group_id);
    return groups_.count(group_id);
  }

 protected:
  void LoadConfig() {
    if (manage_permission_ == permission::ALWAYS_ON) return;
    if (!std::filesystem::exists(config_path_)) {
      config_["enable_on_default"] = enable_on_default_;
      config_["groups"] = R"([])"_json;
      std::ofstream(config_path_) << std::setw(4) << config_ << std::endl;
    } else {
      std::ifstream(config_path_) >> config_;
    }
    enable_on_default_ = config_["enable_on_default"].get<bool>();
    for (std::size_t i = 0; i < config_["groups"].size(); ++i)
      groups_.insert(config_["groups"][i].get<GId>());
  }

  void SaveConfig() {
    config_["groups"] = R"([])"_json;
    for (auto it : groups_) config_["groups"].push_back(it);
    std::ofstream(config_path_) << std::setw(4) << config_ << std::endl;
  }

 protected:
  const int manage_permission_;
  bool enable_on_default_;
  std::mutex mutex_;
  std::unordered_set<GId> groups_;

  const std::string service_name_;

 private:
  const std::string package_name_;

  const std::string description_;

  const std::string config_path_;

  Json config_;
};
}  // namespace white

#endif
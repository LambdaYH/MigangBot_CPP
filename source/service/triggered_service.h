#ifndef MIGANGBOT_SERVICE_TRIGGERED_SERVICE_H_
#define MIGANGBOT_SERVICE_TRIGGERED_SERVICE_H_

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
#include "service/service.h"

namespace white {
class TriggeredService : public Service {
 public:
  template <typename F>
  TriggeredService(const std::string &service_name, F &&func,
                   const int use_permission, const int manage_permission,
                   const bool enable_on_default = true,
                   const bool only_to_me = false)
      : Service(service_name, manage_permission, enable_on_default),
        func_(std::move(func)),
        use_permission_(use_permission),
        only_to_me_(only_to_me) {}
  template <typename F>
  TriggeredService(const std::string &service_name, F &&func,
                   const int use_permission,
                   const bool enable_on_default = true,
                   const bool only_to_me = false)
      : TriggeredService(service_name, std::forward<F>(func), use_permission,
                         permission::GROUP_ADMIN, enable_on_default,
                         only_to_me) {}
  template <typename F>
  TriggeredService(const std::string &service_name, F &&func,
                   const bool only_to_me = false)
      : TriggeredService(service_name, std::forward<F>(func),
                         permission::NORMAL, permission::GROUP_ADMIN, true,
                         only_to_me) {}

 public:
  bool CheckPerm(const int permission) const noexcept {
    return permission >= use_permission_;
  }

  bool CheckIsEnable(const GId group_id) const noexcept {
    if (enable_on_default_) return !groups_.count(group_id);
    return groups_.count(group_id);
  }

  bool CheckToMe(const bool to_me) const noexcept {
    return !only_to_me_ || to_me;
  }

  template <typename... Args>
  void Run(Args &&...args) {
    try {
      func_(std::forward<Args>(args)...);
    } catch (const std::exception &e) {
      LOG_ERROR("Exception Happened: {}", e.what());
    }
  }

 private:
  const plugin_func func_;
  const int use_permission_;

  const bool only_to_me_;
};
}  // namespace white

#endif
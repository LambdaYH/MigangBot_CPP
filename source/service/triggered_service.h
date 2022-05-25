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

#include "bot/onebot_11/api_bot.h"
#include "event/type.h"
#include "global_config.h"
#include "permission/permission.h"
#include "type.h"
#include "event/event.h"
#include "service/service.h"

namespace white {
class TriggeredService : public Service {
 public:
  template <typename Func>
  TriggeredService(const std::string &service_name,
                   const std::string &description, Func &&func,
                   const int use_permission, const int manage_permission,
                   const bool enable_on_default = true,
                   const bool only_to_me = false)
      : Service(service_name, description, manage_permission,
                enable_on_default),
        func_(new onebot11::FunctionForPlugin(std::forward<Func>(func))),
        use_permission_(use_permission),
        only_to_me_(only_to_me) {}

  template <typename Func>
  TriggeredService(const std::string &service_name, Func &&func,
                   const int use_permission, const int manage_permission,
                   const bool enable_on_default = true,
                   const bool only_to_me = false)
      : TriggeredService(service_name, "", std::forward<Func>(func),
                         use_permission, manage_permission, enable_on_default,
                         only_to_me) {}

  template <typename Func>
  TriggeredService(const std::string &service_name, Func &&func,
                   const int use_permission,
                   const bool enable_on_default = true,
                   const bool only_to_me = false)
      : TriggeredService(service_name, "", std::forward<Func>(func),
                         use_permission, permission::GROUP_ADMIN,
                         enable_on_default, only_to_me) {}

  template <typename Func>
  TriggeredService(const std::string &service_name,
                   const std::string &description, Func &&func,
                   const bool only_to_me = false)
      : TriggeredService(service_name, description, std::forward<Func>(func),
                         permission::NORMAL, permission::GROUP_ADMIN, true,
                         only_to_me) {}

  template <typename Func>
  TriggeredService(const std::string &service_name,
                   const std::string &description, Func &&func,
                   const int use_permission,
                   const bool enable_on_default = true,
                   const bool only_to_me = false)
      : TriggeredService(service_name, description, std::forward<Func>(func),
                         use_permission, permission::GROUP_ADMIN,
                         enable_on_default, only_to_me) {}

  template <typename Func>
  TriggeredService(const std::string &service_name, Func &&func,
                   const bool only_to_me = false)
      : TriggeredService(service_name, "", std::forward<Func>(func),
                         permission::NORMAL, permission::GROUP_ADMIN, true,
                         only_to_me) {}

  virtual ~TriggeredService() { delete func_; };

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
  void Run(Args &&...args) const noexcept {
    LOG_INFO("Handled by [{}]", service_name_);
    try {
      func_->Run(std::forward<Args>(args)...);
    } catch (const std::exception &e) {
      LOG_ERROR("Exception Happened: {}", e.what());
    }
  }

 private:
  const onebot11::ClosureForPlugin *const func_;
  const int use_permission_;

  const bool only_to_me_;
};
}  // namespace white

#endif
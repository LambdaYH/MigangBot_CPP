#pragma once

#include <array>
#include "event/Registrar.h"
#include "fmt/core.h"
#include "global_config.h"
#include "message/message_segment.h"
#include "message/utility.h"
#include "modules/module_interface.h"
#include "permission/permission.h"
#include "service/service_manager.h"
#include "type.h"
#include "utility.h"

namespace white {
namespace module {

namespace sv {

const std::array<std::string, 9> permission_str{
    "黑名单", "通常",   "私聊", "群成员",  "群管理员",
    "群主",   "白名单", "白",   "法外之地"};

void HandleEnableSv(const Event &event, onebot11::ApiBot &bot);
void HandleDisableSv(const Event &event, onebot11::ApiBot &bot);
void HandleListSv(const Event &event, onebot11::ApiBot &bot);

}  // namespace sv

class SV : public Module {
 public:
  SV() : Module() {}
  virtual void Register() override {
    OnPrefix({"启用", "enable"}, make_pair("__sv_control_enable__", "服务管理"), "启用特定服务",
             ACT_OutClass(sv::HandleEnableSv), permission::GROUP_MEMBER,
             permission::ALWAYS_ON);
    OnPrefix({"禁用", "disable"}, make_pair("__sv_control_disable__", "服务管理"), "禁用特定服务",
             ACT_OutClass(sv::HandleDisableSv), permission::GROUP_MEMBER,
             permission::ALWAYS_ON);
    OnFullmatch({"服务列表", "lssv", "列举服务"}, make_pair("__lssv__", "服务管理"), "列举各类服务列表",
                ACT_OutClass(sv::HandleListSv), permission::NORMAL,
                permission::SUPERUSER);
  }
};

namespace sv {

inline void HandleEnableSv(const Event &event, onebot11::ApiBot &bot) {
  if (!event.contains("group_id")) return;
  auto group_id = event["group_id"].get<GId>();
  auto service_name = message::Strip(message::ExtraPlainText(event));
  auto perm = permission::GetUserPermission(event);
  if (!service_name.empty()) {
    if (!ServiceManager::GetInstance().CheckService(service_name)) {
      bot.send_group_msg(group_id, fmt::format("服务[{}]不存在", service_name));
      return;
    }
    if (ServiceManager::GetInstance().GroupEnable(service_name, group_id, perm))
      bot.send_group_msg(group_id,
                         fmt::format("已成功启用服务[{}]", service_name));
    else {
      std::string hint_msg{"当前服务(们)所需管理权限(们)为:["};
      for (auto perm :
           ServiceManager::GetInstance().CheckPermission(service_name)) {
        hint_msg += sv::permission_str[perm] + " ";
      }
      hint_msg.back() = ']';
      bot.send_group_msg(group_id,
                         fmt::format("未能(完全)启用服务[{}]，权限不足。{}",
                                     service_name, hint_msg));
    }
  }
}

inline void HandleDisableSv(const Event &event, onebot11::ApiBot &bot) {
  if (!event.contains("group_id")) return;
  auto group_id = event["group_id"].get<GId>();
  auto service_name = message::Strip(message::ExtraPlainText(event));
  auto perm = permission::GetUserPermission(event);
  if (!service_name.empty()) {
    if (!ServiceManager::GetInstance().CheckService(service_name)) {
      bot.send_group_msg(group_id, fmt::format("服务[{}]不存在", service_name));
      return;
    }
    if (ServiceManager::GetInstance().GroupDisable(service_name, group_id,
                                                   perm))
      bot.send_group_msg(group_id,
                         fmt::format("已成功禁用服务[{}]", service_name));
    else {
      std::string hint_msg{"当前服务(们)所需管理权限(们)为:["};
      for (auto perm :
           ServiceManager::GetInstance().CheckPermission(service_name)) {
        hint_msg += sv::permission_str[perm] + " ";
      }
      hint_msg.back() = ']';
      bot.send_group_msg(group_id,
                         fmt::format("未能(完全)禁用服务[{}]，权限不足。{}",
                                     service_name, hint_msg));
    }
  }
}

inline void HandleListSv(const Event &event, onebot11::ApiBot &bot) {
  std::string msg =
      event.contains("group_id")
          ? "以下为可用服务列表以及本群启用情况\n=====================\n"
          : "以下为可用服务列表\n=====================\n";
  if (!event.contains("group_id")) {
    auto sv_list = ServiceManager::GetInstance().GetServiceList();
    for (auto &[name, description] : sv_list)
      if (!name.empty() && name[0] != '_')
        msg += fmt::format("{}: {}\n\n", name,
                           description.empty() ? "无简介" : description);

  } else {
    auto sv_list = ServiceManager::GetInstance().GetServiceList(
        event["group_id"].get<GId>());
    for (auto &[name, description, status] : sv_list)
      if (!name.empty() && name[0] != '_')
        msg += fmt::format("[{}] {}: {}\n\n", status ? "✓" : "×", name,
                           description.empty() ? "无简介" : description);
  }
  msg.pop_back();
  msg.pop_back();
  bot.send(event, message_segment::image(TextToImg(msg)));
}

}  // namespace sv

}  // namespace module
}  // namespace white
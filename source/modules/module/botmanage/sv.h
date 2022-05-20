#pragma once

#include "event/Registrar.h"
#include "fmt/core.h"
#include "global_config.h"
#include "message/message_segment.h"
#include "message/utility.h"
#include "modules/module_interface.h"
#include "permission/permission.h"
#include "service/service.h"
#include "service/service_manager.h"
#include "type.h"
#include "utility.h"
#include "modules/module/botmanage/sv.h"

namespace white {
namespace module {

namespace sv {

const std::array<std::string, 9> permission_str{
    "黑名单", "通常",   "私聊", "群成员",  "群管理员",
    "群主",   "白名单", "白",   "法外之地"};

inline void HandleEnableSv(const Event &event, onebot11::ApiBot &bot);
inline void HandleDisableSv(const Event &event, onebot11::ApiBot &bot);
inline void HandleListSv(const Event &event, onebot11::ApiBot &bot);
inline void HandleListSvBundle(const Event &event, onebot11::ApiBot &bot);

}  // namespace sv

class SV : public Module {
 public:
  SV() : Module() {}
  virtual void Register() override {
    OnPrefix({"启用", "enable"}, make_pair("__sv_control_enable__", "服务管理"),
             "启用特定服务", ACT_OutClass(sv::HandleEnableSv),
             permission::GROUP_MEMBER, permission::ALWAYS_ON);
    OnPrefix({"禁用", "disable"},
             make_pair("__sv_control_disable__", "服务管理"), "禁用特定服务",
             ACT_OutClass(sv::HandleDisableSv), permission::GROUP_MEMBER,
             permission::ALWAYS_ON);
    OnFullmatch({"服务列表", "lssv", "列举服务"},
                make_pair("__lssv__", "服务管理"), "列举各类服务列表",
                ACT_OutClass(sv::HandleListSv), permission::NORMAL,
                permission::SUPERUSER);
    OnPrefix({"服务包列表", "lsb", "列举服务包"},
             make_pair("__lssv_bundle__", "服务管理"), "列举服务包",
             ACT_OutClass(sv::HandleListSvBundle), permission::NORMAL,
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

inline void HandleListSvBundle(const Event &event, onebot11::ApiBot &bot) {
  auto option = message::Strip(message::ExtraPlainText(event));
  if (option.empty()) {
    std::string msg = "以下为所有可用包\n=====================\n";
    auto bundle_list = ServiceManager::GetInstance().GetBundleList();
    for (auto &bundle : bundle_list) {
      auto service_in_bundle =
          ServiceManager::GetInstance().GetBundleService(bundle);
      for (auto &[name, _] : service_in_bundle)
        if (!name.empty() && name[0] != '_') {
          msg += "[" + bundle + "]\n";
          break;
        }
    }
    msg += "发送[lsb 包名]来查看包内的服务列表";
    bot.send(event, message_segment::image(TextToImg(msg)));
  } else {
    if (!ServiceManager::GetInstance().CheckBundle(option)) {
      bot.send(event,
               fmt::format("不存在名为[{}]的服务包，请发送[lsb]查看可用包列表",
                           option));
      return;
    }
    std::string msg = fmt::format(
        "以下为包[{}]中可用服务以及启用情况\n=====================\n", option);
    if (event.contains("group_id")) {
      auto service_in_bundle = ServiceManager::GetInstance().GetBundleService(
          option, event["group_id"].get<GId>());
      for (auto &[name, description, status] : service_in_bundle)
        if (!name.empty() && name[0] != '_')
          msg += fmt::format("[{}] {}: {}\n\n", status ? "✓" : "×", name,
                             description.empty() ? "无简介" : description);
    } else {
      auto service_in_bundle =
          ServiceManager::GetInstance().GetBundleService(option);
      for (auto &[name, description] : service_in_bundle)
        if (!name.empty() && name[0] != '_')
          msg += fmt::format("{}: {}\n\n", name,
                             description.empty() ? "无简介" : description);
    }
    msg.pop_back();
    msg.pop_back();
    bot.send(event, message_segment::image(TextToImg(msg)));
  }
}

}  // namespace sv

}  // namespace module
}  // namespace white
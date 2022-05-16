#pragma once

#include "fmt/core.h"
#include "global_config.h"
#include "message/utility.h"
#include "modules/module_interface.h"
#include "permission/permission.h"
#include "utility.h"

namespace white {
namespace module {

class Dismiss : public Module {
 public:
  Dismiss() : Module() {}
  virtual void Register() override {
    OnPrefix({".dismiss", "。dismiss"}, "__dismiss__",
             ACT_InClass(Dismiss::HandleDismiss), permission::GROUP_ADMIN,
             permission::SUPERUSER);
  }

 private:
  void HandleDismiss(const Event &event, onebot11::ApiBot &bot);
};

inline void Dismiss::HandleDismiss(const Event &event, onebot11::ApiBot &bot) {
  if (!event.contains("group_id")) return;
  auto qid = message::Strip(message::ExtraPlainText(event), ' ');
  if (qid == "2215") {
    auto group_id = event["group_id"].get<GId>();
    bot.send_group_msg(group_id, "哼!走了");
    bot.set_group_leave(group_id);
    for (auto &white : config::SUPERUSERS)
      bot.send_private_msg(white, fmt::format("{}|{}已因dismiss命令退出群{}",
                                              datetime::GetCurrentTime(),
                                              config::BOT_NAME, group_id));
  }
}

}  // namespace module
}  // namespace white
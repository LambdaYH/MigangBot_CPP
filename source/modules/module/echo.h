#pragma once

#include "modules/module_interface.h"

#include <string_view>

#include "co/co.h"

namespace white {
namespace module {

class Echo : public Module {
 public:
  Echo() : Module() {}
  virtual void Register();

 private:
  void DoEcho(const Event &event, onebot11::ApiBot &bot);
};

inline void Echo::Register() {
  RegisterCommand(PREFIX, {"/echo", "/回声"}, ACT(Echo::DoEcho),
                  permission::NORMAL);
}

inline void Echo::DoEcho(const Event &event, onebot11::ApiBot &bot) {
  auto text = message::ExtraPlainText(event);
  auto ret = bot.send_msg(event, text).Ret();
}

}  // namespace module
}  // namespace white
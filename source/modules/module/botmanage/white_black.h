#pragma once

#include "message/utility.h"
#include "modules/module_interface.h"
#include "modules/module/botmanage/config.h"
#include "utility.h"

namespace white {
namespace module {

class WhiteBlack : public Module {
 public:
  WhiteBlack() {}
  virtual void Register() override {
    OnPrefix({".white"}, make_pair("__white__", "机器人管理"), "解除黑名单",
             ACT_InClass(WhiteBlack::HandleWhite), permission::SUPERUSER,
             permission::ALWAYS_ON, true, false);
    OnPrefix({".black"}, make_pair("__black__", "机器人管理"), "添加黑名单",
             ACT_InClass(WhiteBlack::HandleBlack), permission::SUPERUSER,
             permission::ALWAYS_ON, true, false);
  }

 private:
  void HandleWhite(const Event &event, onebot11::ApiBot &bot);
  void HandleBlack(const Event &event, onebot11::ApiBot &bot);
};

inline void WhiteBlack::HandleWhite(const Event &event, onebot11::ApiBot &bot) {
  std::string_view key{message::Strip(message::ExtraPlainText(event))};
  if (key.starts_with("qq")) {
    key.remove_prefix(std::min(key.size(), key.find_first_not_of(' ', 2)));
    if (!IsDigitStr(key)) {
      bot.send(event, fmt::format("[{}]不是一个合法QQ号", key), true);
      return;
    }
    if (botmanage::DelFromQQBlackList(std::atoll(key.data())))
      bot.send(event, fmt::format("已将(Q){}从黑名单移除", key));
    else
      bot.send(event, fmt::format("(Q){}未在黑名单中", key));
  } else if (key.starts_with("group")) {
    key.remove_prefix(std::min(key.size(), key.find_first_not_of(' ', 5)));
    if (!IsDigitStr(key)) {
      bot.send(event, fmt::format("[{}]不是一个合法群号", key), true);
      return;
    }
    if (botmanage::DelFromGroupBlackList(std::atoll(key.data())))
      bot.send(event, fmt::format("已将(群){}从黑名单移除", key));
    else
      bot.send(event, fmt::format("(群){}未在黑名单中", key));
  } else
    bot.send(event, "参数错误:.white qq/group id");
}

inline void WhiteBlack::HandleBlack(const Event &event, onebot11::ApiBot &bot) {
  std::string_view key{message::Strip(message::ExtraPlainText(event))};
  if (key.starts_with("qq")) {
    key.remove_prefix(std::min(key.size(), key.find_first_not_of(' ', 2)));
    auto reason = key.substr(std::min(key.size(), key.find_first_of(' ') + 1));
    message::LStrip(reason, ' ');
    auto id = key.substr(0, std::min(key.size(), key.find_first_of(' ')));
    if(id.empty()) {
      bot.send(event, "请输入所需拉黑的qq号");
      return;
    }
    if(reason.empty()){
      bot.send(event, "请输入黑名单原因");
      return;
    }
    if (!IsDigitStr(id)) {
      bot.send(event, fmt::format("[{}]不是一个合法QQ号", id), true);
      return;
    }
    botmanage::AddToQQBlackList(std::atoll(id.data()), std::string(reason));
    bot.send(event, fmt::format("已将(Q){}加入黑名单", id));
  } else if (key.starts_with("group")) {
    key.remove_prefix(std::min(key.size(), key.find_first_not_of(' ', 5)));
    auto reason = key.substr(std::min(key.size(), key.find_first_of(' ') + 1));
    message::LStrip(reason, ' ');
    auto id = key.substr(0, std::min(key.size(), key.find_first_of(' ')));
    if(id.empty()) {
      bot.send(event, "请输入所需拉黑的群号");
      return;
    }
    if(reason.empty()){
      bot.send(event, "请输入黑名单原因");
      return;
    }
    if (!IsDigitStr(id)) {
      bot.send(event, fmt::format("[{}]不是一个合法群号", id), true);
      return;
    }
    botmanage::AddToGroupBlackList(std::atoll(id.data()), std::string(reason));
    bot.send(event, fmt::format("已将(群){}加入黑名单", id));
  } else
    bot.send(event, "参数错误:.black qq/group id reason");
}

}  // namespace module
}  // namespace white
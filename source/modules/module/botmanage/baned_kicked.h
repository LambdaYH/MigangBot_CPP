#pragma once

#include "modules/module_interface.h"

namespace white {
namespace module {

class BandedOrKicked : public Module {
 public:
  BandedOrKicked()
      : Module("botmanage/botmanage.yml", botmanage::kConfigExample),
        config_(LoadConfig()) {
    botmanage::add_friend_flag = config_["是否同意好友邀请"].as<bool>();
  }
  virtual void Register() override {
    RegisterRequest("friend", "", ACT(FriendInvite::Handle_friend_invite));
  }

 private:
  void Handle_banded(const Event &event, onebot11::ApiBot &bot);
  void Handle_Kicked(const Event &event, onebot11::ApiBot &bot);

 private:
  Config config_;
};

}  // namespace module
}  // namespace white
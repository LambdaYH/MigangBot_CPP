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
    auto service = std::make_shared<Service>(
        "被害检测", ACT(BilibiliParser::Parser), permission::GROUP_MEMBER);
    RegisterRequest("friend", "", "被害检测", ACT(BilibiliParser::Parser), permission::GROUP_MEMBER);
  }

 private:
  void Handle_banded(const Event &event, onebot11::ApiBot &bot);
  void Handle_Kicked(const Event &event, onebot11::ApiBot &bot);

 private:
  Config config_;
};

}  // namespace module
}  // namespace white
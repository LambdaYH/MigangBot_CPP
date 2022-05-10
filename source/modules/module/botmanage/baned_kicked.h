#pragma once

#include "modules/module_interface.h"

namespace white {
namespace module {

class BandedOrKicked : public Module {
 public:
  BandedOrKicked() : Module() {}
  virtual void Register() override {}

 private:
  void Handle_banded(const Event &event, onebot11::ApiBot &bot);
  void Handle_Kicked(const Event &event, onebot11::ApiBot &bot);

 private:
  Config config_;
};

}  // namespace module
}  // namespace white
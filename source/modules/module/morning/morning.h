#pragma once

#include <vector>

#include "modules/module_interface.h"

#include "tools/aiorequests.h"
#include "utility.h"
#include "schedule/schedule.h"

namespace white {
namespace module {

class Morning : public Module {
 public:
  Morning() {}
  virtual void Register() override {
    sv_ = OnSchedule(make_pair("早上问好", "订阅"), "早上定时发送问好消息", false);
    Schedule().cron("8 8 * * *", [this](){SayMorning();});
  }

 private:
  ScheduleServicePtr sv_;

 private:
  void SayMorning();
};

namespace morning {
inline std::string GetMorningMessage() {
  auto r =
      aiorequests::Get(fmt::format("https://timor.tech/api/holiday/tts?t={}",
                                   datetime::GetTimeStampS()),
                       7)
          .get();
  if (!r) return "";
  auto js = r->GetJson();
  if (js["code"].get<int>() == 0) return js["tts"].get<std::string>();
  return "";
}

};  // namespace morning

inline void Morning::SayMorning() {
  static std::vector<std::string> kMorning{"早上好呀", "大家早上好！", "早上好~",
                                      "各位早上好！"};
  auto message = fmt::format("{}\n{}", *select_randomly(kMorning.begin(), kMorning.end()), morning::GetMorningMessage());
  for(const auto &bot : Bots())
    sv_->BroadCast(bot, message);
}

}  // namespace module
}  // namespace white
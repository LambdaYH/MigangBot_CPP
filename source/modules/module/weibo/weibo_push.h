#pragma once

#include <algorithm>
#include <chrono>
#include <initializer_list>
#include <iterator>
#include <unordered_map>

#include "modules/module_interface.h"
#include "modules/module/weibo/weibo_spider.h"
#include "modules/module/weibo/weibo_recorder.h"
#include "permission/permission.h"
#include "schedule/schedule.h"
#include "service/schedule_service.h"
#include "utility.h"
#include "message/message_segment.h"

namespace white {
namespace module {
namespace weibo {

constexpr auto kConfigExample =
    "ff14-weibo:\n"
    "  enable_on_default: true\n"
    "  users:\n"
    "    最终幻想14:\n"
    "      user_id: \"1797798792\"\n"
    "      filter: false\n"
    "      filter_words: [\"微博抽奖平台\"]\n"
    "    丝瓜卡夫卡:\n"
    "      user_id: \"1794603954\"\n"
    "      filter: false\n"
    "      filter_words: [\"微博抽奖平台\"]\n"
    "ShiningNikki-weibo:\n"
    "  enable_on_default: false\n"
    "  users:\n"
    "    闪耀暖暖官微:\n"
    "      user_id: \"6498105282\"\n"
    "      filter: false\n"
    "      filter_words: []";

inline std::pair<std::string, std::string> weibo_to_text(Json &wb) {
  std::string msg(fmt::format("{}\'s Weibo:\n=====================",
                              wb["screen_name"].get<std::string>()));
  auto id_cache = wb["id"].get<std::string>();
  if (wb.contains("retweet")) {
    msg += fmt::format("\n{}\n=======转发微博=======",
                       wb["text"].get<std::string>());
    wb = wb["retweet"];
  }
  msg += "\n" + wb["text"].get<std::string>();
  if (!wb["pics"].empty()) {
    msg += '\n';
    for (std::size_t i = 0; i < wb["pics"].size(); ++i) {
      msg += message_segment::image(wb["pics"][i].get<std::string>());
    }
  }

  if (!wb["video_poster"].get<std::string>().empty()) {
    msg += fmt::format(
        "\n[视频封面]\n{}",
        message_segment::image(wb["video_poster"].get<std::string>()));
  }

  msg += fmt::format("\nURL:https://m.weibo.cn/detail/{}", id_cache);
  return {msg, id_cache};
}

}  // namespace weibo

class Weibo : public Module {
 public:
  Weibo() : Module("weibo/weibo.yml", weibo::kConfigExample) {
    auto config = LoadConfig();
    for (auto weibo_sub_service = config.begin();
         weibo_sub_service != config.end(); ++weibo_sub_service) {
      auto sv_name = weibo_sub_service->first.as<std::string>();
      enable_on_default_.emplace(
          sv_name, weibo_sub_service->second["enable_on_default"].as<bool>());
      for (auto weibo_conc = weibo_sub_service->second["users"].begin();
           weibo_conc != weibo_sub_service->second["users"].end();
           ++weibo_conc) {
        std::vector<std::string> filter_words;
        for (std::size_t i = 0; i < weibo_conc->second["filter_words"].size();
             ++i)
          filter_words.push_back(
              weibo_conc->second["filter_words"][i].as<std::string>());
        spiders_[sv_name].emplace_back(
            std::move(filter_words), weibo_conc->second["filter"].as<bool>(),
            weibo_conc->second["user_id"].as<std::string>());
      }
      LOG_INFO("已成功装载[{}]微博推送服务", sv_name);
    }
  }
  void Register() override {
    for (auto &[name, _] : spiders_) {
      svs_.emplace(name, OnSchedule(name, permission::GROUP_ADMIN,
                                    enable_on_default_.at(name)));
    }
    Schedule().interval(1min, [this]() { WeiboPuller(); });
    Schedule().cron("5 5 * * *", [this]() { CleanBuffer(); });
  }

 private:
  inline void WeiboPuller();
  inline void CleanBuffer();

 private:
  std::unordered_map<std::string, std::shared_ptr<ScheduleService>> svs_;
  std::unordered_map<std::string, std::vector<weibo::WeiboSpider>> spiders_;
  std::unordered_map<std::string, bool> enable_on_default_;

  WeiboRecorder recorder_;
};

inline void Weibo::WeiboPuller() {
  LOG_INFO("开始抓取微博");
  auto cur_time = datetime::GetCurrentTime();
  for (auto &[name, spiders] : spiders_) {
    std::vector<std::string> weibos;
    for (auto &spider : spiders) {
      std::vector<std::pair<std::string, std::string>> formatted_wb;
      auto lastest_wb = spider.GetLastestWeibos();
      for (auto &wb : lastest_wb)
        std::transform(lastest_wb.begin(), lastest_wb.end(),
                       std::back_inserter(weibos), [this, &cur_time](auto &wb) {
                         auto fwb = weibo::weibo_to_text(wb);
                         recorder_.RecordWeibo(fwb.second, cur_time, fwb.first);
                         return fwb.first;
                       });
      if (!lastest_wb.empty())
        LOG_INFO("weibo: 成功获取@{}的新微博{}条", spider.GetUserName(),
                 lastest_wb.size());
      else
        LOG_INFO("weibo: 未检测到@{}的新微博", spider.GetUserName());
    }
    for (auto &weibo : weibos) {
      auto bots = Bots();
      for (auto bot : bots) svs_.at(name)->BroadCast(bot, weibo, 0.5 * 1000);
    }
  }
  LOG_INFO("微博抓取结束");
}

inline void Weibo::CleanBuffer() {
  for (auto &[_, spiders] : spiders_) {
    for (auto &spider : spiders) spider.CleanBuffer();
  }
}

}  // namespace module
}  // namespace white
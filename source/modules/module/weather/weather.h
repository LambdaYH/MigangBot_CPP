#pragma once

#include <cstddef>
#include <string>
#include "co/str.h"
#include "event/Registrar.h"
#include "message/utility.h"
#include "message/message_segment.h"
#include "modules/module_interface.h"
#include "modules/module/weather/eorzean_weather_data.h"
#include "modules/module/weather/eorzean_weather.h"
#include "modules/module/weather/qweather_api.h"
#include "type.h"
#include "utility.h"

namespace white {
namespace module {

namespace weather {

constexpr auto kConfigExample =
    "# 和风天气api_key, https://console.qweather.com/\n"
    "API_KEY: <Your_API_key>"
    "\n"
    "# 腾讯NLP API https://console.cloud.tencent.com/cam/capi\n"
    "SECRET_ID: <SecretId>\n"
    "SECRET_KEY: <SecretKey>";

constexpr auto kManual =
    "[搜天气 北京] 查询城市信息，支持多语言文字、以逗号分隔的经度/纬度坐标\n \
[实时天气 2] 查询搜天气返回列表中序号对应城市的实时天气情况\n \
[今日天气 3] 查询对应城市今日天气情况\n \
[天气简报 0] 查询对应城市近三日天气情况\n \
============\n \
[成都天气]\n \
[天气杭州]\n \
可以以相对不精确的方式进行快速搜索\n \
>>>不附带序号的话默认为首位（0号）城市w";

};  // namespace weather

class Weather : public Module {
 public:
  Weather() : Module("weather.yml", weather::kConfigExample) {
    auto config = LoadConfig();
    api_key_ = config["API_KEY"].as<std::string>();
    secret_id_ = config["SECRET_ID"].as<std::string>();
    secret_key_ = config["SECRET_KEY"].as<std::string>();
    eorzean_weather::Init();
  }
  virtual void Register() override {
    OnFullmatch({"查天气帮助", "天气帮助"}, make_pair("天气帮助", "天气"),
                ACT_InClass(Manual));
    OnPrefix({"搜天气", "查天气"}, make_pair("查天气", "天气"),
             ACT_InClass(SetLocation));
    OnPrefix({"实时天气", "实况天气", "当前天气"},
             make_pair("实时天气", "天气"), ACT_InClass(RealTimeWeather));
    OnPrefix({"今日天气"}, make_pair("今日天气", "天气"),
             ACT_InClass(TodayWeather));
    OnPrefix({"天气简报"}, make_pair("天气简报", "天气"),
             ACT_InClass(WeatherShortDesc));
  }

 private:
  void Manual(const Event &event, onebot11::ApiBot &bot) {
    bot.send(event, weather::kManual, true);
  }
  void SetLocation(const Event &event, onebot11::ApiBot &bot);
  void RealTimeWeather(const Event &event, onebot11::ApiBot &bot);
  void TodayWeather(const Event &event, onebot11::ApiBot &bot);
  void WeatherShortDesc(const Event &event, onebot11::ApiBot &bot);

 private:
  std::string api_key_;
  std::string secret_id_;
  std::string secret_key_;

  std::unordered_map<std::string, std::vector<qweather::LocationInfo>>
      gid_uid_locationid_;
};

inline void Weather::SetLocation(const Event &event, onebot11::ApiBot &bot) {
  std::string key;
  if (event.contains("group_id"))
    key = fmt::format("{}-{}", event["group_id"].get<GId>(),
                      event["user_id"].get<QId>());
  else
    key = std::to_string(event["user_id"].get<QId>());
  gid_uid_locationid_.erase(key);
  auto city_name = message::Strip(message::ExtraPlainText(event));
  if (city_name.empty()) {
    bot.send(event, "请输入要查询的城市名哦~");
    city_name = bot.WaitForNextMessage(event);
    if (city_name.empty()) return;
  }
  auto ret = eorzean_weather::GetEorzeanWeather(city_name);
  if (!ret.empty()) {
    bot.send(event, ret, true);
    return;
  }
  auto location_list = qweather::GetLocationName(city_name, api_key_);
  if (location_list.empty()) {
    bot.send(event, fmt::format("未找到名为[{}的城市或网络异常", city_name));
    return;
  }
  std::string msg = "请选择想要查询的城市~";
  for (std::size_t i = 0; auto &location : location_list) {
    msg += fmt::format("\n{}.{} - {} - {} - {}", i++, location.country,
                       location.adm1, location.adm2, location.name);
    gid_uid_locationid_[key].push_back(location);
  }
  msg +=
      "\n\n发送[实时天气 数字](例如 实时天气 "
      "0)可查看当前天气情况\n可以发送“天气帮助”获取使用说明哦~";
  bot.send(event, message_segment::image(TextToImg(msg)), true);
}

inline void Weather::RealTimeWeather(const Event &event,
                                     onebot11::ApiBot &bot) {
  std::string key;
  if (event.contains("group_id"))
    key = fmt::format("{}-{}", event["group_id"].get<GId>(),
                      event["user_id"].get<QId>());
  else
    key = std::to_string(event["user_id"].get<QId>());
  if (!gid_uid_locationid_.contains(key)) {
    bot.send(event, "请先发送“搜天气 城市”来选择地区哟", true);
    return;
  }
  auto location_idx_str = message::Strip(message::ExtraPlainText(event));
  int location_idx = 0;
  if (!location_idx_str.empty() && IsDigitStr(location_idx_str))
    location_idx = std::stoi(location_idx_str);
  if (location_idx >= gid_uid_locationid_.at(key).size()) location_idx = 0;
  auto location_id = gid_uid_locationid_.at(key)[location_idx].id;
  auto weather = qweather::GetWeatherNow(location_id, api_key_);
  if (weather.empty()) {
    bot.send(event, "查询出错...", true);
    return;
  }
  auto observe_time = fastring(weather["obsTime"].get<std::string>());
  auto obt = str::split(observe_time, '+')[0].replace("T", " ");
  auto msg = fmt::format(
      "==={}实时天气===\n"
      "当地观测时间:{}\n"
      "当前气象:{}\n"
      "当前温度:{}°C\n"
      "当前体感温度:{}°C\n"
      "当前风力等级:{}级\n"
      "当前风速:{}km/h\n"
      "当前相对湿度:{}%\n"
      "当前大气压强:{}hpa\n"
      "当前能见度:{}km\n"
      "当前云量:{}%",
      weather["name"].get<std::string>(), obt.c_str(),
      weather["temp"].get<std::string>(), weather["text"].get<std::string>(),
      weather["temp"].get<std::string>(),
      weather["feelsLike"].get<std::string>(),
      weather["windScale"].get<std::string>(),
      weather["humidity"].get<std::string>(),
      weather["pressure"].get<std::string>(), weather["vis"].get<std::string>(),
      weather["cloud"].get<std::string>());
  msg = fmt::format("{}\n也可以进入{}查看当前城市天气详情哦~", msg,
                    weather["fxLink"].get<std::string>());
  bot.send(event, msg, true);
}

inline void Weather::TodayWeather(const Event &event, onebot11::ApiBot &bot) {
  std::string key;
  if (event.contains("group_id"))
    key = fmt::format("{}-{}", event["group_id"].get<GId>(),
                      event["user_id"].get<QId>());
  else
    key = std::to_string(event["user_id"].get<QId>());
  if (!gid_uid_locationid_.contains(key)) {
    bot.send(event, "请先发送“搜天气 城市”来选择地区哟", true);
    return;
  }
  auto location_idx_str = message::Strip(message::ExtraPlainText(event));
  int location_idx = 0;
  if (!location_idx_str.empty() && IsDigitStr(location_idx_str))
    location_idx = std::stoi(location_idx_str);
  if (location_idx >= gid_uid_locationid_.at(key).size()) location_idx = 0;
  auto location_id = gid_uid_locationid_.at(key)[location_idx].id;
  auto weather = qweather::GetWeatherForcast(location_id, api_key_);
  if (weather.empty()) {
    bot.send(event, "查询出错...", true);
    return;
  }
  auto tenki = weather[0];
  auto text_day =
      qweather::GetWeatherText(tenki["textDay"].get<std::string>(), true);
  auto text_night =
      qweather::GetWeatherText(tenki["textNight"].get<std::string>(), false);
  auto text_uv =
      qweather::GetUVText(std::stoi(tenki["uvIndex"].get<std::string>()));
  auto msg = fmt::format(
      "==={}{}天气预报===\n"
      "今天{}\n"
      "今日最低温度为{}°C，最高温度是{}°C\n"
      "白天的风力等级为{}级，风向是{}哟\n"
      "{}\n"
      "今天{}\n"
      "夜间风力等级为{}级，风向是{}的说\n"
      "日出时间为{}，日落时间则为{}哒\n"
      "今晚的{}将在{}升起，{}落下\n"
      "今日的相对湿度为{}%，大气压强为{}hpa\n"
      "能见度为{}km，云量为{}%，总降水量为{}mm",
      qweather::YMDTOCHS(tenki["fxDate"].get<std::string>()),
      tenki["name"].get<std::string>(), text_day,
      tenki["tempMin"].get<std::string>(), tenki["tempMax"].get<std::string>(),
      tenki["windScaleDay"].get<std::string>(),
      tenki["windDirDay"].get<std::string>(), text_uv, text_night,
      tenki["windScaleNight"].get<std::string>(),
      tenki["windDirNight"].get<std::string>(),
      qweather::HMTOCHS(tenki["sunrise"].get<std::string>()),
      qweather::HMTOCHS(tenki["sunset"].get<std::string>()),
      tenki["moonPhase"].get<std::string>(),
      qweather::HMTOCHS(tenki["moonrise"].get<std::string>()),
      qweather::HMTOCHS(tenki["moonset"].get<std::string>()),
      tenki["humidity"].get<std::string>(),
      tenki["pressure"].get<std::string>(), tenki["vis"].get<std::string>(),
      tenki["cloud"].get<std::string>(), tenki["precip"].get<std::string>());
  msg = fmt::format("{}\n也可以进入{}查看当前城市天气详情哦~", msg,
                    tenki["fxLink"].get<std::string>());
  bot.send(event, message_segment::image(TextToImg(msg)), true);
}

inline void Weather::WeatherShortDesc(const Event &event,
                                      onebot11::ApiBot &bot) {
  std::string key;
  if (event.contains("group_id"))
    key = fmt::format("{}-{}", event["group_id"].get<GId>(),
                      event["user_id"].get<QId>());
  else
    key = std::to_string(event["user_id"].get<QId>());
  if (!gid_uid_locationid_.contains(key)) {
    bot.send(event, "请先发送“搜天气 城市”来选择地区哟", true);
    return;
  }
  auto location_idx_str = message::Strip(message::ExtraPlainText(event));
  int location_idx = 0;
  if (!location_idx_str.empty() && IsDigitStr(location_idx_str))
    location_idx = std::stoi(location_idx_str);
  if (location_idx >= gid_uid_locationid_.at(key).size()) location_idx = 0;
  auto location_id = gid_uid_locationid_.at(key)[location_idx].id;
  auto weather = qweather::GetWeatherForcast(location_id, api_key_);
  if (weather.empty()) {
    bot.send(event, "查询出错...", true);
    return;
  }
  auto city = weather[0]["name"].get<std::string>();
  auto msg = fmt::format("{}近三日天气简报", city);
  for (auto &desc : weather) {
    std::string text_trans;
    if (desc["textDay"].get<std::string>() ==
        desc["textNight"].get<std::string>())
      text_trans = desc["textDay"].get<std::string>();
    else
      text_trans = fmt::format("{}转{}", desc["textDay"].get<std::string>(),
                               desc["textNight"].get<std::string>());
    std::string temp_trans;
    if (desc["tempMax"].get<std::string>() ==
        desc["tempMin"].get<std::string>())
      temp_trans = desc["tempMax"].get<std::string>();
    else
      temp_trans = fmt::format("{}°C~{}°C", desc["tempMax"].get<std::string>(),
                               desc["tempMin"].get<std::string>());
    msg += fmt::format("\n{}\n{}\n{}\n{}", std::string(15, '='),
                       qweather::YMDTOCHS(desc["fxDate"].get<std::string>()),
                       text_trans, temp_trans);
  }
  bot.send(event, message_segment::image(TextToImg(msg)), true);
}

}  // namespace module
}  // namespace white
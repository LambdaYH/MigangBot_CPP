#pragma once

#include <vector>
#include <string>

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "co/fastring.h"
#include "co/hash/url.h"
#include "co/str.h"
#include "hv/hurl.h"
#include "message/utility.h"
#include "tools/aiorequests.h"
#include "logger/logger.h"
#include "modules/module/weather/qweather_settings.h"
#include "utility.h"

namespace white {
namespace qweather {

struct LocationInfo {
  std::string id;
  std::string name;
  std::string country;
  std::string adm1;
  std::string adm2;
};

inline Json GetJson(const std::string &url) {
  auto r = aiorequests::Get(HUrl::escapeUrl(url), 7).get();
  if(!r)
    return {};
  return Json::parse(zlib::gzip_uncompress(r->Body()));
}

inline std::vector<LocationInfo> GetLocationName(const std::string &location,
                                                 const std::string &key) {
  auto url = fmt::format(
      "https://geoapi.qweather.net/v2/city/lookup?key={}&location={}", key,
      location);
  auto resp = GetJson(url);
  if (!resp) {
    LOG_ERROR("获取城市信息数据超时");
    return {};
  };
  if (!resp.contains("code") || resp["code"].get<std::string>() != "200") {
    LOG_ERROR("城市信息搜索接口调用失败, {}", resp["code"].get<std::string>());
    return {};
  }
  std::vector<LocationInfo> ret;
  for (auto &item : resp["location"])
    ret.emplace_back(
        item["id"].get<std::string>(), item["name"].get<std::string>(),
        item["country"].get<std::string>(), item["adm1"].get<std::string>(),
        item["adm2"].get<std::string>());
  return ret;
}

inline nlohmann::json GetWeatherNow(const std::string &location,
                                    const std::string &key) {
  auto url = fmt::format(
      "https://devapi.qweather.net/v7/weather/now?key={}&location={}", key,
      location);
  auto js = GetJson(url);
  if (!js) {
    LOG_ERROR("获取实况天气数据超时");
    return {};
  }
  auto location_info = GetLocationName(location, key);
  if (location_info.empty()) return {};
  auto location_id = location_info[0].id;
  if (js["code"].get<std::string>() != "200") {
    LOG_ERROR("实况天气获取接口调用失败");
    return {};
  }
  js["now"]["fxLink"] = js["fxLink"];
  js["now"]["name"] = location_info[0].name;
  return js["now"];
}

inline std::vector<nlohmann::json> GetWeatherForcast(const std::string &location,
                                        const std::string &key) {
  auto location_info = GetLocationName(location, key);
  if (location_info.empty()) return {};
  auto location_id = location_info[0].id;
  auto url = fmt::format(
      "https://devapi.qweather.net/v7/weather/3d?key={}&location={}", key,
      location);
  auto js = GetJson(url);
  if (!js) {
    LOG_ERROR("获取天气预报数据超时");
    return {};
  }
  if (js["code"].get<std::string>() != "200") {
    LOG_ERROR("天气预报获取接口调用失败");
    return {};
  }
  std::vector<nlohmann::json> ret;
  for(auto &item : js["daily"]) {
    ret.push_back(item);
  }
  return ret;
}

inline std::string GetWeatherText(const std::string &text, bool is_day) {
  if(is_day)
    return kDesciptionDay.at(text);
  return kDesciptionNight.at(text);
}

inline std::string GetUVText(int uv) {
    auto text = fmt::format("今日紫外线强度指数为{}，", uv);
    if(uv >= 0 && uv < 3)
        text += kUVDesciption[0];
    else if(uv >= 3 && uv < 5)
        text += kUVDesciption[1];
    else if(uv >= 5 && uv < 7)
        text += kUVDesciption[2];
    else if(uv >= 7 && uv < 10)
        text += kUVDesciption[3];
    else if(uv >= 10)
        text += kUVDesciption[4];
    return text;
}

inline std::string YMDTOCHS(const std::string &ymd) {
  auto d = str::split(ymd, "-");
  return fmt::format("{}年{}月{}日", d[0].c_str(), d[1].c_str(), d[2].c_str());
}

inline std::string HMTOCHS(fastring &&hm) {
  return (hm.replace(":", "时") + "分").c_str();
}

}  // namespace qweather
}  // namespace white
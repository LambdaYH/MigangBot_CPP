#pragma once

#include <ctime>
#include "fmt/format.h"
#include "modules/module/weather/eorzean_weather_data.h"
#include "utility.h"

namespace white {
namespace eorzean_weather {

inline int ForcastTarget(int date) {
  auto bell = date / 175;
  auto increment = (bell + 8 - (bell % 8)) % 24;
  auto total_days = date / 4200;
  auto calc_base = total_days * 100 + increment;

  unsigned int step_1 = (calc_base << 11) ^ calc_base;
  unsigned int step_2 = (step_1 >> 8) ^ step_1;
  return step_2 % 100;
}

inline std::string CalWeather(int date, const std::string &location) {
  if (!location_idx.contains(location)) return "";
  auto weather_rate = weather_rate_idx.at(location_idx.at(location));
  auto target = ForcastTarget(date);
  for (auto &rate : weather_rate)
    if (target < rate.rate) return weather_idx.at(rate.weather);
  return "";
}

inline int NextWeatherTime(int date, int count) {
  auto increment = (date + 1400 - (date % 1400)) % 4200;
  auto cur_time = date % 4200;
  int left_sec;
  if (increment > cur_time)
    left_sec = increment - cur_time;
  else
    left_sec = 4200 - cur_time;
  left_sec += 1400 * count;
  return left_sec;
}

inline int GetEorzeaHour(int time) { return (time / 175) % 24; }

inline std::string GetEorzeanWeather(std::string &location) {
  if (alter_name_idx.contains(location)) location = alter_name_idx.at(location);
  auto date = datetime::GetTimeStampS();
  auto cur_weather = CalWeather(date, location);
  if (cur_weather.empty()) return "";
  std::string ret = fmt::format("[{}]\n当前天气: {}", location, cur_weather);
  for (std::size_t i = 0; i < 9; ++i) {
    auto next_date = date + 1400 * (i + 1);
    auto future_weather = CalWeather(next_date, location);
    auto next_time = NextWeatherTime(date, i) + date;
    ret += fmt::format("\nLT:{:%m-%d %H:%M:%S}  ET:{:02}:00 -> {}",
                       *localtime(&next_time), GetEorzeaHour(next_time),
                       future_weather);
  }
  return ret;
}

}  // namespace eorzean_weather
}  // namespace white

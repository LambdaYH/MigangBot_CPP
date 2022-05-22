#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace white {
namespace eorzean_weather {
struct WeatherRate {
  int weather;
  int rate;
};

extern std::unordered_map<std::string, std::string> alter_name_idx;
extern std::vector<std::string> weather_idx;
extern std::unordered_map<std::string, int> location_idx;
extern std::unordered_map<int, std::vector<WeatherRate>> weather_rate_idx;

void Init();

void Update();

}  // namespace eorzean_weather
}  // namespace white

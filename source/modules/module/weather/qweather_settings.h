#pragma once

#include <string>
#include <unordered_map>
#include <array>

namespace white {
namespace qweather {
extern const std::unordered_map<std::string, std::string> kStatusCode;
extern const std::unordered_map<std::string, std::string> kDesciptionDay;
extern const std::unordered_map<std::string, std::string> kDesciptionNight;
extern const std::array<std::string, 5> kUVDesciption;
}  // namespace qweather
}  // namespace white
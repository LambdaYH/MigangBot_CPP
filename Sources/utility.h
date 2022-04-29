#ifndef MIGANGBOTCPP_UTILITY_H_
#define MIGANGBOTCPP_UTILITY_H_

#include <iterator>
#include <string>
#include <codecvt>
#include <chrono>
#include <ctime>
#include <fmt/chrono.h>
#include <algorithm>
#include "utf8.h"

namespace white
{

inline std::string ReverseUTF8(const std::string &str)
{
    std::u16string str_u16 = utf8::utf8to16(str);
    std::reverse(str_u16.begin(), str_u16.end());
    return utf8::utf16to8(str_u16);
}

inline bool IsDigitStr(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), isdigit);
}

namespace datatime
{
    std::string GetCurrentTime()
    {
        std::time_t result = std::time(nullptr);
        std::tm *tm = localtime(&result);
        return fmt::format("{:%Y-%m-%d %H:%M:%S}", *tm);
    }

} // namespace datatime

} // namespace white

#endif
#ifndef MIGANGBOTCPP_UTILITY_H_
#define MIGANGBOTCPP_UTILITY_H_

#include <string>
#include <codecvt>
#include "utf8.h"

namespace white
{

std::string ReverseUTF8(const std::string &str)
{
    std::u16string str_u16 = utf8::utf8to16(str);
    std::reverse(str_u16.begin(), str_u16.end());
    return utf8::utf16to8(str_u16);
}

} // namespace white

#endif
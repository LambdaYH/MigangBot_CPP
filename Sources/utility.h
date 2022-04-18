#ifndef MIGANGBOTCPP_UTILITY_H_
#define MIGANGBOTCPP_UTILITY_H_

#include <string>
#include <codecvt>

namespace white
{

// by @SilverCicada(https://github.com/SilverCicada)
inline std::wstring str_to_wstr(const std::string &src)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(src);
}

inline std::string wstr_to_str(const std::wstring &src)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(src);
}

} // namespace white

#endif
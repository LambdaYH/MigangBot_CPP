#ifndef MIGANGBOT_BOT_CONVERT_TO_STRING_H_
#define MIGANGBOT_BOT_CONVERT_TO_STRING_H_

#include <string>

#include <nlohmann/json.hpp>

namespace white
{

template<typename T>
inline std::string ConvertToString(T msg)
{
    return std::string(msg);
}

template<>
inline std::string ConvertToString(const std::string &msg)
{
    return msg;
}

template<>
inline std::string ConvertToString(const std::string_view &msg)
{
    return std::string(msg);
}

template<>
inline std::string ConvertToString(const nlohmann::json &msg)
{
    return msg.dump();
}

template<>
inline std::string ConvertToString(const char msg)
{
    return std::string(1, msg);
}

template<>
inline std::string ConvertToString(short msg)
{
    return std::to_string(static_cast<int>(msg));
}

template<>
inline std::string ConvertToString(unsigned short msg)
{
    return std::to_string(static_cast<unsigned int>(msg));
}

template<>
inline std::string ConvertToString(int msg)
{
    return std::to_string(msg);
}

template<>
inline std::string ConvertToString(unsigned int msg)
{
    return std::to_string(msg);
}

template<>
inline std::string ConvertToString(long msg)
{
    return std::to_string(msg);
}

template<>
inline std::string ConvertToString(unsigned long msg)
{
    return std::to_string(msg);
}

template<>
inline std::string ConvertToString(long long msg)
{
    return std::to_string(msg);
}

template<>
inline std::string ConvertToString(unsigned long long msg)
{
    return std::to_string(msg);
}

template<>
inline std::string ConvertToString(float msg)
{
    return std::to_string(msg);
}

template<>
inline std::string ConvertToString(double msg)
{
    return std::to_string(msg);
}

template<>
inline std::string ConvertToString(long double msg)
{
    return std::to_string(msg);
}

} // namespace white

#endif
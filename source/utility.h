#ifndef MIGANGBOT_UTILITY_H_
#define MIGANGBOT_UTILITY_H_

#include <ctime>

#include <algorithm>
#include <chrono>
#include <codecvt>
#include <iterator>
#include <random>
#include <regex>
#include <string>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <utf8.h>

namespace white {

inline std::string ReverseUTF8(const std::string &str) {
  std::u16string str_u16 = utf8::utf8to16(str);
  std::reverse(str_u16.begin(), str_u16.end());
  return utf8::utf16to8(str_u16);
}

inline bool IsDigitStr(const std::string_view &str) {
  return std::all_of(str.begin(), str.end(), isdigit);
}

inline bool IsDigitStr(const std::string &str) {
  return std::all_of(str.begin(), str.end(), isdigit);
}

// https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
inline bool Replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

inline void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

// https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container
template <typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g) {
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(g));
  return start;
}

template <typename Iter>
Iter select_randomly(Iter start, Iter end) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return select_randomly(start, end, gen);
}

inline int GetRandomNum(int min, int max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return std::uniform_int_distribution<>(min, max)(gen);
}

namespace datetime {

inline std::string GetCurrentTime() {
  std::time_t result = std::time(nullptr);
  std::tm *tm = localtime(&result);
  return fmt::format("{:%Y-%m-%d %H:%M:%S}", *tm);
}

inline std::time_t GetTimeStampMs() {
  auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now());
  return tp.time_since_epoch().count();
}

inline std::time_t GetTimeStampS() {
  auto tp = std::chrono::time_point_cast<std::chrono::seconds>(
      std::chrono::high_resolution_clock::now());
  return tp.time_since_epoch().count();
}

inline std::time_t LastSecondOfToday() {
  std::time_t result = std::time(nullptr);
  std::tm *tm = localtime(&result);
  tm->tm_hour = 23;
  tm->tm_min = 59;
  tm->tm_sec = 59;
  return std::mktime(tm);
}

inline std::time_t GetCurrentLocalTimeStamp() {
  std::time_t result = std::time(nullptr);
  return std::mktime(localtime(&result));
}

}  // namespace datetime

namespace html {

inline std::string encode(const std::string &data) {
  std::string buffer;
  buffer.reserve(data.size());
  for (size_t pos = 0; pos != data.size(); ++pos) {
    switch (data[pos]) {
      case '&':
        buffer.append("&amp;");
        break;
      case '\"':
        buffer.append("&quot;");
        break;
      case '\'':
        buffer.append("&apos;");
        break;
      case '<':
        buffer.append("&lt;");
        break;
      case '>':
        buffer.append("&gt;");
        break;
      default:
        buffer.append(&data[pos], 1);
        break;
    }
  }
  return buffer;
}

inline std::string unreliable_decode(const std::string &data) {
  std::string ret;
  int n = data.size();
  char ch;
  for(std::size_t i = 0; i < n; ++i) {
    ch = data[i];
    if(ch != '&') {
      ret.push_back(ch);
      continue;
    }
    if(i == n - 1)
      return "";
    auto end = data.find_first_of(';', i);
    if(end == std::string::npos)
      return "";
    switch(data[i + 1]){
      case 'a':
        if(end - i == 4)
          ret.push_back('&');
        else
          ret.push_back('\'');
        break;
      case 'q':
        ret.push_back('\"');
        break;
      case 'l':
        ret.push_back('<');
        break;
      case 'g':
        ret.push_back('>');
    }
    i = end;
  }
  return ret;
}

}  // namespace html

}  // namespace white

#endif
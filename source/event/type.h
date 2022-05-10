#ifndef MIGANGBOT_EVENT_TYPE_H_
#define MIGANGBOT_EVENT_TYPE_H_

#include "bot/onebot_11/api_bot.h"
#include "event/event.h"

namespace white {

using plugin_func = std::function<void(const Event &, onebot11::ApiBot &)>;

constexpr auto FULLMATCH = 0;
constexpr auto PREFIX = 1;
constexpr auto SUFFIX = 2;
constexpr auto KEYWORD = 3;
constexpr auto ALLMSG = 4;

}  // namespace white

#endif
#ifndef MIGANGBOT_EVENT_TYPE_H_
#define MIGANGBOT_EVENT_TYPE_H_

#include "event/event.h"

namespace white {
  
constexpr auto FULLMATCH = 0;
constexpr auto PREFIX = 1;
constexpr auto SUFFIX = 2;
constexpr auto KEYWORD = 3;
constexpr auto ALLMSG = 4;

}  // namespace white

#endif
#ifndef MIGANGBOT_EVENT_SEARCH_RESULT_H_
#define MIGANGBOT_EVENT_SEARCH_RESULT_H_

#include <string>

#include "bot/onebot_11/api_bot.h"
#include "event/type.h"

namespace white {

struct SearchResult {
  const plugin_func &func;
  const short command_size;
};

}  // namespace white

#endif
#ifndef MIGANGBOT_EVENT_SEARCH_RESULT_H_
#define MIGANGBOT_EVENT_SEARCH_RESULT_H_

#include <string>

#include "event/type.h"
#include "bot/onebot_11/api_bot.h"

namespace white {

struct SearchResult
{
  const plugin_func &func;
  const short command_size;
};

} // namespace hite

#endif
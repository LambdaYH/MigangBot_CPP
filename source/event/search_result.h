#ifndef MIGANGBOT_EVENT_SEARCH_RESULT_H_
#define MIGANGBOT_EVENT_SEARCH_RESULT_H_

#include <string>
#include <memory>
#include <type_traits>

#include "bot/onebot_11/api_bot.h"
#include "event/type.h"
#include "service/triggered_service.h"

namespace white {

struct SearchResult
{
  const std::shared_ptr<TriggeredService> &service;
  const short command_size;
};

}  // namespace white

#endif
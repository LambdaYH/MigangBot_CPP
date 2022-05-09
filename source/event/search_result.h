#ifndef MIGANGBOT_EVENT_SEARCH_RESULT_H_
#define MIGANGBOT_EVENT_SEARCH_RESULT_H_

#include <string>

#include "bot/onebot_11/api_bot.h"
#include "event/type.h"
#include "service/service.h"

namespace white {

struct SearchResult {
  const std::shared_ptr<Service> &service;
  const short command_size;
};

}  // namespace white

#endif
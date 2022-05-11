#ifndef MIGANGBOT_EVENT_EVENT_FILTER_H_
#define MIGANGBOT_EVENT_EVENT_FILTER_H_

#include <nlohmann/json.hpp>

#include "event/event.h"

namespace white {

class EventFilter {
 public:
  const bool Filter(const Event &event) const noexcept {
    if (event.contains("post_type"))
      if (event.value("message_type", "") == "private" &&
          event.value("sub_type", "") == "group")
        return false;
    if (event.contains("retcode")) return false;
    return true;
  }
};

}  // namespace white

#endif
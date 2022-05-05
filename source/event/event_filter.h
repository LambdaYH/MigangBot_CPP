#ifndef MIGANGBOT_EVENT_EVENT_FILTER_H_
#define MIGANGBOT_EVENT_EVENT_FILTER_H_

#include <nlohmann/json.hpp>
#include "event/event.h"

namespace white
{

class EventFilter
{
public:
    virtual const bool operator()(const Event &msg) const = 0;
};

} // namespace white

#endif
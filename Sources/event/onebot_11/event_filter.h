#ifndef MIGANGBOTCPP_EVENT_ONEBOT_11_EVENT_FILTER_H_
#define MIGANGBOTCPP_EVENT_ONEBOT_11_EVENT_FILTER_H_

#include "event/event_filter.h"
#include "event/event.h"

namespace white
{
namespace onebot11
{

class EventFilterOnebot11 : public EventFilter
{
public:
    virtual const bool operator()(const Event &msg) const
    {
        // 过滤临时会话
        if(msg.contains("post_type"))
            if(msg.value("message_type", "") == "private" && msg.value("sub_type", "") == "group")
                return false;
        if(msg.contains("retcode"))
            return false;
        return true;
    }
};

} // namespace onebot11
} // namespace white

#endif
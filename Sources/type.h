#ifndef MIGANGBOTCPP_TYPE_H_
#define MIGANGBOTCPP_TYPE_H_

#include <stdint.h>
#include <string>

namespace white
{

using MsgId = int32_t;
using QId   = uint64_t;
using GId   = uint64_t;

struct MessageID
{
    MsgId message_id;
};

struct GroupInfo
{
    GId group_id;
    std::string group_name;
    int member_count;
    int max_member_count;
};
    
} // namespace white

#endif

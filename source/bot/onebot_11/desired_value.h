#ifndef MIGANGBOT_BOT_ONEBOT_11_DESIRED_VALUE_H_
#define MIGANGBOT_BOT_ONEBOT_11_DESIRED_VALUE_H_

#include <string>

#include "type.h"

namespace white {
namespace onebot11 {
template <typename T>
inline T DesiredValue(const Json &value) {
  if constexpr (std::is_same<T, MessageID>::value) {
    return {value["message_id"].get<MsgId>()};
  } else if constexpr (std::is_same<T, GroupInfo>::value) {
    return {
        value["group_id"].get<GId>(), value["group_name"].get<std::string>(),
        value["member_count"].get<int>(), value["max_member_count"].get<int>()};
  } else if constexpr (std::is_same<T, UserInfo>::value) {
    return {value["user_id"].get<QId>(), value["nickname"].get<std::string>(),
            value["sex"].get<std::string>(), value["age"].get<int32_t>()};
  } else if constexpr (std::is_same<T, std::vector<GroupInfo>>::value) {
    std::vector<GroupInfo> v;
    for (std::size_t i = 0; i < value.size(); ++i)
      v.push_back({value[i]["group_id"].get<GId>(),
                   value[i]["group_name"].get<std::string>(),
                   value[i]["member_count"].get<int>(),
                   value[i]["max_member_count"].get<int>()});
    return v;
  } else if constexpr (std::is_same<T, GroupMemberInfo>::value)
    return {value["group_id"].get<GId>(),
            value["user_id"].get<QId>(),
            value["nickname"].get<std::string>(),
            value["card"].get<std::string>(),
            value["sex"].get<std::string>(),
            value["age"].get<int>(),
            value["area"].get<std::string>(),
            value["join_time"].get<int>(),
            value["last_sent_time"].get<int>(),
            value["level"].get<std::string>(),
            value["role"].get<std::string>(),
            value["unfriendly"].get<bool>(),
            value["title"].get<std::string>(),
            value["title_expire_time"].get<int>(),
            value["card_changeable"].get<bool>()};
}
}  // namespace onebot11
}  // namespace white

#endif
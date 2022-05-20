#ifndef MIGANGBOT_BOT_ONEBOT_11_DEFAULT_VALUE_H_
#define MIGANGBOT_BOT_ONEBOT_11_DEFAULT_VALUE_H_

#include <string>

#include "type.h"

namespace white {
namespace onebot11 {
template <typename T>
inline T DefaultValue() {
  if constexpr (std::is_same<T, MessageID>::value)
    return {0};
  else if constexpr (std::is_same<T, std::string>::value)
    return "";
  else if constexpr (std::is_same<T, GroupInfo>::value)
    return {0, "", 0, 0};
  else if constexpr (std::is_same<T, UserInfo>::value)
    return {0, "", "", 0};
  else if constexpr (std::is_same<T, std::vector<GroupInfo>>::value)
    return {};
  else if constexpr (std::is_same<T, GroupMemberInfo>::value)
    return GroupMemberInfo{};
}
}  // namespace onebot11
}  // namespace white

#endif
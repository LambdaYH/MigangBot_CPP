#ifndef MIGANGBOT_TYPE_H_
#define MIGANGBOT_TYPE_H_

#include <stdint.h>

#include <nlohmann/json.hpp>

#include <string>

namespace white {

using MsgId = int32_t;
using QId = uint64_t;
using GId = uint64_t;
using Json = nlohmann::json;

struct MessageID {
  MsgId message_id;
};

struct GroupInfo {
  GId group_id;
  std::string group_name;
  int member_count;
  int max_member_count;
};

struct UserInfo {
  QId user_id;
  std::string nickname;
  std::string sex;
  int32_t age;
};

struct GroupMemberInfo {
  GId group_id;
  QId user_id;
  std::string nickname;
  std::string card;
  std::string sex;
  int age;
  std::string area;
  int join_time;
  int last_sent_time;
  std::string level;
  std::string role;
  bool unfriendly;
  std::string title;
  int title_expire_time;
  bool card_changeable;
};

}  // namespace white

#endif

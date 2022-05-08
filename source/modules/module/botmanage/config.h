#ifndef MIGANGBOT_MODULES_MODULE_CONFIG_H_
#define MIGANGBOT_MODULES_MODULE_CONFIG_H_

#include <string>

#include "type.h"

namespace white
{
namespace module
{

namespace botmanage
{

extern bool add_friend_flag;
extern bool add_group_flag;
extern bool reject_group_force_flag;
extern std::string help_msg_group_;
extern std::string help_msg_friend_;

constexpr auto kConfigExample =     
                                    "是否同意好友邀请: false\n"
                                    "是否同意加群邀请: false\n"
                                    "是否自动退出被迫自动入的群: false\n"
                                    "拒绝理由: 抱歉，目前暂停入群了\n"
                                    "自动退群理由: 抱歉，因近日遭到冻结/目前群数量过多，目前处于关闭自动入群(严格模式)状态，随后将自动退群，敬请谅解！";

inline bool IsBlackUser(QId uid)
{
    return false;
}

inline bool IsBlackGroup(GId gid)
{
    return false;
}

} // namespace botmanage
} // namespace white
} // namespace module

#endif
#ifndef MIGANGBOTCPP_PERMISSION_PERMISSION_H_
#define MIGANGBOTCPP_PERMISSION_PERMISSION_H_

#include "event/event.h"
#include "global_config.h"
#include "type.h"

namespace white
{
namespace permission
{

constexpr auto BLACK = -999;
constexpr auto NORMAL = 0;
constexpr auto PRIVATE = 1;
constexpr auto GROUP_MEMBER = 2;
constexpr auto GROUP_ADMIN = 3;
constexpr auto GROUP_OWNER = 4;
constexpr auto WHITE_LIST = 5;
constexpr auto SUPERUSER = 999;

int GetUserPermission(const Event &event)
{
    QId user_id = event["user_id"].get<QId>();
    if(config::SUPERUSERS.count(user_id))
        return SUPERUSER;
    if(config::WHITE_LIST.count(user_id))
        return WHITE_LIST;
    // group
    if(event["message_type"].get<std::string>()[0] == 'g')
    {
        if(!event.contains("anonymous") || event["anonymous"].is_null())
        {
            std::string role;
            if(event.contains("sender") && event["sender"].contains("role"))
            {
                auto role = event["sender"]["role"].get<std::string>();
                switch(role[0])
                {
                    case 'm':
                        return GROUP_MEMBER;
                        break;
                    case 'a':
                        return GROUP_ADMIN;
                        break;
                    case 'o':
                        return GROUP_OWNER;
                        break;
                    default:
                        return GROUP_MEMBER;
                }
            }else
                return GROUP_MEMBER;
        }else
            return NORMAL;
    }else
        return PRIVATE;
    return NORMAL;
}

} // namespace permission
} // namespace white

#endif
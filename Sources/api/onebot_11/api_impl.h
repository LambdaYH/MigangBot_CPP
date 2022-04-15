#ifndef MIGANGBOTCPP_API_ONTBOT_11_API_IMPL_H_
#define MIGANGBOTCPP_API_ONTBOT_11_API_IMPL_H_

#include "event/event.h"
#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <string>

namespace white
{
namespace onebot11
{
using Json = nlohmann::json;

inline void ErrorHanding(boost::beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

class ApiImpl : std::enable_shared_from_this<ApiImpl>
{
public:
    ApiImpl();
    ~ApiImpl();

public:
    Json handle_quick_operation(const Event &event, const std::string &message, bool at_sender, bool auto_escape = false) const;

    Json send_private_msg(const Event &event, const uint64_t user_id, const std::string &message, bool auto_escape = false) const;

    Json send_group_msg(const Event &event, const uint64_t group_id, const std::string &message, bool auto_escape = false) const;

    Json send_msg(const Event &event, const std::string &message, bool auto_escape = false) const;

    Json delete_msg(int32_t msg_id) const;

    Json get_msg(const Event &event, const std::string &message, bool auto_escape = false) const;

    Json get_forward_msg(const Event &event, const std::string &message, bool auto_escape = false) const;

    Json send_like(const Event &event, const std::string &message, bool auto_escape = false) const;

    Json set_group_kick(const Event &event, const std::string &message, bool auto_escape = false) const;

    Json set_group_ban(const Event &event, const std::string &message, bool auto_escape = false) const;

    Json set_group_anonymous_ban(const Event &event, const std::string &message, bool auto_escape = false) const;

    Json set_group_whole_ban(const Event &event, const std::string &message, bool auto_escape = false) const;

    Json set_group_admin(const Event &event, const std::string &message, bool auto_escape = false) const;

    Json set_group_anonymous(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json set_group_card(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json set_group_name(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json set_group_leave(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json set_group_special_title(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json set_friend_add_request(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json set_group_add_request(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_login_info(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_stranger_info(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_friend_list(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_group_info(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_group_list(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_group_member_info(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_group_member_list(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_group_honor_info(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_cookies(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_csrf_token(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_credentials(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_record(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_image(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json can_send_image(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json can_send_record(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_status(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json get_version_info(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json set_restart(const Event &event, const std::string &message, bool auto_escape =  false) const;

    Json clean_cache(const Event &event, const std::string &message, bool auto_escape =  false) const;

};

inline ApiImpl::ApiImpl()
{

}

inline ApiImpl::~ApiImpl()
{

}

inline Json ApiImpl::handle_quick_operation(const Event &event, const std::string &message, bool at_sender, bool auto_escape) const
{
    Event msg;
    msg["action"] = ".handle_quick_operation";
    msg["params"]["context"] = {
        {"post_type", "message"},
        {"user_id"     , event["user_id"]},
        {"sender", event["sender"]},
        {"message_type", event["message_type"]}
        };  
    if(event["message_type"] == "group")
        msg["params"]["context"]["group_id"] = event["group_id"];
    msg["params"]["operation"]["reply"] = message;
    msg["params"]["operation"]["auto_escape"] = auto_escape;
    if(at_sender)
        msg["params"]["operation"]["at_sender"] = "true";
    return msg;
}

inline Json ApiImpl::send_private_msg(const Event &event, const uint64_t user_id, const std::string &message, bool auto_escape) const
{
    Event msg = {
    {"action", "send_private_msg_async"},
    {"params", {
        {"user_id"     , user_id},
        {"message"     , message},
        {"auto_escape" , auto_escape},
        {"message_type", "private"}
    }}
    };
    return msg;
}

inline Json ApiImpl::send_group_msg(const Event &event, const uint64_t group_id, const std::string &message, bool auto_escape) const
{
    Event msg = {
    {"action", "send_group_msg_async"},
    {"params", {
        {"group_id"    , group_id},
        {"message"     , message},
        {"auto_escape" , auto_escape},
        {"message_type", "group"}
    }}
    };
    return msg;
}

inline Json ApiImpl::send_msg(const Event &event, const std::string &message, bool auto_escape) const
{
    Event msg = {
    {"action", "send_msg_async"},
    {"params", {
        {"user_id"     , event["user_id"]},
        {"message"     , message},
        {"auto_escape" , auto_escape},
        {"message_type", event["message_type"]}
    }},
    };
    if(event["message_type"] == "group")
        msg["params"]["group_id"] = event["group_id"];
    return msg;
}

inline Json ApiImpl::delete_msg(int32_t msg_id) const
{
    Event msg = {
    {"action", "delete_msg_async"},
    {"params", 
        {"message_id", msg_id}
    }
    };
    return msg;
}

inline Json ApiImpl::get_msg(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_forward_msg(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::send_like(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_kick(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_ban(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_anonymous_ban(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_whole_ban(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_admin(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_anonymous(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_card(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_name(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_leave(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_special_title(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_friend_add_request(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_group_add_request(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_login_info(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_stranger_info(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_friend_list(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_group_info(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_group_list(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_group_member_info(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_group_member_list(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_group_honor_info(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_cookies(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_csrf_token(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_credentials(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_record(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_image(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::can_send_image(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::can_send_record(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_status(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::get_version_info(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::set_restart(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

inline Json ApiImpl::clean_cache(const Event &event, const std::string &message, bool auto_escape) const
{
    
}

} // namespace onebot11
} // namespace white

#endif
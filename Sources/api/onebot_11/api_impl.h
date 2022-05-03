#ifndef MIGANGBOTCPP_API_ONTBOT_11_API_IMPL_H_
#define MIGANGBOTCPP_API_ONTBOT_11_API_IMPL_H_

#include "event/event.h"
#include "type.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

namespace white
{
namespace onebot11
{
using Json = nlohmann::json;
using Event = Json;

class ApiImpl : std::enable_shared_from_this<ApiImpl>
{
public:
    ApiImpl();
    ~ApiImpl();

public:
    // onebot
    Json handle_quick_operation_message_reply(const Event &event, const std::string &message, bool at_sender, bool auto_escape = false) const;

    Json handle_quick_operation_message_delete(const Event &event, const MsgId message_id) const;

    Json handle_quick_operation_message_kick(const Event &event, const MsgId message_id, bool reject_add_request) const;

    Json handle_quick_operation_message_ban(const Event &event, const uint32_t ban_duration) const;

    Json handle_quick_operation_request_friend(const Event &event, const std::string &flag, bool approve) const;

    Json handle_quick_operation_request_group(const Event &event, const std::string &sub_type, const std::string &reason, bool approve) const;

    Json send_private_msg(const QId user_id, const std::string &message, bool auto_escape = false) const;

    Json send_group_msg(const QId group_id, const std::string &message, bool auto_escape = false) const;

    template<bool is_group>
    Json send_msg(const QId qid, const GId gid, const std::string &message, bool auto_escape = true) const;

    Json delete_msg(MsgId msg_id) const;

    Json get_msg(MsgId msg_id) const;

    Json get_forward_msg(const std::string &id) const;

    Json send_like(const QId qid, const uint8_t time) const;

    Json set_group_kick(const GId gid, const QId, const std::string &message, bool reject_add_request) const;

    Json set_group_ban(const GId gid, const QId qid, const uint32_t duration = 1800) const;

    Json set_group_anonymous_ban(const GId gid, int32_t duration, const std::string &anonymous_flag) const;

    Json set_group_whole_ban(const GId gid, bool enable = true) const;

    Json set_group_admin(const GId gid, const QId qid, bool enable = true) const;

    Json set_group_anonymous(const GId gid, bool enable = true) const;

    Json set_group_card(const GId gid, const QId qid, const std::string &card) const;

    Json set_group_name(const GId gid, const std::string &name) const;

    Json set_group_leave(const GId gid) const;

    Json set_group_special_title(const GId gid, const QId qid, const std::string &special_title) const;

    Json set_friend_add_request(const std::string &flag, bool approve = true) const;

    Json set_group_add_request(const std::string &flag, const std::string &sub_type, const std::string &reason, bool approve = true) const;

    Json get_login_info() const;

    Json get_stranger_info(const QId qid) const;

    Json get_friend_list() const;

    Json get_group_info(const GId gid, bool no_cache) const;

    Json get_group_list(bool no_cache) const;

    Json get_group_member_info(const GId gid, const QId qid, bool no_cache) const;

    Json get_group_member_list(const GId gid, bool no_cache) const;

    Json get_group_honor_info(const GId gid, const std::string &type) const;

    Json get_cookies(const std::string &domain) const;

    Json get_csrf_token() const;

    Json get_credentials(const std::string &domain) const;

    Json get_record(const std::string &file, const std::string &out_format) const;

    Json get_image(const std::string &out_format) const;

    Json can_send_image() const;

    Json can_send_record() const;

    Json get_status() const;

    Json get_version_info() const;

    Json set_restart(int delay) const;

    Json clean_cache() const;

    // go-cqhttp
    Json get_word_slices(const std::string &content) const;

    Json ocr_image(const std::string &image) const;

    Json get_model_show(const std::string &model) const;

    Json get_vip_info(const QId qid) const;

    Json send_group_notice(const GId gid, const std::string &content, const std::string &image) const;

    Json set_model_show(const std::string &model, const std::string &model_show) const;

    Json check_url_safely(const std::string &url) const;

};

inline ApiImpl::ApiImpl()
{

}

inline ApiImpl::~ApiImpl()
{

}

inline Json ApiImpl::handle_quick_operation_message_reply(const Event &event, const std::string &message, bool at_sender, bool auto_escape) const
{
    Event msg;
    msg["action"] = ".handle_quick_operation";
    msg["params"]["context"] = {
        {"anonymous"        , event.value("anonymous", nullptr)},
        {"post_type"        , "message"},
        {"user_id"          , event.value("user_id", 0)},
        {"group_id"         , event.value("group_id", 0)},
        {"message_type"     , event.value("message_type", "private")}
    }; 
    if(event.contains("sender"))
        msg["params"]["context"]["sender"] = event["sender"];
    msg["params"]["operation"] = {
        {"reply"            , message},
        {"auto_escape"      , auto_escape},
        {"at_sender"        , at_sender}
    };
    return msg;
}

inline Json ApiImpl::handle_quick_operation_message_delete(const Event &event, const MsgId message_id) const
{
    Event msg;
    msg["action"] = ".handle_quick_operation";
    msg["params"]["context"] = {
        {"anonymous"        , event.value("anonymous", nullptr)},
        {"post_type"        , "message"},
    }; 
    msg["params"]["operation"] = {
        {"delete"           , true},
        {"message_id"       , message_id}
    };
    return msg;
}

inline Json ApiImpl::handle_quick_operation_message_kick(const Event &event, const MsgId message_id, bool reject_add_request) const
{
    Event msg;
    msg["action"] = ".handle_quick_operation";
    msg["params"]["context"] = {
        {"anonymous"                , event.value("anonymous", nullptr)},
        {"post_type"                , "message"},
        {"user_id"                  , event.value("user_id", 0)},
        {"group_id"                 , event.value("group_id", 0)},
    }; 
    msg["params"]["operation"] = {
        {"kick"                     , true},
        {"reject_add_request"       , reject_add_request}
    };
    return msg;
}

inline Json ApiImpl::handle_quick_operation_message_ban(const Event &event, const uint32_t ban_duration) const
{
    Event msg;
    msg["action"] = ".handle_quick_operation";
    msg["params"]["context"] = {
        {"anonymous"                , event.value("anonymous", nullptr)},
        {"post_type"                , "message"},
        {"user_id"                  , event.value("user_id", 0)},
        {"group_id"                 , event.value("group_id", 0)},
    }; 
    msg["params"]["operation"] = {
        {"ban"                      , true},
        {"ban_duration"             , ban_duration}
    };
    return msg;
}

inline Json ApiImpl::handle_quick_operation_request_friend(const Event &event, const std::string &flag, bool approve) const
{
    Event msg;
    msg["action"] = ".handle_quick_operation";
    msg["params"]["context"] = {
        {"anonymous"        , event.value("anonymous", nullptr)},
        {"post_type"        , "request"},
        {"request_type"     , "friend"},
        {"flag"             , flag}
    }; 
    msg["params"]["operation"] = {
        {"approve"          , approve}
    };
    return msg;
}

inline Json ApiImpl::handle_quick_operation_request_group(const Event &event, const std::string &sub_type, const std::string &reason, bool approve) const
{
    Event msg;
    msg["action"] = ".handle_quick_operation";
    msg["params"]["context"] = {
        {"anonymous"        , event.value("anonymous", nullptr)},
        {"post_type"        , "request"},
        {"request_type"     , "group"},
        {"sub_type"         , sub_type}
    }; 
    msg["params"]["operation"] = {
        {"reason"           , reason},
        {"approve"          , approve}
    };
    return msg;
}

inline Json ApiImpl::send_private_msg(const QId user_id, const std::string &message, bool auto_escape) const
{
    Event msg;
    msg["action"] = "send_private_msg";
    msg["params"] = {
        {"user_id"      , user_id},
        {"message"      , message},
        {"auto_escape"  , auto_escape},
    };
    return msg;
}

inline Json ApiImpl::send_group_msg(const QId group_id, const std::string &message, bool auto_escape) const
{
    Event msg;
    msg["action"] = "send_group_msg";
    msg["params"] = {
        {"group_id"     , group_id},
        {"message"      , message},
        {"auto_escape"  , auto_escape},
    };
    return msg;
}

template<bool is_group>
inline Json ApiImpl::send_msg(const QId qid, const GId gid, const std::string &message, bool auto_escape) const
{
    Event msg;
    msg["action"] = "send_msg";
    msg["params"] = {
        {"user_id"      , qid},
        {"group_id"     , gid},
        {"message"      , message},
        {"auto_escape"  , auto_escape}
    };
    if constexpr (is_group)
        msg["params"]["message_type"] = "group";
    else
        msg["params"]["message_type"] = "private";
    return msg;
}

inline Json ApiImpl::delete_msg(MsgId msg_id) const
{
    Event msg;
    msg["action"] = "delete_msg";
    msg["params"] = {
        {"message_id"       , msg_id},
    };
    return msg;
}

inline Json ApiImpl::get_msg(MsgId msg_id) const
{
    Event msg;
    msg["action"] = "get_msg";
    msg["params"] = {
        {"message_id"       , msg_id},
    };
    return msg;
}

inline Json ApiImpl::get_forward_msg(const std::string &id) const
{
    Event msg;
    msg["action"] = "get_forward_msg";
    msg["params"] = {
        {"id"               , id},
    };
    return msg;
}

inline Json ApiImpl::send_like(const QId qid, const uint8_t time) const
{
    Event msg;
    msg["action"] = "send_like";
    msg["params"] = {
        {"user_id"           , qid},
        {"time"              , time}
    };
    return msg;
}

inline Json ApiImpl::set_group_kick(const GId gid, const QId qid, const std::string &message, bool reject_add_request) const
{
    Event msg;
    msg["action"] = "set_group_kick";
    msg["params"] = {
        {"group_id"             , gid},
        {"user_id"              , qid},
        {"message"              , message},
        {"reject_add_request"   , reject_add_request}
    };
    return msg;
}

inline Json ApiImpl::set_group_ban(const GId gid, const QId qid, const uint32_t duration) const
{
    Event msg;
    msg["action"] = "set_group_ban";
    msg["params"] = {
        {"group_id"             , gid},
        {"user_id"              , qid},
        {"duration"             , duration}
    };
    return msg;
}

inline Json ApiImpl::set_group_anonymous_ban(const GId gid, int32_t duration, const std::string &anonymous_flag) const
{
    Event msg;
    msg["action"] = "set_group_anonymous_ban";
    msg["params"] = {
        {"group_id"             , gid},
        {"anonymous_flag"       , anonymous_flag},
        {"duration"             , duration}
    };
    return msg;
}

inline Json ApiImpl::set_group_whole_ban(const GId gid, bool enable) const
{
    Event msg;
    msg["action"] = "set_group_whole_ban";
    msg["params"] = {
        {"group_id"             , gid},
        {"enable"               , enable}
    };
    return msg;
}

inline Json ApiImpl::set_group_admin(const GId gid, const QId qid, bool enable) const
{
    Event msg;
    msg["action"] = "set_group_admin";
    msg["params"] = {
        {"group_id"             , gid},
        {"user_id"              , qid},
        {"enable"               , enable}
    };
    return msg;
}

inline Json ApiImpl::set_group_anonymous(const GId gid, bool enable) const
{
    Event msg;
    msg["action"] = "set_group_anonymous";
    msg["params"] = {
        {"group_id"             , gid},
        {"enable"               , enable}
    };
    return msg;
}

inline Json ApiImpl::set_group_card(const GId gid, const QId qid, const std::string &card) const
{
    Event msg;
    msg["action"] = "set_group_card";
    msg["params"] = {
        {"group_id"             , gid},
        {"user_id"              , qid},
        {"card"                 , card}
    };
    return msg;
}

inline Json ApiImpl::set_group_name(const GId gid, const std::string &name) const
{
    Event msg;
    msg["action"] = "set_group_name";
    msg["params"] = {
        {"group_id"             , gid},
        {"name"                 , name}
    };
    return msg;
}

inline Json ApiImpl::set_group_leave(const GId gid) const
{
    Event msg;
    msg["action"] = "set_group_leave";
    msg["params"] = {
        {"group_id"             , gid}
    };
    return msg;
}

inline Json ApiImpl::set_group_special_title(const GId gid, const QId qid, const std::string &special_title) const
{
    Event msg;
    msg["action"] = "set_group_special_title";
    msg["params"] = {
        {"group_id"             , gid},
        {"user_id"              , qid},
        {"special_title"        , special_title}
    };
    return msg;   
}

inline Json ApiImpl::set_friend_add_request(const std::string &flag, bool approve) const
{
    Event msg;
    msg["action"] = "set_friend_add_request";
    msg["params"] = {
        {"flag"                 , flag},
        {"approve"              , approve}
    };
    return msg;    
}

inline Json ApiImpl::set_group_add_request(const std::string &flag, const std::string &sub_type, const std::string &reason, bool approve) const
{
    Event msg;
    msg["action"] = "set_group_add_request";
    msg["params"] = {
        {"flag"                 , flag},
        {"sub_type"             , sub_type},
        {"reason"               , reason},
        {"approve"              , approve}
    };
    return msg;  
}

inline Json ApiImpl::get_login_info() const
{
    Event msg;
    msg["action"] = "get_login_info";
    return msg;
}

inline Json ApiImpl::get_stranger_info(const QId qid) const
{
    Event msg;
    msg["action"] = "get_stranger_info";
    msg["params"] = {
        {"user_id"                , qid}
    };
    return msg;
}

inline Json ApiImpl::get_friend_list() const
{
    Event msg;
    msg["action"] = "get_friend_list";
    return msg;
}

inline Json ApiImpl::get_group_info(const GId gid, bool no_cache) const
{
    Event msg;
    msg["action"] = "get_group_info";
    msg["params"] = {
        {"group_id"               , gid},
        {"no_cache"               , no_cache},
    };
    return msg;   
}

inline Json ApiImpl::get_group_list(bool no_cache) const
{
    Event msg;
    msg["action"] = "get_group_list";
    msg["params"] = {
        {"no_cache"               , no_cache}
    };
    return msg;       
}

inline Json ApiImpl::get_group_member_info(const GId gid, const QId qid, bool no_cache) const
{
    Event msg;
    msg["action"] = "get_group_member_info";
    msg["params"] = {
        {"group_id"                 , gid},
        {"user_id"                  , qid},
        {"no_cache"                 , no_cache}
    };
    return msg; 
}

inline Json ApiImpl::get_group_member_list(const GId gid, bool no_cache) const
{
    Event msg;
    msg["action"] = "get_group_member_list";
    msg["params"] = {
        {"group_id"                 , gid},
        {"no_cache"                 , no_cache}
    };
    return msg;     
}

inline Json ApiImpl::get_group_honor_info(const GId gid, const std::string &type) const
{
    Event msg;
    msg["action"] = "get_group_honor_info";
    msg["params"] = {
        {"group_id"                 , gid},
        {"type"                     , type}
    };
    return msg;     
}

inline Json ApiImpl::get_cookies(const std::string &domain) const
{
    Event msg;
    msg["action"] = "get_cookies";
    msg["params"] = {
        {"domain"                 , domain},
    };
    return msg;  
}

inline Json ApiImpl::get_csrf_token() const
{
    Event msg;
    msg["action"] = "get_csrf_token";
    return msg;      
}

inline Json ApiImpl::get_credentials(const std::string &domain) const
{
    Event msg;
    msg["action"] = "get_credentials";
    msg["params"] = {
        {"domain"                 , domain},
    };
    return msg;      
}

inline Json ApiImpl::get_record(const std::string &file, const std::string &out_format) const
{
    Event msg;
    msg["action"] = "get_record";
    msg["params"] = {
        {"file"                 , file},
        {"out_format"           , out_format}
    };
    return msg;  
}

inline Json ApiImpl::get_image(const std::string &file) const
{
    Event msg;
    msg["action"] = "get_image";
    msg["params"] = {
        {"file"                 , file}
    };
    return msg;      
}

inline Json ApiImpl::can_send_image() const
{
    Event msg;
    msg["action"] = "can_send_image";
    return msg;        
}

inline Json ApiImpl::can_send_record() const
{
    Event msg;
    msg["action"] = "can_send_record";
    return msg;       
}

inline Json ApiImpl::get_status() const
{
    Event msg;
    msg["action"] = "get_status";
    return msg;       
}

inline Json ApiImpl::get_version_info() const
{
    Event msg;
    msg["action"] = "get_version_info";
    return msg;       
}

inline Json ApiImpl::set_restart(int delay) const
{
    Event msg;
    msg["action"] = "set_restart";
    msg["params"] = {
        {"delay"                 , delay}
    };
    return msg;     
}

inline Json ApiImpl::clean_cache() const
{
    Event msg;
    msg["action"] = "clean_cache";
    return msg;        
}

// go-cqhttp
inline Json ApiImpl::get_word_slices(const std::string &content) const 
{
    Event msg;
    msg["action"] = ".get_word_slices";
    msg["params"] = {
        {"content"    , content}
    };
    return msg;    
}

inline Json ApiImpl::ocr_image(const std::string &image) const
{
    Event msg;
    msg["action"] = ".ocr_image";
    msg["params"] = {
        {"image"    , image}
    };
    return msg;   
}

inline Json ApiImpl::get_model_show(const std::string &model) const
{
    Event msg;
    msg["action"] = "_get_model_show";
    msg["params"] = {
        {"model"    , model}
    };
    return msg;   
}

inline Json ApiImpl::get_vip_info(const QId qid) const
{
    Event msg;
    msg["action"] = "_get_vip_info";
    msg["params"] = {
        {"user_id"    , qid}
    };
    return msg;   
}

inline Json ApiImpl::send_group_notice(const GId gid, const std::string &content, const std::string &image) const
{
    Event msg;
    msg["action"] = "_send_group_notice";
    msg["params"] = {
        {"group_id"     , gid},
        {"content"      , content},
        {"image"        , image}
    };
    return msg;   
}

inline Json ApiImpl::set_model_show(const std::string &model, const std::string &model_show) const
{
    Event msg;
    msg["action"] = "_set_model_show";
    msg["params"] = {
        {"model"                , model},
        {"model_show"           , model_show}
    };
    return msg;   
}

inline Json ApiImpl::check_url_safely(const std::string &url) const
{
    Event msg;
    msg["action"] = "check_url_safely";
    msg["params"] = {
        {"url"                , url}
    };
    return msg;   
}

} // namespace onebot11
} // namespace white

#endif
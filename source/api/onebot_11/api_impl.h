#ifndef MIGANGBOT_API_ONTBOT_11_API_IMPL_H_
#define MIGANGBOT_API_ONTBOT_11_API_IMPL_H_

#include <string>
#include <iostream>

#include <nlohmann/json.hpp>

#include "type.h"

namespace white
{
namespace onebot11
{

using Event = Json;

namespace api_impl
{

inline Json CallApi(const std::string &action, const Json &params)
{
    return Json{
        {"action", action},
        {"params", params}
    };
}

inline Json handle_quick_operation_message_reply(const Event &event, const std::string &message, bool at_sender, bool auto_escape) 
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

inline Json handle_quick_operation_message_delete(const Event &event,  MsgId message_id) 
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

inline Json handle_quick_operation_message_kick(const Event &event,  MsgId message_id, bool reject_add_request) 
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

inline Json handle_quick_operation_message_ban(const Event &event,  uint32_t ban_duration) 
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

inline Json handle_quick_operation_request_friend(const Event &event, bool approve) 
{
    Event msg;
    msg["action"] = ".handle_quick_operation";
    msg["params"]["context"] = {
        {"anonymous"        , event.value("anonymous", nullptr)},
        {"post_type"        , "request"},
        {"request_type"     , "friend"},
        {"flag"             , event["flag"].get<std::string>()}
    }; 
    msg["params"]["operation"] = {
        {"approve"          , approve}
    };
    return msg;
}

inline Json handle_quick_operation_request_group(const Event &event, const std::string &sub_type, const std::string &reason, bool approve) 
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

inline Json send_private_msg( QId user_id, const std::string &message, bool auto_escape) 
{
    return CallApi("send_private_msg", Json{
                                            {"user_id"      , user_id},
                                            {"message"      , message},
                                            {"auto_escape"  , auto_escape},
    });
}

inline Json send_group_msg( QId group_id, const std::string &message, bool auto_escape) 
{
    return CallApi("send_group_msg", Json{
                                    {"group_id"     , group_id},
                                    {"message"      , message},
                                    {"auto_escape"  , auto_escape},
    });
}

template<bool is_group>
inline Json send_msg(const QId qid, const GId gid, const std::string &message, bool auto_escape) 
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

inline Json delete_msg(const MsgId msg_id) 
{
    Event msg;
    msg["action"] = "delete_msg";
    msg["params"] = {
        {"message_id"       , msg_id},
    };
    return msg;
}

inline Json get_msg(const MsgId msg_id) 
{
    Event msg;
    msg["action"] = "get_msg";
    msg["params"] = {
        {"message_id"       , msg_id},
    };
    return msg;
}

inline Json get_forward_msg(const std::string &id) 
{
    Event msg;
    msg["action"] = "get_forward_msg";
    msg["params"] = {
        {"id"               , id},
    };
    return msg;
}

inline Json send_like(const QId qid,  uint8_t time) 
{
    Event msg;
    msg["action"] = "send_like";
    msg["params"] = {
        {"user_id"           , qid},
        {"time"              , time}
    };
    return msg;
}

inline Json set_group_kick(const GId gid, const QId qid, const std::string &message, bool reject_add_request) 
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

inline Json set_group_ban(const GId gid, const QId qid,  uint32_t duration) 
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

inline Json set_group_anonymous_ban(const GId gid, int32_t duration, const std::string &anonymous_flag) 
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

inline Json set_group_whole_ban(const GId gid, bool enable) 
{
    Event msg;
    msg["action"] = "set_group_whole_ban";
    msg["params"] = {
        {"group_id"             , gid},
        {"enable"               , enable}
    };
    return msg;
}

inline Json set_group_admin(const GId gid, const QId qid, bool enable) 
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

inline Json set_group_anonymous(const GId gid, bool enable) 
{
    Event msg;
    msg["action"] = "set_group_anonymous";
    msg["params"] = {
        {"group_id"             , gid},
        {"enable"               , enable}
    };
    return msg;
}

inline Json set_group_card(const GId gid, const QId qid, const std::string &card) 
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

inline Json set_group_name(const GId gid, const std::string &name) 
{
    Event msg;
    msg["action"] = "set_group_name";
    msg["params"] = {
        {"group_id"             , gid},
        {"name"                 , name}
    };
    return msg;
}

inline Json set_group_leave(const GId gid, bool is_dismiss = false) 
{
    return CallApi("set_group_leave", Json{
        {"group_id"  , gid},
        {"is_dismiss", is_dismiss}
    });
}

inline Json set_group_special_title(const GId gid, const QId qid, const std::string &special_title) 
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

inline Json set_friend_add_request(const std::string &flag, bool approve) 
{
    return CallApi("set_friend_add_request", Json{
                                            {"flag"                 , flag},
                                            {"approve"              , approve}
    }); 
}

inline Json set_group_add_request(const std::string &flag, const std::string &sub_type, const std::string &reason, bool approve) 
{
    return CallApi("set_group_add_request", Json{
                                        {"flag"                 , flag},
                                        {"sub_type"             , sub_type},
                                        {"reason"               , reason},
                                        {"approve"              , approve}
    });
}

inline Json get_login_info() 
{
    Event msg;
    msg["action"] = "get_login_info";
    return msg;
}

inline Json get_stranger_info(QId qid) 
{
    return CallApi("get_stranger_info", Json{
                                    {"user_id", qid}
    });
}

inline Json get_friend_list() 
{
    Event msg;
    msg["action"] = "get_friend_list";
    return msg;
}

inline Json get_group_info(GId gid, bool no_cache) 
{
    return CallApi("get_group_info", Json{
        {"group_id"               , gid},
        {"no_cache"               , no_cache},
    });
}

inline Json get_group_list(bool no_cache) 
{
    Event msg;
    msg["action"] = "get_group_list";
    msg["params"] = {
        {"no_cache"               , no_cache}
    };
    return msg;       
}

inline Json get_group_member_info(const GId gid, const QId qid, bool no_cache) 
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

inline Json get_group_member_list(const GId gid, bool no_cache) 
{
    Event msg;
    msg["action"] = "get_group_member_list";
    msg["params"] = {
        {"group_id"                 , gid},
        {"no_cache"                 , no_cache}
    };
    return msg;     
}

inline Json get_group_honor_info(const GId gid, const std::string &type) 
{
    Event msg;
    msg["action"] = "get_group_honor_info";
    msg["params"] = {
        {"group_id"                 , gid},
        {"type"                     , type}
    };
    return msg;     
}

inline Json get_cookies(const std::string &domain) 
{
    Event msg;
    msg["action"] = "get_cookies";
    msg["params"] = {
        {"domain"                 , domain},
    };
    return msg;  
}

inline Json get_csrf_token() 
{
    Event msg;
    msg["action"] = "get_csrf_token";
    return msg;      
}

inline Json get_credentials(const std::string &domain) 
{
    Event msg;
    msg["action"] = "get_credentials";
    msg["params"] = {
        {"domain"                 , domain},
    };
    return msg;      
}

inline Json get_record(const std::string &file, const std::string &out_format) 
{
    Event msg;
    msg["action"] = "get_record";
    msg["params"] = {
        {"file"                 , file},
        {"out_format"           , out_format}
    };
    return msg;  
}

inline Json get_image(const std::string &file) 
{
    Event msg;
    msg["action"] = "get_image";
    msg["params"] = {
        {"file"                 , file}
    };
    return msg;      
}

inline Json can_send_image() 
{
    Event msg;
    msg["action"] = "can_send_image";
    return msg;        
}

inline Json can_send_record() 
{
    Event msg;
    msg["action"] = "can_send_record";
    return msg;       
}

inline Json get_status() 
{
    Event msg;
    msg["action"] = "get_status";
    return msg;       
}

inline Json get_version_info() 
{
    Event msg;
    msg["action"] = "get_version_info";
    return msg;       
}

inline Json set_restart(int delay) 
{
    Event msg;
    msg["action"] = "set_restart";
    msg["params"] = {
        {"delay"                 , delay}
    };
    return msg;     
}

inline Json clean_cache() 
{
    Event msg;
    msg["action"] = "clean_cache";
    return msg;        
}

// go-cqhttp
inline Json get_word_slices(const std::string &content)  
{
    Event msg;
    msg["action"] = ".get_word_slices";
    msg["params"] = {
        {"content"    , content}
    };
    return msg;    
}

inline Json ocr_image(const std::string &image) 
{
    Event msg;
    msg["action"] = ".ocr_image";
    msg["params"] = {
        {"image"    , image}
    };
    return msg;   
}

inline Json get_model_show(const std::string &model) 
{
    Event msg;
    msg["action"] = "_get_model_show";
    msg["params"] = {
        {"model"    , model}
    };
    return msg;   
}

inline Json get_vip_info(const QId qid) 
{
    Event msg;
    msg["action"] = "_get_vip_info";
    msg["params"] = {
        {"user_id"    , qid}
    };
    return msg;   
}

inline Json send_group_notice(const GId gid, const std::string &content, const std::string &image) 
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

inline Json set_model_show(const std::string &model, const std::string &model_show) 
{
    Event msg;
    msg["action"] = "_set_model_show";
    msg["params"] = {
        {"model"                , model},
        {"model_show"           , model_show}
    };
    return msg;   
}

inline Json check_url_safely(const std::string &url) 
{
    Event msg;
    msg["action"] = "check_url_safely";
    msg["params"] = {
        {"url"                , url}
    };
    return msg;   
}

} // namespace api_impl
} // namespace onebot11
} // namespace white

#endif
#ifndef MIGANGBOT_API_ONTBOT_11_API_IMPL_H_
#define MIGANGBOT_API_ONTBOT_11_API_IMPL_H_

#include <iostream>
#include <string>

#include <nlohmann/json.hpp>
#include <utility>

#include "type.h"

namespace white {
namespace onebot11 {

using Event = Json;

namespace api_impl {

template <typename Action>
inline Json CallApi(Action &&action, Json &&params) {
  return Json{{"action", std::forward<Action>(action)},
              {"params", std::move(params)}};
}

template <typename Str>
inline Json send_private_msg(QId user_id, Str &&message, bool auto_escape) {
  return CallApi("send_private_msg",
                 Json{
                     {"user_id", user_id},
                     {"message", std::forward<Str>(message)},
                     {"auto_escape", auto_escape},
                 });
}

template <typename Str>
inline Json send_group_msg(QId group_id, Str &&message, bool auto_escape) {
  return CallApi("send_group_msg", Json{
                                       {"group_id", group_id},
                                       {"message", std::forward<Str>(message)},
                                       {"auto_escape", auto_escape},
                                   });
}

template <typename Type, typename Str, typename ID>
inline Json send_msg(Type &&type, Str &&message, ID &&id,
                     bool auto_escape = false) {
  if (type == "group")
    return CallApi("send_msg", Json{{"message_type", std::forward<Type>(type)},
                                    {"group_id", std::forward<ID>(id)},
                                    {"message", std::forward<Str>(message)},
                                    {"auto_escape", auto_escape}});
  return CallApi("send_msg", Json{{"message_type", std::forward<Type>(type)},
                                  {"user_id", std::forward<ID>(id)},
                                  {"message", std::forward<Str>(message)},
                                  {"auto_escape", auto_escape}});
}

inline Json delete_msg(const MsgId msg_id) {
  return CallApi("delete_msg", Json{{"message_id", msg_id}});
}

inline Json get_msg(const MsgId msg_id) {
  return CallApi("get_msg", Json{
                                {"message_id", msg_id},
                            });
}

template <typename Str>
inline Json get_forward_msg(Str &&id) {
  return CallApi("get_forward_msg", Json{{"id", std::forward<Str>(id)}});
}

inline Json send_like(const QId qid, uint8_t time) {
  return CallApi("send_like", Json{{"user_id", qid}, {"time", time}});
}

template <typename Str>
inline Json set_group_kick(const GId gid, const QId uid, Str &message,
                           bool reject_add_request) {
  return CallApi("set_group_kick",
                 Json{{"group_id", gid},
                      {"user_id", uid},
                      {"message", std::forward<Str>(message)},
                      {"reject_add_request", reject_add_request}});
}

inline Json set_group_ban(const GId gid, const QId qid, uint32_t duration) {
  return CallApi(
      "set_group_ban",
      Json{{"group_id", gid}, {"user_id", qid}, {"duration", duration}});
}

template <typename Str>
inline Json set_group_anonymous_ban(const GId gid, int32_t duration,
                                    Str &&anonymous_flag) {
  return CallApi("set_group_anonymous_ban",
                 Json{{"group_id", gid},
                      {"anonymous_flag", std::forward<Str>(anonymous_flag)},
                      {"duration", duration}});
}

inline Json set_group_whole_ban(const GId gid, bool enable) {
  return CallApi("set_group_whole_ban",
                 Json{{"group_id", gid}, {"enable", enable}});
}

inline Json set_group_admin(const GId gid, const QId qid, bool enable) {
  return CallApi("set_group_admin",
                 Json{{"group_id", gid}, {"user_id", qid}, {"enable", enable}});
}

inline Json set_group_anonymous(const GId gid, bool enable) {
  return CallApi("set_group_anonymous",
                 Json{{"group_id", gid}, {"enable", enable}});
}

template <typename Str>
inline Json set_group_card(const GId gid, const QId qid, Str &&card) {
  return CallApi("set_group_card", Json{{"group_id", gid},
                                        {"user_id", qid},
                                        {"card", std::forward<Str>(card)}});
}

template <typename Str>
inline Json set_group_name(const GId gid, Str &&name) {
  return CallApi("set_group_name",
                 Json{{"group_id", gid}, {"name", std::forward<Str>(name)}});
}

inline Json set_group_leave(const GId gid, bool is_dismiss = false) {
  return CallApi("set_group_leave",
                 Json{{"group_id", gid}, {"is_dismiss", is_dismiss}});
}

template <typename Str>
inline Json set_group_special_title(const GId gid, const QId qid,
                                    Str &&special_title) {
  return CallApi("set_group_special_title",
                 Json{{"group_id", gid},
                      {"user_id", qid},
                      {"special_title", std::forward<Str>(special_title)}});
}

template <typename Str>
inline Json set_friend_add_request(Str &&flag, bool approve) {
  return CallApi("set_friend_add_request",
                 Json{{"flag", std::forward<Str>(flag)}, {"approve", approve}});
}

template <typename Str, typename SubType, typename Reason>
inline Json set_group_add_request(Str &&flag, SubType &&sub_type,
                                  Reason &&reason, bool approve) {
  return CallApi("set_group_add_request",
                 Json{{"flag", std::forward<Str>(flag)},
                      {"sub_type", std::forward<SubType>(sub_type)},
                      {"reason", std::forward<Reason>(reason)},
                      {"approve", approve}});
}

inline Json get_login_info() { return CallApi("get_login_info", Json{}); }

inline Json get_stranger_info(QId qid) {
  return CallApi("get_stranger_info", Json{{"user_id", qid}});
}

inline Json get_friend_list() { return CallApi("get_friend_list", Json{}); }

inline Json get_group_info(GId gid, bool no_cache) {
  return CallApi("get_group_info", Json{
                                       {"group_id", gid},
                                       {"no_cache", no_cache},
                                   });
}

inline Json get_group_list(bool no_cache) {
  return CallApi("get_group_list", Json{{"no_cache", no_cache}});
}

inline Json get_group_member_info(const GId gid, const QId uid, bool no_cache) {
  return CallApi(
      "get_group_member_info",
      Json{{"group_id", gid}, {"user_id", uid}, {"no_cache", no_cache}});
}

inline Json get_group_member_list(const GId gid, bool no_cache) {
  return CallApi("get_group_member_list",
                 Json{{"group_id", gid}, {"no_cache", no_cache}});
}

template <typename Str>
inline Json get_group_honor_info(const GId gid, Str &&type) {
  return CallApi("get_group_honor_info",
                 Json{{"group_id", gid}, {"type", std::forward<Str>(type)}});
}

template <typename Str>
inline Json get_cookies(Str &&domain) {
  return CallApi("get_cookies", Json{
                                    {"domain", std::forward<Str>(domain)},
                                });
}

inline Json get_csrf_token() { return CallApi("get_csrf_token", Json{}); }

template <typename Str>
inline Json get_credentials(Str &&domain) {
  return CallApi("get_credentials", Json{
                                        {"domain", std::forward<Str>(domain)},
                                    });
}

template <typename File, typename Str>
inline Json get_record(File &&file, Str &&out_format) {
  return CallApi("get_record",
                 Json{{"file", std::forward<File>(file)},
                      {"out_format", std::forward<Str>(out_format)}});
}

template <typename Str>
inline Json get_image(Str &&file) {
  return CallApi("get_image", Json{{"file", std::forward<Str>(file)}});
}

inline Json can_send_image() { return CallApi("can_send_image", Json{}); }

inline Json can_send_record() { return CallApi("can_send_record", Json{}); }

inline Json get_status() { return CallApi("get_status", Json{}); }

inline Json get_version_info() { return CallApi("get_version_info", Json{}); }

inline Json set_restart(int delay) {
  return CallApi("set_restart", Json{{"delay", delay}});
}

inline Json clean_cache() { return CallApi("clean_cache", Json{}); }

// go-cqhttp
template <typename Str>
inline Json get_word_slices(Str &&content) {
  return CallApi(".get_word_slices",
                 Json{{"content", std::forward<Str>(content)}});
}

template <typename Str>
inline Json ocr_image(Str &&image) {
  return CallApi(".ocr_image", Json{{"image", std::forward<Str>(image)}});
}

template <typename Str>
inline Json get_model_show(Str &&model) {
  return CallApi("_get_model_show", Json{{"model", std::forward<Str>(model)}});
}

inline Json get_vip_info(const QId qid) {
  return CallApi("_get_vip_info", {{"user_id", qid}});
}

template <typename Str, typename ImgStr>
inline Json send_group_notice(const GId gid, Str &&content, ImgStr &&image) {
  return CallApi("_send_group_notice",
                 Json{{"group_id", gid},
                      {"content", std::forward<Str>(content)},
                      {"image", std::forward<ImgStr>(image)}});
}

template <typename ModelStr, typename ShowStr>
inline Json set_model_show(ModelStr &&model, ShowStr &&model_show) {
  return CallApi("_set_model_show",
                 {{"model", std::forward<ModelStr>(model)},
                  {"model_show", std::forward<ShowStr>(model_show)}});
}

template <typename Str>
inline Json check_url_safely(Str &&url) {
  return CallApi("check_url_safely", Json{{"url", std::forward<Str>(url)}});
}

}  // namespace api_impl
}  // namespace onebot11
}  // namespace white

#endif
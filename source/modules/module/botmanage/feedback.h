#pragma once

#include "modules/module_interface.h"

#include <cctype>

#include <unordered_set>

#include "global_config.h"
#include "message/message_segment.h"
#include "message/utility.h"
#include "modules/module/botmanage/feedback_recorder.h"
#include "permission/permission.h"
#include "utility.h"

namespace white {
namespace module {

class FeedBack : public Module {
 public:
  FeedBack() : Module() {}
  virtual void Register() {
    OnPrefix({".send", "。send"}, make_pair("feedback_send", "机器人管理"),
             "发送留言给维护者，格式[.send message]",
             ACT_InClass(FeedBack::Send), permission::NORMAL,
             permission::SUPERUSER);
    OnPrefix({".reply", "。reply"},
             make_pair("__feedback_reply__", "机器人管理"),
             "对特定ID的留言予以回复，格式[.reply id message]",
             ACT_InClass(FeedBack::Reply), permission::SUPERUSER,
             permission::SUPERUSER);
    OnPrefix({".feedbacklist", "。feedbacklist"},
             make_pair("__feedback_feedbacklist__", "机器人管理"),
             "显示出特定ID的留言记录，格式[.feedbacklist id]",
             ACT_InClass(FeedBack::List), permission::SUPERUSER,
             permission::SUPERUSER);
  }

 private:
  void Send(const Event &event, onebot11::ApiBot &bot);
  void Reply(const Event &event, onebot11::ApiBot &bot);
  void List(const Event &event, onebot11::ApiBot &bot);

 private:
  FeedbackRecorder record_;
};

inline void FeedBack::Send(const Event &event, onebot11::ApiBot &bot) {
  std::string nickname;
  if (event.contains("sender") && event["sender"].contains("nickname"))
    nickname = event["sender"]["nickname"].get<std::string>();
  else
    nickname = "#未知昵称#";
  QId user_id = event["user_id"].get<QId>();
  auto text = message::Strip(message::ExtraPlainText(event));
  if (text.empty()) {
    bot.send(event, fmt::format("格式错误，请发送[.send + 您的留言]"), true);
    return;
  }
  auto time_now = datetime::GetCurrentTime();
  auto whites = config::SUPERUSERS;
  if (event.contains("group_id")) {
    auto group_id = event["group_id"].get<GId>();
    auto group_info = bot.get_group_info(group_id).get();
    std::string group_name;
    if (group_info.group_id == 0)
      group_name = "#未知群名#";
    else
      group_name = group_info.group_name;
    record_.RecordFeedBack(time_now, user_id, group_id, text);
    auto feedback_id = record_.GetLastID();
    for (auto white : whites)
      bot.send_private_msg(white,
                           fmt::format("留言ID[{}]|{}|@(Q){}({})@(群){}({})\n"
                                       "====================\n"
                                       "{}",
                                       feedback_id, time_now, nickname, user_id,
                                       group_name, group_id, text));
    bot.send(event,
             fmt::format("您ID为[{}]的留言已发送至维护者\n"
                         "====================\n"
                         "{}",
                         feedback_id, text),
             true);
  } else {
    record_.RecordFeedBack(time_now, user_id, 0, text);
    auto feedback_id = record_.GetLastID();
    for (auto white : whites)
      bot.send_private_msg(
          white, fmt::format("留言ID[{}]|{}|@(Q){}({})\n"
                             "====================\n"
                             "{}",
                             feedback_id, time_now, nickname, user_id, text));
    bot.send(event, fmt::format("您ID为[{}]的留言已发送至维护者\n"
                                "====================\n"
                                "{}",
                                feedback_id, text));
  }
}

inline void FeedBack::Reply(const Event &event, onebot11::ApiBot &bot) {
  auto msg = message::Strip(message::ExtraPlainText(event));
  auto split_msg = message::Split(msg, " ");
  if (split_msg.size() < 2) {
    bot.send(event, "参数有误，请按照[.reply id text]格式重新发送", true);
    return;
  }
  auto feedback_id = message::Strip(split_msg[0]);
  if (!IsDigitStr(feedback_id)) {
    bot.send(event, "输入的留言ID有误,请输入数字", true);
    return;
  }
  auto search_ret = record_.GetFeedback(std::stoll(feedback_id));
  if (search_ret.empty()) {
    bot.send(event,
             fmt::format("不存在ID[{}]的留言,请输入1-{}之间的数字", feedback_id,
                         record_.GetLastID()),
             true);
    return;
  }
  auto reply_text = message::Strip(msg.substr(split_msg[0].size()));
  auto msg_re = fmt::format(
      "关于留言ID[{}]的回复\n"
      "========原文========\n"
      "{}\n"
      "========回复========\n"
      "{}",
      feedback_id, search_ret[1], reply_text);
  if (search_ret[3] != "0")
    bot.send_group_msg(std::stoull(search_ret[3]), msg_re);
  else
    bot.send_private_msg(std::stoull(search_ret[2]), msg_re);
  bot.send(event, fmt::format("留言ID[{}]的回复已发送成功", feedback_id), true);
}

inline void FeedBack::List(const Event &event, onebot11::ApiBot &bot) {
  auto feedback_id = message::Strip(message::ExtraPlainText(event));
  if (feedback_id.empty()) {
    bot.send(event,
             fmt::format("留言ID不能为空，请输入1-{}之间的数字",
                         record_.GetLastID()),
             true);
    return;
  }
  if (!IsDigitStr(feedback_id)) {
    bot.send(event, "输入的留言ID有误,请输入数字", true);
    return;
  }
  auto search_ret = record_.GetFeedback(std::stoll(feedback_id));
  if (search_ret.empty()) {
    bot.send(event,
             fmt::format("不存在ID[{}]的留言,请输入1-{}之间的数字", feedback_id,
                         record_.GetLastID()),
             true);
    return;
  }
  bot.send(event, fmt::format("{}|留言ID[{}]\n"
                              "====================\n"
                              "{}",
                              search_ret[0], feedback_id, search_ret[1]));
}

}  // namespace module
}  // namespace white
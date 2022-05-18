#pragma once

#include "modules/module_interface.h"

#include <algorithm>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include <hv/httpdef.h>
#include "Document.h"
#include "Node.h"

#include "tools/aiorequests.h"
#include "database/redis_wrapper.h"
#include "event/type.h"
#include "logger/logger.h"
#include "message/message_segment.h"
#include "message/utility.h"
#include "modules/module_interface.h"
#include "permission/permission.h"
#include "type.h"
#include "utility.h"

namespace white {
namespace module {

class BilibiliParser : public Module {
 public:
  BilibiliParser()
      : Module(),
        pattern_(
            R"(http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\(\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+)"),
        aid_pattern_(R"((av|AV)\d+)"),
        bvid_pattern_(R"((BV|bv)([a-zA-Z0-9])+)"),
        video_keywords_({"https://www.bilibili.com/video",
                         "http://www.bilibili.com/video"}),
        bangumi_keywords_({"https://www.bilibili.com/bangumi",
                           "http://www.bilibili.com/bangumi"}),
        live_keywords_(
            {"https://live.bilibili.com", "http://live.bilibili.com"}),
        alias_domain_({"https://m.bilibili.com", "http://m.bilibili.com",
                       "https://bilibili.com", "http://bilibili.com"}),
        header_(
            {{"user-agent",
              "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
              "(KHTML, like Gecko) Chrome/87.0.4280.141 Safari/537.36"}}) {}
  virtual void Register() {
    OnRegex(
        {R"(http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\(\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+)",
         R"((av|AV)\d+)", R"((BV|bv)([a-zA-Z0-9])+)"},
        "哔哩哔哩解析", ACT_InClass(BilibiliParser::Parser), permission::GROUP_MEMBER);
  }

 private:
  void Parser(const Event &event, onebot11::ApiBot &bot);

  std::string ExtractDetails(const std::string &url, const GId group_id);

  std::string GetBilibiliVideoDetail(const std::string &url, const GId group_id);

  std::string GetBilibiliBangumiDetail(const std::string &url,
                                       const GId group_id);

  std::string GetLiveSummary(const std::string &url, const GId group_id);

  Json GetJson(const std::string &url);

 private:
  std::regex pattern_;
  std::regex aid_pattern_;
  std::regex bvid_pattern_;
  std::unordered_set<std::string> video_keywords_;
  std::unordered_set<std::string> bangumi_keywords_;
  std::unordered_set<std::string> live_keywords_;
  std::unordered_set<std::string> alias_domain_;
  http_headers header_;
};

inline bool IsInCache(const std::string &url, const GId group_id) {
  redis::RedisWrapper redis_wrapper;
  if (!redis_wrapper.Execute(fmt::format(
          "GET {}", fmt::format("bnotrepeat:{}:{}", group_id, url))))
    return false;
  if (redis_wrapper.GetReply().type == REDIS_REPLY_NIL) return false;
  return true;
}

constexpr auto kExpiredTime = 15;

// 防止短时间内重复发送
inline bool AddInCacheNotRepeat(const std::string &url, const GId group_id) {
  redis::RedisWrapper redis_wrapper;
  if (!redis_wrapper.Execute(fmt::format(
          "SET {} 0 EX {}", fmt::format("bnotrepeat:{}:{}", group_id, url),
          kExpiredTime)))
    return false;
  return true;
}

// 防止短时间内多次请求
constexpr auto kExpiredTimeCache = 120;

inline bool AddInCache(const std::string &url, const std::string &value) {
  redis::RedisWrapper redis_wrapper;
  if (!redis_wrapper.Execute(fmt::format("SET {} {} EX {}",
                                         fmt::format("bcache:{}", url), value,
                                         kExpiredTimeCache)))
    return false;
  return true;
}

inline std::string GetFromCache(const std::string &url) {
  redis::RedisWrapper redis_wrapper;
  if (!redis_wrapper.Execute(
          fmt::format("GET {}", fmt::format("bcache:{}", url))))
    return "";
  if (redis_wrapper.GetReply().type == REDIS_REPLY_NIL) return "";
  return redis_wrapper.GetReply().str;
}

inline std::string GetRealUrl(const std::string &url) {
  auto r = aiorequests::Get(url, 15);
  if (HTTP_STATUS_IS_REDIRECT(r->status_code)) return r->GetHeader("location");
  return url;
}

inline Json BilibiliParser::GetJson(const std::string &url) {
  auto r = aiorequests::Get(url, 15, header_);
  if (!r) return Json();
  return r->GetJson();
}

inline std::string BilibiliParser::GetBilibiliVideoDetail(const std::string &url,
                                                          const GId group_id) {
  auto cache = GetFromCache(url);
  if (!cache.empty()) {
    AddInCacheNotRepeat(url, group_id);
    return cache;
  }
  std::smatch aid, bvid;
  std::string api_url;
  if (std::regex_search(url, aid, aid_pattern_))
    api_url = fmt::format("https://api.bilibili.com/x/web-interface/view?aid={}",
                      aid[0].str().substr(2));
  else if (std::regex_search(url, bvid, bvid_pattern_))
    api_url = fmt::format("https://api.bilibili.com/x/web-interface/view?bvid={}",
                      bvid[0].str());
  auto details = GetJson(api_url);
  if (!details.contains("code") || details["code"].get<int>() != 0) {
    LOG_WARN("BilibiliParser: 无法解析Video: {}", api_url);
    return "";
  }
  details = details["data"];
  auto title = details["title"].get<std::string>();
  auto description = details["desc"].get<std::string>();
  auto author = details["owner"]["name"].get<std::string>();
  auto img_url = details["pic"].get<std::string>();
  auto link = fmt::format("https://www.bilibili.com/video/{}",
                          details["bvid"].get<std::string>());
  std::smatch part;
  if (std::regex_search(url, part, std::regex(R"(\?p=\d+)")) &&
      part[0].str() != "?p=1") {
    title +=
        "[P" + std::regex_replace(part[0].str(), std::regex(R"(\?p=)"), "") + "]";
    link += part[0].str();
  }
  if (IsInCache(link, group_id)) return "";
  auto msg = fmt::format(
      "[标题] {}\n"
      "[作者] {}\n"
      "[简介] {}\n"
      "[封面] {}"
      "URL:{}",
      title, author,
      message::Strip(description, ' ').empty()
          ? description
          : fmt::format("\n{}", description),
      message_segment::image(img_url), link);
  AddInCache(url, msg);
  AddInCacheNotRepeat(link, group_id);
  AddInCacheNotRepeat(url, group_id);
  return msg;
}

inline std::string BilibiliParser::GetBilibiliBangumiDetail(
    const std::string &url, const GId group_id) {
  auto cache = GetFromCache(url);
  if (!cache.empty()) {
    AddInCacheNotRepeat(url, group_id);
    return cache;
  }
  auto r = aiorequests::Get(url, 15, header_);
  if (!r) {
    LOG_WARN("BilibiliParser: 无法解析Bangumi: {}", url);
    return "";
  }
  // https://www.w3schools.com/jquery/jquery_ref_selectors.asp
  CDocument doc;
  doc.parse(r->Body().c_str());
  // link
  std::smatch ep_ss_id;
  static std::regex ep_ss_pattern(R"((ss|ep)\d+)");
  std::regex_search(url, ep_ss_id, ep_ss_pattern);
  auto link = std::regex_replace(
      doc.find("[property='og:url']").nodeAt(0).attribute("content"),
      ep_ss_pattern, ep_ss_id[0].str());
  if (IsInCache(link, group_id)) return "";

  // title
  auto title = fmt::format("{} [{}-{}]", doc.find("title").nodeAt(0).text(),
                           doc.find(".home-link").nodeAt(0).text(),
                           doc.find(".pub-info").nodeAt(0).text());

  // desciption
  auto description = doc.find(".absolute").nodeAt(0).text();

  // cover image
  auto cover_image =
      doc.find("[property='og:image']").nodeAt(0).attribute("content");

  auto msg = fmt::format(
      "[标题] {}\n"
      "[简介] {}\n"
      "[封面] {}\n"
      "URL:{}\n",
      title,
      message::Strip(description, ' ').empty()
          ? description
          : fmt::format("\n{}", description),
      message_segment::image(cover_image), link);

  AddInCache(url, msg);
  AddInCacheNotRepeat(link, group_id);
  AddInCacheNotRepeat(url, group_id);
  return msg;
}

inline std::string BilibiliParser::GetLiveSummary(const std::string &url,
                                                  const GId group_id) {
  auto cache = GetFromCache(url);
  if (!cache.empty()) {
    AddInCacheNotRepeat(url, group_id);
    return cache;
  }
  static std::regex link_search_pattern(
      R"((https|http)://live.bilibili.com/\d+)");
  std::smatch link_match;
  if (!std::regex_search(url, link_match, link_search_pattern)) {
    LOG_WARN("BilibiliParser: 无法解析Live: {}", url);
    return "";
  }
  auto link = link_match[0].str();
  if (IsInCache(link, group_id)) return "";
  // get room id
  std::regex_search(link, link_match, std::regex(R"(\d+)"));

  auto r = GetJson(
      fmt::format("http://api.live.bilibili.com/room/v1/Room/room_init?id={}",
                  link_match[0].str()));
  if (!r.contains("code") || r["code"].get<int>() != 0) {
    AddInCacheNotRepeat(link, group_id);
    return "↑ 直播间不存在~";
  }
  r = GetJson(fmt::format("http://api.bilibili.com/x/space/acc/info?mid={}",
                          r["data"]["uid"].get<int>()));
  if (!r.contains("code") || r["code"].get<int>() != 0) {
    LOG_WARN("BilibiliParser: 无法获取直播详情: {}", url);
    return "";
  }
  auto status = r["data"]["live_room"]["liveStatus"].get<int>();
  auto msg = fmt::format(
      "{}\n"
      "[标题] {}\n"
      "[主播] {}\n"
      "[封面]{}\n"
      "URL:{}\n",
      status == 1 ? "[直播中]" : "[未开播]",
      r["data"]["live_room"]["title"].get<std::string>(),
      r["data"]["name"].get<std::string>(),
      message_segment::image(
          r["data"]["live_room"]["cover"].get<std::string>()),
      link);
  AddInCache(url, msg);
  AddInCacheNotRepeat(link, group_id);
  AddInCacheNotRepeat(url, group_id);
  return msg;
}

inline std::string BilibiliParser::ExtractDetails(const std::string &url,
                                                  const GId group_id) {
  static auto start_with_what =
      [](const std::string_view &view,
         const std::unordered_set<std::string> &set) -> bool {
    for (const auto &u : set)
      if (view.starts_with(u)) return true;
    return false;
  };
  if (start_with_what(url, video_keywords_)) {
    return GetBilibiliVideoDetail(url, group_id);
  } else if (start_with_what(url, bangumi_keywords_)) {
    return GetBilibiliBangumiDetail(url, group_id);
  } else if (start_with_what(url, live_keywords_)) {
    return GetLiveSummary(url, group_id);
  }
  return "";
}

inline void BilibiliParser::Parser(const Event &event, onebot11::ApiBot &bot) {
  auto msg = std::regex_replace(message::ExtraPlainText(event),
                                std::regex(R"(\\)"), "");
  std::smatch match;
  std::unordered_set<std::string> url_list_set;
  while (std::regex_search(msg, match, pattern_)) {
    url_list_set.insert(match.str());
    msg = match.suffix();
  }
  if (url_list_set.empty()) {
    const static auto search_end = std::sregex_iterator();
    auto aid_begin = std::sregex_iterator(msg.begin(), msg.end(), aid_pattern_);
    auto bvid_begin =
        std::sregex_iterator(msg.begin(), msg.end(), bvid_pattern_);
    for (; aid_begin != search_end; ++aid_begin)
      url_list_set.insert(
          fmt::format("https://www.bilibili.com/video/{}", (*aid_begin).str()));
    for (; bvid_begin != search_end; ++bvid_begin)
      url_list_set.insert(fmt::format("https://www.bilibili.com/video/{}",
                                      (*bvid_begin).str()));
  }
  if (url_list_set.empty()) return;

  static auto checkurl = [&](const std::string_view &view) -> bool {
    if (view.starts_with("https://b23.tv")) return true;
    for (const auto &alias : alias_domain_)
      if (view.starts_with(alias)) return true;
    return false;
  };
  std::vector<std::string> url_list(url_list_set.begin(), url_list_set.end());
  auto group_id = event.contains("group_id") ? event["group_id"].get<GId>() : 0;
  for (auto &url : url_list) {
    if (checkurl(url)) url = GetRealUrl(url);
    if (IsInCache(url, group_id)) continue;
    url = message::RStrip(url);
    LOG_DEBUG("BilibiliParser: 即将开始解析 {}", url);
    auto msg = ExtractDetails(url, group_id);
    if (msg.empty()) continue;
    auto ret = bot.send(event, msg).Ret();
    if (ret.message_id == 0) {
      LOG_WARN("解析消息发送失败");
      bot.send(
          event, "由于风控等原因链接解析结果无法发送(如有误检测请忽略)", true);
    }
  }
}

}  // namespace module
}  // namespace white
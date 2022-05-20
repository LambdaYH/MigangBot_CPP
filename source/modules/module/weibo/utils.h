#pragma once

#include <ctime>
#include <iomanip>

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <co/hash.h>

#include "logger/logger.h"
#include "message/utility.h"
#include "utility.h"
#include "tools/aiorequests.h"
#include "tools/libxml2_white/html_doc.h"

constexpr auto kHttpPrefix = "https";

using Json = nlohmann::json;

namespace white {
namespace module {
namespace weibo {

inline Json GetJson(const std::string &params) {
  auto r =
      aiorequests::Get(fmt::format("{}://m.weibo.cn/api/container/getIndex?{}",
                                   kHttpPrefix, params),
                       15);
  return r->GetJson();
}

inline Json GetWeiboJson(const std::string &user_id) {
  return GetJson(fmt::format("containerid=107603{}&page=1", user_id));
}

inline Json GetUserInfo(const std::string &user_id) {
  auto js = GetJson(fmt::format("containerid=100505{}", user_id));
  if (js["ok"].get<int>() == 1) {
    auto info = js["data"]["userInfo"];
    Json user_info;
    user_info["id"] = user_id;
    user_info["screen_name"] = info.value("screen_name", "");
    return user_info;
  }
  return Json{};
}

inline std::time_t ParseTime(const std::string &expr) {
  std::tm tm{};  // 局部变量不默认初始化为0
  std::stringstream(expr) >> std::get_time(&tm, "%a %b %d %H:%M:%S +0800 %Y");
  return std::mktime(&tm);
}

inline std::string GetText(const std::string &text_body) {
  HtmlDoc doc;
  doc.LoadFromString(text_body);
  for (auto &node :
       doc.GetRootElement().FindChildrenByAttribute("class", "surl-text")) {
    auto url = fastring(node.Parent().GetAtrribute("href"));
    auto value = node.Child().Content();
    if (!std::string_view(value).starts_with('#') &&
        (url.starts_with("https://weibo.cn/sinaurl?u=") ||
         url.starts_with("https://video.weibo.com"))) {
      node.Child().SetContent(fmt::format(
          "{}({})", value,
          url_decode(url.replace("https://weibo.cn/sinaurl?u=", "")).c_str()));
    }
  }
  return html::unreliable_decode(doc.GetAllText());
}

inline Json GetPics(const Json &weibo_info) {
  Json ret;
  if (weibo_info.contains("pics")) {
    auto &pics = weibo_info["pics"];
    for (auto &pic : pics) {
      ret.push_back(pic["large"]["url"].get<std::string>());
    }
  }
  if (weibo_info.contains("page_info") &&
      weibo_info["page_info"]["type"].get<std::string>() == "article")
    ret.push_back(
        weibo_info["page_info"]["page_pic"]["url"].get<std::string>());
  return ret;
}

inline std::string GetVideoInfo(const Json &weibo_info) {
  if (weibo_info.contains("page_info") &&
      weibo_info["page_info"]["type"].get<std::string>() == "video") {
    return weibo_info["page_info"]["page_pic"]["url"].get<std::string>();
  }
  return "";
}

inline Json ParseWeibo(const Json &weibo_info) {
  Json weibo;
  if (weibo_info.contains("user")) {
    // weibo["user_id"] = weibo_info["user"]["id"].get<uint64_t>();
    weibo["screen_name"] = weibo_info["user"]["screen_name"].get<std::string>();
  }
  weibo["id"] = weibo_info["id"].get<std::string>();
  // weibo["bid"] = weibo_info["bid"].get<std::string>();
  auto text_body = weibo_info["text"].get<std::string>();
  ReplaceAll(text_body, "\u200b", "");
  ReplaceAll(text_body, "<br/>", "\n");
  ReplaceAll(text_body, "<br />", "\n");
  ReplaceAll(text_body, "&", "&amp;");
  weibo["text"] = GetText(text_body);
  weibo["pics"] = GetPics(weibo_info);
  weibo["video_poster"] = GetVideoInfo(weibo_info);
  weibo["created_at"] = ParseTime(weibo_info["created_at"].get<std::string>());
  return weibo;
}

inline Json GetLongWeibo(const std::string &id) {
  for (std::size_t i = 0; i < 5; ++i) {
    auto r = aiorequests::Get(
        fmt::format("{}://m.weibo.cn/detail/{}", kHttpPrefix, id), 15);
    if (!r) {
      co::sleep(1000);
      continue;
    }
    auto html = r->Body();
    std::string_view html_view(html);
    auto pos_start = html_view.find("\"status\":");
    auto pos_end = html_view.rfind("},");
    try {
      html_view = html_view.substr(pos_start, pos_end - pos_start + 1);
      html_view.remove_prefix(html_view.find_first_of("{"));
      Json weibo_info = Json::parse(html_view);
      return ParseWeibo(weibo_info);
    } catch (const Json::exception &e) {
      LOG_WARN("weibo: 抓取异常 {}", e.what());
      LOG_DEBUG("抓取异常的网页信息: {}", html);
      co::sleep(1000);
    }
  }
  return Json{};
}

inline Json GetOneWeibo(const Json &info) {
  auto weibo_info = info["mblog"];
  auto weibo_id = weibo_info["id"].get<std::string>();
  auto is_long = weibo_info["isLongText"].get<bool>();
  Json weibo;
  if (is_long) {
    weibo = GetLongWeibo(weibo_id);
    if (weibo.empty()) weibo = ParseWeibo(weibo_info);
  } else
    weibo = ParseWeibo(weibo_info);
  if (weibo_info.contains("retweeted_status") &&
      weibo_info["retweeted_status"].contains("id")) {
    auto &retweet_status = weibo_info["retweeted_status"];
    auto retweet_id = weibo_info["retweeted_status"]["id"].get<std::string>();
    auto is_retweet_long =
        weibo_info["retweeted_status"]["isLongText"].get<bool>();
    Json retweet;
    if (is_retweet_long) {
      retweet = GetLongWeibo(retweet_id);
      if (retweet.empty()) retweet = ParseWeibo(retweet_status);
    } else
      retweet = ParseWeibo(retweet_status);
    weibo["retweet"] = retweet;
  }
  return weibo;
}

}  // namespace weibo
}  // namespace module
}  // namespace white
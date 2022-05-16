#pragma once

#include <ctime>
#include <iomanip>

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <pugixml.hpp>
#include <co/hash.h>

#include "logger/logger.h"
#include "message/utility.h"
#include "utility.h"
#include "aiorequests.h"

constexpr auto kHttpPrefix = "https";

using Json = nlohmann::json;

namespace white {
namespace module {
namespace weibo {

inline Json GetJson(const std::string &params) {
  auto r = aiorequests::Get(fmt::format(
      "{}://m.weibo.cn/api/container/getIndex?{}", kHttpPrefix, params));
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

struct XmlAllText : pugi::xml_tree_walker {
  XmlAllText(std::string &str) : ret(str) {}
  virtual bool for_each(pugi::xml_node &node) {
    ret += node.value();
    return true;
  }

 private:
  std::string &ret;
};

inline std::time_t ParseTime(const std::string &expr) {
  std::tm tm;
  std::stringstream(expr) >> std::get_time(&tm, "%a %b %d %H:%M:%S +0800 %Y");
  return std::mktime(&tm);
}

inline std::string GetText(const std::string &text_body) {
  struct HandleUrl : pugi::xml_tree_walker {
    virtual bool for_each(pugi::xml_node &node) {
      if (!node.attribute("href").empty()) {
        auto url = fastring(node.attribute("href").value());
        auto text_node =
            node.find_child_by_attribute("class", "surl-text").first_child();
        if (!std::string_view(text_node.value()).starts_with('#') &&
            (url.starts_with("https://weibo.cn/sinaurl?u=") ||
             url.starts_with("https://video.weibo.com"))) {
          text_node.set_value(
              fmt::format(
                  "{}({})", text_node.value(),
                  url_decode(url.replace("https://weibo.cn/sinaurl?u=", ""))
                      .c_str())
                  .c_str());
        }
      }
      return true;
    }
  };
  pugi::xml_document doc;
  auto html = html::CleanHTML(text_body);
  doc.load_buffer(html.c_str(), html.size(), pugi::parse_ws_pcdata);
  HandleUrl handle_url;
  doc.child("html").child("body").traverse(handle_url);
  std::string ret;
  XmlAllText walker(ret);
  doc.child("html").child("body").traverse(walker);
  return html::unescape(message::Strip(ret, '\n'));
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
    weibo["user_id"] = weibo_info["user"]["id"].get<uint64_t>();
    weibo["screen_name"] = weibo_info["user"]["screen_name"].get<std::string>();
  }
  weibo["id"] = weibo_info["id"].get<std::string>();
  weibo["bid"] = weibo_info["bid"].get<std::string>();
  auto text_body = weibo_info["text"].get<std::string>();
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
    auto html = r->Body();
    std::string_view html_view(html);
    auto pos_start = html_view.find("\"status\":");
    auto pos_end = html_view.rfind("},");
    html_view = html_view.substr(pos_start, pos_end - pos_start + 1);
    html_view.remove_prefix(html_view.find_first_of("{"));
    try {
      Json weibo_info = Json::parse(html_view);
      return ParseWeibo(weibo_info);
    } catch (const Json::exception &e) {
      LOG_WARN("weibo: 抓取异常 {}", e.what());
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
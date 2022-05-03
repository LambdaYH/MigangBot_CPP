#ifndef MIGANGBOTCPP_MODULES_MODULE_BILIBILI_PARSER_BILIBILI_PARSER_H_
#define MIGANGBOTCPP_MODULES_MODULE_BILIBILI_PARSER_BILIBILI_PARSERU_H_

#include <string_view>
#include <unordered_set>
#include <vector>
#include <regex>
#include <hv/httpdef.h>

#include "event/types.h"
#include "message/utility.h"
#include "modules/module_interface.h"
#include "database/redis_wrapper.h"
#include "aiorequests.h"
#include "permission/permission.h"
#include "spdlog/fmt/bundled/format.h"
#include "utility.h"
#include "message/message_segment.h"

namespace white
{
namespace module
{

class BilibiliParser : public Module
{
public:
    BilibiliParser() : Module(),
        pattern_(R"(http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\(\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+)"),
        aid_pattern_(R"((av|AV)\d+)"),
        bvid_pattern_(R"((BV|bv)([a-zA-Z0-9])+)"),
        video_keywords_({"https://www.bilibili.com/video", "http://www.bilibili.com/video"}),
        bangumi_keywords_({"https://www.bilibili.com/bangumi", "http://www.bilibili.com/bangumi"}),
        live_keywords_({"https://live.bilibili.com", "http://live.bilibili.com"}),
        alias_domain_({"https://m.bilibili.com", "http://m.bilibili.com", "https://bilibili.com", "http://bilibili.com"}),
        header_({{"user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.141 Safari/537.36"}})
    {

    }
    virtual void Register()
    {
        RegisterAllMessage(func(BilibiliParser::Parser), permission::GROUP_MEMBER);
    }

private:
    void Parser(const Event &event, onebot11::ApiBot &bot);

    std::string ExtractDetails(const std::string &url);

    std::string GetBilibiliVideoDetail(std::string &url);

    std::string GetBilibiliBangumiDetail(const std::string &url);

    std::string GetLiveSummary(const std::string &url);

    nlohmann::json GetJson(const std::string &url);
    

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

inline bool IsInCache(const std::string &url)
{
    redis::RedisWrapper redis_wrapper;
    if(!redis_wrapper.Execute(fmt::format("GET {}", url)))
        return false;
    if(redis_wrapper.GetReply().type == REDIS_REPLY_NIL)
        return false;
    return true;
}

constexpr auto kExpiredTime = 15;

// 防止短时间内重复发送
inline bool AddInCacheNotRepeat(const std::string &url)
{
    redis::RedisWrapper redis_wrapper;
    if(!redis_wrapper.Execute(fmt::format("SET {} 0 EX {}", url, kExpiredTime)))
        return false;
    return true;
}

// 防止短时间内多次请求
constexpr auto kExpiredTimeCache = 120;

inline bool AddInCache(const std::string &url, const std::string &value)
{
    redis::RedisWrapper redis_wrapper;
    if(!redis_wrapper.Execute(fmt::format("SET {} {} EX {}", url, value, kExpiredTimeCache)))
        return false;
    return true;
}

inline std::string GetFromCache(const std::string &url)
{
    redis::RedisWrapper redis_wrapper;
    if(!redis_wrapper.Execute(fmt::format("GET {}", url)))
        return "";
    if(redis_wrapper.GetReply().type == REDIS_REPLY_NIL)
        return "";
    return redis_wrapper.GetReply().str;
}

inline std::string GetRealUrl(const std::string &url)
{
    auto r = aiorequests::Get(url, 15);
    if(HTTP_STATUS_IS_REDIRECT(r->status_code))
        return r->GetHeader("location");
    return url;
}

inline nlohmann::json BilibiliParser::GetJson(const std::string &url)
{
    auto r = aiorequests::Get(url, 15, header_);
    if(!r)
        return nlohmann::json();
    return r->GetJson();
}

inline std::string BilibiliParser::GetBilibiliVideoDetail(std::string &url)
{
    auto cache = GetFromCache(url);
    if(!cache.empty())
        return cache;
    static std::smatch aid, bvid;
    if(std::regex_search(url, aid, aid_pattern_))
        url = fmt::format("https://api.bilibili.com/x/web-interface/view?aid={}", aid[0].str().substr(2));
    else if(std::regex_search(url, bvid, bvid_pattern_))
        url = fmt::format("https://api.bilibili.com/x/web-interface/view?bvid={}", bvid[0].str());
    auto details = GetJson(url);
    LOG_DEBUG("BilibiliParser: {}", details.dump());
    if(!details.contains("code") || details["code"].get<int>() != 0)
    {
        LOG_DEBUG("BilibiliParser: {}", details.dump());
        LOG_WARN("BilibiliParser: 无法解析: {}", url);
        return "";
    }
    details = details["data"];
    auto title = details["title"].get<std::string>();
    auto description = details["desc"].get<std::string>();
    auto author = details["owner"]["name"].get<std::string>();
    auto img_url = details["pic"].get<std::string>();
    auto link = fmt::format("https://www.bilibili.com/video/{}", details["bvid"].get<std::string>());
    std::smatch part;
    if(std::regex_search(url, part, std::regex(R"(\?p=\d+)")) && part[0].str() != "?p=1")
    {
        title += "[P" + std::regex_replace(part[0].str() , std::regex("?p="), "") + "]";
        link += part[0].str();
    }
    if(IsInCache(link))
        return "";
    auto msg = fmt::format(
            "[标题] {}\n"
            "[作者] {}\n"
            "[简介] {}\n"
            "[封面] {}"
            "URL:{}",
            title,
            author,
            message::Strip(description, ' ').empty() ? description : ("\n" + description),
            message_segment::image(img_url),
            link
        );
    AddInCache(url, msg);
    AddInCacheNotRepeat(link);
    return msg;
}

inline std::string BilibiliParser::GetBilibiliBangumiDetail(const std::string &url)
{
    auto cache = GetFromCache(url);
    if(!cache.empty())
        return cache;
}

inline std::string BilibiliParser::GetLiveSummary(const std::string &url)
{
    auto cache = GetFromCache(url);
    if(!cache.empty())
        return cache;
}

inline std::string BilibiliParser::ExtractDetails(const std::string &url)
{
    auto r_url = message::RStrip(url);
    static auto start_with_what = [](const std::string_view &view, const std::unordered_set<std::string> &set)->bool
    {
        for(const auto &u : set)
            if(view.starts_with(u))
                return true;
        return false;
    };
    if(start_with_what(r_url, video_keywords_))
    {
        return GetBilibiliVideoDetail(r_url);
    }else if(start_with_what(r_url, bangumi_keywords_))
    {

    }else if(start_with_what(r_url, live_keywords_))
    {

    }
    return "";
}

inline void BilibiliParser::Parser(const Event &event, onebot11::ApiBot &bot)
{
    auto msg = message::ExtraPlainText(event);
    static std::smatch match;
    std::unordered_set<std::string> url_list_set;
    if(!std::regex_search(msg, match, pattern_))
    {
        static std::smatch aid, bvid;
        if(std::regex_search(msg, aid, aid_pattern_))
            for(auto &m : aid)
                url_list_set.insert(fmt::format("https://www.bilibili.com/video/{}", m.str()));
        if(std::regex_search(msg, bvid, bvid_pattern_))
            for(auto &m : bvid)
                url_list_set.insert(fmt::format("https://www.bilibili.com/video/{}", m.str()));
    }else
    {
        for(auto &m : match)
            url_list_set.insert(m.str());
    }
    if(url_list_set.empty())
        return;

    static auto checkurl = [&](const std::string_view &view)->bool{
        if(view.starts_with("https://b23.tv"))
            return true;
        for(const auto &alias : alias_domain_)
            if(view.starts_with(alias))
                return true;
        return false;
    };
    std::vector<std::string> url_list(url_list_set.begin(), url_list_set.end());
    for(auto &url : url_list)
    {
        if(checkurl(url))
            url = GetRealUrl(url);
        auto msg = ExtractDetails(url);
        if(msg.empty())
            continue;
        auto ret = bot.send_msg(event, msg).Ret();
        if(ret.message_id == 0)
        {
            LOG_WARN("解析消息发送失败");
            bot.send_msg(event, "由于风控等原因链接解析结果无法发送(如有误检测请忽略)", true);
        }
    }  
}

} // namespace module
} // namespace white

#endif
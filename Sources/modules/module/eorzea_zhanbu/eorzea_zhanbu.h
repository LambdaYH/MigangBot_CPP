#ifndef MIGANGBOTCPP_MODULES_MODULE_EORZEA_ZHANBU_EORZEA_ZHANBU_H_
#define MIGANGBOTCPP_MODULES_MODULE_EORZEA_ZHANBU_EORZEA_ZHANBU_H_

#include "fmt/core.h"
#include "global_config.h"
#include "modules/module_interface.h"
#include <filesystem>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "aiorequests.h"
#include "modules/module/eorzea_zhanbu/zhanbu_recorder.h"
#include "modules/module/eorzea_zhanbu/zhanbu_utils.h"
#include "utility.h"

namespace white
{
namespace module
{
namespace eorzea_zhanbu
{
    constexpr auto kConfigExample = "职业: [\"白魔\", \"骑士\"]\n\n"
                                    "染料: [\"无瑕白染剂\"]\n\n"
                                    "事件: [\"诸事皆宜\"]\n\n"
                                    "宜:\n"
                                    "  诸事皆宜: [\"萨纳兰今天也是艳阳高照啊\", \"美好的一天，适合挂机呢~\"]\n\n"
                                    "忌:\n"
                                    "  2002: [\"那就90002\"]";

} // namespace eorzea_zhanbu
class EorzeaZhanbu : public Module
{
public:
    EorzeaZhanbu() : Module("zhanbu.yml", eorzea_zhanbu::kConfigExample),
                    exception_msg_({"占卜水晶球滑落了~正在重新捡起(擦擦)", "水晶球化为碎片，正在尝试重组~", "星天开门！"})
    {
        auto config_ = LoadConfig();
        for(std::size_t i = 0; i < config_["职业"].size(); ++i)
            occupations_.push_back(config_["职业"][i].as<std::string>());
        
        for(std::size_t i = 0; i < config_["染料"].size(); ++i)
            dye_.push_back(config_["染料"][i].as<std::string>());
        
        for(std::size_t i = 0; i < config_["事件"].size(); ++i)
            events_.push_back(config_["事件"][i].as<std::string>());
        
        for(auto &event : events_)
        {
            if(config_["宜"][event])
            {
                for(std::size_t i = 0; i < config_["宜"][event].size(); ++i)
                    luck_yi_reply_[event].push_back(config_["宜"][event][i].as<std::string>());
            }
            if(config_["忌"][event])
            {
                for(std::size_t i = 0; i < config_["忌"][event].size(); ++i)
                luck_ji_reply_[event].push_back(config_["忌"][event][i].as<std::string>());
            }
        }

        for(const auto &occupation : occupations_)
        {
            auto dir = config::kAssetsDir / "images" / "zhanbu" / occupation;
            for(const auto &file : std::filesystem::directory_iterator{dir})
            {
                if(std::string_view(file.path().native()).ends_with(".png"))
                    occupation_basemaps_[occupation].push_back(file.path().filename());
            }
        }
    }
    virtual void Register()
    {
        RegisterCommand(PREFIX, {"/zhanbu", "/占卜", "、占卜"}, func(EorzeaZhanbu::Zhanbu), permission::NORMAL);
    }

private:

    void Zhanbu(const Event &event, onebot11::ApiBot &bot);
    
    std::string GetEorzeaZhanbu(const QId uid);

    std::tuple<std::string, std::string, std::string, std::string, std::string, std::string> 
    GetZhanbuResult(const QId uid);

    std::string GetBasemap(const std::string &occupation);

private:
    std::vector<std::string> occupations_;
    std::vector<std::string> dye_;
    std::vector<std::string> events_;
    std::unordered_map<std::string, std::vector<std::string>> luck_yi_reply_;
    std::unordered_map<std::string, std::vector<std::string>> luck_ji_reply_;
    const std::vector<std::string> exception_msg_;

    std::unordered_map<std::string, std::vector<std::string>> occupation_basemaps_;
    ZhanbuRecorder recorder_;
};

inline void EorzeaZhanbu::Zhanbu(const Event &event, onebot11::ApiBot &bot)
{
    auto text = message::Strip(message::ExtraPlainText(event));
    if(!text.empty())
    {
        auto msg_id = bot.send_msg(event, eorzea_zhanbu::GetEventZhanbu(event["user_id"].get<QId>(), text), true).Ret();
        if(msg_id.message_id == 0)
            bot.send_msg(event, *select_randomly(exception_msg_.begin(), exception_msg_.end()), true);
    }else
    {
        auto msg_id = bot.send_msg(event, GetEorzeaZhanbu(event["user_id"].get<QId>()), true).Ret();
        if(msg_id.message_id == 0)
            bot.send_msg(event, *select_randomly(exception_msg_.begin(), exception_msg_.end()), true);
    }
}

inline std::string EorzeaZhanbu::GetEorzeaZhanbu(const QId uid)
{
    auto cur_time = datetime::GetTimeStampS();
    auto record = recorder_.GetZhanbuRecord(uid);
    static auto basemap_base_path = config::kAssetsDir / "images" / "zhanbu";
    if(record.empty() || std::stoll(record.back()) <= cur_time)
    {
        std::string luck, yi, ji, dye, append_msg, occupation;
        std::tie(luck, yi, ji, dye, append_msg, occupation) = GetZhanbuResult(uid);
        auto basemap = fmt::format("{}/{}", occupation, GetBasemap(occupation));
        recorder_.RecordZhanbu(uid, luck, yi, ji, dye, append_msg, basemap, datetime::LastSecondOfToday());
        return eorzea_zhanbu::Draw(luck, yi, ji, dye, append_msg, basemap_base_path / basemap);
    }else {
        return eorzea_zhanbu::Draw(record[1], record[2], record[3], record[4], record[5], basemap_base_path / record[6]);
    }
    return "";
}

inline std::tuple<std::string, std::string, std::string, std::string, std::string, std::string> 
EorzeaZhanbu::GetZhanbuResult(const QId uid)
{
    auto luck = eorzea_zhanbu::GetLuckNum(uid);
    auto luck_occupation = *select_randomly(occupations_.begin(), occupations_.end());
    auto luck_dye = *select_randomly(dye_.begin(), dye_.end());
    auto luck_yi = *select_randomly(events_.begin(), events_.end());
    auto luck_ji = *select_randomly(events_.begin(), events_.end());
    while(luck_yi == luck_ji)
        luck_ji = *select_randomly(events_.begin(), events_.end());
    
    if(luck_yi == "诸事皆宜")
        luck_ji = "无";
    else if(luck_ji == "诸事皆宜")
    {
        luck_ji = "无";
        luck_yi = "诸事皆宜";
    }

    std::string append_msg;
    if(luck == 100)
        append_msg = "是欧皇中的欧皇！";
    else if(luck == 0)
        append_msg = "非极必欧！";
    else if(luck > 94)
        append_msg = "是欧皇哦~";
    else if(luck < 6)
        append_msg = "是非酋呢~";
    else if(luck_occupation == "舞者" && luck >= 50)
        append_msg = fmt::format("最佳舞伴->{}", *select_randomly(occupations_.begin(), occupations_.end()));
    else if(luck_yi_reply_.count(luck_yi) && luck_ji_reply_.count(luck_ji))
    {
        if(GetRandomNum(0, 1) == 0)
            append_msg = *select_randomly(luck_yi_reply_.at(luck_yi).begin(), luck_yi_reply_.at(luck_yi).end());
        else
            append_msg = *select_randomly(luck_ji_reply_.at(luck_ji).begin(), luck_ji_reply_.at(luck_ji).end());
    }else if(luck_yi_reply_.count(luck_yi))
        append_msg = *select_randomly(luck_yi_reply_.at(luck_yi).begin(), luck_yi_reply_.at(luck_yi).end());
    else if(luck_ji_reply_.count(luck_ji))
        append_msg = *select_randomly(luck_ji_reply_.at(luck_ji).begin(), luck_ji_reply_.at(luck_ji).end());
    else
        append_msg = fmt::format("\"{}\"", eorzea_zhanbu::GetHitokoto());

    return {fmt::format("{}%", luck),
            fmt::format("宜 {}", luck_yi),
            fmt::format("忌 {}", luck_ji),
            luck_dye,
            append_msg,
            luck_occupation
        };
}

inline std::string EorzeaZhanbu::GetBasemap(const std::string &occupation)
{
    return *select_randomly(occupation_basemaps_.at(occupation).begin(), occupation_basemaps_.at(occupation).end());
}

} // namespace module
} // namespace white

#endif
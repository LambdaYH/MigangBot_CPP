#ifndef MIGANGBOT_MODULES_MODULE_EORZEA_ZHANBU_ZHANBU_UTILS_H_
#define MIGANGBOT_MODULES_MODULE_EORZEA_ZHANBU_ZHANBU_UTILS_H_

#include <ctime>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <string>
#include <openssl/md5.h>
#include <fmt/chrono.h>
#include <random>
#include <functional>
#include <opencv2/core/mat.hpp>
#include <vector>
#include <utf8.h>

#include "aiorequests.h"
#include "event/event.h"
#include "global_config.h"
#include "utility.h"
#include "message/utility.h"
#include "message/message_segment.h"

#include <logger/logger.h>

namespace white
{
namespace module
{
namespace eorzea_zhanbu
{

inline int GetLuckNum(const QId uid)
{
    auto t = std::time(nullptr);
    auto today_int = std::stoi(fmt::format("{:%Y%m%d}", *std::localtime(&t)));
    auto str_for_md5 = fmt::format("{}",
                        today_int * (uid + GetRandomNum(0, 1000) / 10000.0 - 0.05)
                        );
    std::string md5('\0', MD5_DIGEST_LENGTH);
    MD5((const unsigned char*)str_for_md5.c_str(), str_for_md5.size(), (unsigned char*)md5.data());
    std::mt19937 e(std::hash<std::string>()(md5));
    return std::uniform_int_distribution<int>(0, 100)(e);
}

inline std::string GetEventZhanbu(const QId uid, const std::string &thing)
{
    auto t = std::time(nullptr);
    auto today_int = std::stoi(fmt::format("{:%Y%m%d}", *std::localtime(&t)));
    auto str_for_md5 = fmt::format("{}{}{}", 
                            thing.size() * today_int,
                            today_int * uid,
                            thing
                        );
    std::string md5('\0', MD5_DIGEST_LENGTH);
    MD5((const unsigned char*)str_for_md5.c_str(), str_for_md5.size(), (unsigned char*)md5.data());
    std::mt19937 e(std::hash<std::string>()(md5));
    auto luck_today = std::uniform_int_distribution<unsigned int>(0, 100)(e);
    std::string comment;
    if(luck_today <= 20)
        comment = "大凶";
    else if(luck_today <= 40)
        comment = "凶";
    else if(luck_today <= 60)
        comment = "小吉";
    else if(luck_today <= 80)
        comment = "中吉";
    else
        comment = "大吉";
    return fmt::format(
        "占卜[{}]:\n"
        "    {}({})",
        thing,
        comment,
        luck_today
    );
}

inline std::string GetHitokoto()
{
    auto r = aiorequests::Get("https://v1.hitokoto.cn/?encode=text&max_length=16&c=d&c=e&c=i&c=k", 5);
    if(!r)
        return "......( )";
    return std::regex_replace(r->Body(), std::regex("\""), "\\\"");
}

inline std::vector<std::u16string> Decrement(const std::string &text_u8)
{
    std::u16string text = utf8::utf8to16(text_u8);
    auto length = text.size();
    auto cardinality = 9;
    if(length > 4 * cardinality)
        return {};
    auto number_of_slices = (length % cardinality == 0) ? length / cardinality : length / cardinality + 1;
    
    if(number_of_slices == 2)
    {
        if(length & 1)
        {
            std::u16string fill_space(cardinality - (length + 1) / 2, ' ');
            return {text.substr(0, (length + 1) / 2) + fill_space,
                    fill_space + std::u16string(1, ' ') + text.substr((length + 1) / 2)
                    };
        }else {
            std::u16string fill_space(cardinality - length / 2, ' ');
            return {text.substr(0, length / 2) + fill_space,
                    fill_space + text.substr(length / 2)
                    };
        }
    }
    std::vector<std::u16string> ret;
    for(std::size_t i = 0; i < number_of_slices; ++i)
        ret.push_back(text.substr(i * cardinality, cardinality));
    return ret;
}

inline std::string Draw( 
            const std::string &luck, 
            const std::string &yi, 
            const std::string &ji, 
            const std::string &dye,
            const std::string &append_msg,
            const std::string &basemap
)
{
    static const auto kTitleFont = config::kAssetsDir / "fonts" / "zhanbu" / "Mamelon.otf";
    static const auto kTextFont = config::kAssetsDir / "fonts" / "zhanbu" / "sakura.ttf";
    cv::Mat img = cv::imread(basemap);
    auto freetype2 = cv::freetype::createFreeType2();
    int baseline = 0;
    // draw luck
    int font_height = 45;
    cv::Scalar color(245, 245, 245);
    std::pair image_font_center{140, 89};
    freetype2->loadFontData(kTitleFont, 0);
    auto font_size = freetype2->getTextSize(luck, font_height, 1, &baseline);
    freetype2->putText(img, 
                        luck, 
                        cv::Point(image_font_center.first - font_size.width / 2, image_font_center.second - font_size.height / 2), 
                        font_height, 
                        color, 
                        -1, 
                        CV_AA,
                        false);
    // draw dye
    font_height = 18;
    color = cv::Scalar(50, 50, 50);
    image_font_center = std::make_pair(140, 152);
    freetype2->loadFontData(kTextFont, 0);
    font_size = freetype2->getTextSize(dye, font_height, 1, &baseline);
    freetype2->putText(img, 
                    dye, 
                    cv::Point(image_font_center.first - font_size.width / 2, image_font_center.second - font_size.height / 2), 
                    font_height, 
                    color, 
                    -1, 
                    CV_AA,
                    false);

    // draw yi, ji, append_msg
    font_height = 25;
    std::vector<std::u16string> text_slice;
    for(std::size_t k = 0; k < 3; ++k)
    {
        switch(k)
        {
            case 0:
                image_font_center = std::make_pair(180, 290);
                text_slice = Decrement(yi);
                break;
            case 1:
                image_font_center = std::make_pair(150, 290);
                text_slice = Decrement(ji);
                break;
            case 2:
                image_font_center = std::make_pair(90, 299);
                text_slice = Decrement(append_msg);
        }
        if(text_slice.empty())
            return "";
        auto slice_num = text_slice.size();
        for(std::size_t i = 0; i < slice_num; ++i)
        {
            auto h = text_slice[i].size() * (font_height + 4);
            auto ori_x =    image_font_center.first 
                        + (slice_num - 2) * font_height / 2
                        + (slice_num - 1) * 4
                        - i * (font_height + 4);
            auto ori_y = image_font_center.second - h / 2;
            for(std::size_t j = 0; j < text_slice[i].size(); ++j)
            {
                freetype2->putText(img, 
                                utf8::utf16to8(text_slice[i].substr(j, 1)), 
                                cv::Point(ori_x, ori_y), 
                                font_height, 
                                color, 
                                -1, 
                                CV_AA,
                                false);
                ori_y += font_height + 4;
            }
        }
    }
    return message_segment::image(ImageTobase64(img));
}


} // namespace eorzea_zhanbu
} // namespace module
} // namespace white

#endif
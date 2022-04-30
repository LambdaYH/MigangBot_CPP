#ifndef MIGANGBOTCPP_MESSAGE_UTILITY_H_
#define MIGANGBOTCPP_MESSAGE_UTILITY_H_

#include <cstddef>
#include <opencv2/core.hpp>
#include <string>
#include <string_view>
#include <sstream>
#include <algorithm>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/freetype.hpp>
#include <opencv2/imgcodecs.hpp>
#include <filesystem>
#include <iostream>
#include <regex>
#include <vector>

#include "base64.h"
#include "event/event.h"

namespace white
{
namespace message
{

inline std::string Escape(const std::string &str, bool escape_comma = true)
{
    auto ret = std::regex_replace(str, std::regex("&"), "&amp;");
    ret = std::regex_replace(ret, std::regex("["), "&#91;");
    ret = std::regex_replace(ret, std::regex("]"), "&#93;");
    if(escape_comma)
        ret = std::regex_replace(ret, std::regex(","), "&#44;");
    return ret;
}

inline std::string Unescape(const std::string &str)
{
    auto ret = std::regex_replace(str, std::regex("&#44;"), ",");
    ret = std::regex_replace(ret, std::regex("&#91;"), "[");
    ret = std::regex_replace(ret, std::regex("&#93;"), "]");
    ret = std::regex_replace(ret, std::regex("&amp;"), "&");
    return ret;
}

inline std::vector<std::string> Split(const std::string_view &view, const std::string &delimiter)
{
    std::size_t pos = 0;
    std::vector<std::string> ret;
    for(std::size_t new_pos = view.find(delimiter, pos); new_pos != std::string_view::npos; pos = new_pos + 1, new_pos = view.find(delimiter, pos))
        ret.push_back(std::string(view.substr(pos, new_pos - pos)));
    ret.push_back(std::string(view.substr(pos)));
    return ret;
}

inline std::string ExtraPlainText(const Event &event)
{
    auto msg_raw_str = event["message"].get<std::string>();
    msg_raw_str = Unescape(msg_raw_str);
    std::string_view view{msg_raw_str};
    if(event.contains("__to_me__"))
    {
        view = view.substr(view.find_first_of(']') + 1);
        view = view.substr(view.find_first_not_of(' '));
    }
    if(event.contains("__command_size__"))
    {
        auto command_size = event["__command_size__"].get<short>();
        if(command_size == 0)
            return std::string(view);
        if(command_size > 0)
            view.remove_prefix(command_size);
        else
            view.remove_suffix(-command_size);
    }
    return std::string(view);
}

inline void ExtraPlainText(std::string_view &str)
{
    str = str.substr(std::min(str.find_first_of(' ') + 1, str.size()));
}

inline void LStrip(std::string_view &str, char ch = ' ')
{
    str.remove_prefix(std::min(str.find_first_not_of(ch), str.size()));
}

inline void RStrip(std::string_view &str, char ch = ' ')
{
    str.remove_suffix(std::min(str.size() - str.find_last_not_of(ch), str.size()));
}

inline void Strip(std::string_view &str, char ch = ' ')
{
    str.remove_prefix(std::min(str.find_first_not_of(ch), str.size()));
    str.remove_suffix(std::min(str.size() - str.find_last_not_of(ch) - 1, str.size()));
}

inline std::string LStrip(const std::string &str, char ch = ' ')
{
    auto view = std::string_view(str);
    view.remove_prefix(std::min(view.find_first_not_of(ch), view.size()));
    return std::string(view);
}

inline std::string RStrip(const std::string &str, char ch = ' ')
{
    auto view = std::string_view(str);
    view.remove_suffix(std::min(view.size() - view.find_last_not_of(ch) - 1, view.size()));
    return std::string(view);
}

inline std::string Strip(const std::string &str, char ch = ' ')
{
    auto view = std::string_view(str);
    view.remove_prefix(std::min(view.find_first_not_of(ch), view.size()));
    view.remove_suffix(std::min(view.size() - view.find_last_not_of(ch) - 1, view.size()));
    return std::string(view);
}

} // namespace message

// https://docs.opencv.org/3.4/d9/dfa/classcv_1_1freetype_1_1FreeType2.html#af059d49b806b916ffdd6380b9eb2f59a
inline std::string TextToImg(const std::string &text)
{
    int font_height = 24;
    auto path = std::filesystem::current_path();
    auto font_path = path / "assets" / "fonts" / "SourceHanSansHWSC-Regular.otf";
    int img_height = 0, img_width = 0;
    auto freetype2 = cv::freetype::createFreeType2();
    freetype2->loadFontData(font_path, 0);
    std::string line;
    std::istringstream iss(text);
    std::vector<std::string> lines;
    while(getline(iss, line))
    {
        if(text.empty())
        {
            img_height += font_height;
        }else
        {
            int baseline = 0;
            cv::Size text_size = freetype2->getTextSize(line, font_height, 1, &baseline);
            img_width = std::max(img_width, text_size.width);
        }
        lines.push_back(line);
    }
    int border = 10;
    img_height = lines.size() * font_height;
    int gap = 5;
    auto real_width = img_width + border * 4;
    auto real_height = img_height + border * 4 + gap * (lines.size() - 1);
    cv::Mat img(real_height, real_width, CV_8UC3, cv::Scalar(239, 204, 175)); // B-G-R in opencv
    cv::Mat roi = img(cv::Rect(border, border, real_width - 2 * border, real_height - 2 * border));
    cv::Mat mask(roi.size(), CV_8UC3, cv::Scalar(255, 255, 255));
    double alpha = 0.6;
    // https://stackoverflow.com/questions/24480751/how-to-create-a-semi-transparent-shape
    cv::addWeighted(mask, alpha, roi, 1.0 - alpha, 0.0, roi);
    int start_y = border * 2;
    for(auto &line : lines)
    {
        freetype2->putText(img, 
                            line, 
                            cv::Point{border * 2, start_y}, 
                            font_height, 
                            cv::Scalar(0, 0, 0), 
                            -1, 
                            CV_AA,
                            false
        );
        start_y += font_height + gap;
    }
    // https://stackoverflow.com/questions/29772271/c-opencv-convert-mat-to-base64-and-vice-versa
    std::vector<uchar> buf;
    cv::imencode(".jpg", img, buf);
    auto *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
    std::string encoded = base64_encode(enc_msg, buf.size());
    return "base64://" + encoded;
}

} // namespace white

#endif
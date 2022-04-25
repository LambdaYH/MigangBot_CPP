#ifndef MIGANGBOTCPP_MESSAGE_UTILITY_H_
#define MIGANGBOTCPP_MESSAGE_UTILITY_H_

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

#include "base64.h"

namespace white
{

inline std::string_view ExtraPlainText(const std::string &str)
{
    std::string_view view{str};
    view = view.substr(std::min(view.find_first_of(' ') + 1, view.size()));
    return view;
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

// 需要修复中文
inline void Strip(std::string_view &str, char ch = ' ')
{
    str.remove_prefix(std::min(str.find_first_not_of(ch), str.size()));
    str.remove_suffix(std::min(str.size() - str.find_last_not_of(ch) - 1, str.size()));
}

// https://docs.opencv.org/3.4/d9/dfa/classcv_1_1freetype_1_1FreeType2.html#af059d49b806b916ffdd6380b9eb2f59a
inline std::string TextToImg(const std::string &text)
{
    int font_height = 24;
    auto path = std::filesystem::current_path().parent_path();
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
    cv::Mat img(img_height + border * 2 + gap * (lines.size() - 1), img_width + border * 2, CV_8UC3, cv::Scalar(253, 253, 245)); // B-G-R in opencv
    int start_y = border;
    for(auto &line : lines)
    {
        freetype2->putText(img, 
                            line, 
                            cv::Point{border, start_y}, 
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
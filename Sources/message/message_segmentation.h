#ifndef MIGANGBOTCPP_MESSAGE_MESSAGE_SEGMENTATION_H_
#define MIGANGBOTCPP_MESSAGE_MESSAGE_SEGMENTATION_H_

#include <string>
#include <string_view>
#include <fmt/core.h>

namespace white
{
namespace message_segmentation
{

inline std::string image(const std::string_view &view)
{
    return fmt::format("[CQ:image,file={}]", view);
}

} // namespace message_segmentation
} // namespace white

#endif
#ifndef MIGANGBOTCPP_API_ONTBOT_11_API_IMPL_H_
#define MIGANGBOTCPP_API_ONTBOT_11_API_IMPL_H_

#include "event/event.h"
#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <string>

namespace white
{
namespace onebot11
{

inline void ErrorHanding(boost::beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

class ApiImpl : std::enable_shared_from_this<ApiImpl>
{
public:
    ApiImpl();
    ~ApiImpl();

public:
    std::string send_msg(const Event &event, const std::string &message, bool auto_escape = false);
    
};

inline ApiImpl::ApiImpl()
{

}

inline ApiImpl::~ApiImpl()
{

}

inline std::string ApiImpl::send_msg(const Event &event, const std::string &message, bool auto_escape)
{
    Event msg = {
        {"action", "send_msg_async"},
        {"params", {
            {"message_type", event["message_type"]},
            {"user_id"     , event["user_id"]},
            {"group_id"    , event["group_id"]},
            {"message"     , message},
            {"auto_escape" , auto_escape}
        }}
    };
    return msg.dump();
}

} // namespace onebot11
} // namespace white

#endif
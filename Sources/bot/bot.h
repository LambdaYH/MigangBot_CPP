#ifndef MIGANGBOTCPP_BOT_BOT_H_
#define MIGANGBOTCPP_BOT_BOT_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <condition_variable>
#include <nlohmann/json.hpp>
#include <oneapi/tbb/concurrent_unordered_map.h>
#include <oneapi/tbb/concurrent_queue.h>
#include <thread>
#include <mutex>
#include <string>
#include <utility>
#include <functional>
#include <queue>

#include "bot/onebot_11/api_bot.h"
#include "version.h"
#include "event/event.h"
#include "event/event_handler.h"

namespace white
{

using Json = nlohmann::json;

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>

inline void fail(boost::beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

class Bot : public std::enable_shared_from_this<Bot>
{
public:
    explicit 
    Bot(tcp::socket&& socket);
    ~Bot();

    void Run();

    // Start the asynchronous operation
    void OnRun();

    void OnAccept(beast::error_code ec);

    void DoRead();

    void OnRead(beast::error_code ec, std::size_t bytes_transferred);

private:

    void OnSend(const std::string &message);

    void OnWrite(beast::error_code ec, std::size_t);

    void Process(const std::string &message);

    void Notify(const std::string &msg);

    void SetEchoFunction(const std::time_t echo_code, std::function<void(const Json &)> &&func);

    bool EventProcess(const Event &event);

private:
    beast::websocket::stream<boost::beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::queue<std::string> writable_msg_queue_;
    tbb::concurrent_unordered_map<std::time_t, std::function<void(const Json &)>> echo_function_;
    std::function<void(const std::string &)> notify_;
    std::function<void(const std::time_t, std::function<void(const Json &)> &&)> set_echo_function_;
    onebot11::ApiBot api_bot_;
    std::function<bool(Event &)> event_handler_;

    bool stop_;
};

inline Bot::Bot(tcp::socket&& socket) :
        ws_(std::move(socket)),
        notify_(std::bind(&Bot::Notify, this, std::placeholders::_1)),
        set_echo_function_(std::bind(&Bot::SetEchoFunction, this, std::placeholders::_1, std::placeholders::_2)),
        api_bot_(notify_, set_echo_function_),
        event_handler_( std::bind(&EventHandler::Handle, &EventHandler::GetInstance(), std::placeholders::_1, std::ref(api_bot_)) )
{

}

inline Bot::~Bot()
{

}

inline void Bot::Run()
{
    net::dispatch(ws_.get_executor(),
                beast::bind_front_handler(
                    &Bot::OnRun,
                    shared_from_this()));
}

inline void Bot::OnRun()
{
    ws_.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
        {
            res.set(http::field::server,
                std::string(MIGANGBOTCPP_VERSION) +
                    " server-async");
        }));
    // Accept the websocket handshake
    ws_.async_accept(
        beast::bind_front_handler(
            &Bot::OnAccept,
            shared_from_this()));
}

inline void Bot::OnAccept(beast::error_code ec)
{
    if(ec)
        return fail(ec, "accept");

    DoRead();
}

inline void Bot::DoRead()
{
    ws_.async_read(
    buffer_,
    beast::bind_front_handler(
        &Bot::OnRead,
        shared_from_this()));
}

inline void Bot::OnRead(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if(ec == websocket::error::closed)
        return;

    if(ec)
        return fail(ec, "read");

    auto msg_str = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());
    net::post(
        ws_.get_executor(),
        beast::bind_front_handler(
            &Bot::Process,
            shared_from_this(),
            msg_str
        )
    );
    DoRead();
}


inline void Bot::Notify(const std::string &msg)
{
    net::post(
        ws_.get_executor(),
        beast::bind_front_handler(
            &Bot::OnSend,
            shared_from_this(),
            msg));
}

inline void Bot::SetEchoFunction(const std::time_t echo_code, std::function<void(const Json &)> &&func)
{
    echo_function_[echo_code] = std::move(func);
}

inline void Bot::Process(const std::string &message)
{
    LOG_DEBUG("Recieve: {}", message);
    try
    {
        auto msg = nlohmann::json::parse(message);
        if(EventProcess(msg))
            event_handler_(msg);
    }catch(nlohmann::json::exception &e)
    {
        LOG_ERROR("Exception: {}", e.what());
    }
}

inline void Bot::OnSend(const std::string &message)
{
    writable_msg_queue_.push(message);

    // Are we already writing?
    if(writable_msg_queue_.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    ws_.async_write(
        net::buffer(writable_msg_queue_.front()),
        beast::bind_front_handler(
            &Bot::OnWrite,
            shared_from_this()
        )
    );
}

inline void Bot::OnWrite(beast::error_code ec, std::size_t bytes_transferred)
{
    if(ec)
        return fail(ec, "write");

    // Remove the string from the queue
    writable_msg_queue_.pop();

    // Send the next message if any
    if(!writable_msg_queue_.empty())
        ws_.async_write(
            net::buffer(writable_msg_queue_.front()),
            beast::bind_front_handler(
                &Bot::OnWrite,
                shared_from_this()
            )
        );
}

inline bool Bot::EventProcess(const Event &event)
{
    if(event.contains("retcode"))
    {
        if(event.value("status", "failed") == "ok")
        {
            std::time_t echo_code = 0;
            if (event.contains("echo"))
                echo_code = event["echo"].get<std::time_t>();
            if (echo_function_.count(echo_code))
            {
                echo_function_.at(echo_code)(event["data"]);
                echo_function_.unsafe_erase(echo_code);
            }
        }
        return false;
    }else if(event.contains("message"))
    {
        QId user_id = event["user_id"].get<QId>();
        if(event.contains("group_id"))
        {
            GId group_id = event["group_id"].get<GId>();
            if(api_bot_.IsNeedMessage(group_id, user_id))
                api_bot_.FeedMessage(group_id, user_id, event["message"].get<std::string>());
        }
        if(api_bot_.IsSomeOneNeedMessage(user_id))
            api_bot_.FeedMessageTo(user_id, event["message"].get<std::string>());
    }
    return true;
}

} // namespace white

#endif
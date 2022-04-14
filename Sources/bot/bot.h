#ifndef MIGANGBOTCPP_BOT_BOT_H_
#define MIGANGBOTCPP_BOT_BOT_H_

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <condition_variable>
#include <nlohmann/json.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <utility>
#include <functional>

#include "event/event.h"

namespace white
{

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>

class Bot : public std::enable_shared_from_this<Bot>
{
public:
    explicit 
    Bot(tcp::socket&& socket, std::size_t write_thread_num = 2, std::size_t process_thread_num = 2);
    ~Bot();

    void Run();

    // Start the asynchronous operation
    void OnRun();

    void OnAccept(beast::error_code ec);

    void DoRead();

    void OnRead(beast::error_code ec, std::size_t bytes_transferred);

private:
    void StartThread(std::size_t write_thread_num, std::size_t process_thread_num);

    void ThreadFunctionWrite();

    void ThreadFunctionProcess();

    void Notify(const std::string &msg);

private:
    beast::websocket::stream<boost::beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::condition_variable cond_write_;
    std::condition_variable cond_process_;
    std::queue<std::string> writable_msg_queue_;
    std::queue<std::string> processable_msg_queue_;
    std::mutex mutex_write_;
    std::mutex mutex_process_;
    std::function<void(const std::string &)> notify_;
    std::function<bool(const Event &)> event_handler_;

    bool stop_;
};

inline void Bot::Notify(const std::string &msg)
{
    {
        std::lock_guard<std::mutex> locker(mutex_write_);
        writable_msg_queue_.push(msg);
    }
    cond_write_.notify_one();
}

} // namespace white

#endif
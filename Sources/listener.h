#ifndef MIGANGBOTCPP_LISTENER_H_
#define MIGANGBOTCPP_LISTENER_H_

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include "bot/bot.h"

namespace white
{

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>

inline void Fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

class listener : public std::enable_shared_from_this<listener>
{
public:
    listener(
        net::io_context& ioc,
        tcp::endpoint endpoint)
        : ioc_(ioc)
        , acceptor_(ioc)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            Fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            Fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            Fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            Fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    Run()
    {
        DoAccept();
    }

private:
    void
    DoAccept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                &listener::OnAccept,
                shared_from_this()));
    }

    void
    OnAccept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
            Fail(ec, "accept");
        else
            std::make_shared<white::Bot>(std::move(socket))->Run();
        
        // Accept another connection
        DoAccept();
    }
private:
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    
};

} // namespace white

#endif
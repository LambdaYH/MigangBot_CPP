//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket server, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "event/event_handler.h"
#include "event/onebot_11/event_filter.h"
#include "listener.h"
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include "module_list.h"
#include "logger/logger.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

template<typename T>
inline void InitEventFilter()
{
    if(std::is_base_of<white::EventFilter, T>::value)
        white::EventHandler::GetInstance().InitFilter(std::unique_ptr<white::EventFilter>(new T));
}

int main(int argc, char* argv[])
{
    // load config
    std::filesystem::path current_working_dir = std::filesystem::current_path().parent_path();
    std::filesystem::path config_doc_path = current_working_dir / "config.yml";
    if(!std::filesystem::exists(config_doc_path))
    {
        std::cerr << "Config file: " << config_doc_path << " not exists!" << std::endl;
        return 1;
    }
    YAML::Node config = YAML::LoadFile(config_doc_path);
    auto const address = net::ip::make_address(config["listen"].as<std::string>());
    auto const port = static_cast<unsigned short>(config["port"].as<unsigned short>());
    auto const threads = std::max<int>(1, config["thread_num"].as<int>());

    auto const log_file = config["log_path"].as<std::string>();
    auto const log_level = config["log_level"].as<std::string>();
    white::LOG_INIT(log_file, log_level);


    white::InitModuleList();
    InitEventFilter<white::onebot11::EventFilterOnebot11>();
    // The io_context is required for all I/O
    net::io_context ioc{threads};
    // Create and launch a listening port
    std::make_shared<white::listener>(ioc, tcp::endpoint{address, port})->Run();
    white::LOG_INFO("MigangBot start");
    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    return EXIT_SUCCESS;
}
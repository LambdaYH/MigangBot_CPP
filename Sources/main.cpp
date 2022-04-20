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
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <filesystem>

#include "event/event_handler.h"
#include "event/onebot_11/event_filter.h"
#include "listener.h"
#include "module_list.h"
#include "logger/logger.h"
#include "global_config.h"
#include "database/mysql_conn.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// thread number of async_read
constexpr auto kThreadNum = 2;

YAML::Node white::global_config;
constexpr auto kGlobalConfigExample =   "Server:\n"
                                        "  Listen: 0.0.0.0\n" 
                                        "  Port: 8080\n"
                                        "  Log_path: logs/MigangBot.log     # the log path\n"
                                        "  Log_level: INFO\n"
                                        "\n"
                                        "Bot:\n"
                                        "  Name: <YOUR_BOT_NAME>            # 机器人名字\n"
                                        "  SuperUser: []                    # 管理员账号\n"
                                        "\n"
                                        "DataBase:\n"
                                        "  Host: 127.0.0.1\n"
                                        "  Port: 3306                       # 数据库端口\n"
                                        "  Name: <YOUR_DATABASE_NAME>       # 数据库名\n"
                                        "  Username: <YOUR_USERNAME>        # 数据库用户名\n"
                                        "  Password: <YOUR_PASSWORD>        # 数据库密码\n"
                                        "\n"
                                        "# 不懂就不改\n"
                                        "Dev:\n"
                                        "  ThreadNum:\n"
                                        "    Read: 1                        # 异步读取线程数\n"
                                        "    Process: 2                     # 处理指令线程数\n"
                                        "    Write: 2                       # 发送线程数\n"
                                        "    ThreadPool: 4                  # 处理各个命令对应操作线程池的线程数\n"
                                        "  SqlPool: 5                       # 数据库连接池连接数";

template<typename T>
inline void InitEventFilter()
{
    if(std::is_base_of<white::EventFilter, T>::value)
        white::EventHandler::GetInstance().InitFilter(std::unique_ptr<white::EventFilter>(new T));
}

int main(int argc, char* argv[])
{
    // load config
    std::filesystem::path current_working_dir = std::filesystem::current_path();
    std::filesystem::path config_doc_path = current_working_dir / "config.yml";
    if(!std::filesystem::exists(config_doc_path))
    {
        std::cerr << "未找到[" << config_doc_path << "]，已生成配置文件，请在修改配置文件后重新启动" << std::endl;
        std::fstream config_file(config_doc_path, std::ios_base::out);
        config_file << kGlobalConfigExample;
        config_file.close();
        return 1;
    }

    // 加载配置文件
    white::global_config = YAML::LoadFile(config_doc_path);

    // 初始化websocket
    auto const address = net::ip::make_address(white::global_config["Server"]["Listen"].as<std::string>());
    auto const port = static_cast<unsigned short>(white::global_config["Server"]["Port"].as<unsigned short>());

    // 初始化日志
    auto const log_file = white::global_config["Server"]["Log_path"].as<std::string>();
    auto const log_level = white::global_config["Server"]["Log_level"].as<std::string>();
    white::LOG_INIT(log_file, log_level);

    // 初始化事件处理器
    white::EventHandler::GetInstance().Init(white::global_config["Dev"]["ThreadNum"]["ThreadPool"].as<std::size_t>());
    InitEventFilter<white::onebot11::EventFilterOnebot11>();

    // 初始化数据库连接池
    white::sql::MySQLConnPool::GetInstance().Init(
        white::global_config["DataBase"]["Host"].as<std::string>(),
        white::global_config["DataBase"]["Username"].as<std::string>(),
        white::global_config["DataBase"]["Password"].as<std::string>(),
        white::global_config["DataBase"]["Name"].as<std::string>(),
        white::global_config["DataBase"]["Port"].as<unsigned int>(),
        white::global_config["Dev"]["SqlPool"].as<std::size_t>()
    );

    // 初始化模块
    white::InitModuleList();

    white::LOG_INFO("MigangBot已初始化");

    // 开始监听
    // The io_context is required for all I/O
    auto const thread_num = white::global_config["Dev"]["ThreadNum"]["Read"].as<int>();
    net::io_context ioc{thread_num};
    // Create and launch a listening port
    std::make_shared<white::listener>(ioc, tcp::endpoint{address, port}, white::global_config["Dev"]["ThreadNum"]["Write"].as<std::size_t>(), white::global_config["Dev"]["ThreadNum"]["Process"].as<std::size_t>())->Run();
    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(thread_num);
    for(auto i = thread_num - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    return EXIT_SUCCESS;
}
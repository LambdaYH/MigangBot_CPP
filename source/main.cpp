#include <cstdlib>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <co/co.h>
#include <hv/hlog.h>
#include <yaml-cpp/yaml.h>

#include "database/mysql_conn_pool.h"
#include "database/redis_conn_pool.h"
#include "event/event_handler.h"
#include "event/onebot_11/event_filter.h"
#include "global_config.h"
#include "logger/logger.h"
#include "module_list.h"
#include "server.h"

YAML::Node white::global_config;
std::filesystem::path white::config::kConfigDir;
std::filesystem::path white::config::kAssetsDir;
std::string white::config::BOT_NAME;
std::unordered_set<white::QId> white::config::SUPERUSERS;
std::unordered_set<white::QId> white::config::WHITE_LIST;

constexpr auto kGlobalConfigExample =
    "Server:\n"
    "  Listen: 0.0.0.0\n"
    "  Port: 8080\n"
    "  Log_path: logs/MigangBot.log     # the log path\n"
    "  Log_level: INFO\n"
    "\n"
    "Bot:\n"
    "  Name: <YOUR_BOT_NAME>            # 机器人名字\n"
    "  SuperUsers: []                   # 管理员账号\n"
    "  WhiteList: []                    # 白名单\n"
    "\n"
    "DataBase:\n"
    "  Host: 127.0.0.1\n"
    "  Port: 3306                       # 数据库端口\n"
    "  Name: <YOUR_DATABASE_NAME>       # 数据库名\n"
    "  Username: <YOUR_USERNAME>        # 数据库用户名\n"
    "  Password: <YOUR_PASSWORD>        # 数据库密码\n"
    "\n"
    "Redis:\n"
    "  Host: 127.0.0.1\n"
    "  Port: 6379\n"
    "\n"
    "# 不懂就不改，0表示默认值\n"
    "Dev:\n"
    "  SqlPool: 5                       # 数据库连接池连接数\n"
    "  RedisPool: 5                     # Redis连接池连接数";

template <typename T>
inline void InitEventFilter() {
  if (std::is_base_of<white::EventFilter, T>::value)
    white::EventHandler::GetInstance().InitFilter(
        std::unique_ptr<white::EventFilter>(new T));
}

int main(int argc, char** argv) {
  // load config
  std::filesystem::path current_working_dir = std::filesystem::current_path();
  std::filesystem::path config_doc_path = current_working_dir / "config.yml";
  if (!std::filesystem::exists(config_doc_path)) {
    std::cerr << "未找到[" << config_doc_path
              << "]，已生成配置文件，请在修改配置文件后重新启动" << std::endl;
    std::fstream config_file(config_doc_path, std::ios_base::out);
    config_file << kGlobalConfigExample;
    config_file.close();
    return 1;
  }
  white::config::kConfigDir = current_working_dir / "configs";
  white::config::kAssetsDir = current_working_dir / "assets";
  if (!std::filesystem::exists(white::config::kConfigDir)) {
    if (!std::filesystem::create_directory(white::config::kConfigDir)) {
      std::cerr << "[" << config_doc_path << "]，配置目录创建失败" << std::endl;
      return 1;
    }
    std::cout << "插件配置文件目录已创建" << std::endl;
  }

  // 加载配置文件
  white::global_config = YAML::LoadFile(config_doc_path);

  // 初始化websocket
  auto const address =
      white::global_config["Server"]["Listen"].as<std::string>();
  auto const port = static_cast<unsigned short>(
      white::global_config["Server"]["Port"].as<unsigned short>());

  // 初始化日志
  auto const log_file =
      white::global_config["Server"]["Log_path"].as<std::string>();
  auto const log_level =
      white::global_config["Server"]["Log_level"].as<std::string>();
  white::LOG_INIT(log_file, log_level);

  // 初始化事件处理器
  InitEventFilter<white::onebot11::EventFilterOnebot11>();

  // 初始化数据库连接池
  white::sql::MySQLConnPool::GetInstance().Init(
      white::global_config["DataBase"]["Host"].as<std::string>(),
      white::global_config["DataBase"]["Username"].as<std::string>(),
      white::global_config["DataBase"]["Password"].as<std::string>(),
      white::global_config["DataBase"]["Name"].as<std::string>(),
      white::global_config["DataBase"]["Port"].as<unsigned int>(),
      white::global_config["Dev"]["SqlPool"].as<std::size_t>());

  // 初始化Redis连接池
  white::redis::RedisConnPool::GetInstance().Init(
      white::global_config["Redis"]["Host"].as<std::string>(),
      white::global_config["Redis"]["Port"].as<unsigned int>(),
      white::global_config["Dev"]["RedisPool"].as<std::size_t>());
  // 初始化模块
  white::module::InitModuleList();

  // 加载全局配置
  white::config::BOT_NAME =
      white::global_config["Bot"]["Name"].as<std::string>();
  auto superusers_yaml_node = white::global_config["Bot"]["SuperUsers"];
  for (std::size_t i = 0; i < superusers_yaml_node.size(); ++i)
    white::config::SUPERUSERS.insert(superusers_yaml_node[i].as<white::QId>());

  auto whitelist_yaml_node = white::global_config["Bot"]["WhiteList"];
  for (std::size_t i = 0; i < whitelist_yaml_node.size(); ++i)
    white::config::WHITE_LIST.insert(whitelist_yaml_node[i].as<white::QId>());

  white::LOG_INFO("MigangBot已初始化");
  white::LOG_INFO("监听地址: {}:{}", address, port);

  hlog_disable();

  white::Server server(port, address);

  // init schedule ...
  flag::init(argc, argv);
  server.Run();

  return EXIT_SUCCESS;
}
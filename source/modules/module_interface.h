#ifndef MIGANGBOT_MODULES_MODULE_INTERFACE_H_
#define MIGANGBOT_MODULES_MODULE_INTERFACE_H_

#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <string>
#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <fmt/core.h>

#include "logger/logger.h"
#include "message/utility.h"
#include "permission/permission.h"
#include "utility.h"
#include "event/Registrar.h"
#include "global_config.h"

namespace white {
namespace module {

#define ACT_InClass(func) \
  [this](const Event &event, onebot11::ApiBot &bot) { func(event, bot); }

#define ACT_OutClass(func) \
  [](const Event &event, onebot11::ApiBot &bot) { func(event, bot); }

using std::make_pair;
using ScheduleServicePtr = std::shared_ptr<ScheduleService>;

using Config = YAML::Node;

inline YAML::Node LoadConfig(const std::string &config_file,
                             const std::string &config_example) {
  YAML::Node ret;
  if (config_file.empty()) {
    LOG_WARN("未配置配置文件路径", config_file);
    return ret;
  }
  if (!std::string_view(config_file).ends_with("yml")) {
    LOG_WARN("配置文件格式错误，应当为.yml");
    return ret;
  }
  std::filesystem::path path;
  if (config_file[0] != '/') path = config::kConfigDir / config_file;
  if (!std::filesystem::exists(path)) {
    LOG_WARN("无法加载配置文件[{}]", config_file);
    LOG_INFO("尝试生成配置文件");
    auto parent_path = path.parent_path();
    if (!std::filesystem::exists(parent_path) &&
        !std::filesystem::create_directories(parent_path)) {
      LOG_ERROR("创建配置文件[{}]失败", config_file);
      return ret;
    }
    std::fstream f(path, std::ios_base::out);
    f << config_example;
    f.close();
    LOG_INFO("已成功创建[{}]", config_file);
  }
  ret = YAML::LoadFile(path);
  return ret;
}

class Module {
 public:
  // 如果没有配置文件，就留空
  Module(const std::string_view &config_file = "",
         const std::string_view &config_example = "")
      : config_file_(config_file), config_example_(config_example) {}

  virtual void Register() = 0;

 protected:
  YAML::Node LoadConfig() { return module::LoadConfig(config_file_, config_example_); }

 private:
  const std::string config_file_;
  const std::string config_example_;
};

class ModuleBundle {
 public:
  virtual void Register() = 0;
};

}  // namespace module
}  // namespace white
#endif
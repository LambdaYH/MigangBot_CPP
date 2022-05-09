#ifndef MIGANGBOT_MODULE_MANAGER_H_
#define MIGANGBOT_MODULE_MANAGER_H_

#include <memory>
#include <vector>

#include "modules/module_interface.h"

namespace white {

class ModuleManager {
 public:
  static ModuleManager &GetInstance() {
    static ModuleManager module_manager;
    return module_manager;
  }
  template <typename T>
  void InitModule() {
    if (!std::is_base_of<module::Module, T>::value)
      return;  // add log here
    else {
      try {
        module_container_.push_back(std::move(std::unique_ptr<T>(new T)));
        module_container_.back()->Register();
        LOG_INFO("已成功装载模块：{}", typeid(T).name());
      } catch (std::exception &e) {
        LOG_ERROR("加载[{}]时发生异常: {}", typeid(T).name(), e.what());
      }
    }
  }

 public:
  ModuleManager(const ModuleManager &) = delete;
  ModuleManager &operator=(const ModuleManager &) = delete;
  ModuleManager(const ModuleManager &&) = delete;
  ModuleManager &operator=(const ModuleManager &&) = delete;

 private:
  ModuleManager() {}
  ~ModuleManager() {}

 private:
  std::vector<std::unique_ptr<module::Module>> module_container_;
};

template <typename T>
inline void AddModule() {
  white::ModuleManager::GetInstance().InitModule<T>();
}
}  // namespace white

#endif
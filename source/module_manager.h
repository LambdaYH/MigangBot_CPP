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
    if constexpr (std::is_base_of<module::Module, T>::value) {
      try {
        module_container_.push_back(std::make_unique<T>());
        module_container_.back()->Register();
        LOG_INFO("已成功装载模块：{}", typeid(T).name());
      } catch (std::exception &e) {
        LOG_ERROR("加载[{}]时发生异常: {}", typeid(T).name(), e.what());
      }
    } else if constexpr (std::is_base_of<module::ModulePack, T>::value) {
      T().Register();
    }
  }

 public:
  ModuleManager(const ModuleManager &) = delete;
  ModuleManager &operator=(const ModuleManager &) = delete;
  ModuleManager(ModuleManager &&) = delete;
  ModuleManager &operator=(ModuleManager &&) = delete;

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

template <typename T>
inline void SubModule() {
  white::ModuleManager::GetInstance().InitModule<T>();
}

}  // namespace white

#endif
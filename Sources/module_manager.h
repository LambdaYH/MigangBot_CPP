#ifndef MIGANGBOTCPP_MODULE_MANAGER_H_
#define MIGANGBOTCPP_MODULE_MANAGER_H_

#include "modules/module_interface.h"
#include "event/event_handler.h"
#include <vector>
#include <memory>

namespace white
{

class ModuleManager
{
public:
    static ModuleManager &GetInstance()
    {
        static ModuleManager module_manager;
        return module_manager;
    }
    template<typename T>
    void InitModule()
    {
        if(!std::is_base_of<module::Module, T>::value)
            return; // add log here
        else
        {
            try
            {
                module_container_.push_back(std::move(std::unique_ptr<T>(new T)));
                module_container_.back()->Register();
                LOG_INFO("已成功装载插件：{}", typeid(T).name());
            }catch(...)
            {
                LOG_ERROR("创建[{}]时发生异常", typeid(T).name());
            }
        }
    }
private:
    ModuleManager() {}
    ~ModuleManager() {}

    ModuleManager(const ModuleManager &) = delete;
    ModuleManager &operator=(const ModuleManager &) = delete;
    ModuleManager(const ModuleManager &&) = delete;
    ModuleManager &operator=(const ModuleManager &&) = delete;

private:
    std::vector<std::unique_ptr<module::Module>> module_container_;
};

template<typename T>
inline void AddModule()
{
    white::ModuleManager::GetInstance().InitModule<T>();
}
} // namespace white

#endif
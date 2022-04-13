#ifndef MIGANGBOTCPP_MODULE_MANAGER_H_
#define MIGANGBOTCPP_MODULE_MANAGER_H_

#include "plugins/plugin_interface.h"
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
        if(!std::is_base_of<PluginInterface, T>::value)
            return; // add log here
        else
        {
            module_container_.push_back(std::move(std::unique_ptr<T>(new T)));
            module_container_.back()->Register(EventHandler::GetInstance());
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
    std::vector<std::unique_ptr<PluginInterface>> module_container_;
};

template<typename T>
inline void AddModule()
{
    white::ModuleManager::GetInstance().InitModule<T>();
}
} // namespace white

#endif
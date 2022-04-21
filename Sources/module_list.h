#ifndef MIGANGBOTCPP_MODULE_LIST_H_
#define MIGANGBOTCPP_MODULE_LIST_H_

#include "module_manager.h"

#include "modules/module/echo.h"
#include "modules/module/botmanage/help.h"

namespace white
{
namespace module
{

// add your module here
inline void InitModuleList()
{
    AddModule<Echo>();
    AddModule<Help>();
}

} // namespace module
} // namespace white

#endif
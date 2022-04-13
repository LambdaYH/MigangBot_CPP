#ifndef MIGANGBOTCPP_MODULE_LIST_H_
#define MIGANGBOTCPP_MODULE_LIST_H_

#include "module_manager.h"

#include "plugins/module/echo.h"
#include "plugins/module/botmanage/help.h"

namespace white
{

// add your module here
inline void InitModuleList()
{
    AddModule<Echo>();
    AddModule<Help>();
}

} // namespace white

#endif
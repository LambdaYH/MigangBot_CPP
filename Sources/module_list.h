#ifndef MIGANGBOTCPP_MODULE_LIST_H_
#define MIGANGBOTCPP_MODULE_LIST_H_

#include "module_manager.h"

#include "modules/module/echo.h"

#include "modules/module/botmanage/help.h"
#include "modules/module/botmanage/status_info.h"
#include "modules/module/botmanage/feedback.h"

#include "modules/module/tencentcloud_nlp/auto_summarization.h"
#include "modules/module/tencentcloud_nlp/keywords_extraction.h"

#include "modules/module/eorzea_zhanbu/eorzea_zhanbu.h"

namespace white
{
namespace module
{

// add your module here
inline void InitModuleList()
{
    AddModule<Echo>();

    // botmanage
    AddModule<Help>();
    AddModule<StatusInfo>();
    AddModule<FeedBack>();

    // tencentcloud_nlp
    AddModule<AutoSummarization>();
    AddModule<KeywordsExtraction>();

    // eorzea_zhan
    AddModule<EorzeaZhanbu>();
}

} // namespace module
} // namespace white

#endif
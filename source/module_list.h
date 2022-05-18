#ifndef MIGANGBOT_MODULE_LIST_H_
#define MIGANGBOT_MODULE_LIST_H_

#include "module_manager.h"

#include "modules/module/echo.h"

#include "modules/module/botmanage/botmanage.h"

#include "modules/module/tencentcloud_nlp/auto_summarization.h"
#include "modules/module/tencentcloud_nlp/keywords_extraction.h"

#include "modules/module/eorzea_zhanbu/eorzea_zhanbu.h"

#include "modules/module/bilibili_parser/bilibili_parser.h"

#include "modules/module/weibo/weibo_push.h"

namespace white {
namespace module {

// add your module here
inline void InitModuleList() {
  AddModule<Echo>();

  // botmanage
  AddModule<BotManage>();

  // tencentcloud_nlp
  AddModule<AutoSummarization>();
  AddModule<KeywordsExtraction>();

  // eorzea_zhan
  AddModule<EorzeaZhanbu>();

  // bilibili_parser
  AddModule<BilibiliParser>();

  // weibo
  AddModule<Weibo>();
}

}  // namespace module
}  // namespace white

#endif
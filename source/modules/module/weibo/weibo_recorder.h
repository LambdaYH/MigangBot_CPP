#pragma once

#include "db/db.h"
#include "db/db_orm.h"
#include "fmt/format.h"
#include "logger/logger.h"
#include "sqlpp11/insert.h"
#include "type.h"

namespace white {
namespace module {

class WeiboRecorder {
 public:
  WeiboRecorder() {
    try {
      mariadb::DB().Execute(
          "CREATE TABLE IF NOT EXISTS Weibos\n"
          "(id          INT         NOT NULL AUTO_INCREMENT,\n"
          "weibo_id     char(25)    NOT NULL,\n"
          "push_time    char(20)    NOT NULL,\n"
          "content      TEXT        NOT NULL,\n"
          "PRIMARY KEY(id))");
    } catch (const sqlpp::exception &e) {
      LOG_ERROR("WeiboRecorder: 创建表发生错误。code: {}", e.what());
      return;
    }
  }

  bool RecordWeibo(const std::string &weibo_id, const std::string &push_time,
                   const std::string &content) {
    try {
      mariadb::DB()(sqlpp::insert_into(wb_).set(wb_.weiboId = weibo_id,
                                                wb_.pushTime = push_time,
                                                wb_.content = content));
    } catch (const sqlpp::exception &e) {
      LOG_ERROR("WeiboRecorder: 更新表发生错误。code: {}", e.what());
      return false;
    }
    return true;
  }

  bool IsExist(const std::string &weibo_id) {
    try {
      return !mariadb::DB()(sqlpp::select(wb_.weiboId)
                                .from(wb_)
                                .where(wb_.weiboId == weibo_id))
                  .empty();
    } catch (const sqlpp::exception &e) {
      LOG_ERROR("WeiboRecorder: 更新表发生错误。code: {}", e.what());
      return false;
    }
    return true;
  }

 private:
  db::Weibos wb_;
};

}  // namespace module
}  // namespace white
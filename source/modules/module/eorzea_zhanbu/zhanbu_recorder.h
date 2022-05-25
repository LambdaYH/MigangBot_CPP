#pragma once

#include "db/db.h"
#include "db/db_orm.h"
#include "fmt/format.h"
#include "logger/logger.h"
#include "sqlpp11/insert.h"
#include "sqlpp11/update.h"
#include "type.h"

namespace white {
namespace module {

class ZhanbuRecorder {
 public:
  ZhanbuRecorder() {
    try {
      mariadb::DB().Execute(
          "CREATE TABLE IF NOT EXISTS ZhanbuResults\n"
          "(UID         BIGINT UNSIGNED        NOT NULL,\n"
          "luck         VARCHAR(255)           NOT NULL,\n"
          "yi           VARCHAR(255)           NOT NULL,\n"
          "ji           VARCHAR(255)           NOT NULL,\n"
          "dye          VARCHAR(255)           NOT NULL,\n"
          "append_msg   VARCHAR(255)           NOT NULL,\n"
          "basemap      VARCHAR(255)           NOT NULL,\n"
          "expire_time  BIGINT UNSIGNED   NOT NULL,\n"
          "PRIMARY KEY(UID))");
    } catch (const sqlpp::exception &e) {
      LOG_ERROR("ZhanbuRecorder: 创建表发生错误。code: {}", e.what());
      return;
    }
  }

  bool RecordZhanbu(const QId uid, const std::string &luck,
                    const std::string &yi, const std::string &ji,
                    const std::string &dye, const std::string &append_msg,
                    const std::string &basemap, const std::time_t expire_time) {
    try {
      mariadb::DB db;
      if (db(sqlpp::select(result_.UID).from(result_).where(result_.UID == uid))
              .empty())
        db(sqlpp::insert_into(result_).set(
            result_.UID = uid, result_.luck = luck, result_.yi = yi,
            result_.ji = ji, result_.dye = dye, result_.appendMsg = append_msg,
            result_.basemap = basemap, result_.expireTime = expire_time));
      else
        db(sqlpp::update(result_)
               .set(result_.luck = luck, result_.yi = yi, result_.ji = ji,
                    result_.dye = dye, result_.appendMsg = append_msg,
                    result_.basemap = basemap, result_.expireTime = expire_time)
               .where(result_.UID == uid));
    } catch (const sqlpp::exception &e) {
      LOG_ERROR("ZhanbuRecorder: 更新表发生错误。code: {}", e.what());
      return false;
    }

    return true;
  }

  std::tuple<std::string, std::string, std::string, std::string, std::string,
             std::string, std::time_t>
  GetZhanbuRecord(const QId uid) {
    const auto &row_r = mariadb::DB()(
        select(all_of(result_)).from(result_).where(result_.UID == uid));
    if (row_r.empty()) return {};
    const auto &row = row_r.front();
    return {row.luck, row.yi,      row.ji,        row.appendMsg,
            row.dye,  row.basemap, row.expireTime};
  }

 private:
  db::ZhanbuResults result_;
};

}  // namespace module
}  // namespace white
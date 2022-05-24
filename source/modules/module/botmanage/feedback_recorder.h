#pragma once

#include "db/db.h"
#include "db/db_orm.h"
#include "fmt/format.h"
#include "logger/logger.h"
#include "sqlpp11/exception.h"
#include "sqlpp11/insert.h"
#include "type.h"

namespace white {
namespace module {

class FeedbackRecorder {
 public:
  FeedbackRecorder() {
    try {
      mariadb::DB().Execute(
          "CREATE TABLE IF NOT EXISTS Feedbacks\n"
          "(feedbackID   INT    NOT NULL AUTO_INCREMENT,\n"
          "time         TEXT    NOT NULL,\n"
          "UID           INT    NOT NULL,\n"
          "GID           INT    NOT NULL,\n"
          "content      TEXT    NOT NULL,\n"
          "PRIMARY KEY(feedbackID))");
    } catch (const sqlpp::exception &e) {
      LOG_ERROR("FeedbackRecorder: 创建表发生错误。code: {}", e.what());
      return;
    }
  }

  std::size_t GetLastID() {
    auto id = mariadb::DB()(sqlpp::select(sqlpp::max(fb_.feedbackID))
                                .from(fb_)
                                .unconditionally())
                  .front()
                  .max;
    if (id.is_null()) return 0;
    return id.value();
  }

  bool RecordFeedBack(const std::string &time, QId uid, GId gid,
                      const std::string &content) {
    try {
      mariadb::DB()(sqlpp::insert_into(fb_).set(fb_.time = time, fb_.GID = gid,
                                                fb_.UID = uid,
                                                fb_.content = content));
    } catch (const sqlpp::exception &e) {
      return false;
    }
    return true;
  }

  std::vector<std::string> GetFeedback(const std::size_t &feedback_id) {
    auto &r =
        mariadb::DB()(sqlpp::select(fb_.time, fb_.content, fb_.UID, fb_.GID)
                          .from(fb_)
                          .where(fb_.feedbackID == feedback_id))
            .front();
    return {r.time, r.content, std::to_string(r.GID), std::to_string(r.UID)};
  }

 private:
  db::Feedbacks fb_;
};

}  // namespace module
}  // namespace white
#pragma once

#include "database/mysql_wrapper.h"
#include "fmt/format.h"
#include "logger/logger.h"
#include "type.h"

namespace white {
namespace module {

class WeiboRecorder {
 public:
  WeiboRecorder() {
    sql::MySQLWrapper sql_wrapper;
    std::string query =
        "CREATE TABLE IF NOT EXISTS Weibos\n"
        "(weibo_id    char(25)    NOT NULL,\n"
        "push_time    char(20)    NOT NULL,\n"
        "PRIMARY KEY(weibo_id))";
    int ec;
    sql_wrapper.Execute(query, &ec);
    if (ec) {
      LOG_ERROR("WeiboRecorder: 创建表发生错误。code: {}", ec);
      return;
    }
  }

  bool RecordWeibo(const std::string &weibo_id,
                   const std::string &push_time) {
    sql::MySQLWrapper sql_wrapper;
    auto query = fmt::format(
        "INSERT INTO Weibos(weibo_id, push_time)\n"
        "VALUES(\n"
        "\"{}\",\n"
        "\"{}\")",
        weibo_id, push_time);
    int ec;
    sql_wrapper.Execute(query, &ec);
    if (ec) {
      LOG_ERROR("WeiboRecorder: 更新表发生错误。");
      return false;
    }
    return true;
  }

  bool IsExist(const std::string &weibo_id){
    sql::MySQLWrapper sql_wrapper;
    int ec;
    sql_wrapper.Execute(fmt::format("SELECT weibo_id FROM Weibos WHERE weibo_id=\"{}\"", weibo_id), &ec);
    if (ec) {
      LOG_ERROR("WeiboRecorder: 更新表发生错误。");
      return false;
    }
    return !sql_wrapper.FetchOne().empty();
  }
};

}  // namespace module
}  // namespace white
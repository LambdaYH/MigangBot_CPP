#ifndef MIGANGBOT_DB_DB_H_
#define MIGANGBOT_DB_DB_H_

#include "sqlpp11/mysql/connection.h"
#include "sqlpp11/custom_query.h"
#include "sqlpp11/verbatim.h"
#include "sqlpp11/sqlpp11.h"

#include "db/db_conn/orm_pool.h"
#include "db/db_conn/redis_conn.h"
// #include "db/db_conn/mysql_conn.h"
#include "db/db_orm.h"

namespace white {
namespace mariadb {
class DB {
 public:
  DB() : conn_(orm::mariadb::OrmPool::GetInstance().Get()) {}
  ~DB() { orm::mariadb::OrmPool::GetInstance().Free(conn_); }

  sqlpp::mysql::connection &Get() { return conn_; }

  template <typename Str>
  void Execute(Str &&query) {
    conn_.execute(std::forward<Str>(query));
  }

  template <typename... Args>
  decltype(auto) operator()(Args &&...args) {
    return conn_(std::forward<Args>(args)...);
  }

 private:
  sqlpp::mysql::connection &conn_;
};
}  // namespace mariadb

namespace redis {
class DB {
 public:
  DB() : reply_(nullptr), conn_(RedisConn()), ctx_(conn_()) {}
  ~DB() { freeReplyObject(reply_); }

 public:
  bool Execute(const std::string &command) {
    if (reply_) {
      freeReplyObject(reply_);
      reply_ = nullptr;
    }
    reply_ = (redisReply *)redisCommand(&ctx_, command.c_str());
    if (reply_ == nullptr || ctx_.err) {
      LOG_ERROR("发送Redis指令失败: , error: {}", command, ctx_.errstr);
      return false;
    }
    return true;
  }

  const redisReply &GetReply() { return *reply_; }

 private:
  redisReply *reply_;
  unsigned int field_num_;
  RedisConn conn_;
  redisContext &ctx_;
};
}  // namespace redis

// namespace mysql {
// class DB {
//  public:
//   DB() : res_(nullptr), conn_(MySQLConn()), sql_(conn_()) {}
//   ~DB() { mysql_free_result(res_); }

//  public:
//   DB &Execute(const std::string &query, int *ec = nullptr) {
//     int ret = mysql_real_query(&sql_, query.c_str(), query.size());
//     if (ec) *ec = ret;
//     if (ret) {
//       LOG_ERROR("Mysql语句 [{}] 执行失败。retcode: {}", query, ret);
//       return *this;
//     }
//     if (res_) {
//       mysql_free_result(res_);
//       res_ = nullptr;
//     }
//     res_ = mysql_store_result(&sql_);
//     field_num_ = mysql_field_count(&sql_);
//     return *this;
//   }

//   // Both null and empty are represented by an empty string
//   std::vector<std::string> FetchOne() {
//     if (!res_) return {};
//     MYSQL_ROW row = mysql_fetch_row(res_);
//     std::vector<std::string> ret;
//     ret.reserve(field_num_);
//     if (row) {
//       unsigned long *lengths = mysql_fetch_lengths(res_);
//       for (unsigned int i = 0; i < field_num_; ++i)
//         ret.push_back(std::string(row[i], lengths[i]));
//     }
//     return ret;
//   }

//   std::vector<std::vector<std::string>> FetchAll() {
//     if (!res_) return {};
//     std::vector<std::vector<std::string>> ret;
//     while (MYSQL_ROW row = mysql_fetch_row(res_)) {
//       ret.emplace_back();
//       unsigned long *lengths = mysql_fetch_lengths(res_);
//       for (unsigned int i = 0; i < field_num_; ++i)
//         ret.back().push_back(std::string(row[i], lengths[i]));
//     }
//     mysql_free_result(res_);
//     res_ = nullptr;
//     return ret;
//   }

//  private:
//   MYSQL_RES *res_;
//   unsigned int field_num_;
//   MySQLConn conn_;
//   MYSQL &sql_;
// };
// }  // namespace mysql

}  // namespace white

#endif
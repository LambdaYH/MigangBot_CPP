#ifndef MIGANGBOT_DB_DB_CONN_MYSQL_CONN_POOL_H_
#define MIGANGBOT_DB_DB_CONN_MYSQL_CONN_POOL_H_

#include <string>
#include <utility>

#include <mysql/mysql.h>

#include "db/db_conn/conn_pool.h"
#include "logger/logger.h"

namespace white {
namespace mysql {

class MySQLConnPool : public ConnPool<MYSQL> {
 public:
  static MySQLConnPool &GetInstance() {
    static MySQLConnPool pool;
    return pool;
  }

  void Init(const std::string &host, const std::string &user,
            const std::string &passwd, const std::string &db,
            const unsigned int port, std::size_t conn_num);

 private:
  ~MySQLConnPool();
};

inline void MySQLConnPool::Init(const std::string &host,
                                const std::string &user,
                                const std::string &passwd,
                                const std::string &db, const unsigned int port,
                                std::size_t conn_num) {
  pool_.resize(conn_num);
  for (std::size_t i = 0; i < conn_num; ++i) {
    if (!mysql_init(&pool_[i])) {
      LOG_ERROR("数据库连接初始化失败");
      exit(3);
    }

    bool reconnect_opt = true;
    if (mysql_options(&pool_[i], MYSQL_OPT_RECONNECT, (bool *)&reconnect_opt)) {
      LOG_ERROR("数据库连接初始化失败");
      exit(3);
    }

    if (!mysql_real_connect(&pool_[i], host.c_str(), user.c_str(),
                            passwd.c_str(), db.c_str(), port, nullptr, 0)) {
      LOG_ERROR("数据库连接初始化失败");
      exit(3);
    }

    conn_to_id_.emplace(&pool_[i], i);
    conn_id_queue_.push(i);
  }
  LOG_INFO("已初始化数据库连接池，连接数：{}", conn_id_queue_.size());
}

inline MySQLConnPool::~MySQLConnPool() {
  for (auto &conn : pool_) mysql_close(&conn);
  mysql_library_end();
}

}  // namespace sql
}  // namespace white

#endif
#ifndef MIGANGBOT_DB_DB_CONN_MYSQL_CONN_H_
#define MIGANGBOT_DB_DB_CONN_MYSQL_CONN_H_

#include <memory>
#include <string>

#include "db/db_conn/mysql_conn_pool.h"

namespace white {
namespace mysql {

class MySQLConn {
 public:
  MYSQL& operator()() { return sql_conn_; }

 public:
  MySQLConn() : sql_conn_(MySQLConnPool::GetInstance().Get()) {}

  ~MySQLConn() { MySQLConnPool::GetInstance().Free(sql_conn_); }

 private:
  MYSQL& sql_conn_;
};

}  // namespace sql
}  // namespace white

#endif
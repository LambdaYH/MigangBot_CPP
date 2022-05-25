#ifndef MIGANGBOT_DB_DB_CONN_ORM_POOL_H_
#define MIGANGBOT_DB_DB_CONN_ORM_POOL_H_

#include <string>
#include <vector>

#include "sqlpp11/exception.h"
#include "sqlpp11/mysql/connection.h"
#include "sqlpp11/mysql/connection_config.h"
#include <sqlpp11/mysql/mysql.h>

#include "db/db_conn/conn_pool.h"
#include "logger/logger.h"

namespace white {
namespace orm {

namespace mariadb {
class OrmPool : public ConnPool<sqlpp::mysql::connection> {
 public:
  static OrmPool &GetInstance() {
    static OrmPool pool;
    return pool;
  }
  void Init(std::shared_ptr<sqlpp::mysql::connection_config> &config,
            std::size_t conncection_num) {
    pool_.reserve(conncection_num);
    for (std::size_t i = 0; i < conncection_num; ++i) {
      try {
        pool_.emplace_back(config);
        conn_to_id_.emplace(&pool_[i], i);
        conn_id_queue_.push(i);
      } catch (const sqlpp::exception &e) {
        LOG_ERROR("ORM MariaDB连接池初始化失败，请检查配置项。{}", e.what());
        exit(3);
      }
    }
    LOG_INFO("已初始化ORM MariaDB连接池，连接数：{}", pool_.size());
  }
};
}  // namespace mariadb

}  // namespace orm
}  // namespace white

#endif
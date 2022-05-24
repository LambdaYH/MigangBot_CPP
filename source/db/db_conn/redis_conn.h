#ifndef MIGANGBOT_DB_DB_CONN_REDIS_CONN_H_
#define MIGANGBOT_DB_DB_CONN_REDIS_CONN_H_

#include <memory>
#include <string>

#include "db/db_conn/redis_conn_pool.h"

namespace white {
namespace redis {

class RedisConn {
 public:
  redisContext& operator()() { return redis_conn_; }

 public:
  RedisConn() : redis_conn_(RedisConnPool::GetInstance().Get()) {}

  ~RedisConn() { RedisConnPool::GetInstance().Free(redis_conn_); }

 private:
  redisContext& redis_conn_;
};

}  // namespace redis
}  // namespace white

#endif
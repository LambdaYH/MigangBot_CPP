#ifndef MIGANGBOT_DATABASE_REDIS_CONN_H_
#define MIGANGBOT_DATABASE_REDIS_CONN_H_

#include <string>
#include <memory>

#include "database/redis_conn_pool.h"
#include "hiredis.h"

namespace white
{
namespace redis
{

class RedisConn
{
public:
    redisContext& operator()()
    {
        return redis_conn_;
    }

public:
    RedisConn() : redis_conn_(RedisConnPool::GetInstance().GetConn())
    {

    }

    ~RedisConn()
    {
        RedisConnPool::GetInstance().FreeConn(redis_conn_);
    }

private:
    redisContext &redis_conn_;
};


} // namespace redis
} // namespace white

#endif
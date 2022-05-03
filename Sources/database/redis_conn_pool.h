#ifndef MIGANGBOTCPP_DATABASE_REDIS_CONN_POOL_H_
#define MIGANGBOTCPP_DATABASE_REDIS_CONN_POOL_H_

#include <mutex>
#include <string>
#include <condition_variable>
#include <hiredis.h>
#include <unordered_map>
#include <utility>
#include <oneapi/tbb/concurrent_queue.h>

#include "logger/logger.h"

namespace white
{
namespace redis
{

class RedisConnPool
{
public:
    static RedisConnPool &GetInstance()
    {
        static RedisConnPool pool;
        return pool;
    }

    redisContext &GetConn();

    void FreeConn(redisContext &conn);

    void Init(
        const std::string &host,
        const unsigned int port,
        std::size_t conn_num);

private:
    RedisConnPool() {}
    ~RedisConnPool();

    RedisConnPool(const RedisConnPool &) = delete;
    RedisConnPool &operator=(const RedisConnPool &) = delete;
    RedisConnPool(const RedisConnPool &&) = delete;
    RedisConnPool &operator=(const RedisConnPool &&) = delete;

private:
    std::string host_;
    unsigned int port_;
    std::vector<redisContext*> redis_conn_pool_;
    tbb::concurrent_bounded_queue<std::size_t> redis_conn_id_queue_;
    std::unordered_map<redisContext*, std::size_t> redis_conn_to_id_;

};

inline redisContext &RedisConnPool::GetConn()
{
    std::size_t id;
    redis_conn_id_queue_.pop(id);
    if(redis_conn_pool_[id]->err)
    {
        LOG_WARN("Redis disconnected, try to reconnecting");
        redis_conn_to_id_.erase(redis_conn_pool_[id]);
        redisFree(redis_conn_pool_[id]);
        redis_conn_pool_[id] = redisConnect(host_.c_str(), port_);
        if(redis_conn_pool_[id] == nullptr || redis_conn_pool_[id]->err)
        {
            LOG_ERROR("Redis reconnect error: {}", redis_conn_pool_[id]->errstr);
            exit(3);
        }
        redis_conn_to_id_.emplace(redis_conn_pool_[id], id);
        LOG_INFO("Redis reconnect successfully");
    }
    return *redis_conn_pool_[id];
}

inline void RedisConnPool::FreeConn(redisContext &conn)
{
    std::size_t id = redis_conn_to_id_[&conn];
    redis_conn_id_queue_.push(id);
}

inline void RedisConnPool::Init(        
    const std::string &host,
    const unsigned int port,
    std::size_t conn_num)
{
    host_ = host;
    port_ = port;
    redis_conn_pool_.reserve(conn_num);
    for(std::size_t i = 0; i < conn_num; ++i)
    {
        redisContext *c = redisConnect(host.c_str(), port);
        if(c == nullptr || c->err)
        {
            LOG_ERROR("Redis连接初始化失败: {}", c->errstr);
            exit(3);
        }
        redis_conn_pool_.push_back(c);
        redis_conn_to_id_.emplace(redis_conn_pool_.back(), i);
        redis_conn_id_queue_.push(i);
    }
    LOG_INFO("已初始化Redis连接池，连接数：{}", redis_conn_id_queue_.size());
}

inline RedisConnPool::~RedisConnPool()
{
    for(auto &conn : redis_conn_pool_)
        redisFree(conn);
}

} // namespace redis
} // namespace white

#endif
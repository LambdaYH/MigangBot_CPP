#ifndef MIGANGBOTCPP_DATABASE_MYSQL_CONN_POOL_H_
#define MIGANGBOTCPP_DATABASE_MYSQL_CONN_POOL_H_

#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>
#include <mysql/mysql.h>
#include <unordered_map>
#include <utility>

#include "logger/logger.h"

namespace white
{
namespace sql
{

class MySQLConnPool
{
public:
    static MySQLConnPool &GetInstance()
    {
        static MySQLConnPool pool;
        return pool;
    }

    MYSQL &GetConn();

    void FreeConn(MYSQL &conn);

    void Init(
        const std::string &host,
        const std::string &user,
        const std::string &passwd,
        const std::string &db,
        const unsigned int port,
        std::size_t conn_num);

private:
    MySQLConnPool() {}
    ~MySQLConnPool();

    MySQLConnPool(const MySQLConnPool &) = delete;
    MySQLConnPool &operator=(const MySQLConnPool &) = delete;
    MySQLConnPool(const MySQLConnPool &&) = delete;
    MySQLConnPool &operator=(const MySQLConnPool &&) = delete;

private:
    std::vector<MYSQL> sql_conn_pool_;
    std::queue<std::size_t> sql_conn_id_queue_;
    std::mutex sql_conn_id_queue_mutex_;
    std::condition_variable sql_conn_id_queue_cond_;
    std::unordered_map<MYSQL*, std::size_t> sql_conn_to_id_;

};

inline MYSQL &MySQLConnPool::GetConn()
{
    std::size_t id = 0;
    {
        std::unique_lock<std::mutex> locker(sql_conn_id_queue_mutex_);
        while(sql_conn_id_queue_.empty())
            sql_conn_id_queue_cond_.wait(locker);
        id = sql_conn_id_queue_.front();
        sql_conn_id_queue_.pop();
    }
    return sql_conn_pool_[id];
}

inline void MySQLConnPool::FreeConn(MYSQL &conn)
{
    std::size_t id = sql_conn_to_id_[&conn];
    {
        std::lock_guard<std::mutex> locker(sql_conn_id_queue_mutex_);
        sql_conn_id_queue_.push(id);
    }
    sql_conn_id_queue_cond_.notify_one();
}

inline void MySQLConnPool::Init(        
    const std::string &host,
    const std::string &user,
    const std::string &passwd,
    const std::string &db,
    const unsigned int port,
    std::size_t conn_num)
{
    sql_conn_pool_.resize(conn_num);
    for(std::size_t i = 0; i < conn_num; ++i)
    {
        if(!mysql_init(&sql_conn_pool_[i]))
        {
            LOG_ERROR("数据库连接初始化失败");
            exit(3);
        }

        bool reconnect_opt = true;
        if(mysql_options(&sql_conn_pool_[i], MYSQL_OPT_RECONNECT, (bool*)&reconnect_opt))
        {
            LOG_ERROR("数据库连接初始化失败");
            exit(3);
        }

        if(!mysql_real_connect(&sql_conn_pool_[i],
                            host.c_str(),
                            user.c_str(),
                            passwd.c_str(),
                            db.c_str(),
                            port,
                            nullptr,
                            0))
        {
            LOG_ERROR("数据库连接初始化失败");
            exit(3);
        }

        sql_conn_to_id_.emplace(&sql_conn_pool_[i], i);
        sql_conn_id_queue_.push(i);
    }
}

inline MySQLConnPool::~MySQLConnPool()
{
    for(auto &conn : sql_conn_pool_)
        mysql_close(&conn);
    mysql_library_end();
}

} // namespace sql
} // namespace white

#endif
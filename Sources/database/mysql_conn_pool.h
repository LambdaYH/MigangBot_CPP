#ifndef MIGANGBOTCPP_DATABASE_MYSQL_CONN_POOL_H_
#define MIGANGBOTCPP_DATABASE_MYSQL_CONN_POOL_H_

#include <mutex>
#include <string>
#include <condition_variable>
#include <mysql/mysql.h>
#include <unordered_map>
#include <utility>
#include <oneapi/tbb/concurrent_queue.h>

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
    tbb::concurrent_bounded_queue<std::size_t> sql_conn_id_queue_;
    std::unordered_map<MYSQL*, std::size_t> sql_conn_to_id_;

};

inline MYSQL &MySQLConnPool::GetConn()
{
    std::size_t id;
    sql_conn_id_queue_.pop(id);
    return sql_conn_pool_[id];
}

inline void MySQLConnPool::FreeConn(MYSQL &conn)
{
    std::size_t id = sql_conn_to_id_[&conn];
    sql_conn_id_queue_.push(id);
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
    LOG_INFO("已初始化数据库连接池，连接数：{}", sql_conn_id_queue_.size());
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
#ifndef MIGANGBOTCPP_DATABASE_MYSQL_CONN_H_
#define MIGANGBOTCPP_DATABASE_MYSQL_CONN_H_

#include <string>
#include <memory>

#include "database/mysql_conn_pool.h"

namespace white
{
namespace sql
{

class MySQLConn
{
public:
    MYSQL& operator()()
    {
        return sql_conn_;
    }

public:
    MySQLConn() : sql_conn_(MySQLConnPool::GetInstance().GetConn())
    {

    }

    ~MySQLConn()
    {
        MySQLConnPool::GetInstance().FreeConn(sql_conn_);
    }

private:
    MYSQL &sql_conn_;
};


} // namespace sql
} // namespace white

#endif
#ifndef MIGANGBOT_DATABASE_MYSQL_WRAPPER_H_
#define MIGANGBOT_DATABASE_MYSQL_WRAPPER_H_

#include <iterator>
#include <string>
#include <vector>

#include "database/mysql_conn.h"
#include "mysql.h"
#include "logger/logger.h"

namespace white
{
namespace sql
{

class MySQLWrapper
{
public:
    MySQLWrapper() : res_(nullptr), conn_(MySQLConn()), sql_(conn_()) {}
    ~MySQLWrapper() 
    {
        mysql_free_result(res_);
    }
public:
    MySQLWrapper &Execute(const std::string &query, int *ec = nullptr)
    {
        int ret = mysql_real_query(&sql_, query.c_str(), query.size());
        if(ec)
            *ec = ret;
        if(ret)
        {
            LOG_ERROR("Mysql语句 [{}] 执行失败。retcode: {}", query, ret);
            return *this;
        }
        if(res_)
        {
            mysql_free_result(res_);
            res_ = nullptr;
        }
        res_ = mysql_store_result(&sql_);
        field_num_ = mysql_field_count(&sql_);
        return *this;
    }

    // Both null and empty are represented by an empty string
    std::vector<std::string> FetchOne()
    {
        if(!res_)
            return {};
        MYSQL_ROW row = mysql_fetch_row(res_);
        std::vector<std::string> ret;
        ret.reserve(field_num_);
        if(row)
        {
            unsigned long *lengths = mysql_fetch_lengths(res_);
            for(unsigned int i = 0; i < field_num_; ++i)
                ret.push_back(std::string(row[i], lengths[i]));
        }
        return ret;
    }

    std::vector<std::vector<std::string>> FetchAll()
    {
        if(!res_)
            return {};
        std::vector<std::vector<std::string>> ret;
        while(MYSQL_ROW row = mysql_fetch_row(res_))
        {
            ret.emplace_back();
            unsigned long *lengths = mysql_fetch_lengths(res_);
            for(unsigned int i = 0; i < field_num_; ++i)
                ret.back().push_back(std::string(row[i], lengths[i]));
        }
        mysql_free_result(res_);
        res_ = nullptr;
        return ret;
    }
    
private:
    MYSQL_RES *res_;
    unsigned int field_num_;
    MySQLConn conn_;
    MYSQL &sql_;
};


} // namespace sql
} // namespace white

#endif
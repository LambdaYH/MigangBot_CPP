#ifndef MIGANGBOT_MODULES_MODULE_EORZEA_ZHANBU_ZHANBU_RECORDER_H_
#define MIGANGBOT_MODULES_MODULE_EORZEA_ZHANBU_ZHANBU_RECORDER_H_

#include "database/mysql_wrapper.h"
#include "fmt/format.h"
#include "logger/logger.h"
#include "type.h"

namespace white
{
namespace module
{

class ZhanbuRecorder
{
public:
    ZhanbuRecorder()
    {
        sql::MySQLWrapper sql_wrapper;
        std::string query = "CREATE TABLE IF NOT EXISTS ZhanbuResults\n"
                            "(UID         BIGINT UNSIGNED        NOT NULL,\n"
                            "luck         VARCHAR(255)           NOT NULL,\n"
                            "yi           VARCHAR(255)           NOT NULL,\n"
                            "ji           VARCHAR(255)           NOT NULL,\n"
                            "dye          VARCHAR(255)           NOT NULL,\n"
                            "append_msg   VARCHAR(255)           NOT NULL,\n"
                            "basemap      VARCHAR(255)           NOT NULL,\n"
                            "expire_time  BIGINT UNSIGNED   NOT NULL,\n"
                            "PRIMARY KEY(UID))";
        int ec;
        sql_wrapper.Execute(query, &ec);
        if(ec)
        {
            LOG_ERROR("ZhanbuRecorder: 创建表发生错误。code: {}", ec);
            return;
        }
    }

    bool RecordZhanbu(  const QId uid, 
                        const std::string &luck, 
                        const std::string &yi, 
                        const std::string &ji, 
                        const std::string &dye,
                        const std::string &append_msg,
                        const std::string &basemap,
                        const std::time_t expire_time
                    )
    {
        sql::MySQLWrapper sql_wrapper;
        auto query = fmt::format("REPLACE INTO ZhanbuResults\n"
                                "VALUES(\n"
                                    "{},\n"
                                    "\"{}\",\n"
                                    "\"{}\",\n"
                                    "\"{}\",\n"
                                    "\"{}\",\n"
                                    "\"{}\",\n"
                                    "\"{}\",\n"
                                    "{})",  
                                uid,
                                luck,
                                yi,
                                ji,
                                dye,
                                append_msg,
                                basemap,
                                expire_time);
        int ec;
        sql_wrapper.Execute(query, &ec);
        if(ec)
        {
            LOG_ERROR("ZhanbuRecorder: 更新表发生错误。");
            return false;
        }
        return true;
    }

    std::vector<std::string> GetZhanbuRecord(const QId uid)
    {
        return sql::MySQLWrapper()
                .Execute(fmt::format("SELECT * FROM ZhanbuResults WHERE UID={}", uid))
                .FetchOne();
    }

};

} // namespace module
} // namespace white

#endif
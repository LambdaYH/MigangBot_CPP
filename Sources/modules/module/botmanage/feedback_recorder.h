#ifndef MIGANGBOTCPP_MODULES_MODULE_FEEDBACK_RECORDER_H_
#define MIGANGBOTCPP_MODULES_MODULE_FEEDBACK_RECORDER_H_

#include "database/mysql_wrapper.h"
#include "fmt/format.h"
#include "logger/logger.h"
#include "type.h"

namespace white
{
namespace module
{

class FeedbackRecorder
{
public:
    FeedbackRecorder()
    {
        sql::MySQLWrapper sql_wrapper;
        std::string query = "CREATE TABLE IF NOT EXISTS Feedbacks\n"
                            "(feedbackID   INT    NOT NULL AUTO_INCREMENT,\n"
                            "time         TEXT    NOT NULL,\n"
                            "UID           INT    NOT NULL,\n"
                            "GID           INT    NOT NULL,\n"
                            "content      TEXT    NOT NULL,\n"
                            "PRIMARY KEY(feedbackID))";
        int ec;
        sql_wrapper.Execute(query, &ec);
        if(ec)
        {
            LOG_ERROR("FeedbackRecorder: 创建表发生错误。code: {}", ec);
            return;
        }
    }

    std::size_t GetLastID()
    {
        sql::MySQLWrapper sql_wrapper;
        int ec;
        sql_wrapper.Execute("SELECT max(feedbackID) FROM Feedbacks", &ec);
        if(ec)
        {
            LOG_ERROR("FeedbackRecorder: 获取最后一行ID错误。code: {}", ec);
            return 0;
        }
        auto one_row = sql_wrapper.FetchOne();
        if(one_row.empty() || one_row[0].empty())
            return 0;
        return std::stoull(one_row[0]);
    }

    bool RecordFeedBack(const std::string &time, QId uid, GId gid, const std::string &content)
    {
        sql::MySQLWrapper sql_wrapper;
        auto query = fmt::format("INSERT INTO Feedbacks(time, UID, GID, content)\n"
                                "VALUES(\n"
                                    "\"{}\",\n"
                                    "{},\n"
                                    "{},\n"
                                    "\"{}\")",  
                                time,
                                uid,
                                gid,
                                content);
        int ec;
        sql_wrapper.Execute(query, &ec);
        if(ec)
        {
            LOG_ERROR("FeedbackRecorder: 更新表发生错误。");
            return false;
        }
        return true;
    }

    std::vector<std::string> GetFeedback(const std::string &feedback_id)
    {
        return sql::MySQLWrapper()
                .Execute(fmt::format("SELECT time, content, UID, GID FROM Feedbacks WHERE feedbackID={}", feedback_id))
                .FetchOne();
    }

};

} // namespace module
} // namespace white

#endif
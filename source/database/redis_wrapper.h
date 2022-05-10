#ifndef MIGANGBOT_DATABASE_REDIS_WRAPPER_H_
#define MIGANGBOT_DATABASE_REDIS_WRAPPER_H_

#include <iterator>
#include <string>
#include <vector>

#include "database/redis_conn.h"
#include "logger/logger.h"

namespace white {
namespace redis {

class RedisWrapper {
 public:
  RedisWrapper() : reply_(nullptr), conn_(RedisConn()), ctx_(conn_()) {}
  ~RedisWrapper() { freeReplyObject(reply_); }

 public:
  bool Execute(const std::string &command) {
    if (reply_) {
      freeReplyObject(reply_);
      reply_ = nullptr;
    }
    reply_ = (redisReply *)redisCommand(&ctx_, command.c_str());
    if (reply_ == nullptr || ctx_.err) {
      LOG_ERROR("发送Redis指令失败: , error: {}", command, ctx_.errstr);
      return false;
    }
    return true;
  }

  const redisReply &GetReply() { return *reply_; }

 private:
  redisReply *reply_;
  unsigned int field_num_;
  RedisConn conn_;
  redisContext &ctx_;
};

}  // namespace redis
}  // namespace white

#endif
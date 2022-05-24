#ifndef MIGANGBOT_DB_DB_CONN_CONN_POOL_H_
#define MIGANGBOT_DB_DB_CONN_CONN_POOL_H_

#include <vector>
#include <unordered_map>

#include <oneapi/tbb/concurrent_queue.h>

namespace white {

template <typename Connection>
class ConnPool {
 public:
  Connection &Get() {
    std::size_t id;
    conn_id_queue_.pop(id);
    return pool_[id];
  }

  void Free(Connection &conn) {
    std::size_t id = conn_to_id_[&conn];
    conn_id_queue_.push(id);
  }

  ConnPool(const ConnPool &) = delete;
  ConnPool &operator=(const ConnPool &) = delete;
  ConnPool(ConnPool &&) = delete;
  ConnPool &operator=(ConnPool &&) = delete;

 protected:
  ConnPool(){}
  virtual ~ConnPool(){}

 protected:
  std::vector<Connection> pool_;
  tbb::concurrent_bounded_queue<std::size_t> conn_id_queue_;
  std::unordered_map<Connection *, std::size_t> conn_to_id_;
};

}

#endif
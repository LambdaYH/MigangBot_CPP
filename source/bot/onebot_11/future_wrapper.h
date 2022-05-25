#ifndef MIGANGBOT_BOT_ONEBOT_11_FUTURE_WRAPPER_H_
#define MIGANGBOT_BOT_ONEBOT_11_FUTURE_WRAPPER_H_

#include <future>

#include "co_future.h"
#include "type.h"
#include "bot/onebot_11/default_value.h"

namespace white {
namespace onebot11 {

template <typename T>
class FutureWrapper {
 public:
  FutureWrapper(std::shared_ptr<std::promise<T>> &&p)
      : promise_(std::move(p)), future_(promise_->get_future()) {}

  T get(std::size_t seconds = 60) {
    auto status = future_.wait_for(std::chrono::seconds(seconds));
    switch (status) {
      case std::future_status::timeout:
      case std::future_status::deferred:
        return DefaultValue<T>();
        break;
      case std::future_status::ready:
        break;
    }
    return future_.get();
  }

 private:
  std::shared_ptr<std::promise<T>> promise_;
  std::future<T> future_;
};

template <typename T>
class CoFutureWrapper {
 public:
  CoFutureWrapper(std::shared_ptr<co_promise<T>> &&p)
      : promise_(std::move(p)), future_(std::move(promise_->get_future())) {}

  T get(uint32 ms = 60000) {
    auto status = future_.wait_for(ms);
    switch (status) {
      case co_future_status::timeout:
        return DefaultValue<T>();
        break;
      case co_future_status::ready:
        break;
    }
    return future_.get();
  }

 private:
  std::shared_ptr<co_promise<T>> promise_;
  co_future<T> future_;
};

}  // namespace onebot11
}  // namespace white
#endif
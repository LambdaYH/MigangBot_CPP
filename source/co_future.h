#ifndef MIGANGBOT_CO_FUTURE_H_
#define MIGANGBOT_CO_FUTURE_H_

#include <co/co.h>
#include <memory>

namespace white {

template <typename T>
class co_future;
template <typename T>
class co_promise;

enum class co_future_status { ready, timeout };

template <typename T>
struct shared_state {
  co::Event event_;
  T value_;
  bool is_complete_;
  shared_state(bool is_complete) : is_complete_(is_complete) {}
};

template <typename T>
class co_future {
  friend class co_promise<T>;

 public:
  // block in coroutine
  T get() {
    if (!state_->is_complete_) state_->event_.wait();
    return std::move(state_->value_);
  }

  void wait() {
    if (!state_->is_complete_) state_->event_.wait();
  }

  co_future_status wait_for(uint32 ms) {
    if (state_->is_complete_) return co_future_status::ready;
    if (state_->event_.wait(ms)) return co_future_status::ready;
    return co_future_status::timeout;
  }

  co_future(const co_future &) = delete;
  co_future &operator=(const co_future &) = delete;

  co_future(co_future &&rhs) noexcept : state_(std::move(rhs.state_)) {}

  co_future &operator=(co_future &&rhs) noexcept {
    if (&rhs != this) {
      state_ = std::move(rhs.state_);
    }
    return *this;
  }

 private:
  co_future(std::shared_ptr<shared_state<T>> state)
      : state_(std::move(state)) {}

 private:
  std::shared_ptr<shared_state<T>> state_;
};

template <typename T>
class co_promise {
 public:
  co_promise() : state_(std::make_unique<shared_state<T>>(false)) {}
  ~co_promise() {}

  co_promise(const co_promise &) = delete;
  co_promise &operator=(const co_promise &) = delete;

  co_promise(co_promise &&rhs) noexcept : state_(std::move(rhs.state_)) {}

  co_promise &operator=(co_promise &&rhs) noexcept {
    if (&rhs != this) {
      this->state_ = std::move(rhs.state_);
    }
    return *this;
  }

 public:
  co_future<T> get_future() { return co_future<T>(state_); }

  void set_value(const T &value) {
    state_->value_ = value;
    state_->is_complete_ = true;
    state_->event_.signal();
  }

  void set_value(T &value) {
    state_->value_ = value;
    state_->is_complete_ = true;
    state_->event_.signal();
  }

  void set_value(T &&value) {
    state_->value_ = std::move(value);
    state_->is_complete_ = true;
    state_->event_.signal();
  }

  void set_value() {
    state_->is_complete_ = true;
    state_->event_.signal();
  }

 private:
  std::shared_ptr<shared_state<T>> state_;
};

}  // namespace white

#endif
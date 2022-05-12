#ifndef MIGANGBOT_CLOSURE_H_
#define MIGANGBOT_CLOSURE_H_

#include <ctime>

#include <string>
#include <functional>
#include <type_traits>

#include "type.h"
#include "event/event.h"

namespace white {

// run faster than function
template <typename... Params>
class Closure {
 public:
  Closure() = default;
  virtual ~Closure() = default;

  virtual void Run(Params...) const = 0;
};

using ClosureNotify = Closure<std::string &&>;

template <typename F>
class FunctionForNotify : public ClosureNotify {
 public:
  FunctionForNotify(F &&func) : func_(std::forward<F>(func)) {}
  virtual ~FunctionForNotify() = default;

  virtual void Run(std::string &&str) const { func_(std::move(str)); }

 private:
  std::remove_reference_t<F> func_;
};

using ClosureSetEcho =
    Closure<const std::time_t, std::function<void(const Json &)> &&>;

template <typename F>
class FunctionForSetEcho : public ClosureSetEcho {
 public:
  FunctionForSetEcho(F &&func) : func_(std::forward<F>(func)) {}
  virtual ~FunctionForSetEcho() = default;

  virtual void Run(const std::time_t echo_code,
                   std::function<void(const Json &)> &&func) const {
    func_(echo_code, std::move(func));
  }

 private:
  std::remove_reference_t<F> func_;
};

}  // namespace white

#endif

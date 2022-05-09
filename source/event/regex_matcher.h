#ifndef MIGANGBOT_EVENT_EVENT_REGEX_MATCHER_H_
#define MIGANGBOT_EVENT_EVENT_REGEX_MATCHER_H_

#include <initializer_list>
#include <string_view>
#include <vector>

#include <jpcre2.hpp>

#include "event/type.h"
#include "service/service.h"

namespace white {

using jp = jpcre2::select<char>;

class RegexMatcher {
 public:
  RegexMatcher(const std::initializer_list<std::string> &patterns,
               std::shared_ptr<Service> service)
      : service_(service) {
    for (const auto &pattern : patterns)
      regex_.push_back(jp::Regex(pattern, "mSi"));
  }

  bool Check(const std::string &str) noexcept {
    for (auto &re : regex_)
      if (re.match(str)) return true;
    return false;
  }

  const std::shared_ptr<Service> &GetService() const noexcept { return service_; }

 private:
  std::vector<jp::Regex> regex_;
  const std::shared_ptr<Service> service_;
};

}  // namespace white

#endif
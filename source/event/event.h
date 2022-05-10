#ifndef MIGANGBOT_EVENT_EVENT_H_
#define MIGANGBOT_EVENT_EVENT_H_

#include <memory>

#include <nlohmann/json.hpp>

namespace white {

using Event = nlohmann::json;

using EventPtr = std::shared_ptr<nlohmann::json>;

}  // namespace white

#endif
#ifndef MIGANGBOT_SCHEDULE_SCHEDULE_H_
#define MIGANGBOT_SCHEDULE_SCHEDULE_H_

#include "Scheduler.h"

namespace white {
Bosma::Scheduler &Schedule() {
  static Bosma::Scheduler scheduler;
  return scheduler;
}
}  // namespace white

#endif
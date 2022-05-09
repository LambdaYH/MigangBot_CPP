#ifndef MIGANGBOT_SERVICE_MANAGER_H_
#define MIGANGBOT_SERVICE_MANAGER_H_

#include <map>
#include <memory>
#include <string>
#include "type.h"
#include <service/service.h>
#include <service/schedule_service.h>

namespace white{
class ServiceManager {
 public:
  static ServiceManager &GetInstance()
  {
    static ServiceManager sm;
    return sm;
  }

 public:
  bool GroupEnable(const std::string &service_name, const GId group_id, const int permission)
  {
    auto range = service_name_map_.equal_range(service_name);
    auto ret = true;
    for(auto it = range.first; it != range.second; ++it)
      ret |= it->second->GroupEnable(group_id, permission);
    auto range_s = schedule_sevice_name_map_.equal_range(service_name);
    for(auto it = range_s.first; it != range_s.second; ++it)
      ret |= it->second->GroupEnable(group_id, permission);
    return ret;
  }

  bool GroupDisable(const std::string &service_name, const GId group_id, const int permission)
  {
    auto range = service_name_map_.equal_range(service_name);
    auto ret = true;
    for(auto it = range.first; it != range.second; ++it)
      ret |= it->second->GroupDisable(group_id, permission);
    auto range_s = schedule_sevice_name_map_.equal_range(service_name);
    for(auto it = range_s.first; it != range_s.second; ++it)
      ret |= it->second->GroupDisable(group_id, permission);
    return ret;
  }

  void RegisterService(std::shared_ptr<Service> service)
  {
    service_name_map_.emplace(service->GetServiceName(), service);
  }

  void RegisterScheduleService(std::shared_ptr<ScheduleService> service)
  {
    schedule_sevice_name_map_.emplace(service->GetServiceName(), service);
  }
 public:
  ServiceManager(const ServiceManager &b) = delete;
  ServiceManager(const ServiceManager &&b) = delete;
  ServiceManager &operator=(const ServiceManager &b) = delete;
  ServiceManager &operator=(const ServiceManager &&b) = delete;

 private:
  ServiceManager() {}
  ~ServiceManager() {}

 private:
  std::multimap<const std::string, std::shared_ptr<Service>> service_name_map_;
  std::multimap<const std::string, std::shared_ptr<ScheduleService>> schedule_sevice_name_map_;
};
}

#endif
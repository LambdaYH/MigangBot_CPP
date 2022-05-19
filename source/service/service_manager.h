#ifndef MIGANGBOT_SERVICE_MANAGER_H_
#define MIGANGBOT_SERVICE_MANAGER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "type.h"
#include <service/service.h>
#include <service/triggered_service.h>
#include <service/schedule_service.h>

namespace white {
class ServiceManager {
 public:
  static ServiceManager &GetInstance() {
    static ServiceManager sm;
    return sm;
  }

 public:
  bool CheckService(const std::string &service_name) {
    return service_name_map_.contains(service_name);
  }

  std::vector<int> CheckPermission(const std::string &service_name) {
    auto range = service_name_map_.equal_range(service_name);
    std::vector<int> ret;
    for (auto it = range.first; it != range.second; ++it)
      ret.emplace_back(it->second->Permission());
    return ret;
  }

  bool GroupEnable(const std::string &service_name, const GId group_id,
                   const int permission) {
    auto range = service_name_map_.equal_range(service_name);
    auto ret = true;
    for (auto it = range.first; it != range.second; ++it)
      ret &= it->second->GroupEnable(group_id, permission);
    return ret;
  }

  bool GroupDisable(const std::string &service_name, const GId group_id,
                    const int permission) {
    auto range = service_name_map_.equal_range(service_name);
    auto ret = true;
    for (auto it = range.first; it != range.second; ++it)
      ret &= it->second->GroupDisable(group_id, permission);
    return ret;
  }

  void RegisterService(std::shared_ptr<Service> service) {
    service_name_map_.emplace(service->GetServiceName(), service);
    bundle_service_map_["通用"].emplace(service->GetServiceName(), service);
  }

  void RegisterService(const std::string &bundle_name,
                       std::shared_ptr<Service> service) {
    service_name_map_.emplace(service->GetServiceName(), service);
    bundle_service_map_[bundle_name].emplace(service->GetServiceName(),
                                             service);
  }

  auto GetServiceList() {
    std::vector<std::pair<std::string, std::string>> ret;
    for (auto &[name, sv] : service_name_map_)
      ret.emplace_back(name, sv->GetDescription());
    return ret;
  }

  auto GetServiceList(GId group_id) {
    std::vector<std::tuple<std::string, std::string, bool>> ret;
    for (auto &[name, sv] : service_name_map_)
      ret.emplace_back(name, sv->GetDescription(), sv->GroupStatus(group_id));
    return ret;
  }

  std::vector<std::pair<std::string, std::string>> GetPackageService(
      const std::string &pacakge_name) {
    if (!bundle_service_map_.count(pacakge_name)) return {};
    std::vector<std::pair<std::string, std::string>> ret;
    for (auto &[name, sv] : bundle_service_map_.at(pacakge_name))
      ret.emplace_back(name, sv->GetDescription());
    return ret;
  }

  std::vector<std::tuple<std::string, std::string, bool>> GetPackageService(
      const std::string &pacakge_name, GId group_id) {
    if (!bundle_service_map_.count(pacakge_name)) return {};
    std::vector<std::tuple<std::string, std::string, bool>> ret;
    for (auto &[name, sv] : bundle_service_map_.at(pacakge_name))
      ret.emplace_back(name, sv->GetDescription(), sv->GroupStatus(group_id));
    return ret;
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
  std::map<std::string, std::multimap<std::string, std::shared_ptr<Service>>>
      bundle_service_map_;
  std::multimap<std::string, std::shared_ptr<Service>> service_name_map_;
};
}  // namespace white

#endif
#ifndef MIGANGBOT_EVENT_REGISTRAR_H_
#define MIGANGBOT_EVENT_REGISTRAR_H_

#include "event/event_handler.h"
#include "event/type.h"
#include "logger/logger.h"
#include "service/service_manager.h"

namespace white {

template <typename... Args>
inline void OnMessage(
    const std::pair<std::string, std::string> &serv_bundle_name,
    Args &&...args) {
  auto service = std::make_shared<Service>(serv_bundle_name.first,
                                           std::forward<Args>(args)...);
  EventHandler::GetInstance().RegisterCommand(ALLMSG, "", service);
  ServiceManager::GetInstance().RegisterService(serv_bundle_name.second,
                                                service);
}

template <typename... Args>
inline void OnMessage(const std::string &service_name, Args &&...args) {
  OnMessage({service_name, "通用"}, std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnCommand(
    int type, const std::initializer_list<std::string> &commands,
    const std::pair<std::string, std::string> &serv_bundle_name,
    Args &&...args) {
  auto service = std::make_shared<TriggeredService>(
      serv_bundle_name.first, std::forward<Args>(args)...);
  for (auto &command : commands) {
    if (!EventHandler::GetInstance().RegisterCommand(type, command, service))
      LOG_WARN("注册指令[{}]失败", command);
  }
  ServiceManager::GetInstance().RegisterService(serv_bundle_name.second,
                                                service);
}

template <typename... Args>
inline void OnCommand(int type,
                      const std::initializer_list<std::string> &commands,
                      const std::string &service_name, Args &&...args) {
  OnCommand(type, commands, {service_name, "通用"},
            std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnFullmatch(const std::initializer_list<std::string> &commands,
                        const std::string &service_name, Args &&...args) {
  OnCommand(FULLMATCH, commands, service_name, std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnFullmatch(
    const std::initializer_list<std::string> &commands,
    const std::pair<std::string, std::string> &serv_bundle_name,
    Args &&...args) {
  OnCommand(FULLMATCH, commands, serv_bundle_name, std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnPrefix(const std::initializer_list<std::string> &commands,
                     const std::string &service_name, Args &&...args) {
  OnCommand(PREFIX, commands, service_name, std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnPrefix(
    const std::initializer_list<std::string> &commands,
    const std::pair<std::string, std::string> &serv_bundle_name,
    Args &&...args) {
  OnCommand(PREFIX, commands, serv_bundle_name, std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnSuffix(const std::initializer_list<std::string> &commands,
                     const std::string &service_name, Args &&...args) {
  OnCommand(SUFFIX, commands, service_name, std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnSuffix(
    const std::initializer_list<std::string> &commands,
    const std::pair<std::string, std::string> &serv_bundle_name,
    Args &&...args) {
  OnCommand(SUFFIX, commands, serv_bundle_name, std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnRegex(const std::initializer_list<std::string> &patterns,
                    const std::pair<std::string, std::string> &serv_bundle_name,
                    Args &&...args) {
  auto service = std::make_shared<TriggeredService>(
      serv_bundle_name.first, std::forward<Args>(args)...);
  EventHandler::GetInstance().RegisterRegex(patterns, service);
  ServiceManager::GetInstance().RegisterService(serv_bundle_name.second,
                                                service);
}

template <typename... Args>
inline void OnRegex(const std::initializer_list<std::string> &patterns,
                    const std::string &service_name, Args &&...args) {
  OnRegex(patterns, {service_name, "通用正则"}, std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnNotice(
    const std::string &notice_type, const std::string &sub_type,
    const std::pair<std::string, std::string> &serv_bundle_name,
    Args &&...args) {
  auto service = std::make_shared<TriggeredService>(
      serv_bundle_name.first, std::forward<Args>(args)...);
  EventHandler::GetInstance().RegisterNotice(notice_type, sub_type, service);
  ServiceManager::GetInstance().RegisterService(serv_bundle_name.second,
                                                service);
}

template <typename... Args>
inline void OnNotice(const std::string &notice_type,
                     const std::string &sub_type,
                     const std::string &service_name, Args &&...args) {
  OnNotice(notice_type, sub_type, {service_name, "通用通知"},
           std::forward<Args>(args)...);
}

template <typename... Args>
inline void OnRequest(
    const std::string &request_type, const std::string &sub_type,
    const std::pair<std::string, std::string> &serv_bundle_name,
    Args &&...args) {
  auto service = std::make_shared<TriggeredService>(
      serv_bundle_name.first, std::forward<Args>(args)...);
  EventHandler::GetInstance().RegisterRequest(request_type, sub_type, service);
  ServiceManager::GetInstance().RegisterService(serv_bundle_name.second,
                                                service);
}

template <typename... Args>
inline void OnRequest(const std::string &request_type,
                      const std::string &sub_type,
                      const std::string &service_name, Args &&...args) {
  OnRequest(request_type, sub_type, {service_name, "通用请求"},
            std::forward<Args>(args)...);
}

template <typename... Args>
inline std::shared_ptr<ScheduleService> OnSchedule(
    const std::pair<std::string, std::string> &serv_bundle_name,
    Args &&...args) {
  auto service = std::make_shared<ScheduleService>(serv_bundle_name.first,
                                                   std::forward<Args>(args)...);
  ServiceManager::GetInstance().RegisterService(serv_bundle_name.second,
                                                service);
  return service;
}

template <typename... Args>
inline std::shared_ptr<ScheduleService> OnSchedule(
    const std::string &service_name, Args &&...args) {
  return OnSchedule({service_name, "通用定时"}, std::forward<Args>(args)...);
}

}  // namespace white

#endif
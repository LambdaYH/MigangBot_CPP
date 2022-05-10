#ifndef MIGANGBOT_EVENT_REGISTRAR_H_
#define MIGANGBOT_EVENT_REGISTRAR_H_

#include "event/event_handler.h"
#include "logger/logger.h"
#include "service/service_manager.h"

namespace white {

template <typename... Args>
inline void OnMessage(Args &&...args) {
  auto service = std::make_shared<Service>(std::forward<Args>(args)...);
  EventHandler::GetInstance().RegisterCommand(ALLMSG, "", service);
  ServiceManager::GetInstance().RegisterService(service);
}

template <typename... Args>
inline void OnFullmatch(const std::initializer_list<std::string> &commands,
                        Args &&...args) {
  auto service =
      std::make_shared<TriggeredService>(std::forward<Args>(args)...);
  for (auto &command : commands) {
    if (!EventHandler::GetInstance().RegisterCommand(FULLMATCH, command,
                                                     service))
      LOG_WARN("注册指令[{}]失败", command);
  }
  ServiceManager::GetInstance().RegisterService(service);
}

template <typename... Args>
inline void OnPrefix(const std::initializer_list<std::string> &commands,
                     Args &&...args) {
  auto service =
      std::make_shared<TriggeredService>(std::forward<Args>(args)...);
  for (auto &command : commands) {
    if (!EventHandler::GetInstance().RegisterCommand(PREFIX, command, service))
      LOG_WARN("注册指令[{}]失败", command);
  }
  ServiceManager::GetInstance().RegisterService(service);
}

template <typename... Args>
inline void OnSuffix(const std::initializer_list<std::string> &commands,
                     Args &&...args) {
  auto service =
      std::make_shared<TriggeredService>(std::forward<Args>(args)...);
  for (auto &command : commands) {
    if (!EventHandler::GetInstance().RegisterCommand(SUFFIX, command, service))
      LOG_WARN("注册指令[{}]失败", command);
  }
  ServiceManager::GetInstance().RegisterService(service);
}

template <typename... Args>
inline void OnRegex(const std::initializer_list<std::string> &patterns,
                    Args &&...args) {
  auto service =
      std::make_shared<TriggeredService>(std::forward<Args>(args)...);
  EventHandler::GetInstance().RegisterRegex(patterns, service);
  ServiceManager::GetInstance().RegisterService(service);
}

template <typename... Args>
inline void OnNotice(const std::string &notice_type,
                     const std::string &sub_type, Args &&...args) {
  auto service =
      std::make_shared<TriggeredService>(std::forward<Args>(args)...);
  EventHandler::GetInstance().RegisterNotice(notice_type, sub_type, service);
  ServiceManager::GetInstance().RegisterService(service);
}

template <typename... Args>
inline void OnRequest(const std::string &request_type,
                      const std::string &sub_type, Args &&...args) {
  auto service =
      std::make_shared<TriggeredService>(std::forward<Args>(args)...);
  EventHandler::GetInstance().RegisterRequest(request_type, sub_type, service);
  ServiceManager::GetInstance().RegisterService(service);
}

}  // namespace white

#endif
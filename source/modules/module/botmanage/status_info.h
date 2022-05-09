#pragma once

#include "modules/module_interface.h"

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <chrono>
#include <filesystem>
#include <mutex>

#include "aiorequests.h"
#include "message/message_segment.h"
#include "message/utility.h"

namespace white {
namespace module {
namespace status_info {
inline std::string GetLatency(const Event &event) {
  return fmt::format("[延迟] {:0.2f}s", datetime::GetTimeStampMs() / 1000.0 -
                                            event["time"].get<double>());
}

inline std::string GetMemoryStatus() {
  struct sysinfo mem_info;
  sysinfo(&mem_info);
  long long total_virtual_mem = mem_info.totalram;
  total_virtual_mem += mem_info.totalswap;
  total_virtual_mem *= mem_info.mem_unit;
  long long virtual_mem_used =
      mem_info.totalram - mem_info.freeram - mem_info.bufferram;
  virtual_mem_used += mem_info.totalswap - mem_info.freeswap;
  virtual_mem_used *= mem_info.mem_unit;

  return fmt::format("[内存] 使用率: {:0.1f}%",
                     100 * ((double)virtual_mem_used / total_virtual_mem));
}

inline std::string GetNetworkStatus() {
  int baidu_status, google_status;
  auto resp_baidu = aiorequests::Get("https://www.baidu.com", 5);
  if (!resp_baidu)
    baidu_status = 404;
  else
    baidu_status = resp_baidu->status_code;

  auto resp_google = aiorequests::Get("https://www.google.com", 5);
  if (!resp_google)
    google_status = 404;
  else
    google_status = resp_google->status_code;

  return fmt::format("[网络] Baidu: {} | Google: {}", baidu_status,
                     google_status);
}

inline std::string GetDiskStatus() {
  auto space_info = std::filesystem::space("/");
  double space_cap = space_info.capacity / 1024.0 / 1024;
  double space_avl = space_info.available / 1024.0 / 1024;
  return fmt::format("[硬盘] 使用率：{:0.2f}% 剩余空间：{:0.2f}G",
                     100 * (space_cap - space_avl) / space_cap,
                     space_avl / 1024);
}
}  // namespace status_info

class StatusInfo : public Module {
 public:
  StatusInfo() : Module() {
    FILE *file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &last_total_user_,
           &last_total_user_low, &last_total_sys_, &last_total_idle_);
    fclose(file);
  }
  virtual void Register();
  void Ping(const Event &event, onebot11::ApiBot &bot);
  void Status(const Event &event, onebot11::ApiBot &bot);
  void Network(const Event &event, onebot11::ApiBot &bot);

 public:
  std::string GetCPUStatus();

 private:
  std::mutex mutex_;
  unsigned long long last_total_user_;
  unsigned long long last_total_user_low;
  unsigned long long last_total_sys_;
  unsigned long long last_total_idle_;
};

inline void StatusInfo::Register() {
  RegisterCommand(FULLMATCH, {"/ping"}, ACT(StatusInfo::Ping));
  RegisterCommand(FULLMATCH, {"status", "状态"}, ACT(StatusInfo::Status),
                  permission::SUPERUSER);
  RegisterCommand(FULLMATCH, {"network", "网络状况"}, ACT(StatusInfo::Network),
                  permission::SUPERUSER);
}

inline void StatusInfo::Ping(const Event &event, onebot11::ApiBot &bot) {
  bot.send_msg(event, fmt::format("[MigangBot]\n{}\n{}", GetCPUStatus(),
                                  status_info::GetLatency(event)));
}

inline void StatusInfo::Status(const Event &event, onebot11::ApiBot &bot) {
  bot.send_msg(event, fmt::format("[MigangBot]\n{}\n{}\n{}\n{}",
                                  status_info::GetDiskStatus(), GetCPUStatus(),
                                  status_info::GetMemoryStatus(),
                                  status_info::GetLatency(event)));
}

inline void StatusInfo::Network(const Event &event, onebot11::ApiBot &bot) {
  bot.send_msg(event, status_info::GetNetworkStatus());
}

// https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
inline std::string StatusInfo::GetCPUStatus() {
  double percent;
  FILE *file;
  unsigned long long total_user, total_user_low, total_sys, total_idle, total;

  file = fopen("/proc/stat", "r");
  fscanf(file, "cpu %llu %llu %llu %llu", &total_user, &total_user_low,
         &total_sys, &total_idle);
  fclose(file);

  if (total_user < last_total_user_ || total_user_low < last_total_user_low ||
      total_sys < last_total_sys_ || total_idle < last_total_idle_) {
    // Overflow detection. Just skip this value.
    percent = -1.0;
  } else {
    total = (total_user - last_total_user_) +
            (total_user_low - last_total_user_low) +
            (total_sys - last_total_sys_);
    percent = total;
    total += (total_idle - last_total_idle_);
    percent /= total;
    percent *= 100;
  }

  {
    std::lock_guard<std::mutex> locker(mutex_);
    last_total_user_ = total_user;
    last_total_user_low = total_user_low;
    last_total_sys_ = total_sys;
    last_total_idle_ = total_idle;
  }

  return fmt::format("[CPU] 使用率：{:0.1f}%", percent);
}

}  // namespace module
}  // namespace white
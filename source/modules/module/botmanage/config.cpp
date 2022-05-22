#include "modules/module/botmanage/config.h"

#include <unordered_map>

#include <fmt/format.h>

#include "type.h"
#include <database/mysql_wrapper.h>

namespace white {
namespace module {
namespace botmanage {

bool add_friend_flag;
bool add_group_flag;
bool reject_group_force_flag;
std::string help_msg_group_;
std::string help_msg_friend_;
int auto_clean_after;

std::unordered_map<QId, std::string> Qid_blacklist;
std::unordered_map<GId, std::string> Gid_blacklist;

void CreateQQTable() {
  sql::MySQLWrapper sql_wrapper;
  std::string query =
      "CREATE TABLE IF NOT EXISTS BlackListQQ\n"
      "(UID        INT             NOT NULL ,\n"
      "reason      VARCHAR(255)    NOT NULL,\n"
      "PRIMARY KEY(UID))";
  int ec;
  sql_wrapper.Execute(query, &ec);
  if (ec) {
    LOG_ERROR("Botmanage_qqblackList: 创建表发生错误。code: {}", ec);
    return;
  }
}

void CreateGroupTable() {
  sql::MySQLWrapper sql_wrapper;
  std::string query =
      "CREATE TABLE IF NOT EXISTS BlackListGroup\n"
      "(GID        INT             NOT NULL ,\n"
      "reason      VARCHAR(255)    NOT NULL,\n"
      "PRIMARY KEY(GID))";
  int ec;
  sql_wrapper.Execute(query, &ec);
  if (ec) {
    LOG_ERROR("Botmanage_qqblackList: 创建表发生错误。code: {}", ec);
    return;
  }
}

void LoadFromQQTable() {
  auto all_bLack_qq = sql::MySQLWrapper()
                          .Execute("SELECT UID, reason FROM BlackListQQ")
                          .FetchAll();
  for (auto &qq : all_bLack_qq) {
    Qid_blacklist.emplace(stoull(qq[0]), qq[1]);
  }
}

void LoadFromGroupTable() {
  auto all_bLack_group = sql::MySQLWrapper()
                             .Execute("SELECT GID, reason FROM BlackListGroup")
                             .FetchAll();
  for (auto &group : all_bLack_group) {
    Gid_blacklist.emplace(stoull(group[0]), group[1]);
  }
}

bool AddToQQTable(QId uid, const std::string &reason) {
  int ec;
  sql::MySQLWrapper().Execute(fmt::format("REPLACE INTO BlackListGroup\n"
                                          "VALUES(\n"
                                          "{},\n"
                                          "\"{}\")",
                                          uid, reason),
                              &ec);
  if (ec) return false;
  return true;
}

bool AddToGroupTable(GId gid, const std::string &reason) {
  int ec;
  sql::MySQLWrapper().Execute(fmt::format("REPLACE INTO BlackListQQ\n"
                                          "VALUES(\n"
                                          "{},\n"
                                          "\"{}\")",
                                          gid, reason),
                              &ec);
  if (ec) return false;
  return true;
}

bool DelFromQQTable(QId uid) {
  int ec;
  sql::MySQLWrapper().Execute(
      fmt::format("DELETE FROM BlackListQQ WHERE UID = {}", uid), &ec);
  if (ec) return false;
  return true;
}

bool DelFromGroupTable(GId gid) {
  int ec;
  sql::MySQLWrapper().Execute(
      fmt::format("DELETE FROM BlackListGroup WHERE GID = {}", gid), &ec);
  if (ec) return false;
  return true;
}

bool DelFromQQBlackList(QId uid) {
  if (!Qid_blacklist.contains(uid)) return false;
  Qid_blacklist.erase(uid);
  DelFromQQTable(uid);
  return true;
}

bool DelFromGroupBlackList(GId gid) {
  if (!Gid_blacklist.contains(gid)) return false;
  Gid_blacklist.erase(gid);
  DelFromGroupTable(gid);
  return true;
}

void Init() {
  CreateQQTable();
  CreateGroupTable();
  LoadFromQQTable();
  LoadFromGroupTable();
}

void AddToQQBlackList(QId uid, const std::string &reason) {
  AddToQQTable(uid, reason);
  Qid_blacklist[uid] = reason;
}

void AddToGroupBlackList(GId gid, const std::string &reason) {
  AddToGroupTable(gid, reason);
  Gid_blacklist[gid] = reason;
}

const std::unordered_map<QId, std::string> &GetQQBlackList() {
  return Qid_blacklist;
}

const std::unordered_map<GId, std::string> &GetGroupBlackList() {
  return Gid_blacklist;
}

bool IsBlackGroup(GId gid) { return Gid_blacklist.count(gid); }

bool IsBlackUser(QId uid) { return Qid_blacklist.count(uid); }

}  // namespace botmanage
}  // namespace module
}  // namespace white
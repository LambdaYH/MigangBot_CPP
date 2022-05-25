#include "modules/module/botmanage/config.h"

#include <unordered_map>

#include <fmt/format.h>

#include "db/db_orm.h"
#include "sqlpp11/exception.h"
#include "sqlpp11/remove.h"
#include "sqlpp11/verbatim.h"
#include "type.h"
#include "db/db.h"

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
  try {
    mariadb::DB().Execute(
        "CREATE TABLE IF NOT EXISTS BlackListQQ\n"
        "(UID        INT             NOT NULL ,\n"
        "reason      VARCHAR(255)    NOT NULL,\n"
        "PRIMARY KEY(UID))");
  } catch (const sqlpp::exception &e) {
    LOG_ERROR("Botmanage_qqblackList: 创建表发生错误。code: {}", e.what());
    return;
  }
}

void CreateGroupTable() {
  try {
    mariadb::DB().Execute(
        "CREATE TABLE IF NOT EXISTS BlackListGroup\n"
        "(GID        INT             NOT NULL ,\n"
        "reason      VARCHAR(255)    NOT NULL,\n"
        "PRIMARY KEY(GID))");
  } catch (const sqlpp::exception &e) {
    LOG_ERROR("Botmanage_groupblackList: 创建表发生错误。code: {}", e.what());
    return;
  }
}

void LoadFromQQTable() {
  db::BlackListQQ bq;
  for (const auto &qq : mariadb::DB()(
           sqlpp::select(bq.UID, bq.reason).from(bq).unconditionally())) {
    Qid_blacklist.emplace(qq.UID, qq.reason);
  }
}

void LoadFromGroupTable() {
  db::BlackListGroup bg;
  for (const auto &group : mariadb::DB()(
           sqlpp::select(bg.GID, bg.reason).from(bg).unconditionally())) {
    Gid_blacklist.emplace(group.GID, group.reason);
  }
}

bool AddToQQTable(QId uid, const std::string &reason) {
  mariadb::DB db;
  db::BlackListQQ bq;
  try {
    if (!db(sqlpp::select(bq.UID).from(bq).where(bq.UID == uid)).empty())
      db(sqlpp::update(bq).set(bq.reason = reason).where(bq.UID == uid));
    else
      db(sqlpp::insert_into(bq).set(bq.UID = uid, bq.reason = reason));
  } catch (const sqlpp::exception &e) {
    return false;
  }
  return true;
}

bool AddToGroupTable(GId gid, const std::string &reason) {
  mariadb::DB db;
  db::BlackListGroup bg;
  try {
    if (!db(sqlpp::select(bg.GID).from(bg).where(bg.GID == gid)).empty())
      db(sqlpp::update(bg).set(bg.reason = reason).where(bg.GID == gid));
    else
      db(sqlpp::insert_into(bg).set(bg.GID = gid, bg.reason = reason));
  } catch (const sqlpp::exception &e) {
    return false;
  }
  return true;
}

bool DelFromQQTable(QId uid) {
  db::BlackListQQ bq;
  try {
    mariadb::DB()(sqlpp::remove_from(bq).where(bq.UID == uid));
  } catch (const sqlpp::exception &e) {
    return false;
  }
  return true;
}

bool DelFromGroupTable(GId gid) {
  db::BlackListGroup bg;
  try {
    mariadb::DB()(sqlpp::remove_from(bg).where(bg.GID == gid));
  } catch (const sqlpp::exception &e) {
    return false;
  }
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
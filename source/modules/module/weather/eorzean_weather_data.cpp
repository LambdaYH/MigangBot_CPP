#include <filesystem>
#include <stdexcept>

#include "db/db_orm.h"
#include "nlohmann/json.hpp"

#include "message/utility.h"
#include "modules/module_interface.h"
#include "sqlpp11/insert.h"
#include "sqlpp11/select.h"
#include "tools/aiorequests.h"
#include "tools/csv.hpp"
#include "logger/logger.h"
#include "db/db.h"
#include "modules/module/weather/eorzean_weather_data.h"
#include "global_config.h"
#include "type.h"

namespace white {
namespace eorzean_weather {

std::unordered_map<std::string, std::string> alter_name_idx;
std::vector<std::string> weather_idx;
std::unordered_map<std::string, int> location_idx;
std::unordered_map<int, std::vector<WeatherRate>> weather_rate_idx;

void InitWeatherTable() {
  try {
    mariadb::DB().Execute(
        "CREATE TABLE IF NOT EXISTS Weather\n"
        "(weather_id     INT            NOT NULL AUTO_INCREMENT,\n"
        "weather         varchar(15)    NOT NULL,\n"
        "PRIMARY KEY(weather_id))");
  } catch (const sqlpp::exception &e) {
    LOG_ERROR("Weather: 创建表发生错误。code: {}", e.what());
    return;
  }
}

void InitLocationTable() {
  try {
    mariadb::DB().Execute(
        "CREATE TABLE IF NOT EXISTS Location\n"
        "(id     INT          NOT NULL AUTO_INCREMENT,\n"
        "location             varchar(30)    NOT NULL,\n"
        "alter_name           varchar(255) DEFAULT \"[]\" NOT NULL,\n"
        "weather_rate         INT    NOT NULL,\n"
        "PRIMARY KEY(id))");
  } catch (const sqlpp::exception &e) {
    LOG_ERROR("Weather: 创建表发生错误。code: {}", e.what());
    return;
  }
}

void InitWeatherRateTable() {
  try {
    mariadb::DB().Execute(
        "CREATE TABLE IF NOT EXISTS WeatherRate\n"
        "(weather_rate     INT             NOT NULL,\n"
        "w_r               varchar(255) DEFAULT \"[]\" NOT NULL,\n"
        "PRIMARY KEY(weather_rate))");
  } catch (const sqlpp::exception &e) {
    LOG_ERROR("Weather: 创建表发生错误。code: {}", e.what());
    return;
  }
}

constexpr auto TerritoryType_url =
    "https://raw.githubusercontent.com/thewakingsands/ffxiv-datamining-cn/"
    "master/TerritoryType.csv";
constexpr auto PlaceName_url =
    "https://raw.githubusercontent.com/thewakingsands/ffxiv-datamining-cn/"
    "master/PlaceName.csv";
constexpr auto WeatherRate_url =
    "https://raw.githubusercontent.com/thewakingsands/ffxiv-datamining-cn/"
    "master/WeatherRate.csv";
constexpr auto Weather_url =
    "https://raw.githubusercontent.com/thewakingsands/ffxiv-datamining-cn/"
    "master/Weather.csv";

void Init() {
  InitWeatherTable();
  InitLocationTable();
  InitWeatherRateTable();
  db::Weather w_table;
  for (const auto &row : mariadb::DB()(sqlpp::select(w_table.weather)
                                           .from(w_table)
                                           .order_by(w_table.weatherId.asc())
                                           .unconditionally()))
    weather_idx.emplace_back(row.weather);

  db::WeatherRate wr_table;
  for (const auto &row :
       mariadb::DB()(sqlpp::select(wr_table.weatherRate, wr_table.wR)
                         .from(wr_table)
                         .unconditionally())) {
    auto rate = row.weatherRate;
    auto json_list = Json::parse(std::string(row.wR));
    for (auto &w : json_list)
      weather_rate_idx[rate].emplace_back(w["weather"].get<int>(),
                                          w["rate"].get<int>());
  }
  db::Location loc_table;
  for (const auto &row :
       mariadb::DB()(sqlpp::select(loc_table.location, loc_table.alterName,
                                   loc_table.weatherRate)
                         .from(loc_table)
                         .unconditionally())) {
    location_idx.emplace(row.location, row.weatherRate);
    auto alter_name = Json::parse(std::string(row.alterName));
    for (auto &name : alter_name) alter_name_idx.emplace(name, row.location);
  }
}

void UpdateWeather() {
  auto r = aiorequests::Get(Weather_url).get();
  if (!r) throw std::runtime_error("无法获取Weather.csv");
  auto csv_file_ss = std::stringstream(r->Body());
  std::string line;
  weather_idx.clear();
  csv::CSVReader reader(csv_file_ss);
  for (std::size_t i = 0; auto &row : reader) {
    if (i++ < 2) continue;
    weather_idx.push_back(row[2].get<std::string>());
  }
  mariadb::DB().Execute("TRUNCATE TABLE Weather");
  db::Weather w_tb;
  for (auto &weather : weather_idx)
    mariadb::DB()(sqlpp::insert_into(w_tb).set(w_tb.weather = weather));
}

void UpdateWeatherRate() {
  auto r = aiorequests::Get(WeatherRate_url).get();
  if (!r) throw std::runtime_error("无法获取WeatherRate.csv");
  auto csv_file_ss = std::stringstream(r->Body());
  csv::CSVReader reader(csv_file_ss);
  mariadb::DB().Execute("TRUNCATE TABLE WeatherRate");
  db::WeatherRate wr_tb;
  for (std::size_t i = 0; auto &row : reader) {
    if (i++ < 2) continue;
    auto rate = row[0].get<int>();
    int tmp = 0;
    Json list;
    for (std::size_t i = 0; i < row.size() - 1; ++i) {
      if (tmp == 100) break;
      if (i & 1) {
        tmp += row[i + 1].get<int>();
        weather_rate_idx[rate].emplace_back(row[i].get<int>(), tmp);
        list.push_back(Json::parse("{" +
                                   fmt::format("\"weather\": {}, \"rate\": {}",
                                               row[i].get<int>(), tmp) +
                                   "}"));
      }
    }
    mariadb::DB()(sqlpp::insert_into(wr_tb).set(wr_tb.weatherRate = rate,
                                                wr_tb.wR = list.dump()));
  }
}

std::unordered_map<int, std::string> GetCodeLocation() {
  std::unordered_map<int, std::string> ret;
  auto r = aiorequests::Get(PlaceName_url).get();
  if (!r) throw std::runtime_error("无法获取PlaceName.csv");
  auto csv_file_ss = std::stringstream(r->Body());
  csv::CSVReader reader(csv_file_ss);
  for (std::size_t i = 0; auto &row : reader) {
    if (i++ < 2) continue;
    ret.emplace(row[0].get<int>(), row[1].get<std::string>());
  }
  return ret;
}

std::unordered_map<std::string, Json> GetAlterName() {
  std::unordered_map<std::string, Json> ret;
  for (auto &[alter, name] : alter_name_idx) ret[name].push_back(alter);
  return ret;
}

void UpdateTerritoryInfo() {
  auto r = aiorequests::Get(TerritoryType_url).get();
  if (!r) throw std::runtime_error("无法获取TerritoryType.csv");
  auto csv_file_ss = std::stringstream(r->Body());
  csv::CSVReader reader(csv_file_ss);
  auto code_location = GetCodeLocation();
  auto alter_name = GetAlterName();
  mariadb::DB().Execute("TRUNCATE TABLE Location");
  db::Location loc_tb;
  for (std::size_t i = 0; auto &row : reader) {
    if (i++ < 2) continue;
    if (row[6].get<int>() != 0) {
      if (code_location.contains(row[6].get<int>())) {
        auto location_name = code_location.at(row[6].get<int>());
        if (location_idx.contains(location_name) ||
            row[1].get<std::string>().find('_') != std::string::npos)
          continue;
        location_idx.emplace(location_name, row[13].get<int>());
        mariadb::DB()(sqlpp::insert_into(loc_tb).set(
            loc_tb.location = location_name,
            loc_tb.alterName = alter_name.count(location_name)
                                   ? alter_name.at(location_name).dump()
                                   : "[]",
            loc_tb.weatherRate = row[13].get<int>()));
      }
    }
  }
}

bool Update() {
  try {
    Init();
    UpdateWeather();
    UpdateTerritoryInfo();
    UpdateWeatherRate();
  } catch (const std::exception &e) {
    LOG_ERROR("更新艾欧泽亚天气数据失败: {}", e.what());
    return false;
  }
  return true;
}

}  // namespace eorzean_weather
}  // namespace white
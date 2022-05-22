#include <filesystem>
#include <stdexcept>

#include "nlohmann/json.hpp"

#include "message/utility.h"
#include "modules/module_interface.h"
#include "tools/aiorequests.h"
#include "tools/csv.hpp"
#include "logger/logger.h"
#include "database/mysql_wrapper.h"
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
  sql::MySQLWrapper sql_wrapper;
  std::string query =
      "CREATE TABLE IF NOT EXISTS Weather\n"
      "(weather_id     INT            NOT NULL AUTO_INCREMENT,\n"
      "weather         varchar(15)    NOT NULL,\n"
      "PRIMARY KEY(weather_id))";
  int ec;
  sql_wrapper.Execute(query, &ec);
  if (ec) {
    LOG_ERROR("Weather: 创建表发生错误。code: {}", ec);
    return;
  }
}

void InitLocationTable() {
  sql::MySQLWrapper sql_wrapper;
  std::string query =
      "CREATE TABLE IF NOT EXISTS Location\n"
      "(id     INT          NOT NULL AUTO_INCREMENT,\n"
      "location             varchar(30)    NOT NULL,\n"
      "alter_name           varchar(255) DEFAULT \"[]\" NOT NULL,\n"
      "weather_rate         INT    NOT NULL,\n"
      "PRIMARY KEY(id))";
  int ec;
  sql_wrapper.Execute(query, &ec);
  if (ec) {
    LOG_ERROR("Weather: 创建表发生错误。code: {}", ec);
    return;
  }
}

void InitWeatherRateTable() {
  sql::MySQLWrapper sql_wrapper;
  std::string query =
      "CREATE TABLE IF NOT EXISTS WeatherRate\n"
      "(weather_rate     INT             NOT NULL,\n"
      "w_r               varchar(255) DEFAULT \"[]\" NOT NULL,\n"
      "PRIMARY KEY(weather_rate))";
  int ec;
  sql_wrapper.Execute(query, &ec);
  if (ec) {
    LOG_ERROR("Weather: 创建表发生错误。code: {}", ec);
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
  auto weathers =
      sql::MySQLWrapper()
          .Execute("SELECT weather FROM Weather ORDER BY weather_id")
          .FetchAll();
  for (auto &weather : weathers) weather_idx.push_back(weather[0]);
  auto weather_rate = sql::MySQLWrapper()
                          .Execute("SELECT weather_rate, w_r FROM WeatherRate")
                          .FetchAll();
  for (auto &rate : weather_rate) {
    auto r = std::stoi(rate[0]);
    auto json_list = Json::parse(rate[1]);
    for (auto &w : json_list)
      weather_rate_idx[r].emplace_back(json_list["weather"].get<int>(),
                                       json_list["rate"].get<int>());
  }
  auto location =
      sql::MySQLWrapper()
          .Execute("SELECT location, alter_name, weather_rate FROM Location")
          .FetchAll();
  for (auto &loc : location) {
    location_idx.emplace(loc[0], std::stoi(loc[2]));
    auto alter_name = Json::parse(loc[1]);
    for (auto &name : alter_name) alter_name_idx.emplace(name, loc[0]);
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
  sql::MySQLWrapper().Execute("TRUNCATE TABLE Weather");
  for (auto &weather : weather_idx)
    sql::MySQLWrapper().Execute(
        fmt::format("INSERT INTO Weather(weather)\n"
                    "VALUES(\n"
                    "\"{}\")",
                    weather));
}

void UpdateWeatherRate() {
  auto r = aiorequests::Get(WeatherRate_url).get();
  if (!r) throw std::runtime_error("无法获取WeatherRate.csv");
  auto csv_file_ss = std::stringstream(r->Body());
  csv::CSVReader reader(csv_file_ss);
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
    sql::MySQLWrapper().Execute(
        fmt::format("REPLACE INTO WeatherRate(weather_rate, w_r)\n"
                    "VALUES(\n"
                    "{}\n"
                    "\"{}\")",
                    rate, list.dump()));
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
  for (std::size_t i = 0; auto &row : reader) {
    if (i++ < 2) continue;
    if (row[6].get<int>() != 0) {
      if (code_location.contains(row[6].get<int>())) {
        auto location_name = code_location.at(row[6].get<int>());
        if (location_idx.contains(location_name) ||
            row[1].get<std::string>().find('_') != std::string::npos)
          continue;
        location_idx.emplace(location_name, row[13].get<int>());
        sql::MySQLWrapper().Execute(fmt::format(
            "REPLACE INTO Location(location, alter_name, weather_rate)\n"
            "VALUES(\n"
            "\"{}\"\n"
            "\"{}\"\n"
            "{})",
            location_name, alter_name[location_name].dump(),
            row[13].get<int>()));
      }
    }
  }
}

void Update() {
  try {
    Init();
    UpdateWeather();
    UpdateTerritoryInfo();
    UpdateWeatherRate();
  } catch (const std::exception &e) {
    LOG_ERROR("更新艾欧泽亚天气数据失败: {}", e.what());
  }
}

}  // namespace eorzean_weather
}  // namespace white
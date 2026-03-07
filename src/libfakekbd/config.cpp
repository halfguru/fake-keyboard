#include "libfakekbd/config.hpp"
#include <fstream>
#include <spdlog/spdlog.h>

namespace fakekbd {

auto
config_parser::from_file(std::string_view path) -> std::optional<config>
{
  try {
    std::ifstream file{ std::string(path) };
    if (!file.is_open()) {
      spdlog::error("Failed to open config file: {}", path);
      return std::nullopt;
    }

    nlohmann::json json;
    file >> json;
    return from_json(json);
  } catch (nlohmann::json::exception const& e) {
    spdlog::error("JSON parse error: {}", e.what());
    return std::nullopt;
  }
}

auto
config_parser::from_json(std::string_view json_str) -> std::optional<config>
{
  try {
    auto json = nlohmann::json::parse(json_str);
    return from_json(json);
  } catch (nlohmann::json::exception const& e) {
    spdlog::error("JSON parse error: {}", e.what());
    return std::nullopt;
  }
}

auto
config_parser::from_json(nlohmann::json const& json) -> std::optional<config>
{
  try {
    config cfg;

    if (json.contains("device")) {
      cfg.device = parse_device_config(json["device"]);
    }

    if (json.contains("bluetooth")) {
      cfg.bluetooth = parse_bluetooth_config(json["bluetooth"]);
    }

    return cfg;
  } catch (nlohmann::json::exception const& e) {
    spdlog::error("JSON parse error: {}", e.what());
    return std::nullopt;
  }
}

auto
config_parser::parse_device_config(nlohmann::json const& json) -> device_config
{
  device_config cfg;

  if (json.contains("name")) {
    cfg.name = json["name"].get<std::string>();
  }

  if (json.contains("vendor_id")) {
    cfg.vendor_id = json["vendor_id"].get<uint16_t>();
  }

  if (json.contains("product_id")) {
    cfg.product_id = json["product_id"].get<uint16_t>();
  }

  if (json.contains("version")) {
    cfg.version = json["version"].get<uint16_t>();
  }

  return cfg;
}

auto
config_parser::parse_bluetooth_config(nlohmann::json const& json) -> bluetooth_config
{
  bluetooth_config cfg;

  if (json.contains("adapter")) {
    cfg.adapter = json["adapter"].get<std::string>();
  }

  if (json.contains("auto_connect")) {
    cfg.auto_connect = json["auto_connect"].get<bool>();
  }

  if (json.contains("trusted_devices")) {
    cfg.trusted_devices = json["trusted_devices"].get<std::vector<std::string>>();
  }

  return cfg;
}

}

#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fakekbd {

struct device_config
{
  std::string name = "Fake Keyboard";
  uint16_t vendor_id = 0x1234;
  uint16_t product_id = 0x5678;
  uint16_t version = 0x0001;
};

struct bluetooth_config
{
  std::string adapter = "hci0";
  bool auto_connect = false;
  std::vector<std::string> trusted_devices;
};

struct config
{
  device_config device;
  bluetooth_config bluetooth;
};

class config_parser
{
public:
  static auto from_file(std::string_view path) -> std::optional<config>;
  static auto from_json(std::string_view json_str) -> std::optional<config>;
  static auto from_json(nlohmann::json const& json) -> std::optional<config>;

private:
  static auto parse_device_config(nlohmann::json const& json) -> device_config;
  static auto parse_bluetooth_config(nlohmann::json const& json) -> bluetooth_config;
};

}

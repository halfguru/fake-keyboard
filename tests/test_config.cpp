#include <catch2/catch_test_macros.hpp>
#include <config.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

using namespace fakekbd;

TEST_CASE("config_parser parses valid JSON string", "[unit][config]")
{
  auto config = config_parser::from_json(std::string_view(R"({
    "device": {
      "name": "Test Keyboard",
      "vendor_id": 43981,
      "product_id": 4660,
      "version": 2
    },
    "bluetooth": {
      "adapter": "hci1",
      "auto_connect": true,
      "trusted_devices": ["00:11:22:33:44:55"]
    }
  })"));
  REQUIRE(config.has_value());
  REQUIRE(config->device.name == "Test Keyboard");
  REQUIRE(config->device.vendor_id == 0xABCD);
  REQUIRE(config->device.product_id == 0x1234);
  REQUIRE(config->device.version == 0x0002);
  REQUIRE(config->bluetooth.adapter == "hci1");
  REQUIRE(config->bluetooth.auto_connect == true);
  REQUIRE(config->bluetooth.trusted_devices.size() == 1);
  REQUIRE(config->bluetooth.trusted_devices[0] == "00:11:22:33:44:55");
}

TEST_CASE("config_parser uses default values for missing fields", "[unit][config]")
{
  auto config = config_parser::from_json(std::string_view(R"({})"));
  REQUIRE(config.has_value());
  REQUIRE(config->device.name == "Fake Keyboard");
  REQUIRE(config->device.vendor_id == 0x1234);
  REQUIRE(config->device.product_id == 0x5678);
  REQUIRE(config->device.version == 0x0001);
  REQUIRE(config->bluetooth.adapter == "hci0");
  REQUIRE(config->bluetooth.auto_connect == false);
  REQUIRE(config->bluetooth.trusted_devices.empty());
}

TEST_CASE("config_parser handles partial config", "[unit][config]")
{
  auto config = config_parser::from_json(std::string_view(R"({
    "device": {
      "name": "Partial Keyboard"
    }
  })"));
  REQUIRE(config.has_value());
  REQUIRE(config->device.name == "Partial Keyboard");
  REQUIRE(config->device.vendor_id == 0x1234);
  REQUIRE(config->bluetooth.adapter == "hci0");
}

TEST_CASE("config_parser returns nullopt for invalid JSON", "[unit][config]")
{
  auto config = config_parser::from_json(std::string_view("{ invalid }"));
  REQUIRE_FALSE(config.has_value());
}

TEST_CASE("config_parser handles nlohmann::json object", "[unit][config]")
{
  nlohmann::json json = { { "device", { { "name", "JSON Object Keyboard" }, { "vendor_id", 0xDEAD } } },
                          { "bluetooth", { { "adapter", "hci2" } } } };

  auto config = config_parser::from_json(json);
  REQUIRE(config.has_value());
  REQUIRE(config->device.name == "JSON Object Keyboard");
  REQUIRE(config->device.vendor_id == 0xDEAD);
  REQUIRE(config->bluetooth.adapter == "hci2");
}

TEST_CASE("device_config has correct defaults", "[unit][config]")
{
  device_config cfg;
  REQUIRE(cfg.name == "Fake Keyboard");
  REQUIRE(cfg.vendor_id == 0x1234);
  REQUIRE(cfg.product_id == 0x5678);
  REQUIRE(cfg.version == 0x0001);
}

TEST_CASE("bluetooth_config has correct defaults", "[unit][config]")
{
  bluetooth_config cfg;
  REQUIRE(cfg.adapter == "hci0");
  REQUIRE(cfg.auto_connect == false);
  REQUIRE(cfg.trusted_devices.empty());
}

TEST_CASE("config structure combines device and bluetooth configs", "[unit][config]")
{
  config cfg;
  REQUIRE(cfg.device.name == "Fake Keyboard");
  REQUIRE(cfg.bluetooth.adapter == "hci0");
}

TEST_CASE("config_parser handles empty trusted_devices array", "[unit][config]")
{
  auto config = config_parser::from_json(std::string_view(R"({
    "bluetooth": {
      "trusted_devices": []
    }
  })"));
  REQUIRE(config.has_value());
  REQUIRE(config->bluetooth.trusted_devices.empty());
}

TEST_CASE("config_parser handles multiple trusted devices", "[unit][config]")
{
  auto config = config_parser::from_json(std::string_view(R"({
    "bluetooth": {
      "trusted_devices": ["AA:BB:CC:DD:EE:FF", "11:22:33:44:55:66", "AA:BB:CC:DD:EE:FF"]
    }
  })"));
  REQUIRE(config.has_value());
  REQUIRE(config->bluetooth.trusted_devices.size() == 3);
  REQUIRE(config->bluetooth.trusted_devices[0] == "AA:BB:CC:DD:EE:FF");
  REQUIRE(config->bluetooth.trusted_devices[1] == "11:22:33:44:55:66");
  REQUIRE(config->bluetooth.trusted_devices[2] == "AA:BB:CC:DD:EE:FF");
}

TEST_CASE("config_parser handles boolean auto_connect", "[unit][config]")
{
  auto config_true = config_parser::from_json(std::string_view(R"({"bluetooth": {"auto_connect": true}})"));
  auto config_false = config_parser::from_json(std::string_view(R"({"bluetooth": {"auto_connect": false}})"));

  REQUIRE(config_true.has_value());
  REQUIRE(config_true->bluetooth.auto_connect == true);

  REQUIRE(config_false.has_value());
  REQUIRE(config_false->bluetooth.auto_connect == false);
}

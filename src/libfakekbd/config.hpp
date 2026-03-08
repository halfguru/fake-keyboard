/// @file config.hpp
/// @brief Configuration management for fake-keyboard
///
/// Provides JSON-based configuration for device settings and Bluetooth options.
/// Configuration can be loaded from files or parsed from JSON strings.

#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fakekbd
{

/// @brief Device identification settings
///
/// These values appear in the SDP record and identify the device to hosts.
/// They mimic USB device descriptors for compatibility.
struct device_config
{
    std::string name = "Fake Keyboard"; ///< Device name shown to hosts
    uint16_t vendor_id = 0x1234;        ///< USB vendor ID
    uint16_t product_id = 0x5678;       ///< USB product ID
    uint16_t version = 0x0001;          ///< Device version (BCD)
};

/// @brief Bluetooth adapter settings
///
/// Controls which Bluetooth adapter to use and connection behavior.
struct bluetooth_config
{
    std::string adapter = "hci0";             ///< Bluetooth adapter name
    bool auto_connect = false;                ///< Auto-connect to trusted devices
    std::vector<std::string> trusted_devices; ///< MAC addresses of trusted devices
};

/// @brief Complete configuration
///
/// Combines device and Bluetooth settings. Parse from JSON using config_parser.
struct config
{
    device_config device;       ///< Device identification
    bluetooth_config bluetooth; ///< Bluetooth settings
};

/// @brief JSON configuration parser
///
/// Parses configuration from files or JSON strings. Missing fields use defaults.
///
/// @example JSON format:
/// @code{.json}
/// {
///     "device": {
///         "name": "My Keyboard",
///         "vendor_id": 4660,
///         "product_id": 22136,
///         "version": 1
///     },
///     "bluetooth": {
///         "adapter": "hci0",
///         "auto_connect": false,
///         "trusted_devices": ["AA:BB:CC:DD:EE:FF"]
///     }
/// }
/// @endcode
class config_parser
{
  public:
    /// @brief Load configuration from a file
    /// @param path Path to JSON configuration file
    /// @return Parsed config or nullopt on error
    static auto from_file(std::string_view path) -> std::optional<config>;

    /// @brief Parse configuration from a JSON string
    /// @param json_str JSON string
    /// @return Parsed config or nullopt on error
    static auto from_json(std::string_view json_str) -> std::optional<config>;

    /// @brief Parse configuration from a JSON object
    /// @param json Parsed JSON object
    /// @return Parsed config or nullopt on error
    static auto from_json(nlohmann::json const& json) -> std::optional<config>;

  private:
    static auto parse_device_config(nlohmann::json const& json) -> device_config;
    static auto parse_bluetooth_config(nlohmann::json const& json) -> bluetooth_config;
};

}

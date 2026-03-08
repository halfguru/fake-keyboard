/// @file sdp.hpp
/// @brief SDP (Service Discovery Protocol) record builder
///
/// SDP allows Bluetooth devices to discover what services are available.
/// For HID, we advertise the keyboard service with its characteristics
/// (name, vendor ID, product ID, report descriptor).
///
/// The SDP record is registered with BlueZ via D-Bus and made available
/// to scanning hosts.
///
/// @see DESIGN.md for SDP architecture details

#pragma once

#include "libfakekbd/hid/report.hpp"
#include <string>
#include <vector>

namespace fakekbd::bluetooth
{

/// @brief Builder for SDP service records
///
/// Constructs an XML SDP record describing a HID device. The record
/// includes device identification and the HID report descriptor.
///
/// @example
/// @code
/// auto record = sdp_record{}
///     .set_device_name("My Keyboard")
///     .set_vendor_id(0x1234)
///     .set_product_id(0x5678)
///     .set_report_descriptor(descriptor)
///     .build_xml();
/// @endcode
class sdp_record
{
  public:
    sdp_record() = default;

    /// @brief Set the device name shown to hosts
    /// @param name Device name (UTF-8)
    /// @return Reference to this builder
    auto set_device_name(std::string_view name) -> sdp_record&;

    /// @brief Set USB-style vendor ID
    /// @param id 16-bit vendor ID (e.g., 0x1234)
    /// @return Reference to this builder
    auto set_vendor_id(uint16_t id) -> sdp_record&;

    /// @brief Set USB-style product ID
    /// @param id 16-bit product ID (e.g., 0x5678)
    /// @return Reference to this builder
    auto set_product_id(uint16_t id) -> sdp_record&;

    /// @brief Set device version
    /// @param version 16-bit version (BCD format)
    /// @return Reference to this builder
    auto set_version(uint16_t version) -> sdp_record&;

    /// @brief Set the HID report descriptor
    /// @param descriptor Report descriptor bytes
    /// @return Reference to this builder
    auto set_report_descriptor(std::vector<uint8_t> const& descriptor) -> sdp_record&;

    /// @brief Build the SDP record as XML
    /// @return XML string in BlueZ SDP format
    [[nodiscard]] auto build_xml() const -> std::string;

  private:
    std::string device_name_ = "Fake Keyboard";
    uint16_t vendor_id_ = 0x1234;
    uint16_t product_id_ = 0x5678;
    uint16_t version_ = 0x0100;
    std::vector<uint8_t> report_descriptor_;
};

/// @brief Build a complete HID SDP record
/// @param device_name Device name shown during discovery
/// @param vendor_id USB vendor ID
/// @param product_id USB product ID
/// @param version Device version
/// @param report_descriptor HID report descriptor bytes
/// @return XML SDP record string
auto build_hid_sdp_record(std::string_view device_name,
                          uint16_t vendor_id,
                          uint16_t product_id,
                          uint16_t version,
                          std::vector<uint8_t> const& report_descriptor) -> std::string;

/// @brief Build a standard keyboard SDP record with defaults
/// @param device_name Device name (default: "Fake Keyboard")
/// @return XML SDP record string for a keyboard
auto build_keyboard_sdp_record(std::string_view device_name = "Fake Keyboard") -> std::string;

}

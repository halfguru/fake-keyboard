#pragma once

#include "hid/report.hpp"
#include <string>
#include <vector>

namespace fakekbd::bluetooth {

class sdp_record
{
public:
  sdp_record() = default;

  auto set_device_name(std::string_view name) -> sdp_record&;
  auto set_vendor_id(uint16_t id) -> sdp_record&;
  auto set_product_id(uint16_t id) -> sdp_record&;
  auto set_version(uint16_t version) -> sdp_record&;
  auto set_report_descriptor(std::vector<uint8_t> const& descriptor) -> sdp_record&;

  [[nodiscard]] auto build_xml() const -> std::string;

private:
  std::string device_name_ = "Fake Keyboard";
  uint16_t vendor_id_ = 0x1234;
  uint16_t product_id_ = 0x5678;
  uint16_t version_ = 0x0100;
  std::vector<uint8_t> report_descriptor_;
};

auto
build_hid_sdp_record(std::string_view device_name,
                     uint16_t vendor_id,
                     uint16_t product_id,
                     uint16_t version,
                     std::vector<uint8_t> const& report_descriptor) -> std::string;

auto
build_keyboard_sdp_record(std::string_view device_name = "Fake Keyboard") -> std::string;

}

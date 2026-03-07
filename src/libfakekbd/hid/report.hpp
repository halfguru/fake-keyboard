#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace fakekbd::hid {

class report_descriptor_builder
{
public:
  report_descriptor_builder& usage_page(uint16_t page);
  report_descriptor_builder& usage(uint16_t usage);
  report_descriptor_builder& usage_min(uint16_t min);
  report_descriptor_builder& usage_max(uint16_t max);
  report_descriptor_builder& collection(uint8_t type);
  report_descriptor_builder& end_collection();
  report_descriptor_builder& report_size(uint8_t size);
  report_descriptor_builder& report_count(uint8_t count);
  report_descriptor_builder& logical_min(int32_t min);
  report_descriptor_builder& logical_max(int32_t max);
  report_descriptor_builder& input(uint8_t flags);
  report_descriptor_builder& output(uint8_t flags);
  report_descriptor_builder& feature(uint8_t flags);
  report_descriptor_builder& report_id(uint8_t id);

  [[nodiscard]] auto build() const -> std::vector<uint8_t>;
  [[nodiscard]] auto data() const -> std::span<uint8_t const>;

private:
  std::vector<uint8_t> descriptor_;

  void append_byte(uint8_t byte);
  void append_value(uint32_t value);
};

namespace usage_page {
constexpr uint16_t GENERIC_DESKTOP = 0x01;
constexpr uint16_t KEYBOARD = 0x07;
constexpr uint16_t LED = 0x08;
constexpr uint16_t BUTTON = 0x09;
}

namespace usage {
constexpr uint16_t KEYBOARD = 0x06;
constexpr uint16_t KEYBOARD_NUM_LOCK = 0x01;
constexpr uint16_t KEYBOARD_CAPS_LOCK = 0x02;
constexpr uint16_t KEYBOARD_SCROLL_LOCK = 0x03;
constexpr uint16_t KEYBOARD_COMPOSE = 0x04;
constexpr uint16_t KEYBOARD_KANA = 0x05;
}

namespace collection {
constexpr uint8_t PHYSICAL = 0x00;
constexpr uint8_t APPLICATION = 0x01;
constexpr uint8_t LOGICAL = 0x02;
constexpr uint8_t REPORT = 0x03;
}

namespace report_flags {
constexpr uint8_t DATA = 0x00;
constexpr uint8_t CONSTANT = 0x01;
constexpr uint8_t ARRAY = 0x00;
constexpr uint8_t VARIABLE = 0x02;
constexpr uint8_t ABSOLUTE = 0x00;
constexpr uint8_t RELATIVE = 0x04;
constexpr uint8_t NO_WRAP = 0x00;
constexpr uint8_t WRAP = 0x08;
constexpr uint8_t LINEAR = 0x00;
constexpr uint8_t NON_LINEAR = 0x10;
constexpr uint8_t PREFERRED_STATE = 0x00;
constexpr uint8_t NO_PREFERRED = 0x20;
constexpr uint8_t NO_NULL_POSITION = 0x00;
constexpr uint8_t NULL_STATE = 0x40;
constexpr uint8_t NON_VOLATILE = 0x00;
constexpr uint8_t VOLATILE = 0x80;
}

auto
build_keyboard_report_descriptor() -> std::vector<uint8_t>;

}

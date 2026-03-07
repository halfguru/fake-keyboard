#include "report.hpp"
#include <stdexcept>

namespace fakekbd::hid {

namespace {
constexpr uint8_t LONG_ITEM_PREFIX = 0xFE;

enum class item_type : uint8_t
{
  MAIN = 0,
  GLOBAL = 1,
  LOCAL = 2,
  RESERVED = 3
};

enum class main_item_tag : uint8_t
{
  INPUT = 8,
  OUTPUT = 9,
  FEATURE = 11,
  COLLECTION = 10,
  END_COLLECTION = 12
};

enum class global_item_tag : uint8_t
{
  USAGE_PAGE = 0,
  LOGICAL_MINIMUM = 1,
  LOGICAL_MAXIMUM = 2,
  PHYSICAL_MINIMUM = 3,
  PHYSICAL_MAXIMUM = 4,
  UNIT_EXPONENT = 5,
  UNIT = 6,
  REPORT_SIZE = 7,
  REPORT_ID = 8,
  REPORT_COUNT = 9,
  PUSH = 10,
  POP = 11
};

enum class local_item_tag : uint8_t
{
  USAGE = 0,
  USAGE_MINIMUM = 1,
  USAGE_MAXIMUM = 2,
  DESIGNATOR_INDEX = 3,
  DESIGNATOR_MINIMUM = 4,
  DESIGNATOR_MAXIMUM = 5,
  STRING_INDEX = 7,
  STRING_MINIMUM = 8,
  STRING_MAXIMUM = 9,
  DELIMITER = 10
};

auto
encode_short_item(item_type type, uint8_t tag, uint32_t value) -> std::vector<uint8_t>
{
  std::vector<uint8_t> item;
  uint8_t size = 0;

  if (value <= 0xFF) {
    size = 1;
  } else if (value <= 0xFFFF) {
    size = 2;
  } else if (value <= 0xFFFFFF) {
    size = 3;
  } else {
    size = 4;
  }

  uint8_t const prefix = (static_cast<uint8_t>(tag) << 4) | (static_cast<uint8_t>(type) << 2) | (size == 4 ? 3 : size);
  item.push_back(prefix);

  for (uint8_t i = 0; i < size; ++i) {
    item.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
  }

  return item;
}

}

auto
report_descriptor_builder::usage_page(uint16_t page) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::GLOBAL, static_cast<uint8_t>(global_item_tag::USAGE_PAGE), page);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::usage(uint16_t usage) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::LOCAL, static_cast<uint8_t>(local_item_tag::USAGE), usage);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::usage_min(uint16_t min) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::LOCAL, static_cast<uint8_t>(local_item_tag::USAGE_MINIMUM), min);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::usage_max(uint16_t max) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::LOCAL, static_cast<uint8_t>(local_item_tag::USAGE_MAXIMUM), max);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::collection(uint8_t type) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::MAIN, static_cast<uint8_t>(main_item_tag::COLLECTION), type);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::end_collection() -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::MAIN, static_cast<uint8_t>(main_item_tag::END_COLLECTION), 0);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::report_size(uint8_t size) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::GLOBAL, static_cast<uint8_t>(global_item_tag::REPORT_SIZE), size);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::report_count(uint8_t count) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::GLOBAL, static_cast<uint8_t>(global_item_tag::REPORT_COUNT), count);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::logical_min(int32_t min) -> report_descriptor_builder&
{
  auto const item = encode_short_item(
    item_type::GLOBAL, static_cast<uint8_t>(global_item_tag::LOGICAL_MINIMUM), static_cast<uint32_t>(min));
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::logical_max(int32_t max) -> report_descriptor_builder&
{
  auto const item = encode_short_item(
    item_type::GLOBAL, static_cast<uint8_t>(global_item_tag::LOGICAL_MAXIMUM), static_cast<uint32_t>(max));
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::input(uint8_t flags) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::MAIN, static_cast<uint8_t>(main_item_tag::INPUT), flags);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::output(uint8_t flags) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::MAIN, static_cast<uint8_t>(main_item_tag::OUTPUT), flags);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::feature(uint8_t flags) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::MAIN, static_cast<uint8_t>(main_item_tag::FEATURE), flags);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::report_id(uint8_t id) -> report_descriptor_builder&
{
  auto const item = encode_short_item(item_type::GLOBAL, static_cast<uint8_t>(global_item_tag::REPORT_ID), id);
  descriptor_.insert(descriptor_.end(), item.begin(), item.end());
  return *this;
}

auto
report_descriptor_builder::build() const -> std::vector<uint8_t>
{
  return descriptor_;
}

auto
report_descriptor_builder::data() const -> std::span<uint8_t const>
{
  return descriptor_;
}

auto
build_keyboard_report_descriptor() -> std::vector<uint8_t>
{
  return report_descriptor_builder{}
    .usage_page(usage_page::GENERIC_DESKTOP)
    .usage(usage::KEYBOARD)
    .collection(collection::APPLICATION)
    .usage_page(usage_page::KEYBOARD)
    .usage_min(0xE0)
    .usage_max(0xE7)
    .logical_min(0)
    .logical_max(1)
    .report_size(1)
    .report_count(8)
    .input(report_flags::DATA | report_flags::VARIABLE | report_flags::ABSOLUTE)
    .report_count(1)
    .report_size(8)
    .input(report_flags::CONSTANT)
    .usage_min(0x00)
    .usage_max(0x65)
    .logical_min(0)
    .logical_max(0x65)
    .report_size(8)
    .report_count(6)
    .input(report_flags::DATA | report_flags::ARRAY | report_flags::ABSOLUTE)
    .usage_page(usage_page::LED)
    .usage_min(0x01)
    .usage_max(0x05)
    .logical_min(0)
    .logical_max(1)
    .report_size(1)
    .report_count(5)
    .output(report_flags::DATA | report_flags::VARIABLE | report_flags::ABSOLUTE)
    .report_count(1)
    .report_size(3)
    .output(report_flags::CONSTANT)
    .end_collection()
    .build();
}

}

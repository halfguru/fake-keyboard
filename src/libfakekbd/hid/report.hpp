/// @file report.hpp
/// @brief HID report descriptor builder and constants
///
/// Provides a fluent API for building HID report descriptors according to
/// the USB HID specification. Report descriptors tell the host how to
/// interpret the data sent by the device.
///
/// @see https://www.usb.org/hid

#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace fakekbd::hid
{

/// @brief Builder for HID report descriptors
///
/// Provides a fluent interface for constructing HID report descriptors.
/// Each method appends items to the descriptor and returns *this for chaining.
///
/// @example Standard keyboard descriptor:
/// @code
/// auto descriptor = report_descriptor_builder{}
///     .usage_page(usage_page::GENERIC_DESKTOP)
///     .usage(usage::KEYBOARD)
///     .collection(collection::APPLICATION)
///     .usage_page(usage_page::KEYBOARD)
///     .usage_min(0xE0)
///     .usage_max(0xE7)
///     .logical_min(0)
///     .logical_max(1)
///     .report_size(1)
///     .report_count(8)
///     .input(report_flags::DATA | report_flags::VARIABLE)
///     .end_collection()
///     .build();
/// @endcode
class report_descriptor_builder
{
  public:
    /// @brief Set the usage page (top-level usage category)
    /// @param page Usage page ID (see usage_page namespace)
    /// @return Reference to this builder
    report_descriptor_builder& usage_page(uint16_t page);

    /// @brief Set the usage within the current page
    /// @param usage Usage ID (see usage namespace)
    /// @return Reference to this builder
    report_descriptor_builder& usage(uint16_t usage);

    /// @brief Set minimum usage for a range
    /// @param min Minimum usage ID
    /// @return Reference to this builder
    report_descriptor_builder& usage_min(uint16_t min);

    /// @brief Set maximum usage for a range
    /// @param max Maximum usage ID
    /// @return Reference to this builder
    report_descriptor_builder& usage_max(uint16_t max);

    /// @brief Begin a collection
    /// @param type Collection type (see collection namespace)
    /// @return Reference to this builder
    report_descriptor_builder& collection(uint8_t type);

    /// @brief End the current collection
    /// @return Reference to this builder
    report_descriptor_builder& end_collection();

    /// @brief Set report field size in bits
    /// @param size Size of each field in bits
    /// @return Reference to this builder
    report_descriptor_builder& report_size(uint8_t size);

    /// @brief Set number of fields in report
    /// @param count Number of fields
    /// @return Reference to this builder
    report_descriptor_builder& report_count(uint8_t count);

    /// @brief Set logical minimum value
    /// @param min Minimum value that can be reported
    /// @return Reference to this builder
    report_descriptor_builder& logical_min(int32_t min);

    /// @brief Set logical maximum value
    /// @param max Maximum value that can be reported
    /// @return Reference to this builder
    report_descriptor_builder& logical_max(int32_t max);

    /// @brief Define an input field
    /// @param flags Field characteristics (see report_flags namespace)
    /// @return Reference to this builder
    report_descriptor_builder& input(uint8_t flags);

    /// @brief Define an output field (device receives from host)
    /// @param flags Field characteristics (see report_flags namespace)
    /// @return Reference to this builder
    report_descriptor_builder& output(uint8_t flags);

    /// @brief Define a feature field (bidirectional)
    /// @param flags Field characteristics (see report_flags namespace)
    /// @return Reference to this builder
    report_descriptor_builder& feature(uint8_t flags);

    /// @brief Set report ID for multiple reports
    /// @param id Report identifier
    /// @return Reference to this builder
    report_descriptor_builder& report_id(uint8_t id);

    /// @brief Build the final descriptor
    /// @return Vector of bytes representing the complete descriptor
    [[nodiscard]] auto build() const -> std::vector<uint8_t>;

    /// @brief Get current descriptor data without copying
    /// @return Span over the internal descriptor buffer
    [[nodiscard]] auto data() const -> std::span<uint8_t const>;

  private:
    std::vector<uint8_t> descriptor_;

    void append_byte(uint8_t byte);
    void append_value(uint32_t value);
};

/// @brief HID usage page identifiers
namespace usage_page
{
constexpr uint16_t GENERIC_DESKTOP = 0x01; ///< Mice, keyboards, joysticks
constexpr uint16_t KEYBOARD = 0x07;        ///< Keyboard/keypad
constexpr uint16_t LED = 0x08;             ///< Keyboard LEDs
constexpr uint16_t BUTTON = 0x09;          ///< Buttons
}

/// @brief HID usage identifiers (within usage pages)
namespace usage
{
constexpr uint16_t KEYBOARD = 0x06;             ///< Keyboard application
constexpr uint16_t KEYBOARD_NUM_LOCK = 0x01;    ///< Num Lock LED
constexpr uint16_t KEYBOARD_CAPS_LOCK = 0x02;   ///< Caps Lock LED
constexpr uint16_t KEYBOARD_SCROLL_LOCK = 0x03; ///< Scroll Lock LED
constexpr uint16_t KEYBOARD_COMPOSE = 0x04;     ///< Compose LED
constexpr uint16_t KEYBOARD_KANA = 0x05;        ///< Kana LED
}

/// @brief Collection types
namespace collection
{
constexpr uint8_t PHYSICAL = 0x00;    ///< Physical grouping
constexpr uint8_t APPLICATION = 0x01; ///< Logical application (e.g., keyboard)
constexpr uint8_t LOGICAL = 0x02;     ///< Logical grouping
constexpr uint8_t REPORT = 0x03;      ///< Report grouping
}

/// @brief Report field flags
namespace report_flags
{
constexpr uint8_t DATA = 0x00;             ///< Data field (vs Constant)
constexpr uint8_t CONSTANT = 0x01;         ///< Constant value
constexpr uint8_t ARRAY = 0x00;            ///< Array of usages
constexpr uint8_t VARIABLE = 0x02;         ///< Variable (bit field)
constexpr uint8_t ABSOLUTE = 0x00;         ///< Absolute value
constexpr uint8_t RELATIVE = 0x04;         ///< Relative change
constexpr uint8_t NO_WRAP = 0x00;          ///< No wrap
constexpr uint8_t WRAP = 0x08;             ///< Wrap around
constexpr uint8_t LINEAR = 0x00;           ///< Linear
constexpr uint8_t NON_LINEAR = 0x10;       ///< Non-linear
constexpr uint8_t PREFERRED_STATE = 0x00;  ///< Preferred state
constexpr uint8_t NO_PREFERRED = 0x20;     ///< No preferred state
constexpr uint8_t NO_NULL_POSITION = 0x00; ///< No null position
constexpr uint8_t NULL_STATE = 0x40;       ///< Has null state
constexpr uint8_t NON_VOLATILE = 0x00;     ///< Non-volatile
constexpr uint8_t VOLATILE = 0x80;         ///< Volatile
}

/// @brief Build a standard keyboard report descriptor
/// @return Complete HID report descriptor for a boot-protocol keyboard
auto build_keyboard_report_descriptor() -> std::vector<uint8_t>;

}

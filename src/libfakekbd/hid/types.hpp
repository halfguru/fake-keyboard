/// @file types.hpp
/// @brief Core types and constants for the HID library
///
/// Defines HID protocol types, key codes, modifiers, and error handling.
/// These types are used throughout the fake-keyboard library.

#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <string_view>

namespace fakekbd::hid
{

/// @brief HID protocol mode
enum class protocol : uint8_t
{
  Boot = 0x00,  ///< Boot protocol (limited, for BIOS/UEFI)
  Report = 0x01 ///< Report protocol (full functionality)
};

/// @brief HID report type
enum class report_type : uint8_t
{
  Input = 0x01,  ///< Device to host
  Output = 0x02, ///< Host to device (e.g., LEDs)
  Feature = 0x03 ///< Configuration data
};

/// @brief HID transaction type
enum class transaction_type : uint8_t
{
  Handshake = 0x00,
  HidControl = 0x10,
  GetReport = 0x40,
  SetReport = 0x50,
  GetProtocol = 0x60,
  SetProtocol = 0x70,
  GetIdle = 0x80,
  SetIdle = 0x90,
  Data = 0xa0,
  Datc = 0xb0
};

/// @brief Result codes for HID operations
enum class handshake_result : uint8_t
{
  Successful = 0x00,
  NotReady = 0x01,
  ErrInvalidReportId = 0x02,
  ErrUnsupportedRequest = 0x03,
  ErrInvalidParameter = 0x04,
  ErrUnknown = 0x0e,
  ErrFatal = 0x0f
};

/// @brief Control channel operations
enum class control_operation : uint8_t
{
  Nop = 0x00,
  HardReset = 0x01,
  SoftReset = 0x02,
  Suspend = 0x03,
  ExitSuspend = 0x04,
  VirtualCableUnplug = 0x05
};

/// @brief L2CAP PSM for HID control channel
constexpr uint16_t L2CAP_PSM_CONTROL = 0x11;

/// @brief L2CAP PSM for HID interrupt channel
constexpr uint16_t L2CAP_PSM_INTERRUPT = 0x13;

/// @brief Bluetooth HID Profile UUID
constexpr std::string_view HID_PROFILE_UUID = "00001124-0000-1000-8000-00805f9b34fb";

/// @brief HID key codes (from USB HID Usage Tables)
///
/// These are the standard key codes used in HID reports.
/// @see https://usb.org/sites/default/files/hut1_4.pdf
namespace key_code
{
constexpr uint8_t A = 0x04;
constexpr uint8_t B = 0x05;
constexpr uint8_t C = 0x06;
constexpr uint8_t D = 0x07;
constexpr uint8_t E = 0x08;
constexpr uint8_t F = 0x09;
constexpr uint8_t G = 0x0A;
constexpr uint8_t H = 0x0B;
constexpr uint8_t I = 0x0C;
constexpr uint8_t J = 0x0D;
constexpr uint8_t K = 0x0E;
constexpr uint8_t L = 0x0F;
constexpr uint8_t M = 0x10;
constexpr uint8_t N = 0x11;
constexpr uint8_t O = 0x12;
constexpr uint8_t P = 0x13;
constexpr uint8_t Q = 0x14;
constexpr uint8_t R = 0x15;
constexpr uint8_t S = 0x16;
constexpr uint8_t T = 0x17;
constexpr uint8_t U = 0x18;
constexpr uint8_t V = 0x19;
constexpr uint8_t W = 0x1A;
constexpr uint8_t X = 0x1B;
constexpr uint8_t Y = 0x1C;
constexpr uint8_t Z = 0x1D;

constexpr uint8_t ENTER = 0x28;
constexpr uint8_t ESCAPE = 0x29;
constexpr uint8_t BACKSPACE = 0x2A;
constexpr uint8_t TAB = 0x2B;
constexpr uint8_t SPACE = 0x2C;

constexpr uint8_t F1 = 0x3A;
constexpr uint8_t F2 = 0x3B;
constexpr uint8_t F3 = 0x3C;
constexpr uint8_t F4 = 0x3D;
constexpr uint8_t F5 = 0x3E;
constexpr uint8_t F6 = 0x3F;
constexpr uint8_t F7 = 0x40;
constexpr uint8_t F8 = 0x41;
constexpr uint8_t F9 = 0x42;
constexpr uint8_t F10 = 0x43;
constexpr uint8_t F11 = 0x44;
constexpr uint8_t F12 = 0x45;

constexpr uint8_t ARROW_RIGHT = 0x4F;
constexpr uint8_t ARROW_LEFT = 0x50;
constexpr uint8_t ARROW_DOWN = 0x51;
constexpr uint8_t ARROW_UP = 0x52;
}

/// @brief Keyboard modifier key flags
///
/// Bitmask values for the first byte of a keyboard report.
/// Can be combined with bitwise OR.
namespace modifier
{
constexpr uint8_t LEFT_CTRL = 0x01;
constexpr uint8_t LEFT_SHIFT = 0x02;
constexpr uint8_t LEFT_ALT = 0x04;
constexpr uint8_t LEFT_GUI = 0x08; ///< Windows/Command/Super key
constexpr uint8_t RIGHT_CTRL = 0x10;
constexpr uint8_t RIGHT_SHIFT = 0x20;
constexpr uint8_t RIGHT_ALT = 0x40;
constexpr uint8_t RIGHT_GUI = 0x80;
}

/// @brief Standard USB HID keyboard report (8 bytes)
///
/// This is the standard boot-protocol keyboard report format.
/// - Byte 0: Modifier keys (Ctrl, Shift, Alt, GUI)
/// - Byte 1: Reserved (0x00)
/// - Bytes 2-7: Up to 6 simultaneous key codes
///
/// @example
/// @code
/// KeyboardReport report;
/// report.modifiers = modifier::LEFT_SHIFT;
/// report.key_codes[0] = key_code::A;  // 'A' (shifted)
/// @endcode
struct KeyboardReport
{
    uint8_t modifiers = 0;
    uint8_t reserved = 0;
    std::array<uint8_t, 6> key_codes{};
} __attribute__((packed));

static_assert(sizeof(KeyboardReport) == 8, "Keyboard report must be 8 bytes");

/// @brief Error codes for fallible operations
enum class error : uint8_t
{
  SocketCreation,
  Bind,
  Listen,
  Accept,
  Connect,
  Send,
  Receive,
  DbusConnection,
  ProfileRegistration,
  InvalidConfiguration,
  DeviceNotFound,
  NotConnected
};

/// @brief Result type for fallible operations
///
/// Uses std::expected for modern error handling without exceptions.
///
/// @example
/// @code
/// Result<void> result = keyboard.listen("hci0");
/// if (!result) {
///     spdlog::error("Failed: {}", static_cast<int>(result.error()));
/// }
/// @endcode
template<typename T>
using Result = std::expected<T, error>;

}

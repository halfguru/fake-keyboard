/// @file fakekbd.hpp
/// @brief Main header for the fake-keyboard library
///
/// This is the single-include header for libfakekbd. It provides all public
/// types and aliases needed to create virtual Bluetooth HID keyboards.
///
/// @example Basic usage:
/// @code
/// #include <libfakekbd/fakekbd.hpp>
///
/// int main() {
///     fakekbd::keyboard kbd;
///     if (auto result = kbd.listen("hci0"); !result) {
///         return 1;
///     }
///     kbd.send_text("Hello, World!");
/// }
/// @endcode

#pragma once

#include "libfakekbd/config.hpp"
#include "libfakekbd/hid/keyboard.hpp"
#include "libfakekbd/hid/report.hpp"
#include "libfakekbd/hid/types.hpp"

namespace fakekbd
{

/// @brief Virtual Bluetooth HID keyboard
using keyboard = hid::keyboard;

/// @brief Configuration for device and Bluetooth settings
using config = fakekbd::config;

/// @brief Error type for fallible operations
using error = hid::error;

/// @brief Result type wrapping success or error
template<typename T>
using Result = hid::Result<T>;

/// @brief HID key codes (HID Usage Table keyboard/page)
namespace key_code = hid::key_code;

/// @brief Keyboard modifier flags (Ctrl, Shift, Alt, GUI)
namespace modifier = hid::modifier;

}

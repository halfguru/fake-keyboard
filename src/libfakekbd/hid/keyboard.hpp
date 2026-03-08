/// @file keyboard.hpp
/// @brief Virtual Bluetooth HID keyboard device
///
/// Provides the main keyboard class for emulating a Bluetooth HID keyboard.
/// The keyboard can listen for incoming connections and send key events to
/// connected hosts.

#pragma once

#include "libfakekbd/bluetooth/l2cap.hpp"
#include "libfakekbd/hid/types.hpp"
#include <atomic>
#include <bluetooth/bluetooth.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace fakekbd::hid
{

/// @brief Connection state of a HID device
enum class connection_state : uint8_t
{
  Disconnected, ///< No active connection
  Connecting,   ///< Connection in progress
  Connected,    ///< Actively connected to a host
  Disconnecting ///< Disconnection in progress
};

/// @brief Base class for HID devices
///
/// Provides common connection state management for all HID device types.
/// Thread-safe state transitions via atomic operations.
class device
{
  public:
    device();
    virtual ~device();

    device(device const&) = delete;
    device& operator=(device const&) = delete;
    device(device&&) = delete;
    device& operator=(device&&) = delete;

    /// @brief Get current connection state
    /// @return Current state of the device connection
    [[nodiscard]] auto state() const -> connection_state;

    /// @brief Check if device is connected
    /// @return true if state is Connected, false otherwise
    [[nodiscard]] auto is_connected() const -> bool;

  protected:
    std::atomic<connection_state> state_;
    std::mutex connection_mutex_;

    void set_state(connection_state new_state);
};

/// @brief Virtual Bluetooth HID keyboard
///
/// Emulates a Bluetooth Classic HID keyboard that can connect to hosts
/// (phones, tablets, computers). Uses L2CAP channels for communication:
/// - Control channel (PSM 0x11): Device configuration
/// - Interrupt channel (PSM 0x13): HID reports (key events)
///
/// @example
/// @code
/// keyboard kbd;
/// if (auto result = kbd.listen("hci0"); result) {
///     // Wait for connection...
///     kbd.send_key(0x04, true);  // Press 'a'
///     kbd.send_key(0x04, false); // Release 'a'
/// }
/// @endcode
class keyboard : public device
{
  public:
    keyboard();
    ~keyboard() override;

    /// @brief Start listening for incoming connections
    /// @param adapter Bluetooth adapter name (e.g., "hci0")
    /// @return Success or error
    auto listen(std::string const& adapter) -> Result<void>;

    /// @brief Connect to a specific client device
    /// @param client_addr Bluetooth address of the client
    /// @return Success or error
    auto connect(bdaddr_t const& client_addr) -> Result<void>;

    /// @brief Disconnect from current host
    auto disconnect() -> void;

    /// @brief Send a key event
    /// @param key_code HID usage code for the key (see key_code namespace)
    /// @param pressed true for key down, false for key up
    /// @param modifiers Bitmask of modifier keys (see modifier namespace)
    /// @return Success or error
    auto send_key(uint8_t key_code, bool pressed, uint8_t modifiers = 0) -> Result<void>;

    /// @brief Send a string as keystrokes
    /// @param text Text to type (ASCII characters)
    /// @return Success or error
    ///
    /// Automatically handles shift for uppercase and special characters.
    /// Each character is pressed and released before the next.
    auto send_text(std::string_view text) -> Result<void>;

    /// @brief Get control channel file descriptor
    [[nodiscard]] auto control_fd() const -> int;

    /// @brief Get interrupt channel file descriptor
    [[nodiscard]] auto interrupt_fd() const -> int;

  private:
    bluetooth::l2cap_server control_server_;
    bluetooth::l2cap_server interrupt_server_;
    std::unique_ptr<bluetooth::l2cap_client> control_client_;
    std::unique_ptr<bluetooth::l2cap_client> interrupt_client_;

    int control_fd_;
    int interrupt_fd_;
    std::thread accept_thread_;
    std::atomic<bool> running_;

    auto handle_connection(int control_fd, int interrupt_fd) -> void;
    auto build_report(uint8_t key_code, bool pressed, uint8_t modifiers) const -> KeyboardReport;
};

/// @brief Convert ASCII character to HID key code
/// @param c ASCII character
/// @return HID usage code for the key
auto key_from_char(char c) -> uint8_t;

/// @brief Get modifier flags needed for a character
/// @param c ASCII character
/// @return Bitmask of modifier keys (e.g., SHIFT for uppercase)
auto modifiers_from_char(char c) -> uint8_t;

}

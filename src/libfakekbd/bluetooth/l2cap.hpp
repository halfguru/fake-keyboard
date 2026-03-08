/// @file l2cap.hpp
/// @brief L2CAP socket classes for Bluetooth communication
///
/// L2CAP (Logical Link Control and Adaptation Protocol) provides
/// connection-oriented channels over Bluetooth. For HID, we use:
/// - PSM 0x11 (17): Control channel
/// - PSM 0x13 (19): Interrupt channel (HID reports)
///
/// @see DESIGN.md for protocol stack details

#pragma once

#include "libfakekbd/hid/types.hpp"
#include <bluetooth/bluetooth.h>
#include <functional>
#include <memory>
#include <string>

namespace fakekbd::bluetooth
{

/// @brief Base class for L2CAP sockets
///
/// Manages a Bluetooth L2CAP socket file descriptor with RAII semantics.
class l2cap_socket
{
  public:
    l2cap_socket();
    ~l2cap_socket();

    l2cap_socket(l2cap_socket const&) = delete;
    l2cap_socket& operator=(l2cap_socket const&) = delete;
    l2cap_socket(l2cap_socket&&) noexcept;
    l2cap_socket& operator=(l2cap_socket&&) noexcept;

    /// @brief Get the socket file descriptor
    /// @return File descriptor, or -1 if invalid
    [[nodiscard]] auto fd() const -> int;

    /// @brief Check if socket is valid
    /// @return true if socket is open and valid
    [[nodiscard]] auto is_valid() const -> bool;

  protected:
    explicit l2cap_socket(int fd);

    int fd_;
};

/// @brief L2CAP server for accepting incoming connections
///
/// Listens on a specific PSM and accepts incoming connections from hosts.
/// Used for both control and interrupt channels in HID.
///
/// @example
/// @code
/// l2cap_server server;
/// if (auto result = server.listen(addr, 0x13); result) {
///     auto client = server.accept();
///     // Use client.fd() for communication
/// }
/// @endcode
class l2cap_server : public l2cap_socket
{
  public:
    using connection_handler = std::function<void(int client_fd, bdaddr_t client_addr)>;

    l2cap_server() = default;
    ~l2cap_server() = default;

    /// @brief Start listening for connections
    /// @param addr Local Bluetooth adapter address
    /// @param psm Protocol/Service Multiplexer (0x11 or 0x13 for HID)
    /// @param backlog Maximum pending connections
    /// @return Success or error
    auto listen(bdaddr_t const& addr, uint16_t psm, int backlog = 5) -> hid::Result<void>;

    /// @brief Accept an incoming connection
    /// @return Pair of (client_fd, client_address) or error
    auto accept() -> hid::Result<std::pair<int, bdaddr_t>>;

  private:
};

/// @brief L2CAP client for outgoing connections
///
/// Connects to a remote L2CAP server. Used when the keyboard
/// initiates connection to a host (rare for HID).
class l2cap_client : public l2cap_socket
{
  public:
    l2cap_client() = default;
    ~l2cap_client() = default;

    /// @brief Connect to a remote server
    /// @param src Local adapter address
    /// @param dst Remote device address
    /// @param psm Protocol/Service Multiplexer
    /// @return Success or error
    auto connect(bdaddr_t const& src, bdaddr_t const& dst, uint16_t psm) -> hid::Result<void>;
};

/// @brief Get the Bluetooth address of a local adapter
/// @param adapter_name Adapter name (e.g., "hci0")
/// @return Bluetooth address or error
auto get_adapter_address(std::string const& adapter_name) -> hid::Result<bdaddr_t>;

/// @brief Send a HID report over L2CAP
/// @param fd Socket file descriptor
/// @param data Report data
/// @param len Data length in bytes
/// @return Success or error
auto send_hid_report(int fd, void const* data, size_t len) -> hid::Result<void>;

/// @brief Receive a HID report from L2CAP
/// @param fd Socket file descriptor
/// @param data Buffer to receive data
/// @param len Buffer size
/// @return Number of bytes received or error
auto receive_hid_report(int fd, void* data, size_t len) -> hid::Result<size_t>;

}

/// @file dbus.hpp
/// @brief BlueZ D-Bus integration for HID profile registration
///
/// Provides integration with BlueZ (Linux Bluetooth stack) via D-Bus.
/// This handles:
/// - Registering the HID profile with BlueZ
/// - Receiving connection callbacks
/// - Configuring adapter properties (discoverable, pairable)
/// - Managing pairing agents
///
/// @see DESIGN.md for D-Bus architecture details

#pragma once

#include "libfakekbd/hid/types.hpp"
#include <bluetooth/bluetooth.h>
#include <functional>
#include <memory>
#include <string>

namespace fakekbd::bluetooth
{

/// @brief Bluetooth Device Class constants
/// @see Bluetooth Assigned Numbers
constexpr uint32_t DEVICE_CLASS_PERIPHERAL = 0x002540;
constexpr uint32_t DEVICE_CLASS_KEYBOARD = 0x000540;

/// @brief BlueZ D-Bus profile manager for HID
///
/// Manages registration of the HID profile with BlueZ and handles
/// incoming connections via callbacks.
///
/// @example
/// @code
/// DBusProfileManager mgr;
///
/// mgr.registerHidProfile(
///     "hci0",
///     "My Keyboard",
///     sdp_record,
///     [](bdaddr_t const& device, int fd) {
///         // Handle new connection
///     },
///     []() {
///         // Handle profile release
///     }
/// );
///
/// mgr.setAdapterDiscoverable("hci0", true);
/// mgr.processEvents();  // Run event loop
/// @endcode
class DBusProfileManager
{
  public:
    /// @brief Callback for new connections
    /// @param device Remote device address
    /// @param fd File descriptor for communication
    using NewConnectionCallback = std::function<void(bdaddr_t const& device, int fd)>;

    /// @brief Callback for profile release
    using ReleaseCallback = std::function<void()>;

    DBusProfileManager();
    ~DBusProfileManager();

    DBusProfileManager(DBusProfileManager const&) = delete;
    DBusProfileManager& operator=(DBusProfileManager const&) = delete;
    DBusProfileManager(DBusProfileManager&&) = delete;
    DBusProfileManager& operator=(DBusProfileManager&&) = delete;

    /// @brief Register HID profile with BlueZ
    /// @param adapter Bluetooth adapter name (e.g., "hci0")
    /// @param name Profile name
    /// @param sdpRecord SDP record XML
    /// @param onNewConnection Called when a host connects
    /// @param onRelease Called when profile is released
    /// @return Success or error
    auto registerHidProfile(std::string const& adapter,
                            std::string const& name,
                            std::string const& sdpRecord,
                            NewConnectionCallback onNewConnection,
                            ReleaseCallback onRelease) -> hid::Result<void>;

    /// @brief Unregister the HID profile
    auto unregisterProfile() -> void;

    /// @brief Process pending D-Bus events
    /// @note Call this in your event loop
    auto processEvents() -> void;

    /// @brief Check if profile is registered
    /// @return true if profile is active
    [[nodiscard]] auto isRegistered() const -> bool;

    /// @brief Set adapter discoverable state
    /// @param adapter Adapter name
    /// @param enabled true to make discoverable
    /// @return Success or error
    auto setAdapterDiscoverable(std::string const& adapter, bool enabled) -> hid::Result<void>;

    /// @brief Set adapter pairable state
    /// @param adapter Adapter name
    /// @param enabled true to allow pairing
    /// @return Success or error
    auto setAdapterPairable(std::string const& adapter, bool enabled) -> hid::Result<void>;

    /// @brief Set adapter friendly name
    /// @param adapter Adapter name
    /// @param name Display name
    /// @return Success or error
    auto setAdapterName(std::string const& adapter, std::string const& name) -> hid::Result<void>;

    /// @brief Set adapter device class
    /// @param adapter Adapter name
    /// @param device_class Device class code
    /// @return Success or error
    auto setAdapterClass(std::string const& adapter, uint32_t device_class) -> hid::Result<void>;

    /// @brief Register pairing agent for auto-accept
    /// @return Success or error
    auto registerAgent() -> hid::Result<void>;

    /// @brief Unregister pairing agent
    auto unregisterAgent() -> void;

  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

}

#pragma once

#include "libfakekbd/hid/types.hpp"
#include <bluetooth/bluetooth.h>
#include <functional>
#include <memory>
#include <string>

namespace fakekbd::bluetooth
{

// Bluetooth Device Class constants
constexpr uint32_t DEVICE_CLASS_PERIPHERAL = 0x002540;
constexpr uint32_t DEVICE_CLASS_KEYBOARD = 0x000540;

class DBusProfileManager
{
  public:
    using NewConnectionCallback = std::function<void(bdaddr_t const& device, int fd)>;
    using ReleaseCallback = std::function<void()>;

    DBusProfileManager();
    ~DBusProfileManager();

    DBusProfileManager(DBusProfileManager const&) = delete;
    DBusProfileManager& operator=(DBusProfileManager const&) = delete;
    DBusProfileManager(DBusProfileManager&&) = delete;
    DBusProfileManager& operator=(DBusProfileManager&&) = delete;

    auto registerHidProfile(std::string const& adapter,
                            std::string const& name,
                            std::string const& sdpRecord,
                            NewConnectionCallback onNewConnection,
                            ReleaseCallback onRelease) -> hid::Result<void>;

    auto unregisterProfile() -> void;
    auto processEvents() -> void;

    [[nodiscard]] auto isRegistered() const -> bool;

    auto setAdapterDiscoverable(std::string const& adapter, bool enabled) -> hid::Result<void>;
    auto setAdapterPairable(std::string const& adapter, bool enabled) -> hid::Result<void>;
    auto setAdapterName(std::string const& adapter, std::string const& name) -> hid::Result<void>;
    auto setAdapterClass(std::string const& adapter, uint32_t device_class) -> hid::Result<void>;
    auto registerAgent() -> hid::Result<void>;
    auto unregisterAgent() -> void;

  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

}

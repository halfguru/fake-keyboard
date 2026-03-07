#pragma once

#include "libfakekbd/hid/types.hpp"
#include <bluetooth/bluetooth.h>
#include <functional>
#include <memory>
#include <string>

namespace fakekbd::bluetooth {

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
  auto registerAgent() -> hid::Result<void>;
  auto unregisterAgent() -> void;

private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}

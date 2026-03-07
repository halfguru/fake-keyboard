#pragma once

#include "libfakekbd/hid/types.hpp"
#include <bluetooth/bluetooth.h>
#include <functional>
#include <memory>
#include <string>

namespace fakekbd::bluetooth {

class dbus_profile_manager
{
public:
  using new_connection_callback = std::function<void(bdaddr_t const& device, int fd)>;
  using release_callback = std::function<void()>;

  dbus_profile_manager();
  ~dbus_profile_manager();

  dbus_profile_manager(dbus_profile_manager const&) = delete;
  dbus_profile_manager& operator=(dbus_profile_manager const&) = delete;
  dbus_profile_manager(dbus_profile_manager&&) = delete;
  dbus_profile_manager& operator=(dbus_profile_manager&&) = delete;

  auto register_hid_profile(std::string const& adapter,
                            std::string const& name,
                            std::string const& sdp_record,
                            new_connection_callback on_new_connection,
                            release_callback on_release) -> hid::Result<void>;

  auto unregisterProfile() -> void;
  auto processEvents() -> void;

  [[nodiscard]] auto isRegistered() const -> bool;

  auto setAdapterDiscoverable(std::string const& adapter, bool enabled) -> hid::Result<void>;
  auto setAdapterPairable(std::string const& adapter, bool enabled) -> hid::Result<void>;

  struct impl;

private:
  std::unique_ptr<impl> pimpl_;
};

auto
register_hid_profile(std::string const& adapter, std::string const& name, std::string const& sdp_record)
  -> hid::Result<void>;

auto
unregister_hid_profile() -> void;

}

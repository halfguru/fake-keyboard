#include "adapter.hpp"
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <cstdlib>
#include <cstring>
#include <spdlog/spdlog.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace fakekbd::bluetooth {

auto
make_discoverable(std::string const& adapter) -> hid::Result<void>
{
  std::string cmd = "bluetoothctl discoverable on 2>&1 > /dev/null";
  int result = std::system(cmd.c_str());

  if (result != 0) {
    spdlog::error("Failed to make adapter discoverable");
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  spdlog::info("Adapter {} is now discoverable", adapter);
  return {};
}

auto
make_pairable(std::string const& adapter) -> hid::Result<void>
{
  std::string cmd = "bluetoothctl pairable on 2>&1 > /dev/null";
  int result = std::system(cmd.c_str());

  if (result != 0) {
    spdlog::error("Failed to make adapter pairable");
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  spdlog::info("Adapter {} is now pairable", adapter);
  return {};
}

auto
set_device_name(std::string const& adapter, std::string const& name) -> hid::Result<void>
{
  int dev_id = hci_devid(adapter.c_str());
  if (dev_id < 0) {
    spdlog::error("Failed to get device ID for {}", adapter);
    return std::unexpected(hid::error::DeviceNotFound);
  }

  int dd = hci_open_dev(dev_id);
  if (dd < 0) {
    spdlog::error("Failed to open device {}", adapter);
    return std::unexpected(hid::error::DeviceNotFound);
  }

  if (hci_write_local_name(dd, name.c_str(), 1000) < 0) {
    spdlog::error("Failed to set device name");
    hci_close_dev(dd);
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  hci_close_dev(dd);
  spdlog::info("Device name set to: {}", name);
  return {};
}

auto
set_device_class(std::string const& adapter, uint32_t device_class) -> hid::Result<void>
{
  int dev_id = hci_devid(adapter.c_str());
  if (dev_id < 0) {
    spdlog::error("Failed to get device ID for {}", adapter);
    return std::unexpected(hid::error::DeviceNotFound);
  }

  int dd = hci_open_dev(dev_id);
  if (dd < 0) {
    spdlog::error("Failed to open device {}", adapter);
    return std::unexpected(hid::error::DeviceNotFound);
  }

  if (ioctl(dd, HCIDEVUP, dev_id) < 0 && errno != EALREADY) {
    spdlog::warn("Could not bring device up: {}", std::strerror(errno));
  }

  hci_close_dev(dd);

  spdlog::info("Note: Device class 0x{:06X} should be set via D-Bus or mgmt API", device_class);
  spdlog::warn("Setting device class requires BlueZ D-Bus API or management socket");

  return {};
}

}

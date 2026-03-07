#pragma once

#include "libfakekbd/hid/types.hpp"
#include <string>

namespace fakekbd::bluetooth {

auto
make_discoverable(std::string const& adapter = "hci0") -> hid::Result<void>;
auto
make_pairable(std::string const& adapter = "hci0") -> hid::Result<void>;
auto
set_device_name(std::string const& adapter, std::string const& name) -> hid::Result<void>;
auto
set_device_class(std::string const& adapter, uint32_t device_class) -> hid::Result<void>;

constexpr uint32_t DEVICE_CLASS_PERIPHERAL = 0x002540;
constexpr uint32_t DEVICE_CLASS_KEYBOARD = 0x000540;

}

#pragma once

#include "config.hpp"
#include "hid/keyboard.hpp"
#include "hid/report.hpp"
#include "hid/types.hpp"

namespace fakekbd {

using keyboard = hid::keyboard;
using config = fakekbd::config;
using error = hid::error;

template<typename T>
using Result = hid::Result<T>;

namespace key_code = hid::key_code;
namespace modifier = hid::modifier;

}

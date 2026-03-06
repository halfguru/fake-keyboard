# fake-keyboard

[![CI](https://github.com/halfguru/fake-keyboard/actions/workflows/ci.yml/badge.svg)](https://github.com/halfguru/fake-keyboard/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-orange.svg)](https://www.linux.org/)

Modern C++23 Bluetooth HID emulator for Linux. Turn your PC into a virtual keyboard, mouse, gamepad, or braille display.

## What is this?

`fake-keyboard` lets your Linux machine emulate Bluetooth HID devices. Connect to phones, tablets, or other computers and control them remotely.

## Features

- 🚧 Keyboard emulation
- 🚧 Braille display support (planned)
- Modern C++23 with clean API
- Works with Android, Windows, macOS, Linux

## Requirements

- Linux (BlueZ 5.50+)
- Bluetooth adapter supporting peripheral mode
- Clang 17+ or GCC 13+
- CMake 3.25+
- Conan 2.0+

**Check your adapter:**
```bash
btmgmt info | grep advertising
```

## Quick Start

### Install Dependencies

```bash
# Arch Linux
sudo pacman -S bluez bluez-utils clang cmake
pip install conan

# Ubuntu/Debian
sudo apt install bluez libbluetooth-dev clang cmake
pip install conan
```

### Build

```bash
# Install Conan dependencies
conan install . --output-folder=build --build=missing

# Configure and build using presets
cmake --preset dev-release
cmake --build --preset dev-release

# Run
./build/fake-keyboard

# Run tests
ctest --preset dev-release
```

### Usage

```bash
# Listen for connections
./build/fake-keyboard --listen

# Connect to specific device
./build/fake-keyboard --connect 00:11:22:33:44:55

# Type text
./build/fake-keyboard --connect 00:11:22:33:44:55 --type "Hello World"
```

## Pairing

1. Make your device discoverable:
   ```bash
   bluetoothctl discoverable on
   ```

2. Pair from your phone/tablet/computer

3. Trust the connection:
   ```bash
   bluetoothctl trust <device-mac>
   ```

## Configuration

Create `~/.config/fake-keyboard/config.json`:

```json
{
  "device": {
    "name": "My Fake Keyboard",
    "vendor_id": 0x1234,
    "product_id": 0x5678
  },
  "bluetooth": {
    "adapter": "hci0",
    "auto_connect": ["00:11:22:33:44:55"]
  }
}
```

## Development

### Build

```bash
# Install Conan dependencies
conan install . --output-folder=build --build=missing

# Debug build
cmake --preset dev-debug
cmake --build --preset dev-debug

# Release build
cmake --preset dev-release
cmake --build --preset dev-release

# Run tests
ctest --preset dev-release
```

### Code Quality

```bash
# Format code
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Check formatting
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run --Werror

# Run static analysis
run-clang-tidy -p build

# Run clang-tidy during build (automatic)
cmake -S . -B build -DCMAKE_CXX_CLANG_TIDY=clang-tidy
cmake --build build
```

## Limitations

- Classic Bluetooth only (no BLE HID yet)
- Requires Bluetooth adapter with peripheral mode support
- Linux only (BlueZ dependency)

## Contributing

Contributions welcome! See [AGENTS.md](AGENTS.md) for development guidelines.

## License

MIT
```

## Resources

- [Bluetooth HID Specification](https://www.bluetooth.com/specifications/specs/human-interface-device-profile-1-1-1/)
- [USB HID Usage Tables](https://usb.org/sites/default/files/hut1_4.pdf)
- [BlueZ D-Bus API](http://bluez.org/bluez-5-api/)

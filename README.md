# fake-keyboard

[![CI](https://github.com/halfguru/fake-keyboard/actions/workflows/ci.yml/badge.svg)](https://github.com/halfguru/fake-keyboard/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-orange.svg)](https://www.linux.org/)

Modern C++23 Bluetooth HID emulator for Linux. Turn your PC into a virtual keyboard that can control phones, tablets, or other computers.

## What is this?

`fake-keyboard` lets your Linux machine emulate a Bluetooth HID keyboard. Type on your computer and have the keystrokes appear on your phone or tablet.

## Features

- ✅ Keyboard emulation (Classic Bluetooth HID)
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

### 1. Install Dependencies

```bash
# Arch Linux
sudo pacman -S bluez bluez-utils clang cmake
pip install conan

# Ubuntu/Debian
sudo apt install bluez libbluetooth-dev clang cmake
pip install conan
```

### 2. Disable BlueZ Input Plugin

BlueZ's input plugin will intercept HID connections. Disable it:

```bash
# Check current bluetoothd status
ps aux | grep bluetoothd

# Edit the bluetooth service to add --noplugin=input
sudo systemctl edit bluetoothd

# Add these lines:
[Service]
ExecStart=
ExecStart=/usr/lib/bluetooth/bluetoothd --noplugin=input

# Restart bluetooth
sudo systemctl restart bluetooth
```

Or simply run with the flag:
```bash
sudo systemctl stop bluetooth
sudo /usr/lib/bluetooth/bluetoothd --noplugin=input &
```

### 3. Build

```bash
# Install Conan dependencies
conan install . --output-folder=build --build=missing

# Configure and build
cmake --preset dev-release
cmake --build --preset dev-release
```

### 4. Run

```bash
sudo ./build/fake-keyboard
```

You should see:
```
[info] fake-keyboard v0.1.0
[info] Registering HID profile with BlueZ...
[info] Registered HID profile with BlueZ
[info] Making adapter discoverable and pairable...
[info] Adapter hci0 discoverable: on
[info] Adapter hci0 pairable: on
[info] Starting L2CAP HID server on PSM 0x11 (control) and 0x13 (interrupt)...
[info] L2CAP server listening
[info] Keyboard server ready!
[info] Pair from your phone and enable 'Input device' in Bluetooth settings
[info] Press Ctrl+C to exit
```

The adapter is automatically made discoverable and pairable via BlueZ D-Bus API.

### 5. Pair from Your Phone

1. Open Bluetooth settings on your phone
2. Scan for devices
3. Find "Fake Keyboard" and tap to pair
4. Accept pairing on both devices (may require PIN - try "0000")

### 6. Enable Input Device

On your phone:
1. Go to Bluetooth settings → Paired devices
2. Find "Fake Keyboard"
3. Tap it and enable **"Input device"** (uncheck Media audio/Phone calls)

### 7. Type!

When connected, you'll see:
```
[info] Device connected on fd X
[info] Type in this terminal to send keystrokes
```

Type in the fake-keyboard terminal - characters will appear on your phone!

## Troubleshooting

**"UUID already registered"** - Restart bluetooth: `sudo systemctl restart bluetooth`

**"Address already in use"** - Kill existing process: `sudo pkill -9 fake-keyboard`

**Phone connects for audio only** - Make sure to enable "Input device" and disable "Media audio" in your phone's Bluetooth settings

**Device not discoverable** - Check: `sudo btmgmt info` should show `discoverable` in current settings

**Device not found when scanning** - Make sure fake-keyboard is running and adapter is discoverable

## Development

### Build

```bash
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
```

## Configuration

Create `~/.config/fake-keyboard/config.json`:

```json
{
  "device": {
    "name": "My Fake Keyboard"
  },
  "bluetooth": {
    "adapter": "hci0"
  }
}
```

## Limitations

- Classic Bluetooth only (no BLE HID yet)
- Requires Bluetooth adapter with peripheral mode support
- Linux only (BlueZ dependency)
- Some Android phones may have issues with Classic HID

## Contributing

Contributions welcome! See [AGENTS.md](AGENTS.md) for development guidelines.

## License

MIT

## Resources

- [Bluetooth HID Specification](https://www.bluetooth.com/specifications/specs/human-interface-device-profile-1-1-1/)
- [USB HID Usage Tables](https://usb.org/sites/default/files/hut1_4.pdf)
- [BlueZ D-Bus API](http://bluez.org/bluez-5-api/)

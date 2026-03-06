# fake-keyboard

Modern C++23 Bluetooth HID emulator for Linux. Turn your PC into a virtual keyboard, mouse, gamepad, or braille display.

## What is this?

`fake-keyboard` lets your Linux machine emulate Bluetooth HID devices. Connect to phones, tablets, or other computers and control them remotely.

**Use cases:**
- Remote control presentations from your phone
- Use your PC as a bluetooth keyboard for tablets
- Accessibility: custom input device emulation
- Testing: automate Bluetooth HID testing
- Fun: prank your friends (ethically 😉)

## Features

- ✅ Keyboard emulation
- 🚧 Mouse emulation (planned)
- 🚧 Gamepad emulation (planned)
- 🚧 Braille display support (planned)
- Modern C++23 with clean API
- Works with Android, Windows, macOS, Linux

## Requirements

- Linux (BlueZ 5.50+)
- Bluetooth adapter supporting peripheral mode
- Clang 17+ or GCC 13+
- CMake 4.2+
- Conan 2.0+

**Check your adapter:**
```bash
btmgmt info | grep advertising
```

## Quick Start

### Install Dependencies

```bash
# Arch Linux
sudo pacman -S bluez bluez-utils conan

# Ubuntu/Debian
sudo apt install bluez libbluetooth-dev
pip install conan
```

### Build

```bash
# Install Conan dependencies
conan install . --output-folder=build --build=missing

# Configure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run
./build/fake-keyboard
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

## Examples

### Python Script
```python
import subprocess

# Type via fake-keyboard
subprocess.run(["./build/fake-keyboard", "--connect", "00:11:22:33:44:55", "--type", "Hello"])
```

### Bash Script
```bash
#!/bin/bash
# Remote presentation control
./build/fake-keyboard --connect $DEVICE_MAC --key right  # Next slide
./build/fake-keyboard --connect $DEVICE_MAC --key left   # Previous slide
```

## Development

```bash
# Debug build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run tests
cd build && ctest

# Format code
find src -name "*.cpp" | xargs clang-format -i
```

## Architecture

- **Core**: C++23 library (libfakekbd)
- **CLI**: Command-line tools
- **Bluetooth**: BlueZ via D-Bus
- **Protocol**: Classic Bluetooth HID Profile (UUID 0x1124)

## Compatibility

Tested with:
- ✅ Android 10+
- ✅ Windows 10/11
- ✅ macOS 12+
- ✅ Linux (BlueZ)

## Limitations

- Classic Bluetooth only (no BLE HID yet)
- Requires Bluetooth adapter with peripheral mode support
- Linux only (BlueZ dependency)

## Roadmap

- [ ] Keyboard emulation (Phase 1)
- [ ] Mouse emulation (Phase 2)
- [ ] Gamepad emulation (Phase 3)
- [ ] Braille display support (Phase 4)
- [ ] BLE HID (HOGP) support
- [ ] Python bindings
- [ ] Web-based control interface

## Contributing

Contributions welcome! See [AGENTS.md](AGENTS.md) for development guidelines.

## License

MIT

## Credits

Inspired by [Humanware](https://github.com/halfguru/Humanware-code) braille reader implementation.

## Troubleshooting

**"Adapter not found"**
```bash
systemctl start bluetooth
bluetoothctl list
```

**"Permission denied"**
```bash
sudo usermod -a -G bluetooth $USER
# Log out and back in
```

**"Device won't connect"**
```bash
bluetoothctl remove <mac>
bluetoothctl scan on
bluetoothctl pair <mac>
bluetoothctl trust <mac>
```

## Resources

- [Bluetooth HID Specification](https://www.bluetooth.com/specifications/specs/human-interface-device-profile-1-1-1/)
- [USB HID Usage Tables](https://usb.org/sites/default/files/hut1_4.pdf)
- [BlueZ D-Bus API](http://bluez.org/bluez-5-api/)

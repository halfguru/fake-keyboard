# AGENTS.md - Guidelines for AI Coding Agents

## Project Overview

**bthid** is a modern C++23 library and toolkit for emulating Bluetooth HID devices on Linux. It provides a clean, type-safe API for creating virtual keyboards, mice, gamepads, and braille displays that can connect to other devices via Classic Bluetooth HID Profile.

**Architecture:** Shared library (libbthid) + CLI tools + optional daemon  
**Target:** Linux with BlueZ 5.50+, Clang 17+  
**Protocol:** Classic Bluetooth HID (Profile UUID 0x1124)

## Build Commands

### Prerequisites
- **Compiler:** Clang 17+ (C++23 support required)
- **Build:** CMake 3.25+, Conan 2.0+
- **System:** BlueZ 5.50+, libbluetooth-dev
- **Runtime:** Linux kernel with Bluetooth support

### Initial Setup
```bash
# Install Conan dependencies
conan install . --output-folder=build --build=missing

# Configure CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++

# Build everything
cmake --build build

# Run tests
cd build && ctest --output-on-failure
```

### Development Build
```bash
# Debug build with tests
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run specific test suite
./build/tests/bthid_tests "[keyboard]"

# Run single test
./build/tests/bthid_tests "Keyboard creates valid descriptor"
```

### Installation
```bash
# Install to /usr/local
cmake --install build

# Install to custom prefix
cmake --install build --prefix /opt/bthid
```

### Clean Build
```bash
rm -rf build
conan install . --output-folder=build --build=missing
cmake -S . -B build
cmake --build build
```

## Testing

### Running All Tests
```bash
cd build
ctest --output-on-failure
```

### Running Specific Tests
```bash
# Run unit tests only
./build/tests/bthid_tests "[unit]"

# Run keyboard tests
./build/tests/bthid_tests "[keyboard]"

# Run with verbose output
./build/tests/bthid_tests -s
```

### Test Structure
- Tests located in `tests/` directory
- Unit tests: `tests/unit/test_*.cpp`
- Integration tests: `tests/integration/test_*.cpp`
- Framework: Catch2 v3 with BDD-style tests
- Coverage: Aim for >80% on core library

## Code Style Guidelines

### File Naming
- Source files: `snake_case.cpp` (e.g., `bluetooth_manager.cpp`)
- Headers: `snake_case.hpp` (e.g., `bluetooth_manager.hpp`)
- Classes typically in their own file pair

### Naming Conventions

**Namespaces:**
- lowercase: `bthid`, `bthid::detail`

**Classes and Structs:**
- PascalCase: `BluetoothManager`, `Keyboard`, `HidDevice`

**Functions and Methods:**
- snake_case: `register_profile()`, `send_report()`, `press_key()`

**Member Variables:**
- Private: `snake_case_` with trailing underscore (e.g., `adapter_name_`, `connected_`)
- No Hungarian notation

**Local Variables:**
- snake_case: `device_address`, `report_data`

**Constants:**
- constexpr: `snake_case` (e.g., `hid_profile_uuid`)
- Enum values: PascalCase (e.g., `ErrorCode::DeviceNotFound`)

**Macros:**
- UPPER_SNAKE_CASE for logging: `BTHID_DEBUG`, `BTHID_ERROR`

### C++23 Features

**Required Usage:**
- `std::expected<T, E>` for all fallible operations
- `std::format` and `std::print` (no iostream, no fmtlib)
- `std::source_location` for error tracking
- `consteval` for compile-time descriptor validation
- `std::span` for buffer views
- `std::mdspan` for multi-dimensional data
- Deducing `this` for fluent APIs

**Smart Pointers:**
- `std::unique_ptr` for exclusive ownership
- `std::shared_ptr` only when truly needed
- No raw `new`/`delete`

**RAII:**
- All resources managed via RAII
- Use `std::unique_ptr` with custom deleters for C APIs

### Import Order
```cpp
// 1. Related header
#include "keyboard.hpp"

// 2. Project headers
#include "error.hpp"
#include "types.hpp"

// 3. Third-party library headers
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

// 4. System headers
#include <expected>
#include <memory>
#include <string>
```

### Formatting

**Indentation:**
- 4 spaces (no tabs)

**Braces:**
- Opening brace on same line
```cpp
auto result = keyboard.press_key(Key::A);
if (not result) {
    return std::unexpected(result.error());
}
```

**Line Length:**
- 120 characters maximum

**Const:**
- `const` on right for types: `std::string const& name`
- `const` on left for simple types: `const int timeout = 10`

**Auto:**
- Use `auto` for type deduction
- Use `auto*` for pointers
- Use `auto&` for references

### Error Handling

**All fallible operations return `std::expected<T, E>`:**
```cpp
// Good
Result<void> connect(Address const& device);

// Bad - no exceptions for control flow
void connect(Address const& device);  // throws
```

**Error creation:**
```cpp
// Use helper function
return make_error(ErrorCode::DeviceNotFound, 
                 "Device {} not found", device_address);

// Or construct directly
return std::unexpected(Error(ErrorCode::InvalidConfiguration, "Missing required field"));
```

**Monadic operations:**
```cpp
auto result = create_keyboard()
    .and_then([&](auto& kb) { return kb.connect(device); })
    .transform([&](auto& conn) {
        BTHID_INFO("Connected to {}", conn.address());
        return conn;
    })
    .or_else([](Error const& err) {
        BTHID_ERROR("Failed: {}", err.message());
        return std::unexpected(err);
    });
```

### Logging

**Use spdlog wrapper:**
```cpp
// Initialize at program start
Logger::init(spdlog::level::debug);

// Log with macros
BTHID_TRACE("Detailed trace: {}", value);
BTHID_DEBUG("Debug info: {}", state);
BTHID_INFO("Connected to device: {}", address);
BTHID_WARN("Unexpected state: {}", state);
BTHID_ERROR("Failed to connect: {}", error);
```

**Log Levels:**
- TRACE: Very detailed (socket operations, byte dumps)
- DEBUG: Development info (state changes, method entry/exit)
- INFO: Important events (connections, errors)
- WARN: Recoverable issues
- ERROR: Failures requiring attention

### Documentation

**File Headers:**
```cpp
/// @file keyboard.hpp
/// @brief Bluetooth keyboard HID device implementation
```

**Method Documentation:**
```cpp
/// @brief Press a key on the keyboard
/// @param key The key to press
/// @return Success or error
/// @note Keys are automatically released when destructor runs
Result<void> press_key(Key key);
```

**Inline Comments:**
- Use `//` for single-line
- Use `///` for Doxygen
- Comment *why*, not *what*

### Modern C++ Patterns

**Deducing this (fluent API):**
```cpp
template<typename Self>
auto&& set_name(this Self&& self, std::string_view name) {
    self.name_ = name;
    return std::forward<Self>(self);
}

// Usage:
auto kb = Keyboard::create()
    .set_name("My Keyboard")
    .set_vendor_id(0x1234)
    .build();
```

**Compile-time validation:**
```cpp
consteval bool is_valid_descriptor(std::span<std::byte const> desc) {
    return desc.size() >= 2 and desc[0] == std::byte{0x05};
}

constexpr auto keyboard_desc = /* ... */;
static_assert(is_valid_descriptor(keyboard_desc));
```

## Architecture Patterns

### Core Classes

**HidDevice (abstract base):**
```cpp
class HidDevice {
public:
    virtual ~HidDevice() = default;
    virtual Result<void> connect(Address const& device) = 0;
    virtual Result<void> send_report(Report const& report) = 0;
    // ...
};
```

**Keyboard (concrete implementation):**
```cpp
class Keyboard : public HidDevice {
public:
    static Result<std::unique_ptr<Keyboard>> create(DeviceConfig config = {});
    Result<void> press_key(Key key);
    Result<void> type(std::string_view text);
    // ...
};
```

### Result Type Alias
```cpp
template<typename T = void>
using Result = std::expected<T, Error>;
```

### Bluetooth Stack
- **L2CAP Server:** Socket management (PSM 0x11 control, 0x13 interrupt)
- **Service Record:** SDP XML generation for HID profile
- **Bluetooth Manager:** BlueZ D-Bus ProfileManager1 integration
- **Connection Handler:** Accept/manage device connections

### Configuration
- JSON-based (nlohmann/json)
- Device, Bluetooth, and HID sections
- Load from file or parse string
- Validate at runtime

## Dependencies

### Conan (Managed)
- **spdlog/1.12.0:** Fast logging library
- **nlohmann_json/3.11.2:** JSON parsing
- **catch2/3.5.0:** Unit testing framework

### System (pkg-config)
- **bluez:** Bluetooth stack (libbluetooth)
- **libsystemd:** Optional systemd integration

### Compiler
- **Clang 17+** with C++23 support
- **libc++** or **libstdc++** (C++23 complete)

## Important Notes

### Bluetooth Requirements
- BlueZ 5.50 or higher
- Bluetooth adapter supporting peripheral mode
- Check with: `btmgmt info | grep advertising`

### HID Profile Details
- **UUID:** 00001124-0000-1000-8000-00805f9b34fb
- **L2CAP PSMs:** 0x11 (control), 0x13 (interrupt)
- **Report Descriptor:** USB HID specification compliant
- **Boot Protocol:** Optional (disabled by default)

### Platform Support
- **Primary:** Linux (BlueZ)
- **Tested:** Arch Linux, Ubuntu 22.04+
- **Future:** FreeBSD, other Unix-like systems

### Performance Targets
- Report latency: <10ms
- Connection time: <2s
- Memory footprint: <5MB

### Security
- Pairing required by default
- Encryption enforced (BLE Secure Connections not applicable to Classic)
- No hardcoded credentials

### Compatibility
Tested with:
- Android 10+
- Windows 10/11
- macOS 12+
- Linux (BlueZ)
- iOS (limited support for Classic HID)

### Reference Implementation
This project is inspired by the Humanware braille reader codebase (`/home/simon/codemaxxing/Humanware-code/bluetoothmanager/bluetoothHid/`) which demonstrates a production Classic Bluetooth HID implementation. Key learnings:
- L2CAP socket handling
- SDP service record structure
- BlueZ D-Bus ProfileManager1 API
- HID report descriptor format

### Debugging

**Enable verbose logging:**
```bash
export SPDLOG_LEVEL=debug
./bthid-keyboard --listen
```

**Monitor Bluetooth:**
```bash
# Watch D-Bus messages
dbus-monitor --system "sender='org.bluez'"

# Bluetoothctl for pairing
bluetoothctl
[bluetoothctl] scan on
[bluetoothctl] pair <mac>
```

**Check adapter capabilities:**
```bash
btmgmt info
hciconfig hci0 version
```

### Common Issues

**"Adapter not found":**
- Check Bluetooth service: `systemctl status bluetooth`
- Verify adapter: `bluetoothctl list`

**"Permission denied":**
- Add user to bluetooth group: `sudo usermod -a -G bluetooth $USER`
- Restart session

**"Connection refused":**
- Device may not support Classic HID
- Check pairing: `bluetoothctl paired-devices`
- Try removing and re-pairing

**Compilation errors:**
- Ensure Clang 17+: `clang++ --version`
- Check CMake version: `cmake --version`
- Verify Conan profile: `conan profile list`

# AGENTS.md - Coding Agent Guidelines

## Workflow Orchestration

### 1. Plan Mode Default
- Enter plan mode for ANY non-trivial task (3+ steps or architectural decisions)
- If something goes sideways, STOP and re-plan immediately - don't keep pushing
- Use plan mode for verification steps, not just building
- Write detailed specs upfront to reduce ambiguity

### 2. Self-Improvement Loop
- After ANY correction from the user: update `tasks/lessons.md` with the pattern
- Write rules for yourself that prevent the same mistake
- Ruthlessly iterate on these lessons until mistake rate drops
- Review lessons at session start for relevant project

### 3. Verification Before Done
- Never mark a task complete without proving it works
- Diff behavior between main and your changes when relevant
- Ask yourself: "Would a staff engineer approve this?"
- Run tests, check logs, demonstrate correctness

### 4. Demand Elegance (Balanced)
- For non-trivial changes: pause and ask "is there a more elegant way?"
- If a fix feels hacky: "Knowing everything I know now, implement the elegant solution"
- Skip this for simple, obvious fixes - don't over-engineer
- Challenge your own work before presenting it

### 5. Autonomous Bug Fixing
- When given a bug report: just fix it. Don't ask for hand-holding
- Point at logs, errors, failing tests then resolve them
- Zero context switching required from the user
- Go fix failing CI tests without being told how

## Task Management

1. **Plan First**: Write plan to `tasks/todo.md` with checkable items
2. **Verify Plan**: Check in before starting implementation
3. **Track Progress**: Mark items complete as you go
4. **Explain Changes**: High-level summary at each step
5. **Document Results**: Add review section to plan `tasks/todo.md` 
6. **Capture Lessons**: Update `tasks/lessons.md` after corrections

## Core Principles

- **Simplicity First**: Make every change as simple as possible. Impact minimal code.
- **No Laziness**: Find root causes. No temporary fixes. Senior developer standards.
- **Minimal Impact**: Changes should only touch what's necessary. Avoid introducing bugs.

## Project Overview

**fake-keyboard** is a modern C++23 library and toolkit for emulating Bluetooth HID devices on Linux. It provides a clean, type-safe API for creating virtual keyboards and braille displays that can connect to other devices via Classic Bluetooth HID Profile.

**Architecture:** Shared library (libfakekbd) + CLI tools + optional daemon  
**Target:** Linux with BlueZ 5.50+, Clang 17+  
**Protocol:** Classic Bluetooth HID (Profile UUID 0x1124)

## Build Commands

### Prerequisites
- **Compiler:** Clang 17+ (you have 21.1.8 ✅)
- **Tooling:** clang-tidy, clang-format (you have 21.1.8 ✅)
- **Build:** CMake 3.25+, Conan 2.0+
- **System:** BlueZ 5.50+, libbluetooth-dev
- **Runtime:** Linux kernel with Bluetooth support

### Initial Setup
```bash
# Install Conan dependencies
conan install . --output-folder=build --build=missing

# Configure and build using presets
cmake --preset dev-release
cmake --build --preset dev-release

# Run tests
ctest --preset dev-release
```

### Development Build
```bash
# Debug build with presets
cmake --preset dev-debug
cmake --build --preset dev-debug

# Run specific tests
./build/fake-keyboard-tests "[unit]"
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
cmake --preset dev-release
cmake --build --preset dev-release
```

## Code Quality

### Linting with clang-tidy
```bash
# Run clang-tidy on all source files
run-clang-tidy -p build

# Run on specific file
clang-tidy src/main.cpp -p build

# Auto-fix issues
clang-tidy src/main.cpp -p build --fix
```

### Formatting with clang-format
```bash
# Format all files in place
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Check formatting without modifying
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run --Werror

# Format specific file
clang-format -i src/main.cpp
```

### Pre-commit Checks
Always run before committing:
```bash
# 1. Format check
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run --Werror

# 2. Static analysis
run-clang-tidy -p build

# 3. Build with warnings as errors
cmake --build build 2>&1 | grep -E "error:|warning:" && exit 1

# 4. Run tests
ctest --preset dev-release
```

## Testing

### Running All Tests
```bash
ctest --preset dev-release
```

### Running Specific Tests
```bash
# Run unit tests only
./build/fake-keyboard-tests "[unit]"

# Run with verbose output
./build/fake-keyboard-tests -s
```

### Test Structure
- Tests located in `tests/` directory
- Framework: Catch2 v3 with BDD-style tests
- Coverage: Aim for >80% on core library

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

### Debugging

**Enable verbose logging:**
```bash
export SPDLOG_LEVEL=debug
sudo ./build/fake-keyboard
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

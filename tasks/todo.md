# Implementation Plan: libfakekbd Core Library

## Overview
Create the core Bluetooth HID emulation library with clean C++23 API.

## Architecture
```
src/
├── libfakekbd/
│   ├── bluetooth/
│   │   ├── l2cap_server.hpp/cpp      # L2CAP socket management (PSM 0x11, 0x13)
│   │   ├── sdp_record.hpp/cpp        # SDP service record XML generation
│   │   └── adapter.hpp/cpp           # Bluetooth adapter management
│   ├── hid/
│   │   ├── device.hpp/cpp            # HID device base class
│   │   ├── keyboard.hpp/cpp          # Keyboard HID implementation
│   │   ├── report.hpp/cpp            # HID report descriptor builder
│   │   └── types.hpp                 # HID types and constants
│   ├── config.hpp/cpp                # JSON configuration
│   └── fakekbd.hpp                   # Public API header
├── main.cpp                          # CLI entry point (updated)
└── CMakeLists.txt
```

## Implementation Steps

### Phase 1: Core Types and Configuration
- [x] Create `src/libfakekbd/` directory structure
- [x] Implement `hid/types.hpp` - HID constants, usage codes, report types
- [x] Implement `config.hpp/cpp` - JSON config parsing (device name, vendor/product ID, adapter)
- [x] Add unit tests for config parsing

### Phase 2: HID Report Builder
- [x] Implement `hid/report.hpp/cpp` - HID report descriptor builder
- [x] Support keyboard report descriptor (modifier keys + 6 key rollover)
- [x] Add unit tests for report descriptor generation

### Phase 3: Bluetooth Foundation
- [x] Implement `bluetooth/adapter.hpp/cpp` - Adapter discovery and selection
- [x] Implement `bluetooth/l2cap.hpp/cpp` - L2CAP socket server
  - Control channel (PSM 0x11)
  - Interrupt channel (PSM 0x13)
- [x] Implement `bluetooth/sdp.hpp/cpp` - SDP XML service record
- [x] Implement `bluetooth/dbus.hpp/cpp` - D-Bus ProfileManager1 integration
- [ ] Add unit tests with mocked sockets

### Phase 4: HID Device API
- [x] Implement `hid/device.hpp/cpp` - Base HID device class
  - Virtual methods for connect/disconnect/send
  - Connection state management
- [x] Implement `hid/keyboard.hpp/cpp` - Keyboard device
  - `send_key(key_code, pressed)`
  - `send_text(string)`
  - Modifier key support (Ctrl, Alt, Shift, etc.)
- [ ] Add integration tests

### Phase 5: Public API
- [x] Create `fakekbd.hpp` - Unified public header
- [x] Export clean API:
  ```cpp
  namespace fakekbd {
    class keyboard {
    public:
      keyboard();
      auto listen(std::string const& adapter) -> Result<void>;
      auto connect(bdaddr_t const& addr) -> Result<void>;
      auto send_key(uint8_t key, bool pressed, uint8_t modifiers) -> Result<void>;
      auto send_text(std::string_view text) -> Result<void>;
      auto disconnect() -> void;
    };
  }
  ```

### Phase 6: Build System Updates
- [x] Update CMakeLists.txt to build shared library
- [x] Create `fakekbdConfig.cmake` for find_package support
- [x] Link executable to library
- [x] Update Conan recipe for library export

### Phase 7: Documentation and Examples
- [ ] Add inline Doxygen documentation
- [ ] Create `examples/simple_keyboard.cpp`
- [ ] Update README with library usage

## Technical Decisions

### Error Handling
- Use `std::expected<T, E>` for recoverable errors
- Throw exceptions only for unrecoverable system errors
- Custom error types in `fakekbd::error` namespace

### Threading Model
- L2CAP server runs in background thread
- `send_key()` is thread-safe
- Use `std::jthread` for background workers

### Memory Management
- RAII for all resources (sockets, file descriptors)
- Use `std::unique_ptr` for ownership
- No raw pointers in public API

### Dependencies
- System: bluez (libbluetooth)
- Conan: spdlog, nlohmann_json
- Optional: libsystemd (for sd_notify)

## Testing Strategy
- Unit tests: Catch2 with mocks
- Integration tests: Virtual Bluetooth adapters (hci0/hci1)
- Manual testing: Connect to Android/iOS/Windows

**Progress as of 2026-03-06:**

✅ **Completed:**
- Core library structure (`src/libfakekbd/`)
- HID types with C++23 features (`hid/types.hpp`)
- HID report descriptor builder (`hid/report.hpp/cpp`)
- Bluetooth L2CAP server/client (`bluetooth/l2cap.hpp/cpp`)
- Bluetooth adapter utilities (`bluetooth/adapter.hpp/cpp`)
- **SDP service record XML generation (`bluetooth/sdp.hpp/cpp`)**
- **D-Bus ProfileManager1 integration (`bluetooth/dbus.hpp/cpp`)**
- Keyboard device API (`hid/keyboard.hpp/cpp`)
- Configuration parser (`config.hpp/cpp`)
- Shared library `libfakekbd.so` (845KB)
- Public API header (`fakekbd.hpp`)
- Build system with CMake package support
- **Unit tests for config parsing (11 tests)**
- **Unit tests for report descriptor builder (18 tests)**
- **Unit tests for SDP service record (23 tests)**
- **Code quality checks (clang-format, clang-tidy)**

🚧 **In Progress:**
- Unit tests for keyboard helper functions
- Integration testing with real Bluetooth hardware

📋 **Remaining:**
- Documentation (Doxygen)
- Example programs
- Systemd service integration

**Library Size:** 845KB
**Exports:** All major APIs visible (L2CAP, keyboard, report builder)
- [x] All unit tests pass
- [x] Code formatted with clang-format
- [x] No clang-tidy warnings
- [ ] Library compiles with -Wall -Wextra -Wpedantic -Werror
- [ ] Can build example program
- [ ] Documentation builds without warnings

## Technical Decisions
1. **BlueZ Integration**: D-Bus ProfileManager1 (modern BlueZ approach)
2. **L2CAP Sockets**: Raw sockets for data channels (PSM 0x11 control, 0x13 interrupt)
3. **HID Protocol**: Report Protocol only (no Boot Protocol)
4. **Scope**: Keyboard-only MVP (extensible to other HID devices)
5. **Architecture**: Modern C++23 with clean API

## Design Principles
- Use `std::expected<T, E>` for error handling
- RAII for all resources
- Thread-safe by default
- Zero-cost abstractions where possible
- Clean, intuitive public API

# Fake Keyboard Design Document

## Overview

Fake Keyboard is a C++23 library that emulates Bluetooth HID devices on Linux. It allows your computer to act as a virtual keyboard that can connect to other devices (phones, tablets, other computers) via Classic Bluetooth.

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         Application Layer                        │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │   CLI Tool      │  │   Your App      │  │   Daemon        │ │
│  │   (main.cpp)    │  │   (libfakekbd)  │  │   (optional)    │ │
│  └────────┬────────┘  └────────┬────────┘  └────────┬────────┘ │
└───────────┼─────────────────────┼─────────────────────┼─────────┘
            │                     │                     │
            └─────────────────────┼─────────────────────┘
                                  │
┌─────────────────────────────────┼─────────────────────────────┐
│                         Library Layer                           │
│  ┌──────────────────────────────┴──────────────────────────┐  │
│  │                    hid::keyboard                         │  │
│  │  • Connection state management                           │  │
│  │  • Key event sending                                     │  │
│  │  • Text input simulation                                 │  │
│  └────────────┬────────────────────────┬────────────────────┘  │
│               │                        │                        │
│  ┌────────────┴──────────┐  ┌─────────┴──────────┐            │
│  │   hid::report         │  │   bluetooth::*     │            │
│  │   • HID descriptors   │  │   • l2cap_server   │            │
│  │   • Report builder    │  │   • l2cap_client   │            │
│  │   • USB HID spec      │  │   • sdp_record     │            │
│  └───────────────────────┘  │   • DBusProfileMgr │            │
│                             └─────────┬──────────┘            │
└───────────────────────────────────────┼───────────────────────┘
                                        │
┌───────────────────────────────────────┼───────────────────────┐
│                         System Layer                            │
│  ┌────────────────┐  ┌───────────────┴────────────┐           │
│  │   BlueZ        │  │   Linux Kernel             │           │
│  │   (D-Bus API)  │  │   • Bluetooth stack        │           │
│  │                │  │   • L2CAP sockets          │           │
│  │                │  │   • HID protocol           │           │
│  └────────────────┘  └────────────────────────────┘           │
└────────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. HID Report System (`hid::report`)

**Purpose**: Defines the structure of data sent to the host device.

HID (Human Interface Device) reports follow the USB HID specification. Each report describes:
- What data the device sends (keys pressed, LEDs, etc.)
- How the data is formatted (bit fields, byte arrays)
- The logical range of values

```
┌─────────────────────────────────────────┐
│        Keyboard HID Report              │
├─────────────────────────────────────────┤
│ Byte 0: Modifier keys (8 bits)          │
│   • Ctrl, Shift, Alt, GUI (Win/Cmd)     │
├─────────────────────────────────────────┤
│ Byte 1: Reserved (0x00)                 │
├─────────────────────────────────────────┤
│ Bytes 2-7: Key codes (6 concurrent)     │
│   • Up to 6 simultaneous key presses    │
│   • 0x00 = no key                       │
└─────────────────────────────────────────┘
```

**Key Classes**:
- `report_descriptor_builder`: Fluent API for building HID descriptors
- Constants in `usage_page`, `usage`, `collection`, `report_flags` namespaces

**Example**:
```cpp
auto descriptor = report_descriptor_builder{}
    .usage_page(usage_page::GENERIC_DESKTOP)
    .usage(usage::KEYBOARD)
    .collection(collection::APPLICATION)
    .usage_page(usage_page::KEYBOARD)
    .usage_min(0xE0)  // Left Ctrl
    .usage_max(0xE7)  // Right GUI
    .logical_min(0)
    .logical_max(1)
    .report_size(1)
    .report_count(8)
    .input(report_flags::DATA | report_flags::VARIABLE | report_flags::ABSOLUTE)
    .end_collection()
    .build();
```

### 2. L2CAP Layer (`bluetooth::l2cap`)

**Purpose**: Provides reliable data transport over Bluetooth.

L2CAP (Logical Link Control and Adaptation Protocol) is a core Bluetooth protocol that provides:
- Connection-oriented channels
- Protocol multiplexing (multiple services on one connection)
- Segmentation and reassembly

For HID, we use two fixed PSMs (Protocol/Service Multiplexers):
- **PSM 0x11 (17)**: Control channel - device configuration
- **PSM 0x13 (19)**: Interrupt channel - HID reports

```
┌────────────────┐                    ┌────────────────┐
│   Fake         │                    │   Host         │
│   Keyboard     │                    │   (Phone/PC)   │
├────────────────┤                    ├────────────────┤
│ l2cap_server   │◄──── PSM 0x11 ────►│ Control        │
│ (listen)       │     Control        │ Channel        │
│                │                    │                │
│ l2cap_server   │◄──── PSM 0x13 ────►│ Interrupt      │
│ (listen)       │     Interrupt      │ Channel        │
│                │     (HID Reports)  │                │
└────────────────┘                    └────────────────┘
```

**Key Classes**:
- `l2cap_socket`: Base class for socket management
- `l2cap_server`: Accepts incoming connections
- `l2cap_client`: Initiates outgoing connections

**Flow**:
1. Server listens on both PSMs
2. Host connects to control channel first
3. Host connects to interrupt channel second
4. HID reports are sent on interrupt channel

### 3. SDP Layer (`bluetooth::sdp`)

**Purpose**: Advertises device capabilities to potential hosts.

SDP (Service Discovery Protocol) lets hosts discover:
- What services are available
- How to connect to them
- Device characteristics (name, vendor, product ID)

The SDP record is an XML document that describes the HID service:

```xml
<record>
  <!-- Service Class: HID -->
  <attribute id="0x0001">
    <sequence>
      <uuid value="0x1124"/>  <!-- HID UUID -->
    </sequence>
  </attribute>
  
  <!-- Protocol Descriptor List -->
  <attribute id="0x0004">
    <sequence>
      <sequence>
        <uuid value="0x0100"/>  <!-- L2CAP -->
        <uint16 value="0x0011"/> <!-- Control PSM -->
      </sequence>
      <sequence>
        <uuid value="0x0011"/>  <!-- HIDP -->
        <uint16 value="0x0013"/> <!-- Interrupt PSM -->
      </sequence>
    </sequence>
  </attribute>
  
  <!-- HID Descriptor -->
  <attribute id="0x0206">
    <sequence>
      <!-- Base64-encoded report descriptor -->
    </sequence>
  </attribute>
</record>
```

**Key Classes**:
- `sdp_record`: Builder for SDP XML records
- Helper functions: `build_hid_sdp_record()`, `build_keyboard_sdp_record()`

### 4. D-Bus Layer (`bluetooth::dbus`)

**Purpose**: Integrates with BlueZ (Linux Bluetooth stack) via D-Bus.

BlueZ exposes its functionality through D-Bus interfaces:
- `org.bluez.Adapter1`: Bluetooth adapter control
- `org.bluez.ProfileManager1`: Register custom profiles
- `org.bluez.AgentManager1`: Pairing agent registration

```
┌─────────────────────────────────────────────────┐
│              BlueZ D-Bus Architecture            │
├─────────────────────────────────────────────────┤
│                                                 │
│  ┌─────────────┐      ┌──────────────────┐     │
│  │  Your App   │      │  org.bluez       │     │
│  │             │      │  ProfileManager1 │     │
│  │ DBusProfile │─────►│                  │     │
│  │ Manager     │      │  RegisterProfile │     │
│  └─────────────┘      └──────────────────┘     │
│         │                       │               │
│         │                       ▼               │
│         │              ┌──────────────────┐     │
│         │              │  org.bluez       │     │
│         └─────────────►│  Adapter1        │     │
│                        │                  │     │
│         callbacks      │  SetDiscoverable │     │
│         (on connect)   │  SetPairable     │     │
│                        └──────────────────┘     │
│                                                 │
└─────────────────────────────────────────────────┘
```

**Key Class**: `DBusProfileManager`

**Responsibilities**:
1. Register HID profile with BlueZ
2. Handle incoming connections (callbacks)
3. Configure adapter (discoverable, pairable, name)
4. Register pairing agent for auto-accept

**Connection Flow**:
```
1. Register HID Profile
   └─► BlueZ knows we're a keyboard

2. Set Adapter Properties
   └─► Other devices can discover us

3. Register Pairing Agent
   └─► Auto-accept pairing requests

4. Wait for Connection
   └─► Host connects → callback fires

5. Accept Connection
   └─► Get file descriptors for L2CAP channels

6. Send HID Reports
   └─► Write to interrupt channel fd
```

### 5. HID Keyboard (`hid::keyboard`)

**Purpose**: High-level keyboard interface combining all components.

**Key Class**: `hid::keyboard` (extends `hid::device`)

**State Machine**:
```
          listen()
┌─────────┐ ────────► ┌───────────┐
│Disconnected│         │ Connecting │
└─────────┘           └───────────┘
     ▲                     │
     │                     │ on connection
     │ disconnect()        ▼
     │              ┌───────────┐
     └──────────────│ Connected │
                    └───────────┘
```

**Public API**:
```cpp
class keyboard {
  auto listen(std::string const& adapter) -> Result<void>;
  auto connect(bdaddr_t const& client_addr) -> Result<void>;
  auto disconnect() -> void;
  
  auto send_key(uint8_t key_code, bool pressed, uint8_t modifiers = 0) -> Result<void>;
  auto send_text(std::string_view text) -> Result<void>;
};
```

## Data Flow: Typing a Key

Let's trace the complete flow when you type the letter 'A':

```
┌────────────────────────────────────────────────────────────────┐
│ 1. Application Layer                                           │
│    keyboard.send_key(0x04, true, LEFT_SHIFT)                  │
│    • 0x04 = HID code for 'a'                                  │
│    • LEFT_SHIFT modifier makes it 'A'                         │
└────────────────────────┬───────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────────────────┐
│ 2. HID Report Building                                         │
│    build_report(0x04, true, LEFT_SHIFT)                       │
│                                                                │
│    Result (8 bytes):                                          │
│    [0x02] [0x00] [0x04] [0x00] [0x00] [0x00] [0x00] [0x00]   │
│     │      │      │                                           │
│     │      │      └─ Key code 'a'                             │
│     │      └──────── Reserved                                 │
│     └─────────── Left Shift modifier                          │
└────────────────────────┬───────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────────────────┐
│ 3. L2CAP Transmission                                          │
│    send_hid_report(interrupt_fd, report_data, 8)              │
│                                                                │
│    • Uses interrupt channel (PSM 0x13)                        │
│    • Guaranteed delivery via L2CAP                            │
│    • Low latency (no ACK required from host)                  │
└────────────────────────┬───────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────────────────┐
│ 4. Host Processing                                             │
│    • Host receives report on interrupt channel                │
│    • OS interprets according to HID descriptor                │
│    • Generates 'A' keystroke                                  │
│    • Application receives 'A' input                           │
└────────────────────────────────────────────────────────────────┘
```

## Connection Sequence

Complete sequence from startup to connected:

```
Fake Keyboard                              Host (Phone/PC)
    │                                           │
    │  ┌─────────────────────────────┐         │
    │  │ 1. Initialization           │         │
    │  │    • Create L2CAP servers   │         │
    │  │    • Listen on PSM 0x11,13  │         │
    │  └─────────────────────────────┘         │
    │                                           │
    │  ┌─────────────────────────────┐         │
    │  │ 2. SDP Registration         │         │
    │  │    • Register HID profile   │         │
    │  │    • Publish SDP record     │◄────────┤ Host discovers
    │  │    • Set discoverable       │         │
    │  └─────────────────────────────┘         │
    │                                           │
    │  ┌─────────────────────────────┐         │
    │  │ 3. Pairing                  │         │
    │  │    • Host initiates pair    │◄────────┤ Pair request
    │  │    • Agent auto-accepts     │         │
    │  │    • Exchange link keys     │────────►│ Paired
    │  └─────────────────────────────┘         │
    │                                           │
    │  ┌─────────────────────────────┐         │
    │  │ 4. Connection               │         │
    │  │    • Host connects PSM 0x11 │◄────────┤ Control channel
    │  │    • Accept control         │────────►│
    │  │    • Host connects PSM 0x13 │◄────────┤ Interrupt channel
    │  │    • Accept interrupt       │────────►│
    │  │    • Connection established │         │
    │  └─────────────────────────────┘         │
    │                                           │
    │  ┌─────────────────────────────┐         │
    │  │ 5. HID Reports              │         │
    │  │    • send_key()             │────────►│ Key press
    │  │    • send_text()            │────────►│ Text input
    │  └─────────────────────────────┘         │
    │                                           │
```

## Protocol Stack

```
┌────────────────────────────────────────────┐
│           Application Layer                │
│         (Your code / CLI tool)             │
├────────────────────────────────────────────┤
│           HID Layer                        │
│  • Report descriptors                      │
│  • Keyboard/mouse/braille reports          │
│  • Usage tables (keyboard codes)           │
├────────────────────────────────────────────┤
│           HIDP (HID Protocol)              │
│  • Maps HID to Bluetooth                   │
│  • Virtual cable model                     │
│  • No additional framing                   │
├────────────────────────────────────────────┤
│           L2CAP Layer                      │
│  • PSM 0x11: Control channel               │
│  • PSM 0x13: Interrupt channel             │
│  • Reliable delivery                       │
├────────────────────────────────────────────┤
│           Baseband Layer                   │
│  • ACL links (Asynchronous Connection-Less)│
│  • Frequency hopping                       │
│  • Encryption (AES-CCM)                    │
└────────────────────────────────────────────┘
```

## Key Technical Details

### HID Report Descriptor

The report descriptor tells the host how to interpret the data. It's written in a bytecode-like format:

```
Usage Page (Generic Desktop)     05 01
Usage (Keyboard)                 09 06
Collection (Application)         A1 01
  Usage Page (Keyboard)          05 07
  Usage Minimum (224)            19 E0
  Usage Maximum (231)            29 E7
  Logical Minimum (0)            15 00
  Logical Maximum (1)            25 01
  Report Size (1)                75 01
  Report Count (8)               95 08
  Input (Data,Var,Abs)           81 02
  Report Count (1)               95 01
  Report Size (8)                75 08
  Input (Constant)               81 01
  ... (more fields)
End Collection                   C0
```

### Bluetooth Classic vs BLE

This library uses **Bluetooth Classic** (not BLE):
- **Classic**: HID Profile, higher bandwidth, always connected
- **BLE**: HID over GATT, lower power, connectionless

Most desktop OSes support Classic HID better. BLE HID is newer and has compatibility issues.

### Security

- **Pairing**: Required by default
- **Encryption**: AES-CCM after pairing
- **Authentication**: Link keys stored by BlueZ
- **Agent**: Auto-accepts pairing (can be customized)

## Configuration

Configuration is JSON-based:

```json
{
  "device": {
    "name": "My Keyboard",
    "vendor_id": 0x1234,
    "product_id": 0x5678,
    "version": 0x0001
  },
  "bluetooth": {
    "adapter": "hci0",
    "auto_connect": false,
    "trusted_devices": ["AA:BB:CC:DD:EE:FF"]
  }
}
```

## Error Handling

All fallible operations return `Result<T>` (std::expected):

```cpp
auto result = keyboard.listen("hci0");
if (!result) {
    spdlog::error("Failed to listen: {}", result.error().message);
    return 1;
}
```

## Threading Model

- **Main thread**: D-Bus event loop
- **Accept thread**: Blocking accept() on L2CAP servers
- **Atomic state**: Connection state is thread-safe

## Performance

- **Report latency**: <10ms typical
- **Connection time**: 1-2 seconds
- **Memory footprint**: <5MB
- **CPU usage**: Negligible when idle

## Platform Support

- **Primary**: Linux with BlueZ 5.50+
- **Kernel**: Bluetooth subsystem with L2CAP support
- **Hardware**: Any Bluetooth adapter supporting peripheral mode

Check adapter capabilities:
```bash
btmgmt info | grep advertising
```

## References

- [USB HID Specification](https://www.usb.org/hid)
- [Bluetooth HID Profile](https://www.bluetooth.com/specifications/specs/human-interface-device-profile-1-1-1/)
- [BlueZ D-Bus API](https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc)
- [HID Usage Tables](https://usb.org/sites/default/files/hut1_4.pdf)

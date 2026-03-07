#include "keyboard.hpp"
#include <bluetooth/l2cap.h>
#include <cctype>
#include <cstring>
#include <spdlog/spdlog.h>

namespace fakekbd::hid {

device::device()
  : state_(connection_state::Disconnected)
{
}

device::~device() = default;

auto
device::state() const -> connection_state
{
  return state_.load();
}

auto
device::is_connected() const -> bool
{
  return state_.load() == connection_state::Connected;
}

void
device::set_state(connection_state new_state)
{
  state_.store(new_state);
}

keyboard::keyboard()
  : device()
  , control_fd_(-1)
  , interrupt_fd_(-1)
  , running_(false)
{
}

keyboard::~keyboard()
{
  disconnect();
}

auto
keyboard::listen(std::string const& adapter) -> Result<void>
{
  auto addr_result = bluetooth::get_adapter_address(adapter);
  if (!addr_result) {
    return std::unexpected(addr_result.error());
  }

  auto addr = addr_result.value();

  if (auto result = control_server_.listen(addr, L2CAP_PSM_CONTROL); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = interrupt_server_.listen(addr, L2CAP_PSM_INTERRUPT); !result) {
    return std::unexpected(result.error());
  }

  char addr_str[18] = { 0 };
  ba2str(&addr, addr_str);
  spdlog::info("Keyboard HID server listening on {}", addr_str);

  running_ = true;
  accept_thread_ = std::thread([this] {
    while (running_) {
      auto control_result = control_server_.accept();
      if (!control_result) {
        if (running_) {
          spdlog::error("Failed to accept control connection");
        }
        continue;
      }

      auto interrupt_result = interrupt_server_.accept();
      if (!interrupt_result) {
        spdlog::error("Failed to accept interrupt connection");
        close(control_result->first);
        continue;
      }

      auto [ctrl_fd, ctrl_addr] = *control_result;
      auto [intr_fd, intr_addr] = *interrupt_result;

      char addr_str[18] = { 0 };
      ba2str(&ctrl_addr, addr_str);
      spdlog::info("HID connection from {}", addr_str);

      handle_connection(ctrl_fd, intr_fd);
    }
  });

  return {};
}

auto
keyboard::connect(bdaddr_t const& client_addr) -> Result<void>
{
  std::lock_guard lock(connection_mutex_);

  set_state(connection_state::Connecting);

  auto adapter_addr = bluetooth::get_adapter_address("hci0");
  if (!adapter_addr) {
    set_state(connection_state::Disconnected);
    return std::unexpected(adapter_addr.error());
  }

  control_client_ = std::make_unique<bluetooth::l2cap_client>();
  interrupt_client_ = std::make_unique<bluetooth::l2cap_client>();

  if (auto result = control_client_->connect(*adapter_addr, client_addr, L2CAP_PSM_CONTROL); !result) {
    set_state(connection_state::Disconnected);
    return std::unexpected(result.error());
  }

  if (auto result = interrupt_client_->connect(*adapter_addr, client_addr, L2CAP_PSM_INTERRUPT); !result) {
    set_state(connection_state::Disconnected);
    control_client_.reset();
    return std::unexpected(result.error());
  }

  control_fd_ = control_client_->fd();
  interrupt_fd_ = interrupt_client_->fd();

  set_state(connection_state::Connected);

  char addr_str[18] = { 0 };
  ba2str(&client_addr, addr_str);
  spdlog::info("Connected to HID device at {}", addr_str);

  return {};
}

auto
keyboard::disconnect() -> void
{
  running_ = false;

  if (accept_thread_.joinable()) {
    accept_thread_.join();
  }

  if (control_fd_ >= 0) {
    close(control_fd_);
    control_fd_ = -1;
  }

  if (interrupt_fd_ >= 0) {
    close(interrupt_fd_);
    interrupt_fd_ = -1;
  }

  control_client_.reset();
  interrupt_client_.reset();

  set_state(connection_state::Disconnected);
  spdlog::info("Disconnected");
}

auto
keyboard::send_key(uint8_t key_code, bool pressed, uint8_t modifiers) -> Result<void>
{
  if (!is_connected()) {
    return std::unexpected(error::NotConnected);
  }

  auto report = build_report(key_code, pressed, modifiers);

  return bluetooth::send_hid_report(interrupt_fd_, &report, sizeof(report));
}

auto
keyboard::send_text(std::string_view text) -> Result<void>
{
  if (!is_connected()) {
    return std::unexpected(error::NotConnected);
  }

  for (char c : text) {
    uint8_t modifiers = modifiers_from_char(c);
    uint8_t key = key_from_char(c);

    if (auto result = send_key(key, true, modifiers); !result) {
      return result;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (auto result = send_key(0, false, 0); !result) {
      return result;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return {};
}

auto
keyboard::control_fd() const -> int
{
  return control_fd_;
}

auto
keyboard::interrupt_fd() const -> int
{
  return interrupt_fd_;
}

auto
keyboard::handle_connection(int control_fd, int interrupt_fd) -> void
{
  std::lock_guard lock(connection_mutex_);

  control_fd_ = control_fd;
  interrupt_fd_ = interrupt_fd;
  set_state(connection_state::Connected);

  spdlog::info("HID device connected");
}

auto
keyboard::build_report(uint8_t key_code, bool pressed, uint8_t modifiers) const -> KeyboardReport
{
  KeyboardReport report;
  report.modifiers = modifiers;
  report.reserved = 0;

  if (pressed && key_code != 0) {
    report.key_codes[0] = key_code;
  }

  return report;
}

auto
key_from_char(char c) -> uint8_t
{
  if (c >= 'a' && c <= 'z') {
    return key_code::A + (c - 'a');
  }

  if (c >= 'A' && c <= 'Z') {
    return key_code::A + (c - 'A');
  }

  if (c >= '1' && c <= '9') {
    return 0x1E + (c - '1');
  }

  if (c == '0') {
    return 0x27;
  }

  switch (c) {
    case ' ':
      return key_code::SPACE;
    case '\n':
      return key_code::ENTER;
    case '\t':
      return key_code::TAB;
    case '\b':
      return key_code::BACKSPACE;
    case '.':
      return 0x37;
    case ',':
      return 0x36;
    case '/':
      return 0x38;
    case ';':
      return 0x33;
    case '\'':
      return 0x34;
    case '[':
      return 0x2F;
    case ']':
      return 0x30;
    case '\\':
      return 0x31;
    case '-':
      return 0x2D;
    case '=':
      return 0x2E;
    case '`':
      return 0x35;
    default:
      return 0;
  }
}

auto
modifiers_from_char(char c) -> uint8_t
{
  if (c >= 'A' && c <= 'Z') {
    return modifier::LEFT_SHIFT;
  }

  switch (c) {
    case '!':
    case '@':
    case '#':
    case '$':
    case '%':
    case '^':
    case '&':
    case '*':
    case '(':
    case ')':
    case '_':
    case '+':
    case '{':
    case '}':
    case '|':
    case ':':
    case '"':
    case '~':
    case '<':
    case '>':
    case '?':
      return modifier::LEFT_SHIFT;
    default:
      return 0;
  }
}

}

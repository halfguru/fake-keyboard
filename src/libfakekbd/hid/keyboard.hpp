#pragma once

#include "libfakekbd/bluetooth/l2cap.hpp"
#include "libfakekbd/hid/types.hpp"
#include <atomic>
#include <bluetooth/bluetooth.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace fakekbd::hid {

enum class connection_state
{
  Disconnected,
  Connecting,
  Connected,
  Disconnecting
};

class device
{
public:
  device();
  virtual ~device();

  device(device const&) = delete;
  device& operator=(device const&) = delete;
  device(device&&) = delete;
  device& operator=(device&&) = delete;

  [[nodiscard]] auto state() const -> connection_state;
  [[nodiscard]] auto is_connected() const -> bool;

protected:
  std::atomic<connection_state> state_;
  std::mutex connection_mutex_;

  void set_state(connection_state new_state);
};

class keyboard : public device
{
public:
  keyboard();
  ~keyboard() override;

  auto listen(std::string const& adapter) -> Result<void>;
  auto connect(bdaddr_t const& client_addr) -> Result<void>;
  auto disconnect() -> void;

  auto send_key(uint8_t key_code, bool pressed, uint8_t modifiers = 0) -> Result<void>;
  auto send_text(std::string_view text) -> Result<void>;

  [[nodiscard]] auto control_fd() const -> int;
  [[nodiscard]] auto interrupt_fd() const -> int;

private:
  bluetooth::l2cap_server control_server_;
  bluetooth::l2cap_server interrupt_server_;
  std::unique_ptr<bluetooth::l2cap_client> control_client_;
  std::unique_ptr<bluetooth::l2cap_client> interrupt_client_;

  int control_fd_;
  int interrupt_fd_;
  std::thread accept_thread_;
  std::atomic<bool> running_;

  auto handle_connection(int control_fd, int interrupt_fd) -> void;
  auto build_report(uint8_t key_code, bool pressed, uint8_t modifiers) const -> KeyboardReport;
};

auto
key_from_char(char c) -> uint8_t;
auto
modifiers_from_char(char c) -> uint8_t;

}

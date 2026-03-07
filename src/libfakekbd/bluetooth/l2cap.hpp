#pragma once

#include "hid/types.hpp"
#include <bluetooth/bluetooth.h>
#include <functional>
#include <memory>
#include <string>

namespace fakekbd::bluetooth {

class l2cap_socket
{
public:
  l2cap_socket();
  ~l2cap_socket();

  l2cap_socket(l2cap_socket const&) = delete;
  l2cap_socket& operator=(l2cap_socket const&) = delete;
  l2cap_socket(l2cap_socket&&) noexcept;
  l2cap_socket& operator=(l2cap_socket&&) noexcept;

  [[nodiscard]] auto fd() const -> int;
  [[nodiscard]] auto is_valid() const -> bool;

protected:
  explicit l2cap_socket(int fd);

  int fd_;
};

class l2cap_server : public l2cap_socket
{
public:
  using connection_handler = std::function<void(int client_fd, bdaddr_t client_addr)>;

  l2cap_server() = default;
  ~l2cap_server() = default;

  auto listen(bdaddr_t const& addr, uint16_t psm, int backlog = 5) -> hid::Result<void>;
  auto accept() -> hid::Result<std::pair<int, bdaddr_t>>;

private:
};

class l2cap_client : public l2cap_socket
{
public:
  l2cap_client() = default;
  ~l2cap_client() = default;

  auto connect(bdaddr_t const& src, bdaddr_t const& dst, uint16_t psm) -> hid::Result<void>;
};

auto
get_adapter_address(std::string const& adapter_name) -> hid::Result<bdaddr_t>;
auto
send_hid_report(int fd, void const* data, size_t len) -> hid::Result<void>;
auto
receive_hid_report(int fd, void* data, size_t len) -> hid::Result<size_t>;

}

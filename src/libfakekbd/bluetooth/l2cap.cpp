#include "l2cap.hpp"
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <cstring>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

namespace fakekbd::bluetooth {

l2cap_socket::l2cap_socket()
  : fd_(-1)
{
}

l2cap_socket::l2cap_socket(int fd)
  : fd_(fd)
{
}

l2cap_socket::~l2cap_socket()
{
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

l2cap_socket::l2cap_socket(l2cap_socket&& other) noexcept
  : fd_(other.fd_)
{
  other.fd_ = -1;
}

auto
l2cap_socket::operator=(l2cap_socket&& other) noexcept -> l2cap_socket&
{
  if (this != &other) {
    if (fd_ >= 0) {
      close(fd_);
    }
    fd_ = other.fd_;
    other.fd_ = -1;
  }
  return *this;
}

auto
l2cap_socket::fd() const -> int
{
  return fd_;
}

auto
l2cap_socket::is_valid() const -> bool
{
  return fd_ >= 0;
}

auto
l2cap_server::listen(bdaddr_t const& addr, uint16_t psm, int backlog) -> hid::Result<void>
{
  fd_ = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  if (fd_ < 0) {
    spdlog::error("Failed to create L2CAP socket: {}", std::strerror(errno));
    return std::unexpected(hid::error::SocketCreation);
  }

  int opt = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    spdlog::error("Failed to set SO_REUSEADDR: {}", std::strerror(errno));
    close(fd_);
    fd_ = -1;
    return std::unexpected(hid::error::SocketCreation);
  }

  struct sockaddr_l2 addr_l2{};
  std::memset(&addr_l2, 0, sizeof(addr_l2));
  addr_l2.l2_family = AF_BLUETOOTH;
  bacpy(&addr_l2.l2_bdaddr, &addr);
  addr_l2.l2_psm = htobs(psm);

  if (bind(fd_, reinterpret_cast<struct sockaddr*>(&addr_l2), sizeof(addr_l2)) < 0) {
    spdlog::error("Failed to bind L2CAP socket: {}", std::strerror(errno));
    close(fd_);
    fd_ = -1;
    return std::unexpected(hid::error::Bind);
  }

  struct l2cap_options opts{};
  std::memset(&opts, 0, sizeof(opts));
  opts.imtu = 64;
  opts.omtu = 64;
  opts.flush_to = 0xFFFF;

  if (setsockopt(fd_, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts)) < 0) {
    spdlog::error("Failed to set L2CAP options: {}", std::strerror(errno));
    close(fd_);
    fd_ = -1;
    return std::unexpected(hid::error::SocketCreation);
  }

  if (::listen(fd_, backlog) < 0) {
    spdlog::error("Failed to listen on L2CAP socket: {}", std::strerror(errno));
    close(fd_);
    fd_ = -1;
    return std::unexpected(hid::error::Listen);
  }

  char addr_str[18] = { 0 };
  ba2str(&addr, addr_str);
  spdlog::info("L2CAP server listening on {} PSM 0x{:04X}", addr_str, psm);

  return {};
}

auto
l2cap_server::accept() -> hid::Result<std::pair<int, bdaddr_t>>
{
  struct sockaddr_l2 client_addr{};
  socklen_t addr_len = sizeof(client_addr);

  int client_fd = ::accept(fd_, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len);
  if (client_fd < 0) {
    spdlog::error("Failed to accept L2CAP connection: {}", std::strerror(errno));
    return std::unexpected(hid::error::Accept);
  }

  char addr_str[18] = { 0 };
  ba2str(&client_addr.l2_bdaddr, addr_str);
  spdlog::info("Accepted L2CAP connection from {}", addr_str);

  return std::make_pair(client_fd, client_addr.l2_bdaddr);
}

auto
l2cap_client::connect(bdaddr_t const& src, bdaddr_t const& dst, uint16_t psm) -> hid::Result<void>
{
  fd_ = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  if (fd_ < 0) {
    spdlog::error("Failed to create L2CAP socket: {}", std::strerror(errno));
    return std::unexpected(hid::error::SocketCreation);
  }

  struct sockaddr_l2 src_addr{};
  std::memset(&src_addr, 0, sizeof(src_addr));
  src_addr.l2_family = AF_BLUETOOTH;
  bacpy(&src_addr.l2_bdaddr, &src);

  if (bind(fd_, reinterpret_cast<struct sockaddr*>(&src_addr), sizeof(src_addr)) < 0) {
    spdlog::error("Failed to bind L2CAP socket: {}", std::strerror(errno));
    close(fd_);
    fd_ = -1;
    return std::unexpected(hid::error::Bind);
  }

  struct sockaddr_l2 dst_addr{};
  std::memset(&dst_addr, 0, sizeof(dst_addr));
  dst_addr.l2_family = AF_BLUETOOTH;
  bacpy(&dst_addr.l2_bdaddr, &dst);
  dst_addr.l2_psm = htobs(psm);

  if (::connect(fd_, reinterpret_cast<struct sockaddr*>(&dst_addr), sizeof(dst_addr)) < 0) {
    spdlog::error("Failed to connect L2CAP socket: {}", std::strerror(errno));
    close(fd_);
    fd_ = -1;
    return std::unexpected(hid::error::Connect);
  }

  char addr_str[18] = { 0 };
  ba2str(&dst, addr_str);
  spdlog::info("Connected to {} PSM 0x{:04X}", addr_str, psm);

  return {};
}

auto
get_adapter_address(std::string const& adapter_name) -> hid::Result<bdaddr_t>
{
  int dev_id = hci_devid(adapter_name.c_str());
  if (dev_id < 0) {
    spdlog::error("Failed to get device ID for {}: {}", adapter_name, std::strerror(errno));
    return std::unexpected(hid::error::DeviceNotFound);
  }

  bdaddr_t addr;
  if (hci_devba(dev_id, &addr) < 0) {
    spdlog::error("Failed to get address for {}: {}", adapter_name, std::strerror(errno));
    return std::unexpected(hid::error::DeviceNotFound);
  }

  return addr;
}

auto
send_hid_report(int fd, void const* data, size_t len) -> hid::Result<void>
{
  ssize_t sent = ::send(fd, data, len, MSG_NOSIGNAL);
  if (sent < 0) {
    spdlog::error("Failed to send HID report: {}", std::strerror(errno));
    return std::unexpected(hid::error::Send);
  }

  if (static_cast<size_t>(sent) != len) {
    spdlog::warn("Partial send: {} of {} bytes", sent, len);
  }

  return {};
}

auto
receive_hid_report(int fd, void* data, size_t len) -> hid::Result<size_t>
{
  ssize_t received = ::recv(fd, data, len, 0);
  if (received < 0) {
    spdlog::error("Failed to receive HID report: {}", std::strerror(errno));
    return std::unexpected(hid::error::Receive);
  }

  return static_cast<size_t>(received);
}

}

#include <csignal>
#include <fcntl.h>
#include <filesystem>
#include <libfakekbd/bluetooth/dbus.hpp>
#include <libfakekbd/bluetooth/sdp.hpp>
#include <libfakekbd/fakekbd.hpp>
#include <spdlog/spdlog.h>
#include <termios.h>
#include <unistd.h>

namespace {
std::atomic<bool> running{ true };

void
signal_handler(int /*signum*/)
{
  running = false;
}

auto
set_terminal_mode(bool raw) -> void
{
  static struct termios original_termios;
  static bool saved = false;

  if (!saved) {
    tcgetattr(STDIN_FILENO, &original_termios);
    saved = true;
  }

  if (raw) {
    struct termios raw_termios = original_termios;
    raw_termios.c_lflag &= ~(ICANON | ECHO);
    raw_termios.c_cc[VMIN] = 0;
    raw_termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw_termios);
  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
  }
}

auto
get_config_path() -> std::string
{
  auto const* home = std::getenv("HOME");
  if (!home) {
    return "";
  }

  std::filesystem::path config_path = home;
  config_path /= ".config";
  config_path /= "fake-keyboard";
  config_path /= "config.json";

  return config_path.string();
}
}

auto
main() -> int
{
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  spdlog::info("fake-keyboard v0.1.0");

  auto config_path = get_config_path();
  std::optional<fakekbd::config> config;

  if (!config_path.empty() && std::filesystem::exists(config_path)) {
    config = fakekbd::config_parser::from_file(config_path);
    if (config) {
      spdlog::info("Loaded configuration from {}", config_path.c_str());
    } else {
      spdlog::warn("Failed to load configuration from {}", config_path.c_str());
    }
  }

  std::string adapter = config ? config->bluetooth.adapter : "hci0";
  std::string device_name = config ? config->device.name : "Fake Keyboard";

  fakekbd::keyboard kbd;

  spdlog::info("Registering HID profile with BlueZ...");
  fakekbd::bluetooth::dbus_profile_manager dbus_mgr;

  auto sdp_record = fakekbd::bluetooth::build_keyboard_sdp_record(device_name);

  if (auto result = dbus_mgr.register_hid_profile(
        adapter,
        device_name,
        sdp_record,
        [&](bdaddr_t const& device, int fd) {
          char addr_str[18] = { 0 };
          ba2str(&device, addr_str);
          spdlog::info("Device connected: {} on fd {}", addr_str, fd);
        },
        []() { spdlog::info("Device disconnected"); });
      !result) {
    spdlog::error("Failed to register HID profile");
    return 1;
  }

  spdlog::info("Starting L2CAP HID server on PSM 0x11 (control) and 0x13 (interrupt)...");
  if (auto result = kbd.listen(adapter); !result) {
    spdlog::error("Failed to start L2CAP server");
    return 1;
  }

  spdlog::info("Keyboard server ready!");
  spdlog::info("Pair from your phone and enable 'Input device' in Bluetooth settings");
  spdlog::info("Press Ctrl+C to exit");

  set_terminal_mode(true);

  char c;
  while (running) {
    dbus_mgr.process_events();

    if (kbd.is_connected()) {
      ssize_t n = read(STDIN_FILENO, &c, 1);
      if (n > 0) {
        if (c == '\x03') {
          running = false;
          break;
        }

        if (auto result = kbd.send_text(std::string(1, c)); !result) {
          spdlog::error("Failed to send keystroke");
        }
      }
    }

    usleep(10000);
  }

  set_terminal_mode(false);
  spdlog::info("Shutting down...");
  kbd.disconnect();

  return 0;
}

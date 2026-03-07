#include "dbus.hpp"
#include <algorithm>
// NOLINTBEGIN
#include <dbus/dbus.h>
// NOLINTEND
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <unistd.h>

namespace fakekbd::bluetooth {

struct dbus_profile_manager::impl
{
  DBusConnection* connection = nullptr;
  new_connection_callback on_new_connection;
  release_callback on_release;
  std::string object_path;
  bool registered = false;
};

namespace {

extern "C" DBusHandlerResult
handle_profile_message(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
  auto* self = static_cast<dbus_profile_manager::impl*>(user_data);

  spdlog::info("D-Bus message: {}.{}, path={}",
               dbus_message_get_interface(msg) ? dbus_message_get_interface(msg) : "(null)",
               dbus_message_get_member(msg) ? dbus_message_get_member(msg) : "(null)",
               dbus_message_get_path(msg) ? dbus_message_get_path(msg) : "(null)");

  if (dbus_message_is_method_call(msg, "org.bluez.Profile1", "NewConnection")) {
    DBusMessageIter iter;
    bdaddr_t device_addr = {};
    int fd = -1;

    if (dbus_message_iter_init(msg, &iter)) {
      char* device_path = nullptr;
      if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH) {
        dbus_message_iter_get_basic(&iter, &device_path);

        if (device_path) {
          char const* prefix = "/org/bluez/";
          char const* dev_prefix = "/dev_";
          char* dev_pos = strstr(device_path, dev_prefix);
          if (dev_pos) {
            dev_pos += strlen(dev_prefix);
            str2ba(dev_pos, &device_addr);
          }
        }

        dbus_message_iter_next(&iter);
      }

      if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_UNIX_FD) {
        dbus_message_iter_get_basic(&iter, &fd);
      }
    }

    spdlog::info("NewConnection: fd={}", fd);

    if (self->on_new_connection) {
      self->on_new_connection(device_addr, fd);
    }

    auto* reply = dbus_message_new_method_return(msg);
    dbus_connection_send(conn, reply, nullptr);
    dbus_connection_flush(conn);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
  }

  if (dbus_message_is_method_call(msg, "org.bluez.Profile1", "RequestDisconnection")) {
    spdlog::info("RequestDisconnection");

    if (self->on_release) {
      self->on_release();
    }

    auto* reply = dbus_message_new_method_return(msg);
    dbus_connection_send(conn, reply, nullptr);
    dbus_connection_flush(conn);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
  }

  if (dbus_message_is_method_call(msg, "org.bluez.Profile1", "Release")) {
    spdlog::info("Release");

    if (self->on_release) {
      self->on_release();
    }

    auto* reply = dbus_message_new_method_return(msg);
    dbus_connection_send(conn, reply, nullptr);
    dbus_connection_flush(conn);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
  }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

}

dbus_profile_manager::dbus_profile_manager()
  : pimpl_(std::make_unique<impl>())
{
  DBusError error;
  dbus_error_init(&error);

  pimpl_->connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
  if (!pimpl_->connection) {
    spdlog::error("Failed to connect to system bus: {}", error.message);
    dbus_error_free(&error);
    throw std::runtime_error("Failed to connect to D-Bus system bus");
  }

  dbus_connection_set_exit_on_disconnect(pimpl_->connection, false);
}

dbus_profile_manager::~dbus_profile_manager()
{
  unregisterProfile();

  if (pimpl_->connection) {
    dbus_connection_unref(pimpl_->connection);
  }
}

auto
dbus_profile_manager::register_hid_profile(std::string const& adapter,
                                           std::string const& name,
                                           std::string const& sdp_record,
                                           new_connection_callback on_new_conn,
                                           release_callback on_rel) -> hid::Result<void>
{
  if (pimpl_->registered) {
    unregisterProfile();
  }

  std::string safe_name = name;
  std::replace(safe_name.begin(), safe_name.end(), ' ', '_');
  pimpl_->object_path = "/org/bluez/" + adapter + "/hid_" + safe_name;
  pimpl_->on_new_connection = std::move(on_new_conn);
  pimpl_->on_release = std::move(on_rel);

  static DBusObjectPathVTable vtable = { nullptr, handle_profile_message, nullptr, nullptr, nullptr, nullptr };

  DBusError error;
  dbus_error_init(&error);

  if (!dbus_connection_try_register_object_path(
        pimpl_->connection, pimpl_->object_path.c_str(), &vtable, pimpl_.get(), &error)) {
    spdlog::error("Failed to register object path: {}", error.message);
    dbus_error_free(&error);
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  auto* msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.ProfileManager1", "RegisterProfile");

  if (!msg) {
    spdlog::error("Failed to create D-Bus message");
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  DBusMessageIter iter;
  dbus_message_iter_init_append(msg, &iter);

  char const* path = pimpl_->object_path.c_str();
  if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &path)) {
    dbus_message_unref(msg);
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  char const* uuid = "00001124-0000-1000-8000-00805f9b34fb";
  if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &uuid)) {
    dbus_message_unref(msg);
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  DBusMessageIter dict;
  dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);

  {
    DBusMessageIter entry;
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);

    char const* key = "Name";
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);

    DBusMessageIter variant;
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    char const* value = name.c_str();
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &value);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);
  }

  {
    DBusMessageIter entry;
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);

    char const* key = "ServiceRecord";
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);

    DBusMessageIter variant;
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    char const* value = sdp_record.c_str();
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &value);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);
  }

  dbus_message_iter_close_container(&iter, &dict);

  DBusError reply_error;
  dbus_error_init(&reply_error);

  auto* reply = dbus_connection_send_with_reply_and_block(pimpl_->connection, msg, -1, &reply_error);
  dbus_message_unref(msg);

  if (!reply) {
    spdlog::error("Failed to register profile: {}", reply_error.message);
    dbus_error_free(&reply_error);
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  dbus_message_unref(reply);
  pimpl_->registered = true;

  spdlog::info("Registered HID profile with BlueZ");
  return {};
}

auto
dbus_profile_manager::unregisterProfile() -> void
{
  if (!pimpl_->registered || !pimpl_->connection) {
    return;
  }

  auto* msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.ProfileManager1", "UnregisterProfile");

  if (!msg) {
    return;
  }

  char const* path = pimpl_->object_path.c_str();
  DBusMessageIter iter;
  dbus_message_iter_init_append(msg, &iter);
  dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &path);

  dbus_connection_send(pimpl_->connection, msg, nullptr);
  dbus_connection_flush(pimpl_->connection);
  dbus_message_unref(msg);

  dbus_connection_unregister_object_path(pimpl_->connection, pimpl_->object_path.c_str());

  pimpl_->registered = false;
  spdlog::info("Unregistered HID profile");
}

auto
dbus_profile_manager::processEvents() -> void
{
  if (pimpl_->connection) {
    while (dbus_connection_read_write_dispatch(pimpl_->connection, 0)) {
    }
  }
}

auto
dbus_profile_manager::isRegistered() const -> bool
{
  return pimpl_->registered;
}

auto
dbus_profile_manager::setAdapterDiscoverable(std::string const& adapter, bool enabled) -> hid::Result<void>
{
  if (!pimpl_->connection) {
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  std::string adapter_path = "/org/bluez/" + adapter;
  char const* path = adapter_path.c_str();
  char const* interface = "org.bluez.Adapter1";
  char const* property = "Discoverable";
  dbus_bool_t value = enabled ? TRUE : FALSE;

  auto* msg = dbus_message_new_method_call("org.bluez", path, "org.freedesktop.DBus.Properties", "Set");
  if (!msg) {
    spdlog::error("Failed to create D-Bus message");
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  DBusMessageIter iter;
  dbus_message_iter_init_append(msg, &iter);
  dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
  dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property);

  DBusMessageIter variant_iter;
  dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant_iter);
  dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_BOOLEAN, &value);
  dbus_message_iter_close_container(&iter, &variant_iter);

  DBusError error;
  dbus_error_init(&error);
  auto* reply = dbus_connection_send_with_reply_and_block(pimpl_->connection, msg, -1, &error);
  dbus_message_unref(msg);

  if (!reply) {
    spdlog::error("Failed to set Discoverable property: {}", error.message);
    dbus_error_free(&error);
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  dbus_message_unref(reply);
  spdlog::info("Adapter {} discoverable: {}", adapter, enabled ? "on" : "off");
  return {};
}

auto
dbus_profile_manager::setAdapterPairable(std::string const& adapter, bool enabled) -> hid::Result<void>
{
  if (!pimpl_->connection) {
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  std::string adapter_path = "/org/bluez/" + adapter;
  char const* path = adapter_path.c_str();
  char const* interface = "org.bluez.Adapter1";
  char const* property = "Pairable";
  dbus_bool_t value = enabled ? TRUE : FALSE;

  auto* msg = dbus_message_new_method_call("org.bluez", path, "org.freedesktop.DBus.Properties", "Set");
  if (!msg) {
    spdlog::error("Failed to create D-Bus message");
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  DBusMessageIter iter;
  dbus_message_iter_init_append(msg, &iter);
  dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
  dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property);

  DBusMessageIter variant_iter;
  dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant_iter);
  dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_BOOLEAN, &value);
  dbus_message_iter_close_container(&iter, &variant_iter);

  DBusError error;
  dbus_error_init(&error);
  auto* reply = dbus_connection_send_with_reply_and_block(pimpl_->connection, msg, -1, &error);
  dbus_message_unref(msg);

  if (!reply) {
    spdlog::error("Failed to set Pairable property: {}", error.message);
    dbus_error_free(&error);
    return std::unexpected(hid::error::InvalidConfiguration);
  }

  dbus_message_unref(reply);
  spdlog::info("Adapter {} pairable: {}", adapter, enabled ? "on" : "off");
  return {};
}

}

#include "dbus.hpp"
#include <algorithm>
#include <sdbus-c++/sdbus-c++.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

namespace fakekbd::bluetooth
{

struct DBusProfileManager::Impl
{
    std::unique_ptr<sdbus::IConnection> connection;
    std::unique_ptr<sdbus::IObject> profileObject;
    std::unique_ptr<sdbus::IObject> agentObject;
    std::string objectPath;
    bool registered = false;

    NewConnectionCallback onNewConnection;
    ReleaseCallback onRelease;
};

DBusProfileManager::DBusProfileManager()
  : pimpl_(std::make_unique<Impl>())
{
  pimpl_->connection = sdbus::createSystemBusConnection();
}

DBusProfileManager::~DBusProfileManager()
{
  unregisterProfile();
  unregisterAgent();
}

auto DBusProfileManager::registerHidProfile(std::string const& adapter,
                                            std::string const& name,
                                            std::string const& sdpRecord,
                                            NewConnectionCallback onNewConnection,
                                            ReleaseCallback onRelease) -> hid::Result<void>
{
  if (pimpl_->registered)
  {
    unregisterProfile();
  }

  std::string safeName = name;
  std::replace(safeName.begin(), safeName.end(), ' ', '_');
  pimpl_->objectPath = "/org/bluez/" + adapter + "/hid_" + safeName;
  pimpl_->onNewConnection = std::move(onNewConnection);
  pimpl_->onRelease = std::move(onRelease);

  try
  {
    pimpl_->profileObject = sdbus::createObject(*pimpl_->connection, sdbus::ObjectPath{ pimpl_->objectPath });

    pimpl_->profileObject
      ->addVTable(
        sdbus::registerMethod("NewConnection")
          .implementedAs([this](std::string device, sdbus::UnixFd fd, std::map<std::string, sdbus::Variant> /*props*/) {
            spdlog::info("NewConnection called for device: {}", device.c_str());

            if (pimpl_->onNewConnection)
            {
              bdaddr_t bdaddr = {};
              str2ba(device.c_str(), &bdaddr);
              pimpl_->onNewConnection(bdaddr, fd.get());
            }
          }),
        sdbus::registerMethod("Release").implementedAs([this]() {
          spdlog::info("Release called");
          if (pimpl_->onRelease)
          {
            pimpl_->onRelease();
          }
        }),
        sdbus::registerMethod("RequestDisconnection").implementedAs([this](std::string device) {
          spdlog::info("RequestDisconnection for device: {}", device.c_str());
          if (pimpl_->onRelease)
          {
            pimpl_->onRelease();
          }
        }))
      .forInterface("org.bluez.Profile1");

    auto profileManager =
      sdbus::createProxy(*pimpl_->connection, sdbus::ServiceName{ "org.bluez" }, sdbus::ObjectPath{ "/org/bluez" });

    std::map<std::string, sdbus::Variant> options;
    options["ServiceRecord"] = sdbus::Variant(sdpRecord);
    options["Role"] = sdbus::Variant(std::string("server"));

    profileManager->callMethod("RegisterProfile")
      .onInterface("org.bluez.ProfileManager1")
      .withArguments(sdbus::ObjectPath{ pimpl_->objectPath }, "00001124-0000-1000-8000-00805f9b34fb", options);

    pimpl_->registered = true;
    spdlog::info("Registered HID profile with BlueZ");
    return {};
  }
  catch (sdbus::Error const& e)
  {
    spdlog::error("Failed to register HID profile: {}", e.what());
    return std::unexpected(hid::error::InvalidConfiguration);
  }
}

auto DBusProfileManager::unregisterProfile() -> void
{
  if (!pimpl_->registered || !pimpl_->connection)
  {
    return;
  }

  try
  {
    auto profileManager =
      sdbus::createProxy(*pimpl_->connection, sdbus::ServiceName{ "org.bluez" }, sdbus::ObjectPath{ "/org/bluez" });

    profileManager->callMethod("UnregisterProfile")
      .onInterface("org.bluez.ProfileManager1")
      .withArguments(sdbus::ObjectPath{ pimpl_->objectPath });

    pimpl_->profileObject.reset();
    pimpl_->registered = false;
    spdlog::info("Unregistered HID profile");
  }
  catch (sdbus::Error const& e)
  {
    spdlog::error("Failed to unregister profile: {}", e.what());
  }
}

auto DBusProfileManager::processEvents() -> void
{
  if (pimpl_->connection)
  {
    while (pimpl_->connection->processPendingEvent())
    {
    }
  }
}

auto DBusProfileManager::isRegistered() const -> bool
{
  return pimpl_->registered;
}

auto DBusProfileManager::setAdapterDiscoverable(std::string const& adapter, bool enabled) -> hid::Result<void>
{
  try
  {
    auto adapterPath = "/org/bluez/" + adapter;
    auto adapterProxy =
      sdbus::createProxy(*pimpl_->connection, sdbus::ServiceName{ "org.bluez" }, sdbus::ObjectPath{ adapterPath });

    adapterProxy->setProperty("Discoverable").onInterface("org.bluez.Adapter1").toValue(enabled);

    spdlog::info("Adapter {} discoverable: {}", adapter.c_str(), enabled ? "on" : "off");
    return {};
  }
  catch (sdbus::Error const& e)
  {
    spdlog::error("Failed to set Discoverable property: {}", e.what());
    return std::unexpected(hid::error::InvalidConfiguration);
  }
}

auto DBusProfileManager::setAdapterPairable(std::string const& adapter, bool enabled) -> hid::Result<void>
{
  try
  {
    auto adapterPath = "/org/bluez/" + adapter;
    auto adapterProxy =
      sdbus::createProxy(*pimpl_->connection, sdbus::ServiceName{ "org.bluez" }, sdbus::ObjectPath{ adapterPath });

    adapterProxy->setProperty("Pairable").onInterface("org.bluez.Adapter1").toValue(enabled);

    spdlog::info("Adapter {} pairable: {}", adapter.c_str(), enabled ? "on" : "off");
    return {};
  }
  catch (sdbus::Error const& e)
  {
    spdlog::error("Failed to set Pairable property: {}", e.what());
    return std::unexpected(hid::error::InvalidConfiguration);
  }
}

auto DBusProfileManager::setAdapterName(std::string const& adapter, std::string const& name) -> hid::Result<void>
{
  try
  {
    auto adapterPath = "/org/bluez/" + adapter;
    auto adapterProxy =
      sdbus::createProxy(*pimpl_->connection, sdbus::ServiceName{ "org.bluez" }, sdbus::ObjectPath{ adapterPath });

    adapterProxy->setProperty("Alias").onInterface("org.bluez.Adapter1").toValue(name);

    spdlog::info("Adapter {} name: {}", adapter.c_str(), name.c_str());
    return {};
  }
  catch (sdbus::Error const& e)
  {
    spdlog::error("Failed to set Alias property: {}", e.what());
    return std::unexpected(hid::error::InvalidConfiguration);
  }
}

auto DBusProfileManager::setAdapterClass(std::string const& adapter, uint32_t device_class) -> hid::Result<void>
{
  try
  {
    auto adapterPath = "/org/bluez/" + adapter;
    auto adapterProxy =
      sdbus::createProxy(*pimpl_->connection, sdbus::ServiceName{ "org.bluez" }, sdbus::ObjectPath{ adapterPath });

    adapterProxy->setProperty("Class").onInterface("org.bluez.Adapter1").toValue(device_class);

    spdlog::info("Adapter {} class: 0x{:06x}", adapter.c_str(), device_class);
    return {};
  }
  catch (sdbus::Error const& e)
  {
    spdlog::error("Failed to set Class property: {}", e.what());
    return std::unexpected(hid::error::InvalidConfiguration);
  }
}

auto DBusProfileManager::registerAgent() -> hid::Result<void>
{
  try
  {
    pimpl_->agentObject = sdbus::createObject(*pimpl_->connection, sdbus::ObjectPath{ "/org/bluez/agent" });

    pimpl_->agentObject
      ->addVTable(
        sdbus::registerMethod("Release").implementedAs([]() {}),
        sdbus::registerMethod("Cancel").implementedAs([]() {}),
        sdbus::registerMethod("RequestPasskey").implementedAs([](std::string const& device) -> uint32_t {
          spdlog::debug("RequestPasskey for device: {}", device);
          return 0;
        }),
        sdbus::registerMethod("RequestConfirmation").implementedAs([](std::string const& device, uint32_t passkey) {
          spdlog::debug("RequestConfirmation for device: {} passkey: {}", device, passkey);
        }),
        sdbus::registerMethod("AuthorizeService").implementedAs([](std::string const& device, std::string const& uuid) {
          spdlog::debug("AuthorizeService for device: {} uuid: {}", device, uuid);
        }))
      .forInterface("org.bluez.Agent1");

    auto agentManager =
      sdbus::createProxy(*pimpl_->connection, sdbus::ServiceName{ "org.bluez" }, sdbus::ObjectPath{ "/org/bluez" });

    agentManager->callMethod("RegisterAgent")
      .onInterface("org.bluez.AgentManager1")
      .withArguments(sdbus::ObjectPath{ "/org/bluez/agent" }, "NoInputNoOutput");

    agentManager->callMethod("RequestDefaultAgent")
      .onInterface("org.bluez.AgentManager1")
      .withArguments(sdbus::ObjectPath{ "/org/bluez/agent" });

    spdlog::info("Registered pairing agent");
    return {};
  }
  catch (sdbus::Error const& e)
  {
    spdlog::error("Failed to register agent: {}", e.what());
    return std::unexpected(hid::error::InvalidConfiguration);
  }
}

auto DBusProfileManager::unregisterAgent() -> void
{
  try
  {
    auto agentManager =
      sdbus::createProxy(*pimpl_->connection, sdbus::ServiceName{ "org.bluez" }, sdbus::ObjectPath{ "/org/bluez" });

    agentManager->callMethod("UnregisterAgent")
      .onInterface("org.bluez.AgentManager1")
      .withArguments(sdbus::ObjectPath{ "/org/bluez/agent" });

    pimpl_->agentObject.reset();
  }
  catch (sdbus::Error const& e)
  {
    spdlog::debug("Ignoring error during agent unregister: {}", e.what());
  }
}

}

#include <CLI11/CLI11.hpp>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <cerrno>
#include <cstring>
#include <dbus/dbus.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

void discoverDevices() {
  int deviceId, socketHandle;
  inquiry_info *devices = nullptr;
  int numDevices;

  // get the Bluetooth device ID
  deviceId = hci_get_route(nullptr);
  if (deviceId < 0) {
    std::cerr << "No Bluetooth device available" << std::endl;
    return;
  }

  socketHandle = hci_open_dev(deviceId);
  if (socketHandle < 0) {
    std::cerr << "Failed to open Bluetooth socket" << std::endl;
    return;
  }

  // discover devices
  devices = new inquiry_info[255];
  numDevices =
      hci_inquiry(deviceId, 8, 255, nullptr, &devices, IREQ_CACHE_FLUSH);
  if (numDevices < 0) {
    std::cerr << "Device discovery failed" << std::endl;
    delete[] devices;
    hci_close_dev(socketHandle);
    return;
  }

  for (int i = 0; i < numDevices; ++i) {
    char name[248] = {};
    char addr[19] = {};
    ba2str(&devices[i].bdaddr, addr);

    // updated hci_read_remote_name call with timeout parameter
    int timeout = 1000;
    if (hci_read_remote_name(socketHandle, &devices[i].bdaddr, sizeof(name),
                             name, timeout) < 0) {
      strcpy(name, "[unknown]");
    }

    std::cout << "Device found: " << addr << " - " << name << std::endl;
  }

  std::cout << "No more devices were found." << std::endl;
  delete[] devices;
  hci_close_dev(socketHandle);
}

// connect and pair with the Bluetooth device
void connectDevice(const std::string &deviceAddress) {
  DBusConnection *connection;
  DBusError error;
  dbus_error_init(&error);

  // Connect to the system bus
  connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
  if (dbus_error_is_set(&error)) {
    std::cerr << "Connection Error: " << error.message << std::endl;
    dbus_error_free(&error);
    return;
  }

  std::string objectPath = "/org/bluez/hci0/dev_" + deviceAddress;
  for (char &c : objectPath) {
    if (c == ':') {
      c = '_';
    }
  }

  // Create a new method call message to connect to the device
  DBusMessage *msg = dbus_message_new_method_call(
      "org.bluez", objectPath.c_str(), "org.bluez.Device1", "Connect");
  if (msg == nullptr) {
    std::cerr << "Message Null" << std::endl;
    return;
  }

  DBusMessage *reply =
      dbus_connection_send_with_reply_and_block(connection, msg, -1, &error);
  if (dbus_error_is_set(&error)) {
    std::cerr << "Reply Error: " << error.message << std::endl;
    dbus_error_free(&error);
  } else {
    std::cout << "Device connection initiated successfully." << std::endl;
  }

  if (msg != nullptr) {
    dbus_message_unref(msg);
  }
  if (reply != nullptr) {
    dbus_message_unref(reply);
  }

  dbus_connection_unref(connection);
}

int main(int argc, char *argv[]) {
  CLI::App app{"Bluc v1.0.0"};

  std::string input;
  std::string toPair;
  app.add_flag("-f,--find", input, "Find devices.");
  app.add_option("-p,--pair", toPair, "Pair a specific device using its MAC.");

  CLI11_PARSE(app, argc, argv);

  if (input.length() != 0) {
    std::cout << "Looking for nearby devices.." << std::endl;
    discoverDevices();
  } else if (toPair.length() != 0) {
    connectDevice(toPair);
  } else {
    std::cout << "Welcome to Bluc, check help menu by running -h or --help"
              << std::endl;
  }

  return 0;
}

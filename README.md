# Bluc

### Bluc is a simple implementation of a Bluetooth CLI tool.  
### The project is still in its early stages, so it currently only supports device discovery and connection.

## Build from source:
Ensure that the following libraries are installed:

```bash
sudo apt-get install libbluetooth-dev
```
Depending on your Linux distribution, you can install the required package using the appropriate package manager:

On Ubuntu/Debian:

```bash
sudo apt-get install libdbus-1-dev
```

On Fedora:

```bash
sudo dnf install dbus-devel
```

On Arch Linux:

```bash
sudo pacman -S dbus
```

Then, compile the tool:

```bash
g++ -o bluc main.cpp $(pkg-config --cflags --libs dbus-1) -lbluetooth
```



# Setting up the system on Linux

## Using the AppImage

Download the `vscp-simulator` artifact (see [Setting up the system](setting_up_the_system.md#downloading-a-build)), make the AppImage executable and run it:

```bash
chmod +x vscpsim-*.AppImage
./vscpsim-*.AppImage -i udp -b 0
```

The AppImage contains Qt and the other libraries. Some distributions need FUSE 2 to run AppImages (Debian/Ubuntu: `sudo apt install libfuse2`).

## Building from source

Install the build tools and libraries. On Debian and Ubuntu:

```bash
sudo apt install build-essential cmake git \
  qt6-base-dev qt6-declarative-dev \
  libmosquitto-dev libexpat1-dev libssl-dev libcurl4-openssl-dev
```

With these packages the system Qt is used. To use a Qt installed with the Qt online installer instead, skip the two Qt packages and give CMake its path with `-DCMAKE_PREFIX_PATH`, for example `~/Qt/6.8.2/gcc_64`.

```bash
git clone https://github.com/grodansparadis/vscp-simulator.git
cd vscp-simulator
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
sudo cmake --install build
```

`cmake --install` puts `vscpsim` in `/usr/local/bin` and the desktop file, icons and event database below `/usr/local/share`. Use `--prefix` to install somewhere else.

## SocketCAN

The `socketcan` interface is only available on Linux. For testing without hardware, create a virtual CAN interface:

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
vscpsim -i socketcan -c vcan0 -b 0
```

`candump vcan0` and `cansend` from the `can-utils` package show and send frames on the bus.

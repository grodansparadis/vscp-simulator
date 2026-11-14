# Setting up the system on Raspberry Pi

The simulator runs on Raspberry Pi OS, 64-bit (`arm64`) and 32-bit (`armhf`). The ready built programs are made on Debian bookworm, the base of Raspberry Pi OS bookworm, and use the Qt 6 packages of the system. They also run on newer releases.

## Using a ready built program

Download `vscpsim-linux-arm64` (64-bit Raspberry Pi OS) or `vscpsim-linux-armhf` (32-bit), see [Setting up the system](setting_up_the_system.md#downloading-a-build). `uname -m` shows `aarch64` on 64-bit and `armv7l` on 32-bit Raspberry Pi OS.

Install the libraries the program uses, then unpack it. The archive holds `usr/bin/vscpsim` and its data files below `usr/share`:

```bash
sudo apt install libqt6widgets6 libqt6qml6 libmosquitto1 libexpat1 libcurl4 libssl3
sudo tar -C / -xzf vscpsim-linux-arm64.tar.gz
vscpsim -i udp -b 0
```

On a Raspberry Pi without a desktop start it with `QT_QPA_PLATFORM=offscreen vscpsim ...`.

## Building from source

Building on the Raspberry Pi itself works as on any Debian system:

```bash
sudo apt install build-essential cmake git \
  qt6-base-dev qt6-declarative-dev \
  libmosquitto-dev libexpat1-dev libssl-dev libcurl4-openssl-dev
git clone https://github.com/grodansparadis/vscp-simulator.git
cd vscp-simulator
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
sudo cmake --install build
```

Use few parallel jobs (`--parallel 2`) on models with 1 GB of memory or less.

## SocketCAN

With a CAN HAT (for example one with an MCP2515) the Raspberry Pi gets a `can0` interface. Bring it up with the bit rate of the bus and connect the simulator to it:

```bash
sudo ip link set can0 up type can bitrate 125000
vscpsim -i socketcan -c can0 -b 0
```

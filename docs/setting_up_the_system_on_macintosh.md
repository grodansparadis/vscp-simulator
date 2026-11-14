# Setting up the system on Macintosh

## Using a ready built program

Download `vscpsim-macos-arm64` (Apple silicon), see [Setting up the system](setting_up_the_system.md#downloading-a-build), and unpack it:

```bash
tar -xzf vscpsim-macos-arm64.tar.gz
vscpsim.app/Contents/MacOS/vscpsim -i udp -b 0
```

The bundle contains Qt. The program is not signed, so macOS may refuse to open it the first time. Remove the quarantine flag with

```bash
xattr -dr com.apple.quarantine vscpsim.app
```

Start it from a terminal (as above) to use the command line options and see the log; `open vscpsim.app` starts it without options. If it reports a missing `libmosquitto` or OpenSSL library, install them with Homebrew: `brew install mosquitto openssl@3`.

## Building from source

Install the Xcode command line tools (`xcode-select --install`), [Homebrew](https://brew.sh) and Qt 6 with the [Qt online installer](https://www.qt.io/download-qt-installer). Use Qt 6.11 or later with a current Xcode: Qt 6.8 links Apple's old AGL framework, which newer macOS SDKs no longer have.

```bash
brew install cmake ninja expat mosquitto openssl@3
git clone https://github.com/grodansparadis/vscp-simulator.git
cd vscp-simulator
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=~/Qt/6.11.1/macos \
  -DOPENSSL_ROOT_DIR=$(brew --prefix openssl@3)
cmake --build build --parallel
cmake --install build --prefix ~/vscpsim
```

`cmake --install` puts `vscpsim.app` in the given folder with Qt deployed into the bundle.

The `socketcan` interface is not available on macOS. Use `canal` with a CANAL driver for CAN hardware.

# vscp-simulator

![License](https://img.shields.io/badge/license-MIT-blue.svg)
[![Linux](https://github.com/grodansparadis/vscp-simulator/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/cmake.yml?query=branch%3Amain+is%3Asuccess)
[![AppImage](https://github.com/grodansparadis/vscp-simulator/actions/workflows/appimage.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/appimage.yml?query=branch%3Amain+is%3Asuccess)
[![macOS](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-macos.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-macos.yml?query=branch%3Amain+is%3Asuccess)
[![Windows](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-windows.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-windows.yml?query=branch%3Amain+is%3Asuccess)
[![Raspberry Pi arm64](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-raspberry-pi-arm64.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-raspberry-pi-arm64.yml?query=branch%3Amain+is%3Asuccess)
[![Raspberry Pi armhf](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-raspberry-pi-armhf.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-raspberry-pi-armhf.yml?query=branch%3Amain+is%3Asuccess)
[![Documentation](https://github.com/grodansparadis/vscp-simulator/actions/workflows/pages.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/pages.yml)
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](https://www.repostatus.org/badges/latest/active.svg)](https://www.repostatus.org/#active)

<img src="https://vscp.org/images/logo.png" width="100">

This is a simple test software thats simulates all functionality of a VSCP level I or a Level I node. It can be used for testing of the [VSCP bootloader](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_boot_loader_algorithm) or VSCP node functionality.

This software is created for (cross)testing and validation of new functionality and not intended for public use.

The following transport mechanisms are currently available

| Transport | Interface | Availability | OS |
| --------- | --------- | ------------ | -- |
| socketcan | `socketcan` | available | Linux |
| tcp/ip client | `tcpip` | available | Linux, Windows, macOS |
| UDP | `udp` | available | Linux, Windows, macOS |
| Multicast | `multicast` | available | Linux, Windows, macOS |
| Websocket version 1 | `ws1` | available | Linux, Windows, macOS |
| Websocket version 2 | `ws2` | available | Linux, Windows, macOS |
| MQTT | `mqtt` | available (configure topics in the configuration file) | Linux, Windows, macOS |
| CANAL (Level I driver) | `canal` | available | Linux, Windows, macOS |
| VSCP Level II driver | `level2` | available | Linux, Windows, macOS |
| tcp/ip server | | planned | Linux, Windows |
| Ethernet | | planned | Linux, Windows |

## Documentation

See [Using and configuring the simulator](docs/configure.md) for command-line switches, interfaces, firmware and bootloader modes, the configuration file format, and simulations.

The [published documentation](https://grodansparadis.github.io/vscp-simulator/) provides the full manual. The [Markdown sources](docs/README.md) are also available in this repository.

The *Documentation* workflow uploads the static Docsify site from `docs/` and deploys it to GitHub Pages on pushes to `main`, including after the version updater completes. Pull requests build the site artifact without deploying. Set **Settings > Pages > Build and deployment > Source** to **GitHub Actions** when enabling this workflow.

## Version

The version has the form *year.month.patch*, for example `26.11.3`. The patch number counts the pushes to the repository during the month and starts at 0 in a new month. It is updated automatically by the *Update version* GitHub workflow on every push to `main` and lives in [src/vscpsim-version.h](src/vscpsim-version.h), which CMake also uses for the project version. Show it with `vscpsim --version`.

The same workflow updates `document-version` in [docs/variables.xml](docs/variables.xml) to match the simulator version shown on the documentation home page. Both version files are committed together; the bot commit does not trigger another version bump.

## Download

vscpsim is built for Linux (x86_64 tarball and AppImage), Raspberry Pi OS 64-bit (arm64) and 32-bit (armhf), macOS (Apple silicon) and Windows (x64) on pushes and pull requests. The workflows can also be started manually. Click a build badge above to see the successful runs and download the artifact from a run page (GitHub only serves artifacts to signed-in users), or download the newest builds of all platforms with

```bash
scripts/download-artifacts.sh            # needs the GitHub CLI: gh auth login
```

| Platform | Artifact | Run it |
| -------- | -------- | ------ |
| Linux x86_64 | `vscp-simulator` | `chmod +x vscpsim-*.AppImage && ./vscpsim-*.AppImage` |
| Linux x86_64 (system Qt) | `vscpsim-linux-x86_64` | `sudo tar -C / -xzf vscpsim-linux-x86_64.tar.gz`, then `vscpsim` |
| Raspberry Pi 64-bit | `vscpsim-linux-arm64` | `sudo tar -C / -xzf vscpsim-linux-arm64.tar.gz`, then `vscpsim` |
| Raspberry Pi 32-bit | `vscpsim-linux-armhf` | `sudo tar -C / -xzf vscpsim-linux-armhf.tar.gz`, then `vscpsim` |
| macOS | `vscpsim-macos-arm64` | unpack, then `vscpsim.app/Contents/MacOS/vscpsim` |
| Windows x64 | `vscpsim-windows-x64` | unpack, then `bin\vscpsim.exe` |

Every artifact has a `SHA256SUMS-<artifact>` file (`sha256sum -c SHA256SUMS-<artifact>`). The Linux tarball and Raspberry Pi builds use the Qt 6 of the system: `sudo apt install libqt6widgets6 libqt6qml6 libmosquitto1 libexpat1 libcurl4 libssl3`. The x86_64 tarball targets Ubuntu 24.04 or compatible systems; Raspberry Pi tarballs target Debian bookworm. See the [setup documentation](docs/setting_up_the_system.md) for each platform.

## Build from source

You need CMake 3.16 or later, a C++17 compiler, Qt 6 (Widgets and Qml), OpenSSL, libmosquitto, expat and libcurl. The other dependencies are downloaded by CMake at pinned revisions when the build is configured.

```bash
git clone https://github.com/grodansparadis/vscp-simulator.git
cd vscp-simulator
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$HOME/Qt/6.12.0/gcc_64"
cmake --build build --parallel
```

* **Linux and Raspberry Pi (Debian, Ubuntu, Raspberry Pi OS):** `sudo apt install build-essential cmake git qt6-base-dev qt6-declarative-dev libmosquitto-dev libexpat1-dev libssl-dev libcurl4-openssl-dev`
* **macOS:** Qt 6.11 or later (Qt 6.8 does not link with the newest Xcode) and `brew install expat mosquitto openssl@3`. Configure with `-DCMAKE_PREFIX_PATH=~/Qt/6.11.1/macos -DOPENSSL_ROOT_DIR=$(brew --prefix openssl@3)`.
* **Windows:** Visual Studio 2022, Qt 6 for MSVC 2022 64-bit and [vcpkg](https://vcpkg.io) with `vcpkg install curl dlfcn-win32 expat mosquitto openssl pthreads --triplet x64-windows`. Configure from a *x64 Native Tools* prompt with `-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake -DCMAKE_PREFIX_PATH=<Qt>/6.8.1/msvc2022_64`.

The `CMAKE_PREFIX_PATH` example uses a Linux Qt kit under `~/Qt`; change the version and kit directory to match your installation. Use the kit prefix (the directory containing `lib/cmake/Qt6/Qt6Config.cmake`), not the `~/Qt` parent directory. If using system Qt, install the Qt 6 development packages listed above and omit `CMAKE_PREFIX_PATH`. In Visual Studio Code, set this as **cmake: configure environment**. `cmake --install build --prefix <folder>` installs the program; on Windows and macOS the Qt runtime is installed with it.

The executable is named **vscpsim** (`vscpsim.app` on macOS). The `socketcan` interface only exists on Linux; all other interfaces work on every platform.

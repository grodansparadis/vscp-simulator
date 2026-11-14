# Setting up the system

The VSCP Simulator (`vscpsim`) runs on Linux, Windows, macOS and the Raspberry Pi (32-bit and 64-bit Raspberry Pi OS). You can download a ready built program or build it from source.

| Platform | Download (CI artifact) | Contents |
| -------- | ---------------------- | -------- |
| Linux x86_64 | `vscp-simulator` (AppImage workflow) | `vscpsim-*.AppImage`, runs on most distributions |
| Raspberry Pi 64-bit | `vscpsim-linux-arm64` | `vscpsim-linux-arm64.tar.gz`, built on Debian bookworm |
| Raspberry Pi 32-bit | `vscpsim-linux-armhf` | `vscpsim-linux-armhf.tar.gz`, built on Debian bookworm |
| macOS (Apple silicon) | `vscpsim-macos-arm64` | `vscpsim-macos-arm64.tar.gz` with `vscpsim.app` |
| Windows x64 | `vscpsim-windows-x64` | `vscpsim-windows-x64.zip` with `bin/vscpsim.exe` and the DLLs it needs |

## Downloading a build

Every push to the repository builds all platforms on GitHub. The build badges at the top of the [README](https://github.com/grodansparadis/vscp-simulator#readme) show the state of each platform; click a badge to see its successful runs and download the artifact from a run page. GitHub only serves artifacts to signed-in users.

The script `scripts/download-artifacts.sh` in the repository downloads the newest artifacts and checks them. It needs the [GitHub CLI](https://cli.github.com/) (`gh auth login` once):

```bash
scripts/download-artifacts.sh                 # all platforms
scripts/download-artifacts.sh arm64 windows   # only some
```

Each artifact has a `SHA256SUMS-<artifact>` file. The script checks it; to check by hand use `sha256sum -c SHA256SUMS-<artifact>` (on macOS `shasum -a 256 -c`).

## Building from source

The simulator needs

* CMake 3.16 or later and a C++17 compiler
* Qt 6 (Widgets and Qml)
* OpenSSL, libmosquitto, expat and libcurl

The other dependencies (the VSCP library, vscp-firmware, nlohmann json, spdlog, mustache and maddy) are downloaded by CMake at pinned revisions when the build is configured, so the first configure needs network access.

```bash
git clone https://github.com/grodansparadis/vscp-simulator.git
cd vscp-simulator
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Add `-DCMAKE_PREFIX_PATH=<path to Qt>` if Qt is not installed by the system package manager, for example `-DCMAKE_PREFIX_PATH=~/Qt/6.8.2/gcc_64`. The program is `build/vscpsim` (`build/vscpsim.app` on macOS, `build\vscpsim.exe` on Windows).

`cmake --install build --prefix <folder>` installs the program. On Windows and macOS the Qt runtime is installed with it (turn this off with `-DVSCPSIM_DEPLOY_RUNTIME=OFF`).

The pages for each platform list the packages to install:

* [Linux](setting_up_the_system_on_linux.md)
* [Windows](setting_up_the_system_on_windows.md)
* [macOS](setting_up_the_system_on_macintosh.md)
* [Raspberry Pi](setting_up_the_system_on_raspberry_pi.md)

## Interfaces per platform

All interfaces work on all platforms except `socketcan`, which only exists on Linux (including the Raspberry Pi). See [Configure the simulator](configure.md) and the [README](https://github.com/grodansparadis/vscp-simulator#interfaces) for the interfaces and their settings.

## Running without a desktop

`vscpsim` shows a window. On a machine without a desktop, for example a Raspberry Pi used as a headless node, start it with Qt's offscreen platform:

```bash
QT_QPA_PLATFORM=offscreen vscpsim -i udp -b 0
```

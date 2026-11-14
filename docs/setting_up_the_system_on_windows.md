# Setting up the system on Windows

## Using a ready built program

Download `vscpsim-windows-x64`, see [Setting up the system](setting_up_the_system.md#downloading-a-build), and unpack the zip file to a folder, for example `C:\Program Files\vscpsim`. Start it from a command prompt:

```bat
cd "C:\Program Files\vscpsim\bin"
vscpsim.exe -i udp -b 0
```

The `bin` folder holds `vscpsim.exe` together with Qt and the other DLLs it needs (OpenSSL, libmosquitto, expat, curl, pthreads). vscpsim is a console program, so command line options and the log are shown in the command prompt.

## Building from source

Install

* Visual Studio 2022 with *Desktop development with C++* (includes CMake and Ninja)
* Qt 6 for *MSVC 2022 64-bit* with the [Qt online installer](https://www.qt.io/download-qt-installer)
* [vcpkg](https://vcpkg.io)

Install the libraries with vcpkg:

```bat
vcpkg install curl dlfcn-win32 expat mosquitto openssl pthreads --triplet x64-windows
```

Build from a *x64 Native Tools Command Prompt for VS 2022*:

```bat
git clone https://github.com/grodansparadis/vscp-simulator.git
cd vscp-simulator
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x64-windows ^
  -DCMAKE_PREFIX_PATH=C:\Qt\6.8.1\msvc2022_64
cmake --build build --parallel
cmake --install build --prefix C:\vscpsim
```

`cmake --install` copies `vscpsim.exe` and the Qt runtime to `C:\vscpsim\bin`. Copy the DLLs from `C:\vcpkg\installed\x64-windows\bin` to the same folder.

The `socketcan` interface is not available on Windows. Use `canal` with a CANAL driver (DLL) for CAN hardware, for example a CAN4VSCP adapter.

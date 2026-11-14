# Changes in vscp-simulator

Every change that goes into a release is listed here, newest release first.
Add each change under **Unreleased** when it is made. When a release is
tagged, rename **Unreleased** to the version and date, and start a new
**Unreleased** section above it.

## Unreleased

- Man page vscpsim(1), converted from man/vscpsim.1.md with pandoc and installed in share/man/man1
- Build for Windows x64, macOS (Apple silicon) and Raspberry Pi 32-bit (armhf) and 64-bit (arm64), one CI workflow per platform with a packaged artifact and SHA256SUMS
- README: per-platform build badges, download and build instructions; setup documentation for Linux, Windows, macOS and Raspberry Pi
- CMake: Windows dependencies from vcpkg, macOS app bundle, Qt runtime deployed with the program on Windows and macOS, install paths below the install prefix; drop the unused libwebsockets
- Pin vscp to b6d8ff5: binary tcp/ip mode uses the spec stream prefix and works with the VSCP tcp/ip daemon and the updated vscpl2drv-tcpipsrv
- Document binary mode, encryption and HMAC authentication for the tcpip, udp and multicast interfaces
- Add CI build badges to the README
- Add the `canal` (CANAL Level I driver) and `level2` (VSCP Level II driver) interfaces
- Use CMake FetchContent instead of git submodules
- Add scripts/download-artifacts.sh to download and verify the newest CI build artifacts
- Changed min Linux Ubuntu to 22.04 (d4e7653)
- Updated submodules (5cfc1b6)
- Added comment about Work In Progress (ab2ea70)
- Added some simulator pictures (bbf7f12)

## v0.0.1 - 2025-02-12

- Args transfered to appRun (5ccf3b9)
- Corrected AppRun (a64b0d5)
- Testing 6.8.2 (269547b)
- Added link inclue for CMAKE_DL_LIBS to work on older systems (8609120)
- Removed icon from sidebar (4911988)
- Updated supported transports (75d21b4)
- Updated workflow (82420f0)
- Removed some images (3e5ec19)
- Added missing include file (c0586f0)
- Addded .jekyll (a634406)
- Add initial docs (f6ed0b7)
- Now going to home folder on startup (5a69423)
- Removed vscp submodules to try to fix sync problem (5e25aab)
- Updated current available transports (0df74d4)
- Fixed some warnings (1151c4d)
- Added standard submodules again (7ef6ee5)
- Removed submodules (062036e)
- Added  submodules (b50f209)
- Fixed some annoying warnings (908d94c)
- Added functionality for VSCP bootloader (96da973)
- Added about dialog (468c6e2)
- Fixed startup color problem (32dffe3)
- test (a09bbf1)
- test (a5ba013)
- test (59570af)
- test (2ea90b2)
- test (e809d98)
- test (c22e676)
- test mac (99ea571)
- test (1925824)
- test (9a9576e)
- test max action (b3f3c7b)
- test (8c24984)
- test (61ad33f)
- New compiler for windows build (e2da4a9)
- test (2b8663c)
- New icon name (73e8ab8)
- test appbuild (599bbeb)
- test (6d44332)
- test (15b3ced)
- test (d669a1b)
- test (8b38a66)
- Test (4d0736b)
- Updated VSCP ROOT opath (e70d379)
- Changed path to vscp root (fdad421)
- Test (c6acdb3)
- Updated modules (47895a0)
- test (292dc00)
- Update workflows (2858dc8)
- Added appimage workflow (63e1b25)
- ...and 23 earlier commits (see `git log`)

# Introduction

This manual is `Work in progress` and will be updated as the project progresses.

The VSCP simulator is a software package that can simulate a VSCP node. The application can simulate both a level I and a level II node and can work both in bootloader mode and in firmware mode. The simulator can be used to test and debug VSCP applications and is a valuable tool for developers and testers. Typical use is to validate software solutions.

The simulator is written in C++ and is available for Windows, Linux, macOS and the Raspberry Pi (32-bit and 64-bit). See [Setting up the system](setting_up_the_system.md) for downloads and how to build it. It is open-source and can be downloaded from the [vscp-simulator GitHub repository](https://github.com/grodansparadis/vscp-simulator).

![VSCP Simulator in firmware mode](./images/mode_firmware.png)

First tab when the simulator is in firmware mode. You can manually switch to bootloader mode or do so with the VSCP protocol.

![VSCP Simulator in firmware mode](./images/mode_firmware.png)

First tab when the simulator is in bootloader mode. You can manually switch to firmware mode or do so with the VSCP protocol by uploading a valid firmware image.

![VSCP Simulator in firmware mode](./images/simulation1.png)

The simulation have buttons, checkboxes and sliders. All issue events and registers can be set to control them. RGP colors can be set and read. The simulator can also be controlled by the VSCP protocol.

The MDF for the simulator is available [here](http://eurosource.se/vscpsim_1.xml).

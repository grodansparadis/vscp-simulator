# vscp-simulator

![License](https://img.shields.io/badge/license-MIT-blue.svg)
[![Linux](https://github.com/grodansparadis/vscp-simulator/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/cmake.yml?query=branch%3Amain+is%3Asuccess)
[![AppImage](https://github.com/grodansparadis/vscp-simulator/actions/workflows/appimage.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/appimage.yml?query=branch%3Amain+is%3Asuccess)
[![macOS](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-macos.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-macos.yml?query=branch%3Amain+is%3Asuccess)
[![Windows](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-windows.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-windows.yml?query=branch%3Amain+is%3Asuccess)
[![Raspberry Pi arm64](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-raspberry-pi-arm64.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-raspberry-pi-arm64.yml?query=branch%3Amain+is%3Asuccess)
[![Raspberry Pi armhf](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-raspberry-pi-armhf.yml/badge.svg?branch=main)](https://github.com/grodansparadis/vscp-simulator/actions/workflows/build-raspberry-pi-armhf.yml?query=branch%3Amain+is%3Asuccess)
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

## Version

The version has the form *year.month.patch*, for example `26.11.3`. The patch number counts the pushes to the repository during the month and starts at 0 in a new month. It is updated automatically by the *Update version* GitHub workflow on every push to `main` and lives in [src/vscpsim-version.h](src/vscpsim-version.h), which CMake also uses for the project version. Show it with `vscpsim --version`.

## Get source code

Check out the source code from the repository with

  git clone https://github.com/grodansparadis/vscp-simulator.git

Change folder to **vscp-simulator**

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=~/Qt/6.8.2/gcc_64 ..
make
```

Dependencies are fetched at their pinned revisions by CMake during configuration.

*DCMAKE_PREFIX_PATH* is the path to QT you set it to the folder and the verison of QT you want to use with the vscp-simulator. All versions > 5.15 should work. If using visual studio code set this environment variable in **cmake: configure environment** in the settings for vscode.

The executable is named **vscpsim**.

## Command line switches

| Short | Long  | Description |
| ----- | ----- | ----------- |
| -b | --bootmode | Set bootmode flag "-b 0"=start firmware app. "-b 0xff" = start bootloader. |
| -B | --block | Block info on the form "size:count" where size is the size of a block in bytes and count is the number of blocks of that size. |
| -c | --config | Semicolon separated string for interface settings. For socketcan the CAN device (default vcan0). For canal the driver path followed by the CANAL configuration string. For level2 the driver path and the path to the driver configuration file. |
| -f | --cfgfile | Path to JSON configuration file. A sample configuration file named *firmware_example1.json* is available in the debug folder of the project. |
| -g | --guid | GUID for simulated device. |
| -i | --interface | Interface to use: `canal`, `level2`, `socketcan` (Linux), `tcpip`, `udp`, `multicast`, `ws1`, `ws2` or `mqtt`. |
| -l | --vscplevel | 0/1 VSCP level. (0=Level I / 1=Level II) |
| -s | --host | Host to connect to (tcpip, ws1, ws2, mqtt), destination address (udp) or multicast group (multicast). |
| -p | --port | Port to use for the connection. |
| -P | --password | Password to use for the connection. |
| -t | --timeout | Timeout in milliseconds. |
| -u | --user | Username to use for the connection. |
| | --version | Show the version. |

Settings in the configuration file take precedence over the command line.

## Interfaces

The simulated node is connected with one of the VSCP client interfaces. Either give the interface on the command line, or set it in the `interface` object of the configuration file. All keys in the `interface` object except `type` and `connect-timeout` are handed to the client, so every setting the VSCP client supports (filters, TLS, encryption keys...) can be used. Command line values are only used for settings that are not in the configuration file.

| Interface | Defaults | Typical command line |
| --------- | -------- | -------------------- |
| `socketcan` | device vcan0 | `vscpsim -i socketcan -c vcan0` |
| `tcpip` | tcp://localhost:9598, admin/secret | `vscpsim -i tcpip -s 192.168.1.7 -p 9598 -u admin -P secret` |
| `udp` | broadcast to 255.255.255.255:33333 | `vscpsim -i udp -s 192.168.1.255 -p 33333` |
| `multicast` | group 224.0.23.158, port 9598 | `vscpsim -i multicast -s 224.0.23.158 -p 9598` |
| `ws1` / `ws2` | ws://localhost:8884/ws1 (ws2), admin/secret | `vscpsim -i ws2 -s localhost -p 8884 -u admin -P secret` |
| `mqtt` | localhost:1883 | `vscpsim -i mqtt -f node.json` |
| `canal` | flags 0, no configuration | `vscpsim -i canal -c "/var/lib/vscp/drivers/level1/vscpl1drv-can4vscp.so;/dev/ttyUSB0"` |
| `level2` | | `vscpsim -i level2 -c "/var/lib/vscp/drivers/level2/vscpl2drv-tcpiplink.so;/etc/vscp/tcpiplink.json"` |

With `canal` and `level2` the simulator loads a driver (a shared library) and talks to the other nodes through it, the same way the VSCP daemon uses drivers. A CANAL driver connects to a Level I bus, for example through a CAN4VSCP adapter, and a Level II driver connects to whatever the driver supports.

A node connected over tcp/ip, websockets or MQTT talks to the other nodes through the server or broker. Over UDP and multicast it talks directly with the other nodes on the network. Events the node sent itself and that come back (UDP broadcasts, servers that echo sent events) are ignored by the node.

Examples of `interface` objects in the configuration file:

```json
"interface": { "type": "tcpip", "host": "tcp://192.168.1.7:9598", "user": "admin", "password": "secret" }
"interface": { "type": "udp", "ip": "udp://192.168.1.255:33333" }
"interface": { "type": "multicast", "ip": "udp://224.0.23.158:9598" }
"interface": { "type": "ws2", "url": "wss://server.example.com:8884/ws2", "user": "admin", "password": "secret", "key": "2DBB079A38985AF00EBEEFE22F9FFA0E", "cafile": "/etc/vscp/ca.pem" }
"interface": { "type": "socketcan", "device": "vcan0" }
"interface": { "type": "canal", "path": "/var/lib/vscp/drivers/level1/vscpl1drv-can4vscp.so", "config": "/dev/ttyUSB0", "flags": 0 }
"interface": { "type": "level2", "path-driver": "/var/lib/vscp/drivers/level2/vscpl2drv-tcpiplink.so", "path-config": "/etc/vscp/tcpiplink.json" }
```

For `canal` the keys are `path` (driver), `config` (the CANAL configuration string of the driver), `flags` and `datarate`. For `level2` they are `path-driver`, `path-config` (driver configuration file) or `config` (the driver configuration embedded as JSON or a string), `config-suffix` and `guid`.

### Binary mode, encryption and authentication

The `tcpip`, `udp` and `multicast` interfaces can use the [VSCP binary protocol](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_binary) with AES encryption and HMAC-SHA-256 authentication. Keys can only be set in the configuration file. Keys are hex strings: the AES key is 32, 48 or 64 hex characters for AES-128, AES-192 or AES-256 (`openssl rand -hex 16`, `24` or `32`) and the HMAC key is always 64 hex characters (`openssl rand -hex 32`). The HMAC key must differ from the AES key, and HMAC needs an AES key. All nodes and servers must use the same algorithm and keys. Keep the configuration file private, as it holds the keys.

**tcp/ip.** After logging in with the text protocol the connection switches to binary mode with `binary-mode`. Add `binary-encryption` (1 = AES-128, 2 = AES-192, 3 = AES-256), `binary-key` and `binary-hmac-key` to encrypt and authenticate every binary frame. The text login before binary mode is not encrypted, so use TLS (`btls`, `cafile`, ...) if the network can not be trusted.

```json
"interface": {
  "type": "tcpip", "host": "tcp://192.168.1.7:9598", "user": "admin", "password": "secret",
  "binary-mode": true,
  "binary-encryption": 3,
  "binary-key": "<64 hex characters>",
  "binary-hmac-key": "<64 hex characters>"
}
```

The server must support binary mode with the same keys, for example the VSCP tcp/ip daemon or the [vscpl2drv-tcpipsrv](https://github.com/grodansparadis/vscpl2drv-tcpipsrv) driver. A frame that fails authentication closes the connection.

**UDP and multicast.** Use `encryption` (1, 2 or 3), `key` and `hmac-key`. Without `hmac-key` frames are only encrypted; then set `require-encryption` to `true` to drop frames that are not encrypted.

```json
"interface": {
  "type": "udp", "ip": "udp://255.255.255.255:33333",
  "encryption": 1,
  "key": "<32 hex characters>",
  "hmac-key": "<64 hex characters>"
}
```

Frames with a wrong or missing HMAC are dropped. The [vscpl2drv-udp](https://github.com/grodansparadis/vscpl2drv-udp) driver uses the same format; set its `security.encryption`, `security.key` and `security.hmac` to the same values.

### Firmware mode
The code normally starts up in firmware mode and then simulates all functionality of a VSCP level I or level II node. Set the functionality you want with command line switches or in a configuration file (-f switch).

### Bootloader mode
In bootloader mode the interface (-i) must be specified as any parameters for it (-c). bootmode must be set by setting -b0xff and -B must me used to specify block size and number of blocks. Some interfaces like MQTT , tcp/ip etc may need username/password and other settings.

```bash
vscpsim -i socketcan -c vcan0 -b 0xff -B 256:1024
```

is a typical startup line to test the bootloader over a socketcan interface.

## Configuration file format

The configuration file (set path with -f switch) is in JSON format and defines the simulated firmware of the node. If this file is not available a default configuration will be used.

```json
{
  "interface": {
    "type": "socketcan",
    "device": "vcan0",
    "connect-timeout": 5000
  },
  "bootloader": {
    "blocks": 1000,
    "blocksize": "0x100"
  },
  "device": {
    "name": "VSCP simulated node",
    "level": 2,
    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "mdfurl": "eurosource.se/sim1_l1.mdf",
    "bUse16BitNickname": true,
    "nickname":"0x1234",
    "bootflag": 0,
    "hertbeat-interval": 60000,
    "caps-interval": 300000,
    "bEnableLogging": true,
    "log-id": 123,
    "bEnableErrorReporting": true,
    "bHighEndServerResponse": true,
    "bEnableWriteProtectedLocations": true,
    "user-id": [0,1,2,3,4],
    "manufactured-id": [0,1,2,3],
    "manufacturer-sub-id": [0,1,2,3],
    "firmware-version": [0,0,1],
    "bootloader-algorithm": 0,
    "standard-device-family-code": [0,1,2,3],
    "standard-device-type-code": [0,1,2,3],
    "firmware-device-code": [0,1],
    "ip-addr": [0,1,2,3]
  }
}
```
All numerical values can be set as a number or a string with hex (0x) / octal (0o) / binary (0b) / decimal,  prefix signaling how it should be converted to the numerical value. Called *"numerical string"* below.

All arrays can be set using the meachism used for numerical values (value/string converted to value) 

### interface
Client interface. `type` is "canal", "level2", "socketcan", "tcpip", "udp", "multicast", "ws1", "ws2" or "mqtt". The other keys are the settings of that VSCP client (see [Interfaces](#interfaces)).

### config
A semicolon separated string with configuration data for the client interface. For socketcan the first item is the CAN device (the same as `"device"`). For canal it is "driver path;CANAL configuration" and for level2 "driver path;driver configuration file"; these are used when `path` (canal) or `path-driver` (level2) is not set.

### connect-timeout
Milliseconds to wait for the interface to connect.

### name
Name of the device. Can be a maximum of 64 bytes long.

### level
VSCP level to use for device. Can be 1 or 2.

### guid
GUID for device. Can be set as string, or array. LSB byte is the only valid byte for Level I devices.

### mdfurl
URL poiting to external MDF file. If set to empty string ("") the MDF is internal. If the mdf is preceedid with "local://" a local file will be used to load the mdf. Max 32 bytyes can be used for none local file url's

### bUse16BitNickname
Tell that a 16-bit nickname should be used. 

### nickname
This is the initial nickname used when the system starts up. Unititialized devices with nickname
set to 255 (0xff) will do a nickname discovery to find a free nickname. The value can be a number or a string with hex (0x) / octal (0o) / binary (0b) / decimal,  prefix ignaling how it should be converted to the nickname number. 

### bootflag
The boot flag is used by the device to know which code to run. If it is zero the application will run. If it is nonzero (normally 0xff) the bootloader will run.

### hertbeat-interval
Heartbeat event interval in milliseconds. If set to zero no heartbeat events will be sent.

### caps-interval
Capabilities event interval in milliseconds (Level II). If set to zero no capability events will be sent.

### bEnableLogging
If set to true logging events will be sent by the device

### log-id
Logging channel to use, 0-255

### log-level
Set the log level. A sample is

 | Byte | Description     | 
 | :----: | -----------   | 
 | 0    | Emergency.      | 
 | 1    | Alert.          | 
 | 2    | Critical.       | 
 | 3    | Error.          | 
 | 4    | Warning.        | 
 | 5    | Notice.         | 
 | 6    | Informational.  | 
 | 7    | Debug.          | 
 | 8    | Verbose.        | 

 but its up to the designer to choose schema.

### bEnableErrorReporting
Enable error reporting by the device. Error events will be sent.

### bHighEndServerResponse
Enable responseevent on high end sever probe. (Only for Level II devices).

### bEnableWriteProtectedLocations
If set to true manufacturer id, manufacturer sub id and guid can be written. On live units this is used to set initial parameters on a device. The page select register must me set to 0xffff for any writes
to be accepted.

### user-id
The user id is five bytes (registers 0x84-0x88) that are client writable on a device. The written data is persistent. A number sets the first four bytes. The initial value is set here. It can be set as an array, a decimal value or as a "numerical string".

### manufactured-id
The manufacturer id is a four byte id that is set during manufacturing of a device. The value is set here. It can be set as an array, a decimal value or as a "numerical string".

### manufactured-sub-id
The manufacturer sub id is four byte id that is set during manufacturing of a device.  The value is set here. It can be set as an array, a decimal value or as a "numerical string".

### firmware-version
This is the firmware version for the current code. The value is set here. It can be set as an array, a decimal value or as a "numerical string".

### bootloader-algorithm
Bootloader algorithm the device uses.

### standard-device-family-code
This four byte value specifies the VSCP standard family code for the device. The value is set here. It can be set as an array, a decimal value or as a "numerical string".

### standard-device-type-code
This four byte value specifies the VSCP standard type code for the device. The value is set here. It can be set as an array, a decimal value or as a "numerical string".

### firmware-device-code
This is a 16-bit code that the manufacturer set to identify the device type so that application that want to load firmware know which firmware to select. Typically the value identify the hardware reversion after which different firmware is required. Also the running firmware itself can use this value to adopt itself to the hardware.

### ip-addr
Ip address is the ipv4 or ipv6 address for a device that has one. Set to all zero if other mechanism like DHCP set the ip address. It can be set as a 32-bit integer for ipv4, a string or as an array of 8-bit numbers for ipv4 and and array of 16-bit numbers for ipv6.

## Simulation

The software has the ability to provide many simulations. Currently only one called **sim1** is avilable.

### sim1

![](./images/btest1.png)

The simulation has

 * Ten buttons that send [TurnOn](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.control?id=type5) events when pressed.
 * Ten checkboxes that send [Opened](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.information?id=type7) and [Closed](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.information?id=type8) events when selected/deselected.
 * Ten sliders that sends [Relative Level](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement?id=type50) events when they are move.
 * Nine radio buttons that that send [On](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement?id=type50)  and [Off](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.information?id=type4)events when they are selected/deselected.
 * The background color as an RGN value for the simulation tab can be set with register writes.
 * A timed event is available wich have a configurable frequency. The timed event send a simulated temperature value.

The simulated node has a configurable zone and all controls has there own configurable subzone.

There are registers which can be written to set the value for all controls.

The MDF for this simulated node working as a level I  device is available in the MDF folder of the project as **sim1_l1.xml**. The "l1" in the name is the level and thus will the level II equivalent be *sim1_l2.xml*.

The MDF files are available from a public server as well

 * [sim1_l1.xml](http://eurosource.se/sim1_l1.xml)
 * [sim1_l1.json](http://eurosource.se/sim1_l1.xml)




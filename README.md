# vscp-test-bootloader

![License](https://img.shields.io/badge/license-MIT-blue.svg)
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](https://www.repostatus.org/badges/latest/active.svg)](https://www.repostatus.org/#active)

<img src="https://vscp.org/images/logo.png" width="100">

This is a simple test software for testing of the [VSCP bootloader](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_boot_loader_algorithm) just created for testing and validation and not intended for public use.


use 

  cmake -DCMAKE_PREFIX_PATH=~/Qt/6.7.0/gcc_64 ..

to (or similar) build or set it in cmake: configure environment in the settings for vscode.

## Command line switches

| Short | Long  | Desription |
| ----- | ----- | ---------- |
| -b | --bootmode | Set bootmode flag 0=start firmware app. 0!= start bootloader. |
| -B | --block | Block info on the form size:count where size is he size of a block in bytes and count are the number of blocks of that size. |
| -c | --config | Semicolon seperated string for interface settings. |
| -f | --cfgfile | Path to JSON configuration file |
| -g | --guid | GUID for simulated device. LSB byte is the only valid byte for Level I devices. |
| -h | --host | Host to connect to (valid for tcpip, mqtt, etc). |
| -i | --interface | INterface to use ("socketcan", "canal", "tcpip", "mqtt", etc). |
| -l | --level | 0/1 VSCP level. (0=Level I / 1=Level II) |
| -L | --loglevel | Debug level for console/file on the form level-console:level-file |
| -p | --password | Password to use for connection. |
| -P | --port | Port to uise for connection. |
| -S | --sub | Topic to subscribe to (MQTT). Can be semicolon separated list. |
| -t | --timeout | Timeout in milliseconds |
| -T | --pub | Topic to publish to (MQTT). Can be semicolon separated list. |
| -u | --user | Username to use for connection. |

### Bootloader mode
In bootloader mode the interface (-i) must be specified as any parameters for it (-c). bootmode must be set by setting -b0xff and -B must me used to specify block size and number of blocks. Some interfaces like MQTT , tcp/ip etc may need username/password and other settings.

> btest -i socketcan -c vcan0 -b 0xff -B 256:1024

is a typical startup line to tes the bootloader over a socketcan interface.

### Firmware mode

## Configuration file format

The configuration file is in JSON format and defines the simulated firmware of the node. If this file
is not available a default configuration will be used.

```json
{
  "interface": {
    "type": "socketcan",
    "config": "value1;value2;value3",
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
    "mdfurl": "eurosource.se/simulated01.mdf",
    "bUse16BitNickname": true,
    "nickname":"0x1234",
    "bootflag": 0,
    "hertbeat-interval": 60,
    "caps-interval": 300,
    "bEnableLogging": true,
    "log-id": 123,
    "bEnableErrorReporting": true,
    "m_bSendHighEndServerProbe": true,
    "m_bHighEndServerResponse": true,
    "m_bEnableWriteProtectedLocations": true,
    "user-id": [0,1,2,3],
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
All numerical values can be set as a number or a string with hex (0x) / octal (0o) / binary (0b) / decimal,  prefix signaling how it should be converted to the numerical value. Called "numerical string" below.

All arrays can be set using the meachism used for numerical values (value/string converted to value) 

### interface
Client interface. Can currently be "socketcan", "canal", "mqtt", "tcpip", "udp", "multicast","ws1", "ws2"

### config
A semicolon separated string with configuration data for the client interface.



### name
Name of the device. Can be a maximum of 64 bytes long.

### level
VSCP level to use for device. Can be 1 or 2.

### guid
GUID for device. Can be set as string, or array

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
Heartbeat event interval in seconds. If set to zero no heartbeat events will be sent.

### caps-interval
Capabilities event interval in seconds. If set to zero no capability events will be sent.

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

### bSendHighEndServerProbe
Enable sending of high end server probe. (Only for Level II devices).

### bHighEndServerResponse
Enable responseevent on high end sever probe. (Only for Level II devices).

### bEnableWriteProtectedLocations
If set to true manufacturer id, manufacturer sub id and guid can be written. On live units this is used to set initial parameters on a device. The page select register must me set to 0xffff for any writes
to be accepted.

### user-id
The user id is four bytes that are client writable on a device. TGhe written data is persistent. The initial value is set here. It can be set as an array, a decimal value or as a "numerical string".

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
This is a 16-bit code that the manufacturer set to identify the device type so that application that want to load firmware know which firmware to select. Typically the value identify the hardware reversion after which differenet firmwae is required. Also the running firmare itself can use this value to adopt itself to the hardware.

### ip-addr
Ip address is the ipv4 or ipv6 address for a device that has one. Set to all zero if other mechanism like DHCP set the ip address. It can be set as a 32-bit integer for ipv4, a string or as an array of 8-bit numbers for ipv4 and and array of 16-bit numbers for ipv6.
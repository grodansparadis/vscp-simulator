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
| -f | --file | Path to JSON configuration file |
| -g | --guid | GUID for simulated device. LSB byte is the only valid byte for Level I devices. |
| -h | --host | Host to connect to (valid for tcpip, mqtt, etc). |
| -i | --interface | INterface to use ("socketcan", "canal", "tcpip", "mqtt", etc). |
| -l | --VSCP level | 0/1 VSCP level. (0=Level I / 1=Level II) |
| -L | --loglevel | Debug level for console/file on the form level-console:level-file |
| -p | --password | Password to use for connection. |
| -P | --port | Port to uise for connection. |
| -S | --sub | Topic to subscribe to (MQTT). Can be semicolon separated list. |
| -t | --timeout | Timeout in milliseconds |
| -T | --pub | Topic to publish to (MQTT). Can be semicolon separated list. |
| -u | --user | Username to use for connection. |


# How to use

Choose a transport to connect the simulated node to a VSCP server, broker, network or driver.

| Transport | Interface | Platforms | Requires |
| --------- | --------- | --------- | -------- |
| [SocketCAN](#socketcan) | `socketcan` | Linux, including Raspberry Pi | A CAN or virtual CAN interface |
| [TCP/IP](#tcpip) | `tcpip` | All | A VSCP TCP/IP server |
| [UDP](#udp) | `udp` | All | A peer using VSCP over UDP |
| [Multicast](#multicast) | `multicast` | All | Peers on the same multicast group |
| [WebSocket 1](#websocket-1) | `ws1` | All | A VSCP WebSocket 1 endpoint |
| [WebSocket 2](#websocket-2) | `ws2` | All | A VSCP WebSocket 2 endpoint |
| [MQTT](#mqtt) | `mqtt` | All | An MQTT broker and VSCP event topics |
| [CANAL driver](#canal-driver) | `canal` | All | A CANAL Level I driver for your platform |
| [Level II driver](#level-ii-driver) | `level2` | All | A VSCP Level II driver for your platform |

## Before you start

[Install or build the simulator](setting_up_the_system.md). The executable is `vscpsim` (`vscpsim.exe` on Windows or `vscpsim.app/Contents/MacOS/vscpsim` in a macOS app bundle).

The JSON examples below are complete minimal configuration files. Save one as `node.json`, replace the example addresses, credentials and paths, and start it with:

```bash
vscpsim -f node.json
```

You can add `device` and `bootloader` objects as described in the [configuration reference](configure.md#configuration-file-format). Settings in the configuration file take precedence over command-line options. Give each simulated node its own GUID when testing multiple nodes.

The simulator normally starts in firmware mode. To test firmware uploads, see [Bootloader mode](configure.md#bootloader-mode). On a machine without a desktop, use [Qt's offscreen platform](setting_up_the_system.md#running-without-a-desktop).

## SocketCAN

SocketCAN connects directly to a Linux CAN interface. Use a configured hardware interface such as `can0`, or a virtual interface such as `vcan0` for local tests. A virtual interface does not connect to a physical CAN bus.

To create a virtual interface on Linux:

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set dev vcan0 up
```

Create it only once; if `vcan0` already exists, just bring it up. Start the simulator with:

```bash
vscpsim -i socketcan -c vcan0
```

Or use:

```json
{
  "interface": {
    "type": "socketcan",
    "device": "vcan0"
  }
}
```

Configure the bitrate and bring up a physical CAN interface before starting the simulator. The peer must use the same CAN interface or physical bus and exchange VSCP CAN frames. SocketCAN is not available on Windows or macOS; use a [CANAL driver](#canal-driver) for supported CAN hardware there.

## TCP/IP

Connect to a VSCP TCP/IP server, such as the VSCP daemon or a VSCP TCP/IP server driver. This is a VSCP connection, not a raw TCP connection to an arbitrary device.

```bash
vscpsim -i tcpip -s 192.168.1.7 -p 9598 -u simulator -P change-me
```

Or use:

```json
{
  "interface": {
    "type": "tcpip",
    "host": "tcp://192.168.1.7:9598",
    "user": "simulator",
    "password": "change-me"
  }
}
```

Create the account on the server and allow it to send and receive the events needed by the simulation. The default endpoint is `localhost:9598`; the simulator supplies `admin` / `secret` if credentials are omitted. Replace these defaults for your server.

Text login is not encrypted. For untrusted networks, configure TLS on both ends. See [Binary mode, encryption and authentication](configure.md#binary-mode-encryption-and-authentication) for binary event transfer, AES and HMAC settings.

## UDP

UDP sends VSCP datagrams directly to other nodes without a server or broker. Use broadcast for a local subnet, or a peer's address for unicast.

```bash
vscpsim -i udp -s 192.168.1.255 -p 33333
```

Or use:

```json
{
  "interface": {
    "type": "udp",
    "ip": "udp://192.168.1.255:33333"
  }
}
```

Replace the example broadcast address with the address for your subnet. The default destination is `255.255.255.255:33333`. Allow the UDP port through the host firewall and configure peers for the same VSCP datagram format. Broadcast normally stays on the local network.

UDP does not guarantee delivery or ordering. For AES encryption and HMAC authentication, see [Binary mode, encryption and authentication](configure.md#binary-mode-encryption-and-authentication); matching keys are required at every peer.

## Multicast

Multicast distributes VSCP datagrams to nodes that join the same group. It does not need a VSCP server or MQTT broker.

```bash
vscpsim -i multicast -s 224.0.23.158 -p 9598
```

Or use:

```json
{
  "interface": {
    "type": "multicast",
    "ip": "udp://224.0.23.158:9598"
  }
}
```

The default group and port are `224.0.23.158:9598`. All peers must join the same group and use the same port. Check firewall rules, the selected network interface, and multicast support on switches, routers and Wi-Fi access points if events do not arrive. Multicast traffic does not automatically cross routed networks.

Like UDP, multicast does not guarantee delivery. See [Binary mode, encryption and authentication](configure.md#binary-mode-encryption-and-authentication) for AES and HMAC settings.

## WebSocket 1

Use `ws1` with a server that implements the VSCP WebSocket 1 protocol. The default endpoint is `ws://localhost:8884/ws1`.

```bash
vscpsim -i ws1 -s 192.168.1.7 -p 8884 -u simulator -P change-me
```

Or use:

```json
{
  "interface": {
    "type": "ws1",
    "url": "ws://192.168.1.7:8884/ws1",
    "user": "simulator",
    "password": "change-me"
  }
}
```

The server must expose `/ws1` and accept the supplied account. For a different endpoint path, set the full `url` in the configuration file. A generic WebSocket echo server cannot handle VSCP commands.

For TLS, use a `wss://` URL and configure `cafile` if your server uses a private certificate authority. If the server uses a non-default VSCP WebSocket authentication key, set the matching `key` in the `interface` object. The authentication key does not replace TLS.

## WebSocket 2

Use `ws2` with a server that implements the VSCP WebSocket 2 protocol. Select it to match the server's protocol, not merely because the server supports WebSockets. The default endpoint is `ws://localhost:8884/ws2`.

```bash
vscpsim -i ws2 -s 192.168.1.7 -p 8884 -u simulator -P change-me
```

Or use:

```json
{
  "interface": {
    "type": "ws2",
    "url": "ws://192.168.1.7:8884/ws2",
    "user": "simulator",
    "password": "change-me"
  }
}
```

The server must expose `/ws2` and accept the supplied account. As with [WebSocket 1](#websocket-1), set `url` for a custom path, use `wss://` for TLS, and set `cafile` and the WebSocket authentication `key` when required by the server.

## MQTT

MQTT connects through a broker, normally on port `1883`. Configure both publication and subscription topics: connecting to the broker alone does not define where VSCP events are exchanged.

Save this example as `node.json` and start it with `vscpsim -f node.json`:

```json
{
  "interface": {
    "type": "mqtt",
    "host": "localhost",
    "port": 1883,
    "clientid": "vscpsim-node1",
    "subscribe": [
      {
        "topic": "vscp/simulator/node1/rx/#",
        "qos": 0,
        "format": "json"
      }
    ],
    "publish": [
      {
        "topic": "vscp/simulator/node1/tx",
        "qos": 0,
        "bretain": false,
        "format": "json"
      }
    ]
  }
}
```

Other VSCP applications should publish incoming events to a matching receive topic, such as `vscp/simulator/node1/rx/events`, and subscribe to `vscp/simulator/node1/tx` for events emitted by the simulated node. Payloads must be VSCP event JSON, not arbitrary JSON messages.

The topic names above are examples, not mandatory VSCP topic names. Adapt them to your broker's routing and access rules. Keep transmit and receive topics separate to avoid unintended event loops, and use a unique `clientid` for each connection.

Add `user` and `password` if the broker requires authentication. Configure the MQTT client's TLS settings for connections over untrusted networks. Protect configuration files containing credentials.

## CANAL driver

The `canal` interface loads a CANAL Level I driver for a CAN adapter or another Level I connection. Install a driver built for your operating system and the simulator's architecture, along with any hardware access permissions it needs.

For example, with a CAN4VSCP driver on Linux:

```bash
vscpsim -i canal -c "/var/lib/vscp/drivers/level1/vscpl1drv-can4vscp.so;/dev/ttyUSB0;125"
```

The first item is the driver path; the remaining semicolon-separated items form the driver's configuration string. Their meaning is driver-specific.

The equivalent configuration file is:

```json
{
  "interface": {
    "type": "canal",
    "path": "/var/lib/vscp/drivers/level1/vscpl1drv-can4vscp.so",
    "config": "/dev/ttyUSB0;125",
    "flags": 0
  }
}
```

Use the appropriate driver library and configuration for your adapter. On Windows, use the driver's DLL path and escape backslashes in JSON, or use forward slashes. Consult the driver's documentation for `config`, `flags`, bitrate and device permissions.

## Level II driver

The `level2` interface loads a VSCP Level II driver. The driver determines the underlying connection and has its own configuration; it is not interchangeable with a CANAL Level I driver.

For example:

```bash
vscpsim -i level2 -c "/var/lib/vscp/drivers/level2/vscpl2drv-tcpiplink.so;/etc/vscp/tcpiplink.json"
```

Or use:

```json
{
  "interface": {
    "type": "level2",
    "path-driver": "/var/lib/vscp/drivers/level2/vscpl2drv-tcpiplink.so",
    "path-config": "/etc/vscp/tcpiplink.json"
  }
}
```

Install a driver matching the simulator's operating system and architecture, and create its configuration file according to the driver's documentation. For the TCP/IP link driver, that file defines the connection to the remote VSCP server.

Use `path-driver` and `path-config` for driver and configuration paths. A driver configuration can also be supplied through the `config` key when supported. See the [interface reference](configure.md#interfaces) for additional driver settings.

---

[filename](./bottom_copyright.md ':include')

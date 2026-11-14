% VSCPSIM(1) VSCP Simulator
% Åke Hedman, Grodans Paradis AB
% November 2026

# NAME

vscpsim - simulate a VSCP node

# SYNOPSIS

**vscpsim** [*options*]

# DESCRIPTION

**vscpsim** simulates a VSCP Level I or Level II node with registers, a
decision matrix, standard events and a bootloader. It shows a window with
buttons, check boxes and sliders that send events, and answers register reads
and writes like a real node. It is used to test VSCP Works, drivers, servers
and other software without hardware.

The node connects to the other nodes through one of the VSCP client
interfaces: *socketcan*, *tcpip*, *udp*, *multicast*, *ws1*, *ws2*, *mqtt*, or
a *canal* or *level2* driver. The interface is given on the command line or in
the **interface** object of the configuration file (**-f**). All settings of
the VSCP client, such as TLS, AES encryption and HMAC keys, can be set there.

In firmware mode (the default) the simulated firmware runs. In bootloader mode
(**-b** 0xff) the VSCP bootloader runs and firmware can be loaded into the
node.

# OPTIONS

**-h**, **\-\-help**
:   Show the options.

**-v**, **\-\-version**
:   Show the version.

**-i**, **\-\-interface** *interface*
:   Interface to use: **canal**, **level2**, **socketcan** (Linux), **tcpip**,
    **udp**, **multicast**, **ws1**, **ws2** or **mqtt**.

**-c**, **\-\-config** *cfg1;cfg2;...*
:   Configuration string for the interface. socketcan: the CAN device
    (default vcan0). canal: *driver path*;*CANAL configuration*. level2:
    *driver path*;*driver configuration file*.

**-s**, **\-\-host** *host*
:   Server to connect to (tcpip, ws1, ws2, mqtt), destination address (udp)
    or multicast group (multicast).

**-p**, **\-\-port** *port*
:   Port. Default 9598 tcpip, 33333 udp, 9598 multicast, 8884 ws1/ws2, 1883
    mqtt.

**-u**, **\-\-user** *user*
:   User name (default admin).

**-P**, **\-\-password** *password*
:   Password (default secret).

**-t**, **\-\-timeout** *ms*
:   Timeout in milliseconds for the communication.

**-g**, **\-\-guid** *guid*
:   GUID of the client.

**-l**, **\-\-vscplevel** *0|1*
:   VSCP level of the node, 0 = Level I, 1 = Level II.

**-b**, **\-\-bootmode** *mode*
:   0 starts the application firmware, a non zero value (typically 0xff) the
    bootloader.

**-B**, **\-\-blocksize** *size:count*
:   Bootloader flash blocks: the size of a block in bytes and the number of
    blocks.

**-f**, **\-\-cfgfile** *path*
:   JSON configuration file with the node (GUID, nickname, MDF URL,
    registers, simulation) and the **interface** object.

# CONFIGURATION FILE

```
{
  "interface": { "type": "socketcan", "device": "can0" },
  "device": {
    "name": "Test node",
    "level": 1,
    "guid": "FF:FF:FF:FF:FF:FF:FF:FE:00:00:00:00:00:00:00:2A",
    "mdfurl": "eurosource.se/sim1_l1.xml",
    "nickname": 42,
    "simulation": 1
  }
}
```

The keys of **interface** other than **type** are handed to the VSCP client,
for example **binary-encryption**, **binary-key** and **binary-hmac-key** for
an encrypted tcpip connection or **encryption**, **key** and **hmac-key** for
UDP and multicast.

# EXAMPLES

Simulate a node on a virtual CAN bus:

    vscpsim -i socketcan -c vcan0

Connect to a VSCP daemon:

    vscpsim -i tcpip -s 192.168.1.7 -p 9598 -u admin -P secret

Broadcast over UDP:

    vscpsim -i udp -s 192.168.1.255 -p 33333

Test the bootloader over SocketCAN with 1024 blocks of 256 bytes:

    vscpsim -i socketcan -c vcan0 -b 0xff -B 256:1024

Use a CANAL driver:

    vscpsim -i canal -c "/var/lib/vscp/drivers/level1/vscpl1drv-can4vscp.so;/dev/ttyUSB0;125"

# FILES

*~/.config/VSCP/vscpsim.conf*
:   Settings of the program (logging, window).

# SEE ALSO

**vscpworks**(1), **vscp-scan**(1), **vscp-reg**(1), **vscp-dump**(1),
**vscpd**(8)

The VSCP specification: <https://grodansparadis.github.io/vscp-doc-spec>

The source code and the full documentation:
<https://github.com/grodansparadis/vscp-simulator>

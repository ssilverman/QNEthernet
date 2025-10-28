<a href="https://github.com/sponsors/ssilverman" title="Sponsor @ssilverman on GitHub"><img src="https://img.shields.io/badge/Sponsor-30363d.svg?logo=githubsponsors&logoColor=EA4AAA" alt="Sponsor @ssilverman button"></a>
or
<a href="https://www.buymeacoffee.com/ssilverman" title="Donate to this project using Buy Me a Coffee"><img src="https://img.shields.io/badge/buy%20me%20a%20coffee-donate-orange.svg?logo=buy-me-a-coffee&logoColor=FFDD00" alt="Buy Me a Coffee donate button"></a>

# _QNEthernet_, an lwIP-Based Ethernet Library For Teensy 4.1 and possibly some other platforms

_Version: 0.33.0-snapshot_

The _QNEthernet_ library provides Ethernet functionality for the Teensy 4.1 and
possibly some other platforms. It's designed to be compatible with the
Arduino-style Ethernet API.

This library is distributed under the "AGPL-3.0-or-later" license. Please
contact the author if you wish to inquire about other license options.

Please see the _lwip-info/_ directory for the info files provided with the
lwIP release.

## Table of contents

1. [Introduction](#introduction)
   1. [Two notes](#two-notes)
   2. [Other differences and notes](#other-differences-and-notes)
2. [Additional functions and features not in the Arduino-style API](#additional-functions-and-features-not-in-the-arduino-style-api)
   1. [`Ethernet`](#ethernet)
   2. [`EthernetClient`](#ethernetclient)
      1. [TCP socket options](#tcp-socket-options)
      2. [IP header values](#ip-header-values)
   3. [`EthernetServer`](#ethernetserver)
   4. [`EthernetUDP`](#ethernetudp)
      1. [IP header values](#ip-header-values-1)
      2. [`parsePacket()` return values](#parsepacket-return-values)
   5. [`EthernetFrame`](#ethernetframe)
   6. [`MDNS`](#mdns)
   7. [`DNSClient`](#dnsclient)
   8. [Print utilities](#print-utilities)
   9. [`operator bool()` and `explicit`](#operator-bool-and-explicit)
   10. [Use of `errno`](#use-of-errno)
3. [How to run](#how-to-run)
   1. [Concurrent use is not supported](#concurrent-use-is-not-supported)
   2. [How to move the stack forward and receive data](#how-to-move-the-stack-forward-and-receive-data)
   3. [Link detection](#link-detection)
   4. [Notes on `yield()`](#notes-on-yield)
4. [How to write data to connections](#how-to-write-data-to-connections)
   1. [`writeFully()` with more break conditions](#writefully-with-more-break-conditions)
   2. [Write immediacy](#write-immediacy)
5. [A note on the examples](#a-note-on-the-examples)
6. [A survey of how connections (aka `EthernetClient`) work](#a-survey-of-how-connections-aka-ethernetclient-work)
   1. [Connections and link/interface detection](#connections-and-linkinterface-detection)
   2. [`connect()` behaviour and its return values](#connect-behaviour-and-its-return-values)
   3. [Non-blocking connection functions](#non-blocking-connection-functions)
   4. [Getting the TCP state](#getting-the-tcp-state)
7. [How to use multicast](#how-to-use-multicast)
8. [How to use listeners](#how-to-use-listeners)
9. [How to change the number of sockets](#how-to-change-the-number-of-sockets)
10. [UDP receive buffering](#udp-receive-buffering)
11. [mDNS services](#mdns-services)
12. [DNS](#dns)
13. [stdio](#stdio)
    1. [Adapt stdio files to the Print interface](#adapt-stdio-files-to-the-print-interface)
14. [Raw Ethernet frames](#raw-ethernet-frames)
    1. [Promiscuous mode](#promiscuous-mode)
    2. [Raw frame receive buffering](#raw-frame-receive-buffering)
    3. [Raw frame loopback](#raw-frame-loopback)
15. [How to implement VLAN tagging](#how-to-implement-vlan-tagging)
16. [Application layered TCP: TLS, proxies, etc.](#application-layered-tcp-tls-proxies-etc)
    1. [About the allocator functions](#about-the-allocator-functions)
    2. [About the TLS adapter functions](#about-the-tls-adapter-functions)
    3. [How to enable Mbed TLS](#how-to-enable-mbed-tls)
       1. [Installing the Mbed TLS library](#installing-the-mbed-tls-library)
          1. [Mbed TLS library install for Arduino IDE](#mbed-tls-library-install-for-arduino-ide)
          2. [Mbed TLS library install for PlatformIO](#mbed-tls-library-install-for-platformio)
       2. [Implementing the _altcp_tls_adapter_ functions](#implementing-the-altcp_tls_adapter-functions)
       3. [Implementing the Mbed TLS entropy function](#implementing-the-mbed-tls-entropy-function)
17. [On connections that hang around after cable disconnect](#on-connections-that-hang-around-after-cable-disconnect)
18. [Notes on ordering and timing](#notes-on-ordering-and-timing)
19. [Notes on RAM1 usage](#notes-on-ram1-usage)
20. [Heap memory use](#heap-memory-use)
21. [Entropy generation](#entropy-generation)
    1. [The `RandomDevice` _UniformRandomBitGenerator_](#the-randomdevice-uniformrandombitgenerator)
22. [Security features](#security-features)
    1. [Secure TCP initial sequence numbers (ISNs)](#secure-tcp-initial-sequence-numbers-isns)
23. [Configuration macros](#configuration-macros)
    1. [Configuring macros using the Arduino IDE](#configuring-macros-using-the-arduino-ide)
    2. [Configuring macros using PlatformIO](#configuring-macros-using-platformio)
    3. [Changing lwIP configuration macros in `lwipopts.h`](#changing-lwip-configuration-macros-in-lwipoptsh)
24. [Complete list of features](#complete-list-of-features)
25. [Compatibility with other APIs](#compatibility-with-other-apis)
26. [Other notes](#other-notes)
27. [To do](#to-do)
28. [Code style](#code-style)
29. [References](#references)

## Introduction

The _QNEthernet_ library is designed to be a drop-in replacement for code using
the Arduino-style Ethernet API.

**Note: Please read the function docs in the relevant header files for
more information.**

### Three notes

There are three notes, as follows:

1. The `QNEthernet.h` header must be included instead of `Ethernet.h`.
2. Everything is inside the `qindesign::network` namespace. In many cases,
   adding the following at the top of your program will obviate the need to
   qualify any object uses or make any other changes:
   ```c++
   using namespace qindesign::network;
   ```
3. On non-Teensy platforms: `Ethernet.loop()` must be called regularly, either
   at the end of the main `loop()` function, or by hooking into `yield()`.
   (This is already done for you on the Teensy platform by hooking into
   `yield()` via `EventResponder`.)\
   See: [How to move the stack forward and receive data](#how-to-move-the-stack-forward-and-receive-data)

For API additions beyond what the Arduino-style API provides, see:\
[Additional functions and features not in the Arduino-style API](#additional-functions-and-features-not-in-the-arduino-style-api)

### Other differences and notes

* UDP support is already included in _QNEthernet.h_. There's no need to also
  include _QNEthernetUDP.h_.
* Ethernet `loop()` is called from `yield()` (automatically on the Teensy
  platform). The functions that wait for timeouts rely on this. This also means
  that you must use `delay(ms)` (assuming it internally calls `yield()`),
  `yield()`, or `Ethernet.loop()` when waiting on conditions; waiting without
  calling these functions will cause the TCP/IP stack to never refresh. Note
  that many of the I/O functions call `loop()` so that there's less burden on
  the calling code.
* All but one of the `Ethernet.begin(...)` functions don't block.
  `Ethernet.begin(mac[, timeout])` blocks, while waiting for an IP address, to
  match the Arduino-style API. It uses a default timeout of 60 seconds. This
  behaviour can be emulated by following a call to `begin()` with a loop that
  checks `Ethernet.localIP()` for a valid IP. See also the new
  `Ethernet.waitForLocalIP(timeout)` or `Ethernet.onAddressChanged(cb)`.
* The Arduino-style `Ethernet.begin(mac, ...)` functions all accept a NULL MAC
  address. If the address is NULL then the internal or system default MAC
  address will be used. As well, if Ethernet fails to start then the MAC address
  will not be changed.
* `EthernetServer::write(...)` functions always return the write size requested.
  This is because different clients may behave differently.
* The examples at
  https://docs.arduino.cc/libraries/ethernet/#Server%20Class (server.accept())
  and https://docs.arduino.cc/libraries/ethernet/#Client%20Class
  (if&nbsp;(EthernetClient)) directly contradict each other with regard to what
  `operator bool()` means in `EthernetClient`. The first example uses it as
  "already connected", while the second uses it as "available to connect".
  "Connected" is the chosen concept, but different from `connected()` in that it
  doesn't check for unread data.
* The three Arduino-defined `Ethernet.begin(...)` functions that use the MAC
  address and that don't specify a subnet are deprecated because they make some
  incorrect assumptions about the subnet and gateway.
* `Ethernet.hardwareStatus()`: Adds `EthernetOtherHardware` and
  `EthernetTeensy41` to the list of possible return values. Note that these
  values are not defined in the Arduino-style API.
* The following `Ethernet` functions are deprecated and do nothing or return
  some default value:
  * `maintain()`: Returns zero.
  * `setRetransmissionCount(uint8_t number)`: Does nothing.
  * `setRetransmissionTimeout(uint16_t milliseconds)`: Does nothing.
* The `EthernetUDP::flush()` function does nothing because it is ill-defined.
  Note that this is actually defined in the "Arduino WiFi" and Teensy "UDP" APIs
  and not in the main "Arduino Ethernet" API.\
  See: https://docs.arduino.cc/libraries/wifi/#UDP%20class (WiFiUDP.flush())
* The system starts with the Teensy's actual MAC address or some default MAC
  address on other platforms. If you want to use that address with the
  MAC-taking API, you can collect it with `Ethernet.macAddress(mac)` and then
  pass it to one of the MAC-taking `begin(...)` functions.
* All classes and objects are in the `qindesign::network` namespace. This means
  you'll need to fully qualify any types. To avoid this, you could utilize a
  `using` directive:
  ```c++
  using namespace qindesign::network;

  EthernetUDP udp;

  void setup() {
    Ethernet.begin();
  }
  ```
  However, this pollutes the current namespace. An alternative is to choose
  something shorter. For example:
  ```c++
  namespace qn = qindesign::network;

  qn::EthernetUDP udp;

  void setup() {
    qn::Ethernet.begin();
  }
  ```
* Files that configure lwIP for our system:
  * _src/sys_arch.cpp_
  * _src/lwipopts.h_ &larr; use this one for tuning (see _src/lwip/opt.h_ for
    more details)
  * _src/arch/cc.h_
* File that configures _QNEthernet_ options: _src/qnethernet_opts.h_
* The main include file, `QNEthernet.h`, in addition to including the
  `Ethernet`, `EthernetFrame`, and `MDNS` instances, also includes the headers
  for `EthernetClient`, `EthernetServer`, and `EthernetUDP`.
* Most of the `Ethernet` functions do nothing or return some form of
  empty/nothing/false unless the system has been initialized.

## Additional functions and features not in the Arduino-style API

_QNEthernet_ defines functions that don't exist in the Arduino-style API as it's
currently defined. (See:
[Arduino Ethernet Reference](https://docs.arduino.cc/libraries/ethernet/))
This section documents those functions.

Features:
* The `read(buf, len)` functions allow a NULL buffer so that the caller can skip
  data without having to read into a buffer.

### `Ethernet`

The `Ethernet` object is the main Ethernet interface.

* `begin()`: Initializes the library, uses the Teensy's internal MAC address or
  some default MAC address, and starts the DHCP client. This returns whether
  startup was successful. This does not wait for an IP address.
* `begin(ipaddr, netmask, gw)`: Initializes the library, uses the Teensy's
  internal MAC address or some default MAC address, and uses the given
  parameters for the network configuration. This returns whether startup was
  successful. The DNS server is not set. This starts the DHCP client if the IP
  address is `INADDR_NONE`.
* `begin(ipaddr, netmask, gw, dns)`: Initializes the library, uses the Teensy's
  internal MAC address or some default MAC address, and uses the given
  parameters for the network configuration. This returns whether startup was
  successful. The DNS server is only set if `dns` is not `INADDR_NONE`; it
  remains the same if `dns` is `INADDR_NONE`. This starts the DHCP client if the
  IP address is `INADDR_NONE`.

  Passing `dns`, if not `INADDR_NONE`, ensures that the DNS server IP is set
  before the _address-changed_ callback is called. The alternative approach to
  ensure that the callback has all the information is to call
  `setDNSServerIP(ip)` before the three-parameter version.

* `broadcastIP()`: Returns the broadcast IP address associated with the current
  local IP and subnet mask. If Ethernet is not initialized then this will return
  255.255.255.255.
* `dnsServerIP(index)`: Gets a specific DNS server IP address. This returns
  `INADDR_NONE` if the index not in the exclusive range,
  [0, `DNSClient::maxServers()`).
* `driverCapabilities()`: Returns the driver's set of capabilities.\
  Notes:
  * If the link state is not detectable then it must be managed
    with `setLinkState(flag)`.
* `end()`: Shuts down the library, including the Ethernet clocks.
* `hostByName()`: Convenience function that tries to resolve a hostname into an
  IP address. This returns whether successful.
* `hostname()`: Gets the DHCP client hostname. An empty string means that no
  hostname is set. The default is "qnethernet-lwip".
* `interfaceName()`: Returns the interface name, or, if Ethernet has not been
  initialized, an empty string.
* `interfaceStatus()`: Returns the network interface status, `true` for UP and
  `false` for DOWN.
* `isDHCPActive()`: Returns whether DHCP is active.
* `isDHCPEnabled()`: Returns whether the DHCP client is enabled. This is valid
  whether Ethernet has been started or not.
* `linkState()`: Returns a `bool` indicating the link state. This returns `true`
  if the link is on and `false` otherwise. This may be managed manually
  with `setLinkState(flag)`.
* `linkSpeed()`: Returns the link speed in Mbps.
* `linkIsCrossover()`: Returns whether a crossover cable is detected.
* `linkIsFullDuplex()`: Returns whether the link is full duplex (`true`) or half
  duplex (`false`).
* `joinGroup(ip)`: Joins a multicast group.
* `leaveGroup(ip)`: Leaves a multicast group.
* `macAddress()`: Convenience function that returns a pointer to the current
  MAC address.
* `macAddress(mac)`: Fills the 6-byte `mac` array with the current MAC address.
  Note that the equivalent Arduino function is `MACAddress(mac)`.
* `renewDHCP()`: Renews any active DHCP lease and returns whether the request
  was sent successfully.
* `setDHCPEnabled(flag)`: Enables or disables the DHCP client. This may be
  called either before or after Ethernet has started. If DHCP is desired and
  Ethernet is up, but DHCP is not active, an attempt will be made to start the
  DHCP client if the flag is true. This returns whether that attempt was
  successful or if no restart attempt is required.
* `setDNSServerIP(dnsServerIP)`: Sets the DNS server IP address. Note that the
  equivalent Arduino function is `setDnsServerIP(dnsServerIP)`.
* `setDNSServerIP(index, ip)`: Sets a specific DNS server IP address. This does
  nothing if the index is not in the exclusive range,
  [0, `DNSClient::maxServers()`).
* `setHostname(hostname)`: Sets the DHCP client hostname. The empty string will
  set the hostname to nothing. To use something other than the default at system
  start, call this before calling `begin()`.
* `setLinkState(flag)`: Manually sets the link state. This is useful when using
  the loopback feature. Network operations will usually fail unless there's
  a link.
* `setMACAddressAllowed(mac, flag)`: Allows or disallows Ethernet frames
  addressed to the specified MAC address. This is useful when processing raw
  Ethernet frames.
* `waitForLink(timeout)`: Waits for the specified timeout (milliseconds) for
  a link to be detected. This is useful when setting a static IP and making
  connections as a client. Returns whether a link was detected within the
  given timeout.
* `waitForLocalIP(timeout)`: Waits for the specified timeout (milliseconds) for
  the system to have a local IP address. This is useful when waiting for a
  DHCP-assigned address. Returns whether the system obtained an address within
  the given timeout. Note that this also works for a static-assigned address.
* `operator bool()`: Tests if Ethernet is initialized.
* Callback functions: Note that callbacks should be registered before any other
  Ethernet functions are called. This ensures that all events are captured. This
  includes `Ethernet.begin(...)`.
  * `onLinkState(cb)`: The callback is called when the link changes state, for
    example when the Ethernet cable is unplugged.
  * `onAddressChanged(cb)`: The callback is called when any IP settings have
    changed. This might be called before the link or network interface is up if
    a static IP is set.
  * `onInterfaceStatus(cb)`: The callback is called when the network interface
    status changes. It is called _after_ the interface is up but _before_ the
    interface goes down.
* `static constexpr bool isPromiscuousMode()`: Returns whether promiscuous mode
  is enabled.
* `static const char* libraryVersion()`: Returns the library version.
* `static constexpr int maxMulticastGroups()`: Returns the maximum number of
  available multicast groups, not including the "all systems" group.
* `static constexpr size_t mtu()`: Returns the MTU.

### `EthernetClient`

* `abort()`: Aborts a connection without going through the TCP close process.
* `close()`: Closes a connection, but without waiting. It's similar to `stop()`.
  This is superseded by `setConnectionTimeoutEnabled(flag)` with `stop()`.
* `closeOutput()`: Shuts down the transmit side of the socket. This is a
  half-close operation.
* `connectNoWait(ip, port)`: Similar to `connect(ip, port)`, but it doesn't
  wait for a connection. This is superseded by
  `setConnectionTimeoutEnabled(flag)` with `connect(ip, port)`.
* `connectNoWait(host, port)`: Similar to `connect(host, port)`, but it doesn't
  wait for a connection. Note that the DNS lookup will still wait. This is
  superseded by `setConnectionTimeoutEnabled(flag)` with `connect(host, port)`.
* `connecting()`: Returns whether the client is in the middle of connecting.
  This is used when doing a non-blocking connect.
* `connectionId()`: Returns an ID for the connection to which the client refers.
  It will return non-zero if connected and zero if not connected. Note that it's
  possible for new connections to reuse previously-used IDs.
* `connectionTimeout()`: Returns the current timeout value.
* `isConnectionTimeoutEnabled()`: Returns whether connection timeout is enabled.
* `localIP()`: Returns the local IP of the network interface used for the
  client. Currently, This returns the same value as `Ethernet.localIP()`.
* `setConnectionTimeout(timeout)`: The parameter is a `uint32_t` and not a
  `uint16_t`. The spec, as of this writing, specifies a `uint16_t` parameter.
* `setConnectionTimeoutEnabled(flag)`: Enables or disables use of a connection
  timeout. If disabled, then calls to `connect(...)` and `stop()` won't block.
  This supersedes the `connectNoWait(...)` and `close()` calls.
* `status()`: Returns the current TCP connection state. This returns one of
  lwIP's `tcp_state` enum values. To use with _altcp_, define the
  `LWIP_DEBUG` macro.
* `writeFully(b)`: Writes a single byte.
* `writeFully(s)`: Writes a string (`const char *`).
* `writeFully(buf, size)`: Writes a data buffer (`const void *`).
* `static constexpr int maxSockets()`: Returns the maximum number of
  TCP connections.

#### TCP socket options

 * `setNoDelay(flag)`: Sets or clears the TCP_NODELAY flag in order to disable
   or enable Nagle's algorithm, respectively. This must be changed for each
   new connection. Returns `true` if connected and the option was set, and
   `false` otherwise.
 * `isNoDelay()`: Returns whether the TCP_NODELAY flag is set for the current
   connection. Returns `false` if not connected.

#### IP header values

* `outgoingDiffServ()`: Returns the current value of the differentiated services
  (DiffServ) field from the outgoing IP header, or zero if not connected.
* `setOutgoingDiffServ(ds)`: Sets the differentiated services (DiffServ) field
  in the outgoing IP header, if connected. Returns `true` if connected and the
  option was set, and `false` otherwise.
* `outgoingTTL()`: Returns the current value of the TTL field from the outgoing
  IP header, or zero if not connected.
* `setOutgoingTTL(ttl)`: Sets the TTL field in the outgoing IP header, if
  connected. Returns `true` if connected and the option was set, and
  `false` otherwise.

### `EthernetServer`

* `begin(port)`: Starts the server on the given port, first disconnecting any
  existing server if it was listening on a different port. This returns whether
  the server was successfully started.
* `beginWithReuse()`: Similar to `begin()`, but also sets the SO_REUSEADDR
  socket option. This returns whether the server was successfully started.
* `beginWithReuse(port)`: Similar to `begin(port)`, but also sets the
  SO_REUSEADDR socket option. This returns whether the server was
  successfully started.
* `end()`: Shuts down the server.
* `port()`: Returns the server's port, a signed 32-bit value, where -1 means the
  port is not set and a non-negative value is a 16-bit quantity.
* `static constexpr int maxListeners()`: Returns the maximum number of
  TCP listeners.
* `EthernetServer()`: Creates a placeholder server without a port. This form is
  useful when you don't know the port in advance.

All the `begin` functions call `end()` first only if the server is currently
listening and the port or _reuse_ options have changed.

### `EthernetUDP`

* `beginWithReuse(localPort)`: Similar to `begin(localPort)`, but also sets the
  SO_REUSEADDR socket option.
* `beginMulticastWithReuse(ip, localPort)`: Similar to
  `beginMulticast(ip, localPort)`, but also sets the SO_REUSEADDR socket option.
* `data()`: Returns a pointer to the received packet data.
* `droppedReceiveCount()`: Returns the total number of dropped received packets
  since reception was started. Note that this is the count of dropped packets at
  the layer above the driver.
* `localPort()`: Returns the port to which the socket is bound, or zero if it is
  not bound.
* `receiveQueueCapacity()`: Returns the receive queue capacity.
* `receiveQueueSize()`: Returns the number of packets currently in the
  receive queue.
* `receivedTimestamp()`: Returns the approximate packet arrival time, measured
  with `millis()`. This is useful in the case where packets have been queued and
  the caller needs the approximate arrival time. Packets are timestamped when
  the UDP receive callback is called.
* `send(host, port, data, len)`: Sends a packet without having to use
  `beginPacket()`, `write()`, and `endPacket()`. It causes less overhead. The
  host can be either an IP address or a hostname.
* `setReceiveQueueCapacity(capacity)`: Changes the receive queue capacity. The
  minimum possible value is 1 and the default is 1. If a value of zero is used,
  it will default to 1. If the new capacity is smaller than the number of items
  in the queue then all the oldest packets will get dropped.
* `size()`: Returns the total size of the received packet data.
* `totalReceiveCount()`: Returns the total number of received packets, including
  dropped packets, since reception was started. Note that this is the count at
  the layer above the driver.
* `operator bool()`: Tests if the socket is listening.
* `static constexpr int maxSockets()`: Returns the maximum number of
  UDP sockets.
* `EthernetUDP(capacity)`: Creates a new UDP socket having the specified receive
  packet queue capacity. The minimum possible value is 1 and the default is 1.
  If a value of zero is used, it will default to 1.

All the `begin` functions call `stop()` first only if the socket is currently
listening and the local port or _reuse_ options have changed.

#### IP header values

* `outgoingDiffServ()`: Returns the current value of the differentiated services
  (DiffServ) field from the outgoing IP header, or zero if the object hasn't yet
  been set up.
* `receivedDiffServ()`: Returns the DiffServ value of the last received packet.
* `setOutgoingDiffServ(ds)`: Sets the differentiated services (DiffServ) field
  in the outgoing IP header, setting up any necessary internal state. Returns
  whether successful.
* `outgoingTTL()`: Returns the current value of the TTL field from the outgoing
  IP header, or zero if the object hasn't yet been set up.
* `receivedTTL()`: Returns the TTL value of the last received packet.
* `setOutgoingTTL(ttl)`: Sets the TTL field in the outgoing IP header, setting
  up any necessary internal state. Returns whether successful.

#### `parsePacket()` return values

The `EthernetUDP::parsePacket()` function in _QNEthernet_ is able to detect
zero-length UDP packets. This means that a zero return value indicates a
valid packet.

Many Arduino examples do the following to test whether there's packet data:

```c++
int packetSize = udp.parsePacket();
if (packetSize) {  // <-- THIS IS NOT CORRECT
  // ...do something...
}
```

This is not correct because negative values mean that there's no packet
available, and negative values are implicitly converted to a Boolean _true_.
Instead, the code should look like this:

```c++
int packetSize = udp.parsePacket();
if (packetSize >= non_negative_value) {  // non_negative_value >= 0
  // ...do something...
}
```

Note that `if (packetSize > 0)` would also be correct, or even something like
`if (packetSize >= 4)`, just as long as the `if (packetSize)` form is not used.

### `EthernetFrame`

The `EthernetFrame` object adds the ability to send and receive raw Ethernet
frames. It provides an API that is similar in feel to `EthernetUDP`. Because,
like `EthernetUDP`, it derives from `Stream`, the `Stream` API can be used to
read from a frame and the `Print` API can be used to write to the frame.

* `beginFrame()`: Starts a new frame. New data can be added using the
  `Print` API. This is similar to `EthernetUDP::beginPacket()`.
* `beginFrame(dstAddr, srcAddr, typeOrLen)`: Starts a new frame and writes the
  given addresses and EtherType/length.
* `beginVLANFrame(dstAddr, srcAddr, vlanInfo, typeOrLen)`: Starts a new
  VLAN-tagged frame and writes the given addresses, VLAN info, and
  EtherType/length.
* `clear()`: Clears the outgoing and incoming buffers.
* `data()`: Returns a pointer to the frame data.
* `destinationMAC()`: Returns a pointer to the destination MAC.
* `droppedReceiveCount()`: Returns the total number of dropped received frames
  since reception was started. Note that this is the count of dropped frames at
  the layer above the driver.
* `endFrame()`: Sends the frame. This returns whether the send was successful. A
  frame must have been started, its data length must be in the range 14-1514 for
  non-VLAN frames or 18-1518 for VLAN frames, and Ethernet must have been
  initialized. This is similar to `EthernetUDP::endPacket()`.
* `etherTypeOrLength()`: Returns the EtherType/length value immediately
  following the source MAC. Note that VLAN frames are handled specially.
* `parseFrame()`: Checks if a new frame is available. This is similar
  to `EthernetUDP::parseFrame()`.
* `payload()`: Returns a pointer to the payload immediately following the
  EtherType/length field. Note that VLAN frames are handled specially.
* `receiveQueueCapacity()`: Returns the receive queue capacity.
* `receiveQueueSize()`: Returns the number of frames currently in the
  receive queue.
* `receivedTimestamp()`: Returns the approximate frame arrival time, measured
  with `millis()`. This is useful in the case where frames have been queued and
  the caller needs the approximate arrival time. Frames are timestamped when
  the unknown ethernet protocol receive callback is called.
* `send(frame, len)`: Sends a raw Ethernet frame without the overhead of
  `beginFrame()`/`write()`/`endFrame()`. See the description of `endFrame()` for
  size limits. This is similar to `EthernetUDP::send(data, len)`.
* `setReceiveQueueCapacity(capacity)`: Sets the receive queue capacity. The
  minimum possible value is 1 and the default is 1. If a value of zero is used,
  it will default to 1. If the new capacity is smaller than the number of items
  in the queue then all the oldest frames will get dropped.
* `size()`: Returns the total size of the frame data.
* `sourceMAC()`: Returns a pointer to the source MAC.
* `totalReceiveCount()`: Returns the total number of received frames, including
  dropped frames, since reception was started. Note that this is the count at
  the layer above the driver.
* `static constexpr int maxFrameLen()`: Returns the maximum frame length
  including the 4-byte FCS. Subtract 4 to get the maximum length that can be
  sent or received using this API. Note that this size includes VLAN frames,
  which are 4 bytes larger.
* `static constexpr int minFrameLen()`: Returns the minimum frame length
  including the 4-byte FCS. Subtract 4 to get the minimum length that can be
  sent or received using this API. Note that padding does not need to be managed
  by the caller, meaning frames smaller than this size are allowed; the system
  will insert padding as needed.

### `MDNS`

The `MDNS` object provides an mDNS API.

* `begin(hostname)`: Starts the mDNS responder and uses the given hostname as
  the name. If the responder is already running and the hostname is different
  then the current state is renamed.
* `end()`: Stops the mDNS responder.
* `addService(type, protocol, port)`: Adds a service. The protocol will be set
  to `"_udp"` for anything other than `"_tcp"`. The strings should have a `"_"`
  prefix. Uses the hostname as the service name.
* `addService(type, protocol, port, getTXTFunc)`: Adds a service and associated
  TXT records.
* `hostname()`: Returns the hostname if the responder is running and an empty
  string otherwise.
* `removeService(type, protocol, port)`: Removes a service.
* `restart()`: Restarts the responder, for use when the cable has been
  disconnected for a while and then reconnected. This isn't normally needed
  because the responder already watches for link reconnect.
* `operator bool()`: Tests if the mDNS responder is operating.
* `static constexpr int maxServices()`: Returns the maximum number of
  supported services.

### `DNSClient`

The `DNSClient` class provides an interface to the DNS client.

* `setServer(index, ip)`: Sets a DNS server address.
* `getServer(index)`: Gets a DNS server address.
* `getHostByName(hostname, callback, timeout)`: Looks up a host by name and
  calls the callback when there's a result. The callback is not called once the
  timeout has been reached. The timeout is ignored if it's set to zero.
* `getHostByName(hostname, ip, timeout)`: Looks up a host by name.
* `static constexpr int maxServers()`: Returns the maximum number of
  DNS servers.

### Print utilities

The `qnethernet/util/PrintUtils.h` file declares some useful output functions
and classes. Note that this file is included when `QNEthernet.h` is included;
there's no need to include it separately.

The functions and classes are in the `qindesign::network::util` namespace, so if
you've already added `using namespace qindesign::network;` to your code, they
can be called with `util::writeMagic()` syntax. Otherwise, they need to be fully
qualified: `qindesign::network::util::writeMagic()`.

Functions:

1. `writeFully(Print&, buf, size, breakf = nullptr)`: Attempts to completely
   write bytes to the given `Print` object; the optional `breakf` function is
   used as the stopping condition. It returns the number of bytes actually
   written. The return value will be less than the requested size only if
   `breakf` returned `true` before all bytes were written. Note that a NULL
   `breakf` function is assumed to return `false`.

   For example, the `EthernetClient::writeFully(...)` functions use this and
   pass the "am I disconnected" condition as the `breakf` function.

2. `writeMagic(Print&, mac, breakf = nullptr)`: Writes the payload for a
   [Magic packet](https://en.wikipedia.org/wiki/Wake-on-LAN#Magic_packet) to
   the given `Print` object. This uses `writeFully(...)` under the covers and
   passes along the `breakf` function as the stopping condition.

Classes:

1. `NullPrint`: A `Print` object that sends all data nowhere.

2. `PrintDecorator`: A `Print` decorator meant to be used as a base class.

3. `StdioPrint`: A `Print` decorator for `stdio` output files. It provides a
   `Print` interface so that it is easy to print `Printable` objects to `stdout`
   or `stderr` without having to worry about buffering and the need to flush any
   output before printing a `Printable` directly to, say, `Serial`.

### `operator bool()` and `explicit`

All the `operator bool()` functions in the API are marked as `explicit`. This
means that you might get compiler errors in some circumstances when trying to
use a Boolean-convertible object.

You can use the object as a Boolean expression. For example in an `if` statement
or ternary conditional.

You can't return the object as a `bool` from a function. For example, the
following code should give a compiler error:

```c++
EthernetClient client_;

bool isConnected() {
  return client_;
}
```

Instead, use the following code; it fixes the problem:

```c++
EthernetClient client_;

bool isConnected() {
  return client_ ? true : false;
  // Or this:
  // if (client_) {
  //   return true;
  // } else {
  //   return false;
  // }
}
```

This will also work:

```c++
bool isConnected() {
  return static_cast<bool>(client_);
}
```

See also:
1. [The safe bool problem](https://en.cppreference.com/w/cpp/language/implicit_conversion#The_safe_bool_problem)
2. [`explicit` specifier](https://en.cppreference.com/w/cpp/language/explicit)

### Use of `errno`

When a function call fails, it is often the case that `errno` will be set to
something appropriate. See the function documentation of interest in the
relevant header for more information.

## How to run

This library works with both PlatformIO and Arduino. To use it with Arduino,
here are a few steps to follow:

1. Add `#include <QNEthernet.h>`. Note that this include already includes the
   header for `EthernetUDP`. Some external examples also include `SPI.h`. This
   is not needed unless you're actually using SPI in your program.
2. Below that, add: `using namespace qindesign::network;`
3. You likely don't want or need to set/choose your own MAC address, so just
   call `Ethernet.begin()` with no arguments to use DHCP, and the three- or
   four-argument version (IP, subnet mask, gateway[, DNS]) to set your own
   address. If you really want to set your own MAC address, see
   `setMACAddress(mac)` or one of the `begin(...)` functions that takes a MAC
   address parameter.
4. There is an `Ethernet.waitForLocalIP(timeout)` convenience function that can
   be used to wait for DHCP to supply an address because `Ethernet.begin()`
   doesn't wait. Try 15 seconds (15,000 ms) and see if that works for you.

   Alternatively, you can use [listeners](#how-to-use-listeners) to watch for
   address, link, and network interface activity changes. This obviates the need
   for waiting and is the preferred approach.

5. `Ethernet.hardwareStatus()` can detect both the Ethernet and no-Ethernet
   versions of the hardware.
6. Most other things should be the same.

Please see the examples for more things you can do with the API, including but
not limited to:
* Using listeners to watch for network changes,
* Monitoring and sending raw Ethernet frames, and
* Setting up an mDNS service.

### How to move the stack forward and receive data

All reception is processed in `Ethernet.loop()`. There's no thread or ISR that
regularly processes the input. This means that this function must be called
regularly. For example, it could be called at the end of the main `loop()`
function. Another good place is to hook into `yield()` because the Arduino
framework calls that every time `loop()` finishes and likely during a call
to `delay()`.

On the Teensy platform, the call is already hooked into `yield()` via the
built-in `EventResponder` approach, so there's nothing more to add.

### Concurrent use is not supported

Concurrent use of _QNEthernet_ is not currently supported. This includes ISR
approaches and multi-threading approaches.

First, the underlying lwIP stack must be configured and used a certain way in
order to provide concurrent support. _QNEthernet_ does not configure lwIP for
this. Second, the _QNEthernet_ API, the layer on top of lwIP, isn't designed for
concurrent use.

### Link detection

Normally, a link is detected by the driver at some polling rate. (For the
curious, see `driver_poll()` and its uses.) However, it's possible for a driver
to not be able to detect a link. For example, the W5100 chip is unable to read
this state.

Since the underlying lwIP stack depends on the link being up in order to operate
properly, a project will need to manage the link state itself for those drivers.
The suggestion is this:
1. Start Ethernet as you normally would.
2. If successful, check `Ethernet.driverCapabilities().hasLinkState`.
3. If `false`, then call `Ethernet.setLinkState(true)`.

Code example:

```c++
if (ethernet_is_started && !Ethernet.driverCapabilities().hasLinkState) {
  Ethernet.setLinkState(true);
}
```

### Notes on `yield()`

It may be the case that the `yield()` function is overridden, say to implement
cooperative multitasking. If that implementation doesn't call `Ethernet.loop()`
then there are two things to be aware of:
1. `Ethernet.loop()` needs to be called regularly somewhere. One good place is
   at the end of the main program loop.
2. Any library functions that use `yield()` while waiting for an event, say in
  `Ethernet.waitForLocalIP()` or `EthernetClient::connect()`, need to call
  `Ethernet.loop()` during the wait, otherwise the stack won't move forward and
  the event will never occur. A good place to do this is after the
  `yield()` call.

To accomplish #2, there is a configuration macro,
`QNETHERNET_DO_LOOP_IN_YIELD`, that enables a call to `Ethernet.loop()` after
each `yield()` call, guaranteeing that the stack moves forward. See the
[Configuration macros](#configuration-macros) section.

The complete list of functions, as of this writing, that use `yield()` is:
1. `DNSClient::getHostByName()`
2. `EthernetClass::waitForLink()`
3. `EthernetClass::waitForLocalIP()`
4. `EthernetClient::connect()`
5. `EthernetClient::stop()`

## How to write data to connections

I'll start with these statements:
1. **Don't use the <code>print<em>X</em>(...)</code> functions when writing data
   to connections.**
2. **Always check the `write(...)` and <code>print<em>X</em>(...)</code> return
   values, retrying if necessary.**
3. Data isn't necessarily sent immediately.

The `write(...)` and <code>print<em>X</em>(...)</code> functions in the `Print`
API all return the number of bytes actually written. This means that you _must
always check the return value, retrying any missing bytes_ if you want all your
data to get sent.

For example, the following code won't necessarily send all 250&times;102 bytes.
Buffers might get full. There might be retries. Etcetera.

```c++
void sendTestData(EthernetClient& client) {
  for (int i = 0; i < 250; i++) {
    // 102-byte string (println appends CRLF)
    client.println("1234567890"
                   "1234567890"
                   "1234567890"
                   "1234567890"
                   "1234567890"
                   "1234567890"
                   "1234567890"
                   "1234567890"
                   "1234567890"
                   "1234567890");
  }
}
```

The following modification will print a message every time the number of bytes
actually written does not match the number of bytes sent to the function. You
might find that the message prints one or more times.

```c++
void sendTestData(EthernetClient& client) {
  for (int i = 0; i < 250; i++) {
    // 102-byte string (println appends CRLF)
    size_t written = client.println("1234567890"
                                    "1234567890"
                                    "1234567890"
                                    "1234567890"
                                    "1234567890"
                                    "1234567890"
                                    "1234567890"
                                    "1234567890"
                                    "1234567890"
                                    "1234567890");
    if (written != 102) {
      // This is not an error!
      Serial.println("Didn't write fully");
    }
  }
}
```

The solution is to utilize the raw `write(...)` functions and retry any bytes
that aren't sent. Let's create a `writeFully(...)` function and use that to send
the data:

```c++
// Keep writing until all the bytes are sent or the connection
// is closed.
void writeFully(EthernetClient& client, const char* data, int len) {
  // Don't use client.connected() as the "connected" check because
  // that will return true if there's data available, and this loop
  // does not check for data available or remove it if it's there.
  while (len > 0 && client) {
    size_t written = client.write(data, len);
    len -= written;
    data += written;
  }
}

void sendTestData(EthernetClient& client) {
  for (int i = 0; i < 250; i++) {
    // 102-byte string
    size_t written = writeFully(client, "1234567890"
                                        "1234567890"
                                        "1234567890"
                                        "1234567890"
                                        "1234567890"
                                        "1234567890"
                                        "1234567890"
                                        "1234567890"
                                        "1234567890"
                                        "1234567890"
                                        "\r\n");
  }
}
```

_Note that the library implements `writeFully(...)`; you don't have to roll
your own._

Rewriting this to use the library function:

```c++
void sendTestData(EthernetClient& client) {
  for (int i = 0; i < 250; i++) {
    // 102-byte string
    size_t written = client.writeFully("1234567890"
                                       "1234567890"
                                       "1234567890"
                                       "1234567890"
                                       "1234567890"
                                       "1234567890"
                                       "1234567890"
                                       "1234567890"
                                       "1234567890"
                                       "1234567890"
                                       "\r\n");
  }
}
```

Let's go back to our original statement about not using the
<code>print<em>X</em>(...)</code> functions. Their implementation is opaque and
they sometimes make assumptions that the data will be "written fully". For
example, Teensyduino's current `print(const String&)` implementation attempts
to send all the bytes and returns the number of bytes sent, but it doesn't tell
you _which_ bytes were sent. For the string `"12345"`, `print(s)` might send
`"12"`, fail to send `"3"`, and successfully send `"45"`, returning the value 4.

Similarly, we have no idea what `print(const Printable& obj)` does because the
`Printable` implementation passed to it is beyond our control. For example,
Teensyduino's `IPAddress::printTo(Print&)` implementation prints the address
without checking the return value of the `print(...)` calls.

Also, most examples I've seen that use any of the
<code>print<em>X</em>(...)</code> functions never check the return values.
Common practice seems to stem from this style of usage. Network applications
work a little differently, and there's no guarantee all the data gets sent.

The `write(...)` functions don't have this problem (unless, of course, there's a
faulty implementation). They attempt to send bytes and return the number of
bytes actually sent.

In summary, my ***strong*** suggestion is to use the `write(...)` functions when
sending network data, checking the return values and acting on them. Or you can
use the library's `writeFully(...)` functions.

See the discussion at:
https://forum.pjrc.com/index.php?threads/nativeethernet-stalling-with-dropped-packets.68389/

### `writeFully()` with more break conditions

By default, the `EthernetClient` versions of `writeFully()` wait until the
connection is closed. However, TCP connections can sometimes persist for a long
time after a cable/link disconnect, and a user might wish to only wait so long
for a connection to return. This can be solved by adding additional checks to
the stopping condition function (the `breakf` parameter).

For example, to break on connection close or link down:

```c++
size_t writeFully(EthernetClient& c, const uint8_t* buf, size_t size) {
  return qindesign::network::util::writeFully(c, buf, size, [&c]() {
    return !static_cast<bool>(c) || !Ethernet.linkState();
  });
}
```

To break on connection close or timeout:

```c++
size_t writeFully(EthernetClient& c, const uint8_t* buf, size_t size,
                  uint32_t timeout) {
  uint32_t startT = millis();
  return qindesign::network::util::writeFully(
      c, buf, size, [&c, startT, timeout]() {
        return !static_cast<bool>(c) || (millis() - startT) >= timeout;
      });
}
```

See also:
* [writeFully causes program to freeze · Issue #46 · ssilverman/QNEthernet](https://github.com/ssilverman/QNEthernet/issues/46)
* [On connections that hang around after cable disconnect](#on-connections-that-hang-around-after-cable-disconnect)
* [Print utilities](#print-utilities)

### Write immediacy

Data isn't necessarily completely sent across the wire after `write(...)` or
`writeFully(...)` calls. Instead, data is merely enqueued until the internal
buffer is full or a timer expires. Now, if the data to send is larger than the
internal TCP buffer then data will be sent and the extra data will be enqueued.
In other words, data is only sent when either the buffer is full or an internal
timer has expired.

To send any buffered data, call `flush()`.

To quote lwIP's `tcp_write()` docs:
> Write data for sending (but does not send it immediately).
>
> It waits in the expectation of more data being sent soon (as
> it can send them more efficiently by combining them together).
> To prompt the system to send data now, call tcp_output() after
> calling tcp_write().

`flush()` is what always calls `tcp_output()` internally. The `write(...)` and
`writeFully(...)` functions only call this when the buffer is full. The
suggestion is to call `flush()` when done sending a "packet" of data, for some
definition of "packet" specific to your application. For example, after sending
a web page to a client or after a chunk of data is ready for the server
to process.

There is a configuration option, `QNETHERNET_FLUSH_AFTER_WRITE`, that causes an
automatic flush after data is written. However, this may reduce TCP efficiency.
This option is for use with hard-to-modify code or libraries that assume data
will get sent immediately. The preferred approach is to call flush() in the code
or library.

## A note on the examples

The examples aren't meant to be simple. They're meant to be functional. There
are plenty of Arduino-style Ethernet library examples out there. This library
does not aim to be just like the others, it aims to provide some more powerful
tools to enable more powerful programs. The Teensy 4.1 is a serious device; it
demands serious examples.

An attempt was made to use a more robust programming style and modern C++ tools.
For those not as experienced with C++ or larger projects, these may have a
steeper learning curve. Hopefully this brings up lots of questions and an
exposure to new concepts. For example, state machines, lambdas, callbacks,
data structures, vector::emplace_back, and std::move.

For those that do have more C++ and larger project experience, I invite you to
improve or add to the examples so that the set of examples here becomes what
you've always hoped great library examples could be.

## A survey of how connections (aka `EthernetClient`) work

Hopefully this section disambiguates some details about what each function does:
1. `connected()`: Returns whether connected OR data is still available
   (or both).
2. `operator bool()`: Returns whether connected (at least in _QNEthernet_).
3. `available()`: Returns the amount of data available, whether the connection
   is closed or not.
4. `read(...)`: Reads data if there's data available, whether the connection is
   closed or not.

Connections will be closed automatically if the client shuts down a connection,
and _QNEthernet_ will properly handle the state such that the API behaves as
expected. In addition, if a client closes a connection, any buffered data will
still be available via the client API. If it were up to me, I'd have swapped the
meaning of `operator bool()` and `connected()`, but see the above list as
a guide.

Some options:

1. Keep checking `connected()` (or `operator bool()`) and
   `available()`/`read(...)` to keep reading data. The data will run out when
   the connection is closed and after all the buffers are empty. The calls to
   `connected()` (or `operator bool()`) will indicate connection status (plus
   data available in the case of `connected()` or just connection state in the
   case of `operator bool()`).
2. Same as the above, but without one of the two connection-status calls
   (`connected()` or `operator bool()`). The data will just run out after
   connection-closed and after the buffers are empty.

### Connections and link/interface detection

A link and active network interface must be present before a connection can be
made. Either call `Ethernet.waitForLink(timeout)` or check the link state or
network interface status before attempting to connect. Which approach you use
will depend on how your code is structured or intended to be used.

Be aware when using a listener approach to start or stop services that it's
possible, when setting a static IP, for the _address-changed_ callback to be
called before the link or network interface is up.

Note that this section also applies to the DNS client.

### `connect()` behaviour and its return values

Firstly, `connect()` blocks. See the [next section](#connectnowait-doesnt-wait)
for a non-blocking way to connect.

The Arduino-style API,
[here](https://docs.arduino.cc/libraries/ethernet/#Client%20Class
(client.connect())), used to define a set of possible `int` return values for
this function, but now it returns a Boolean value indicating success. Note that
the function signatures still return an `int`.

### Non-blocking connection functions

The `connectNoWait()` functions implement non-blocking TCP connections. These
functions behave similarly to `connect()`, however they do not wait for the
connection to be established.

To check for connection establishment, simply call `connecting()` to determine
if the client is still in the process of connecting, and then either
`connected()` or the Boolean operator. If a connection can't be established then
`close()` must be called on the object.

Note that DNS lookups for hostnames will still wait.

### Getting the TCP state

TCP connections can be in one of
[several states](https://www.rfc-editor.org/rfc/rfc9293#name-state-machine-overview).
Knowing a connection's underlying TCP state can be useful, for example, to avoid
waiting for a timeout via reading data.

This state can be retrieved using an `EthernetClient`'s `status()` function.
Note that, to avoid modiyfing the lwIP code too much, the `LWIP_DEBUG` macro
must be defined when using this function with _altcp_. If not using it with
_altcp_, there's no such concern.

The states, as defined by lwIP's `tcp_state` enum:
1. CLOSED
2. LISTEN
3. SYN_SENT
4. SYN_RCVD
5. ESTABLISHED
6. FIN_WAIT_1
7. FIN_WAIT_2
8. CLOSE_WAIT
9. CLOSING
10. LAST_ACK
11. TIME_WAIT

This enum isn't a C++ "enum class", so its values can be used directly. The
definition is already included by _qnethernet/QNEthernetClient.h_, which is, in
turn, included by _QNEthernet.h_.

References:
1. [TCP states](https://www.rfc-editor.org/rfc/rfc9293#name-state-machine-overview)
2. [consider adding `status()` in EthernetClient · Issue #52 · ssilverman/QNEthernet](https://github.com/ssilverman/QNEthernet/issues/52#issuecomment-1737950354)
3. [WiFiNINA - client.status() - Arduino Reference](https://docs.arduino.cc/libraries/wifinina/#Client%20Class)
   (client.status())

## How to use multicast

There are a few ways in the API to utilize multicast to send or receive packets.
For reception, you must join a multicast group.

The first is by using the `EthernetUDP::beginMulticast(ip, port)` function. This
both binds to a specific port, for only receiving traffic on that port, and
joins the specified group address. It's similar to
`EthernetUDP::begin(localPort)` in that the socket will "own" the port.

Since only one socket at a time can be bound to a specific port, you will need
to use the same socket if you want to receive traffic sent to multiple groups on
the same port. You can accomplish this by either calling
`beginMulticast(ip, port)` multiple times, or by using `begin(localPort)` once,
and then calling `Ethernet.joinGroup(ip)` for each group you want to join.

To send multicast traffic, simply send to the appropriate IP address. There's no
need to join a group.

The lwIP stack keeps track of a group "use count". This means:
1. That `joinGroup(ip)` can be called multiple times, it just needs to be paired
   with a matching number of calls to `leaveGroup(ip)`. Each call increments an
   internal count.
2. Each call to `leaveGroup(ip)` decrements a count, and when that count reaches
   zero, the stack actually leaves the group.

## How to use listeners

Instead of waiting for certain states at system start, for example _link-up_ or
_address-changed_, it's possible to watch for state changes using listeners, and
then act on those state changes. This will make your application more robust and
responsive to state changes during program operation.

Note that callbacks should be registered before any other Ethernet functions are
called. This ensures that all events are captured. This includes
`Ethernet.begin(...)`.

The relevant functions are (see the [`Ethernet`](#ethernet) section for further
descriptions):
1. `Ethernet.onLinkState(cb)`
2. `Ethernet.onAddressChanged(cb)`
3. `Ethernet.onInterfaceStatus(cb)`

_Link-state_ events occur when an Ethernet link is detected or lost.

_Address-changed_ events occur when the IP address changes, but its effects are
a little more subtle. When setting an address via DHCP, the link and network
interface must already be up in order to receive the information. However, when
setting a static IP address, the event may occur before the link or network
interface is up. This means that if a connection or DNS lookup is attempted when
it is detected that the address is valid, the attempt will fail.

_Interface-status_ events happen when the network interface comes up or goes
down. No network operations can happen before the network interface is up. For
example, when setting a static IP address, the _address-changed_ event may occur
before the network interface has come up. This means that, for example, any
connection attempts or DNS lookup attempts will fail.

It is suggested, therefore, that when taking an action based on an
_address-changed_ event, the link state and network interface status are checked
in addition.

Servers, on the other hand, can be brought up even when there's no link or
active network interface.

In summary, no network operations can be done before all three of _link-up_,
_address_changed-to-valid_, and _interface-up_ occur.

Last, no network tasks should be performed inside the listeners. Instead, set a
flag and then process that flag somewhere in the main loop.

See: [Connections and link/interface detection](#connections-and-linkinterface-detection)

## How to change the number of sockets

lwIP preallocates almost everything. This means that the number of sockets (UDP,
TCP, etc.) the stack can handle is set at compile time. The following table
shows how to change the number for each socket type.

| Socket Type     | Macro in `lwipopts.h`     | Notes                 |
| --------------- | ------------------------- | --------------------- |
| UDP             | `MEMP_NUM_UDP_PCB`        |                       |
| TCP             | `MEMP_NUM_TCP_PCB`        | Simultaneously active |
| TCP (listening) | `MEMP_NUM_TCP_PCB_LISTEN` | Listening             |

## UDP receive buffering

If UDP packets come in at a faster rate than they are consumed, some may get
dropped. To help mitigate this, the `EthernetUDP(capacity)` constructor can be
called with a value > 1. The minimum capacity is 1, meaning any new packets will
cause any existing packet to get dropped. If it's set to 2 then there will be
space for one additional packet for a total of 2 packets, and so on. Setting a
value of zero will use the default of 1.

## mDNS services

It's possible to register mDNS services. Some notes:
* Similar to `Ethernet`, there is a global `MDNS` object. It too is in the
  `qindesign::network` namespace.
* It's possible to add TXT items when adding a service. For example, the
  following code adds "path=/" to the TXT of an HTTP service:
  ```c++
    MDNS.begin("Device Name");
    MDNS.addService("_http", "_tcp", 80, []() {
      return std::vector<String>{"path=/"};
    });
  ```
  You can add more than one item to the TXT record by adding to the vector.
* When adding a service, the function that returns TXT items defaults to NULL,
  so it's not necessary to specify that parameter. For example:
  ```c++
    MDNS.begin("Device Name");
    MDNS.addService("_http", "_tcp", 80);
  ```
* The host name is normally used as the service name, but there are also
  functions that let you specify the service name. For example:
  ```c++
  MDNS.begin("Host Name");
  MDNS.addService("my-http-service", "_http", "_tcp", 80);
  ```

## DNS

The library interfaces with DNS using the `DNSClient` class. Note that all the
functions are static.

Things you can do:
1. Look up an IP address by name, and
2. Set multiple DNS servers.

The `Ethernet.setDNSServerIP(ip)` function sets the zeroth DNS server address
and the `Ethernet.setDNSServerIP(index, ip)` function sets the nth DNS server
address. Corresponding `dnsServerIP()` functions get the DNS server addresses.

## stdio

Internally, lwIP uses `printf` for debug output and assertions. _QNEthernet_
defines its own `_write()` function so that it has more control over `stdout`
and `stderr` than that provided by the internal output support.

Note: As of Teensyduino 1.58-beta4, there's internal support for `printf`,
partially obviating the need for _QNEthernet_ to define its own. However, it
always maps all of `stdin`, `stdout`, and `stderr` specifically to `Serial`.

Compared to the internal `printf` support, the _QNEthernet_ version:
1. Can map output to any `Print` interface, not just `Serial`,
2. Can separate `stdout` and `stderr` outputs, and
3. Disallows `stdin` as a valid output.

To enable the _QNEthernet_ version, set the `QNETHERNET_CUSTOM_WRITE` macro to
`1` and set the `stdoutPrint` or `stderrPrint` variables to point to a valid
`Print` implementation. Note that if the feature is disabled, then neither
`stdoutPrint` nor `stderrPrint` will be defined.

Both variables default to NULL.

For example:
```c++
// Define QNETHERNET_CUSTOM_WRITE somewhere

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  qindesign::network::stdoutPrint = &Serial;
}
```

If your application wants to define its own `_write()` implementation or to use
the system default, then leave the `QNETHERNET_CUSTOM_WRITE` macro undefined.

### Adapt stdio files to the Print interface

There is a utility class for decorating stdio `FILE*` objects with the `Print`
interface. See the `StdioPrint` class in _src/qnethernet/util/PrintUtils.h_.

This is useful when:
1. A `FILE*` object does its own buffering and you also need to write to the
   underlying `Print` object directly, `Serial` for example. It avoids having to
   remember to call `fflush()` on the `FILE*` before writing to the underlying
   `Print` object.
2. There's a need to easily print `Printable` objects to the `FILE*`.

## Raw Ethernet frames

There is support for sending and receiving raw Ethernet frames. See the
[`EthernetFrame`](#ethernetframe) API, above.

This API doesn't receive any known Ethernet frame types. These include:
1. IPv4 (0x0800)
2. ARP  (0x0806)
3. IPv6 (0x86DD) (if enabled)

If frames are addressed to a MAC address that doesn't belong to the device and
isn't a multicast group MAC address then it is necessary to tell the Ethernet
stack about it. See the `Ethernet.setMACAddressAllowed(mac, flag)` function.

An example that uses such MAC addresses is the Precision Time Protocol (PTP)
over Ethernet. It uses 01-1B-19-00-00-00 for forwardable frames and
01-80-C2-00-00-0E for non-forwardable frames. See
[PTP Message Transport](https://en.wikipedia.org/wiki/Precision_Time_Protocol#Message_transport)

To enable raw frame support, set the `QNETHERNET_ENABLE_RAW_FRAME_SUPPORT` macro
to `1`. This will use some space.

### Promiscuous mode

It's possible to enable promiscuous mode so that all frames are received, even
ones whose destination MAC address would normally be filtered out by the
Ethernet hardware. To do this, set the `QNETHERNET_ENABLE_PROMISCUOUS_MODE`
macro to `1`.

### Raw frame receive buffering

Similar to [UDP buffering](#udp-receive-buffering), if raw frames come in at a
faster rate than they are consumed, some may get dropped. To help mitigate this,
the receive queue capacity can be adjusted with the
`EthernetFrame.setReceiveQueueCapacity(capacity)` function. The default queue
capacity is 1 and the minimum is also 1 (if a zero is passed in then 1 will be
used instead).

For a size of 1, any new frames will cause any existing frame to get dropped. If
the size is 2 then there will be space for one additional frame for a total of 2
frames, and so on.

### Raw frame loopback

Raw frames having a destination MAC address that matches the local MAC address
or the broadcast MAC address can optionally be looped back up the stack. To
enable this feature, set the `QNETHERNET_ENABLE_RAW_FRAME_LOOPBACK` macro
to `1`.

## How to implement VLAN tagging

The lwIP stack supports VLAN tagging. Here are the steps for how to implement
it. Note that all defines should go inside `lwipopts.h`. Documentation for these
defines can be found in _src/lwip/opt.h_.

1. Define `ETHARP_SUPPORT_VLAN` as `1`.
2. To set VLAN tags, define `LWIP_HOOK_VLAN_SET`.
3. To validate VLAN tags on input, define one of:
   1. `LWIP_HOOK_VLAN_CHECK`, (see `LWIP_HOOK_VLAN_CHECK`)
   2. `ETHARP_VLAN_CHECK_FN`, (see `ETHARP_SUPPORT_VLAN`)
   3. `ETHARP_VLAN_CHECK`. (see `ETHARP_SUPPORT_VLAN`)

## Application layered TCP: TLS, proxies, etc.

lwIP provides a way to decorate the TCP layer. It's called "Application Layered
TCP." It enables an application to add things like TLS and proxies without
changing the source code.

Here are the steps to add decorated TCP:
1. Set `LWIP_ALTCP` and optionally `LWIP_ALTCP_TLS` to `1` in `lwipopts.h`.
2. Implement two functions somewhere in your code, having these names and
   signatures:
   ```c++
   std::function<bool(const ip_addr_t* ipaddr, uint16_t port,
                      altcp_allocator_t& allocator)> qnethernet_altcp_get_allocator;
   std::function<void(const altcp_allocator_t& allocator)> qnethernet_altcp_free_allocator;
   ```
3. Implement all the functions necessary for the wrapping implementation. For
   example, for TLS, this means all the functions declared in
   _src/lwip/altcp_tls.h_.

See _src/lwip/altcp.c_ and the _AltcpTemplate_ example for more information.

Note that if the `QNETHERNET_ENABLE_ALTCP_DEFAULT_FUNCTIONS` macro is enabled,
default, simple, implementations of these functions will be provided. Only the
regular TCP allocator will be used.

### About the allocator functions

The functions from step 2 create and destroy any resources used by the altcp
wrapper. The first function fills in the allocator function (`allocator->alloc`)
and an argument appropriate to that allocator function (`allocator->arg`). For
example, the `altcp_tcp_alloc()` allocator function doesn't need an argument (it
can be set to NULL), but `altcp_tls_alloc()` needs a pointer to a
`struct altcp_tls_config`.

The connection will fail if the allocator function is set to NULL.

The second function frees any resources that haven't already been freed. It's up
to the application and TCP wrapper implementation to properly manage resources
and to provide a way to determine whether a resource needs to be freed. It is
only called if `qnethernet_altcp_get_allocator` returns `true` and a socket
could not be created.

The `ipaddr` and `port` parameters indicate what the calling code is trying
to do:

1. If `ipaddr` is NULL then the application is trying to listen.
2. If `ipaddr` is not NULL then the application is trying to connect.

### About the TLS adapter functions

The _src/altcp_tls_adapter.cpp_ file implements the allocator functions for
altcp TLS integration. It specifies new functions that make it a little easier
to integrate a library. These are as follows:

1. Type: `std::function<bool(const ip_addr_t* ip, uint16_t port)>`\
   Name: `qnethernet_altcp_is_tls`\
   Description: Determines if the connection should use TLS. The IP address will
                be NULL for a server connection.
2. Type: `std::function<void(const ip_addr_t& ipaddr, uint16_t port,
                             const uint8_t*& cert, size_t& cert_len)>`\
   Name: `qnethernet_altcp_tls_client_cert`\
   Note: All the arguments are references.\
   Description: Retrieves the certificate for a client connection. The values
                are initialized to NULL and zero, respectively, before this
                function is called. The IP address and port can be used to
                determine the certificate data, if needed.
3. Type: `std::function<uint8_t(uint16_t port)>`\
   Name: `qnethernet_altcp_tls_server_cert_count`\
   Description: Returns the certificate count for a server connection.
4. Type: `std::function<void(uint16_t port, uint8_t index,
                             const uint8_t*& privkey,      size_t& privkey_len,
                             const uint8_t*& privkey_pass, size_t& privkey_pass_len,
                             const uint8_t*& cert,         size_t& cert_len)>`\
   Name: `qnethernet_altcp_tls_server_cert`\
   Description: Retrieves the certificate and private key for a server
                connection. The values are initialized to NULL and zero before
                this function is called. It will be called for each server
                certificate, a total of N times, where N is the value returned
                by `qnethernet_altcp_tls_server_cert_count`. The `index`
                argument will be in the range zero to N-1. The port and
                certificate index can be used to determine the certificate data,
                if needed.

Currently, this file is only built if the `LWIP_ALTCP`, `LWIP_ALTCP_TLS`, and
`QNETHERNET_ALTCP_TLS_ADAPTER` macros are enabled by setting them to `1`.

### How to enable Mbed TLS

The lwIP distribution comes with a way to use the Mbed TLS library as an ALTCP
TLS layer. It currently only supports the 2.x.x versions; as of this writing,
the latest version is 2.28.9.

More detailed information are in the subsections below, but here is an outline
of how to use this feature:
1. Set the following macros to `1` in _lwipopts.h_:
   1. `LWIP_ALTCP` &mdash; Enables the ALTCP layer
   2. `LWIP_ALTCP_TLS` &mdash; Enables the TLS features of ALTCP
   3. `LWIP_ALTCP_TLS_MBEDTLS` &mdash; Enables the Mbed TLS code for ALTCP TLS
2. Set the `QNETHERNET_ALTCP_TLS_ADAPTER` macro to `1` in _qnethernet_opts.h_.
   This enables the _altcp_tls_adapter_ functions that help ease integration.
3. Install the latest Mbed TLS v2.x.x.
4. Implement the functions required by _src/altcp_tls_adapter.cpp_. This file
   implements the above allocator functions and simplifies the integration.
5. Implement an entropy function for internal Mbed TLS use.

#### Installing the Mbed TLS library

Currently, there doesn't seem to be an Arduino-friendly version of this library.
So, first download or clone a snapshot of the latest 3.6.x version (current as
of this writing is 3.6.3): http://github.com/Mbed-TLS/mbedtls

See the `v3.6.3` or `mbedtls-3.6.3` tags for the 3.6.3 version.

##### Mbed TLS library install for Arduino IDE

In your preferred "Libraries" folder, create a folder named _mbedtls_.
Underneath that, create a _src_ folder. Copy, recursively, all files and folders
from the distribution as follows:
1. _distro_/library/* -> "Libraries"/mbedtls/src
2. _distro_/include/* -> "Libraries"/mbedtls/src

The "Libraries" folder can is the same thing as "Sketchbook location" in the
application's Preferences. There should be a _libraries/_ folder inside
that location.

Next, create an empty _mbedtls.h_ file inside _"Libraries"/mbedtls/src/_.

Next, create a _library.properties_ file inside _"Libraries"/mbedtls/_:
```properties
name=Mbed TLS
version=3.6.3
sentence=Mbed TLS is a C library that implements cryptographic primitives, X.509 certificate manipulation and the SSL/TLS and DTLS protocols.
paragraph=Its small code footprint makes it suitable for embedded systems.
category=Communication
url=https://github.com/Mbed-TLS/mbedtls
includes=mbedtls.h
```
(Ref: https://arduino.github.io/arduino-cli/latest/library-specification/)

Last, modify the _mbedtls/src/mbedtls/mbedtls_config.h_ file by replacing it
with the contents of _examples/MbedTLSDemo/sample_mbedtls_config.h_. Note that
Mbed TLS uses a slightly different configuration mechanism than lwIP: it uses
macro presence rather than macro values.

For posterity, the following changes are the minimum possible set just to be
able to get the library to compile:
1. Define:
   1. `MBEDTLS_NO_PLATFORM_ENTROPY`
   2. `MBEDTLS_PLATFORM_MS_TIME_ALT`
   3. `MBEDTLS_ENTROPY_HARDWARE_ALT` &mdash; Requires `mbedtls_hardware_poll()`
      function implementation
2. Undefine:
   1. `MBEDTLS_NET_C`
   2. `MBEDTLS_TIMING_C`
   3. `MBEDTLS_FS_IO`
   4. `MBEDTLS_PSA_ITS_FILE_C`
   5. `MBEDTLS_PSA_CRYPTO_STORAGE_C`

There are also example configuration headers in Mbed TLS under _configs/_.

It's likely that, if you're using the Arduino IDE, you'll need to restart it
after installing the library.

##### Mbed TLS library install for PlatformIO

In your preferred "Libraries" folder, create a folder named _mbedtls_. Copy all
files and folders, recursively, from the Mbed TLS distribution into that folder.

The "Libraries" folder is either PlatformIO's global libraries location or the
application's local _lib/_ folder.

Next, create a _library.json_ file inside _"Libraries"/mbedtls/_:
```json
{
  "name": "Mbed TLS",
  "version": "3.6.3",
  "description": "Mbed TLS is a C library that implements cryptographic primitives, X.509 certificate manipulation and the SSL/TLS and DTLS protocols. Its small code footprint makes it suitable for embedded systems.",
  "keywords": [
    "tls",
    "networking"
  ],
  "homepage": "https://www.trustedfirmware.org/projects/mbed-tls",
  "repository": {
    "type": "git",
    "url": "https://github.com/Mbed-TLS/mbedtls.git"
  },
  "license": "Apache-2.0 OR GPL-2.0-or-later",
  "build": {
    "srcDir": "library",
    "includeDir": "include"
  }
}
```
(Ref: https://docs.platformio.org/en/latest/manifests/library-json/index.html)

Last, modify the _mbedtls/include/mbedtls/config.h_ file per the instructions in
the previous, Arduino IDE install, section.

#### Implementing the _altcp_tls_adapter_ functions

The _MbedTLSDemo_ example illustrates how to implement these.

#### Implementing the Mbed TLS entropy function

See how the _MbedTLSDemo_ example does it. Look for the
`mbedtls_hardware_poll()` function. The example uses _QNEthernet_'s internal
entropy function, `qnethernet_hal_fill_entropy()`. You can, of course, use your
own entropy source if you like.

If you add the function to a C++ file, then it must be declared `extern "C"`.

## On connections that hang around after cable disconnect

Ref: [EthernetServer accept no longer connects clients after unplugging/plugging ethernet cable ~7 times · Issue #15 · ssilverman/QNEthernet](https://github.com/ssilverman/QNEthernet/issues/15)

TCP tries its best to maintain reliable communication between two endpoints,
even when the physical link is unreliable. It uses techniques such as timeouts,
retries, and exponential backoff. For example, if a cable is disconnected and
then reconnected, there may be some packet loss during the disconnect time, so
TCP will try to resend any lost packets by retrying at successively larger
intervals.

The TCP close process uses some two-way communication to properly shut down a
connection, and therefore is also subject to physical link reliability. If the
physical link is interrupted or the other side doesn't participate in the close
process then the connection may appear to become "stuck", even when told to
close. The TCP stack won't consider the connection closed until all timeouts and
retries have elapsed.

It turns out that some systems drop and forget a connection when the physical
link is disconnected. This means that the other side may still be waiting to
continue or close the connection, timing out and retrying until all attempts
have failed. This can be as long as a half hour, or maybe more, depending on how
the stack is configured.

The above link contains a discussion where a user of this library couldn't
accept any new connections, even when all the connections had been closed, until
all the existing connections timed out after about a half hour. What happened
was this: connections were being made, the Ethernet cable was disconnected and
reconnected, and then more connections were made. When the cable was
disconnected, all connections were closed using the `close()` function. The
Teensy side still maintained connection state for all the connections, choosing
to do what TCP does: make a best effort to maintain or properly close those
connections. Once all the available sockets had been exhausted, no more
connections could be accepted.

Those connections couldn't be cleared and sockets made available until all the
TCP retries had elapsed. The main problem was that the other side simply dropped
the connections when it detected a link disconnect. If the other system had
maintained those connections, it would have continued the close processes as
normal when the Ethernet cable was reconnected. That's why tests on my system
couldn't reproduce the issue: the IP stack on the Mac maintained state across
cable disconnects/reconnects. The issue reporter was using Windows, and the IP
stack there apparently drops a connection if the link disconnects. This left the
Teensy side waiting for replies and retrying, and the Windows side no longer
sending traffic.

To mitigate this problem, there are a few possible solutions, including:
1. Reduce the number of retransmission attempts by changing the `TCP_MAXRTX`
   setting in `lwipopts.h`, or
2. Abort connections upon link disconnect.

To accomplish #2, there's an `EthernetClient::abort()` function that simply
drops a TCP connection without going though the normal TCP close process. This
could be called on connections when the link has been disconnected. (See also
`Ethernet.onLinkState(cb)` or `Ethernet.linkState()`.)

Fun links:
* [Removing Exponential Backoff from TCP](http://ccr.sigcomm.org/online/files/p19-mondal.pdf)
  * Ref: [[PDF] Removing exponential backoff from TCP](https://www.semanticscholar.org/paper/Removing-exponential-backoff-from-TCP-Mondal-Kuzmanovic/2ab0df78bb0aa95c0ed8f3dc687937ccc64f6785)
  * Older link: [Removing Exponential Backoff from TCP | acm sigcomm](http://www.sigcomm.org/node/2736)
* [Exponential backoff](https://en.wikipedia.org/wiki/Exponential_backoff)

## Notes on ordering and timing

* Link status is polled about 8 times a second.
* For static IP addresses, the _address-changed_ callback is called before
  lwIP's `netif_default` is set. `MDNS.begin()` relies on `netif_default`, so
  that function and anything else that relies on `netif_default` should be
  called after `Ethernet.begin(...)`, and not from the listener.
* The DNS lookup timeout is `DNS_MAX_RETRIES * DNS_TMR_INTERVAL`, where
  `DNS_TMR_INTERVAL` is 1000.

## Notes on RAM1 usage

By default, the Ethernet RX and TX buffers will go into RAM2. If, for whatever
reason, you'd prefer to put them into RAM1, set the
`QNETHERNET_BUFFERS_IN_RAM1` macro to `1`. _[As of this writing, no speed
comparison tests have been done.]_

There's a second configuration macro, `QNETHERNET_LWIP_MEMORY_IN_RAM1`, for
indicating that lwIP-declared memory should go into RAM1 instead of RAM2.

These options are useful in the case where a program needs more dynamic memory,
say. Putting more things in RAM1 will free up more space for things like `new`
and STL allocation.

## Heap memory use

The library is configured, by default, to use the system-defined malloc
functions. These include _malloc_, _free_, and _calloc_. The `MEM_LIBC_MALLOC`
option controls this. Setting `MEM_LIBC_MALLOC` to zero will change any internal
malloc calls to use the lwIP-supplied malloc functions with a preallocated heap.

When `MEM_LIBC_MALLOC` is enabled, the `MEM_SIZE` option is not used, and when
disabled, `MEM_SIZE` _is_ used and the heap is preallocated. One of the reasons
this option was enabled by default is that it saves memory if the whole heap
isn't used. Plus, it saves some program memory because it doesn't need to
include the code for the lwIP-defined functions. However, there's no cap on the
amount of memory a program can use which may be a concern for some software.

A second reason this option was enabled is that the current system-supplied
functions are likely optimized for the current platform. Yet a third reason is
it's hard to anticipate what programs will actually need; this way obviates the
need to assume what a good heap size is.

There's a few macros that can be used if you want to use your own malloc
functions and override the defaults. These are: `mem_clib_free`,
`mem_clib_malloc`, and `mem_clib_calloc`. By default, if not set, these point to
the system-provided functions.

For example, if you want to use EXTMEM for the heap, then you can define these
as `extmem_free`, `extmem_malloc`, and `extmem_calloc`, respectively. This could
either be done in _lwipopts.h_ or wherever your build system provides
build flags.

Example that defines these in _lwipopts.h_:

```c++
#define mem_clib_free extmem_free
#define mem_clib_malloc extmem_malloc
#define mem_clib_calloc extmem_calloc
```

## Entropy generation

For the Teensy 4.0 and 4.1, this library defines functions for accessing the
processor's internal "true random number generator" (TRNG) for entropy. If your
project needs to use the _Entropy_ library instead, set the
`QNETHERNET_USE_ENTROPY_LIB` macro to `1` so that any internal entropy
collection doesn't interfere with your project's entropy collection.

The _Entropy_ library does essentially the same things as the internal TRNG
functions, it just requires an additional dependency. This is the reason these
functions are provided: to remove that dependency.

See the function declarations in _src/qnethernet/security/entropy.h_ if you want
to use them yourself.

If the target device isn't a Teensy 4 then the _Entropy_ library will be used,
unless it's not accessible or doesn't exist for the device, in which case an
instance of `std::minstd_rand` will be used.

### The `RandomDevice` _UniformRandomBitGenerator_

Also provided is a class called `RandomDevice` that implements the
[_UniformRandomBitGenerator_](https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator)
C++ named requirement. It's in the `qindesign::security` namespace. An instance
can be accessed by calling its `instance()` static function.

This object works with both the internal entropy functions and with the
_Entropy_ library.

This is the preferred way to acquire entropy. It is meant to be used with a
[Random number distribution](https://en.cppreference.com/w/cpp/numeric/random#Random_number_distributions).

## Security features

This section discusses the security features of this library.
(Caveat: This library was not written by a security expert, so there are no
security guarantees.)

### Secure TCP initial sequence numbers (ISNs)

Secure TCP ISNs are generated using the suggested algorithm from
[RFC 6528 Defending against Sequence Number Attacks](https://datatracker.ietf.org/doc/html/rfc6528).
The SipHash-2-4-64 algorithm is used as the pseudorandom function (PRF), and the
microsecond clock is used as the timer.

The 128-bit key is generated on first use with a call to
`qnethernet_hal_fill_entropy()`.

The ISN is calculated as follows:
> micros() + SipHash-2-4-64(local_port, remote_port, remote_ip, local_ip)

This feature can be enabled or disabled with the
[`QNETHERNET_ENABLE_SECURE_TCP_ISN` macro](#configuration-macros).

## Configuration macros

There are two sets of configuration macros:
1. _QNEthernet_-specific options, and
2. lwIP stack options.

Any of them can either be configured as project build macros or by changing them
in the relevant configuration file:
1. _qnethernet_opts.h_ for the _QNEthernet_-specific options, and
2. _lwipopts.h_ for the lwIP options.

The _QNEthernet_-specific macros are as follows:

| Macro                                       | Default  | Description                                                                                    | Link                                                                                    |
| ------------------------------------------- | -------- | ---------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| `QNETHERNET_ALTCP_TLS_ADAPTER`              | Disabled | Enables the _altcp_tls_adapter_ functions for easier TLS library integration                   | [About the TLS adapter functions](#about-the-tls-adapter-functions)                     |
| `QNETHERNET_BUFFERS_IN_RAM1`                | Disabled | Puts the RX and TX buffers into RAM1                                                           | [Notes on RAM1 usage](#notes-on-ram1-usage)                                             |
| `QNETHERNET_CUSTOM_WRITE`                   | Disabled | Uses expanded `stdio` output behaviour                                                         | [stdio](#stdio)                                                                         |
| `QNETHERNET_DO_LOOP_IN_YIELD`               | Enabled  | The library should try to hook into or override yield() to call Ethernet.loop()                | [Notes on `yield()`](#notes-on-yield)                                                   |
| `QNETHERNET_ENABLE_ALTCP_DEFAULT_FUNCTIONS` | Disabled | Enables default implementations of the altcp interface functions                               | [Application layered TCP: TLS, proxies, etc.](#application-layered-tcp-tls-proxies-etc) |
| `QNETHERNET_ENABLE_PROMISCUOUS_MODE`        | Disabled | Enables promiscuous mode                                                                       | [Promiscuous mode](#promiscuous-mode)                                                   |
| `QNETHERNET_ENABLE_RAW_FRAME_LOOPBACK`      | Disabled | Enables raw frame loopback when the destination MAC matches the local MAC or the broadcast MAC | [Raw frame loopback](#raw-frame-loopback)                                               |
| `QNETHERNET_ENABLE_RAW_FRAME_SUPPORT`       | Disabled | Enables raw frame support                                                                      | [Raw Ethernet Frames](#raw-ethernet-frames)                                             |
| `QNETHERNET_ENABLE_SECURE_TCP_ISN`          | Enabled  | Enables secure TCP initial sequence numbers (ISNs)                                             | [Secure TCP initial sequence numbers (ISNs)](#secure-tcp-initial-sequence-numbers-isns) |
| `QNETHERNET_FLUSH_AFTER_WRITE`              | Disabled | Follows every `EthernetClient::write()` call with a flush; may reduce efficiency               | [Write immediacy](#write-immediacy)                                                     |
| `QNETHERNET_LWIP_MEMORY_IN_RAM1`            | Disabled | Puts lwIP-declared memory into RAM1                                                            | [Notes on RAM1 usage](#notes-on-ram1-usage)                                             |
| `QNETHERNET_USE_ENTROPY_LIB`                | Disabled | Uses _Entropy_ library instead of internal functions                                           | [Entropy collection](#entropy-collection)                                               |

To enable a feature, set the associated macro to `1` or just define it. To
disable a feature, either set the same macro to `0` or leave it undefined.

See
[Changing lwIP configuration macros in `lwipopts.h`](#changing-lwip-configuration-macros-in-lwipoptsh)
for changing the IP stack configuration.

Note that disabling features means that the build will not include those
features, thus saving space.

### Configuring macros using the Arduino IDE

_[Current as of this writing: Arduino IDE 2.3.5, Teensyduino 1.60-beta3]_

The Arduino IDE provides a facility to override the build options specified in a
platform's build configuration file, _platform.txt_. It does this by looking for
a file named _platform.local.txt_ in the same place. Any options in that "local"
file override equivalent options in the main file.

Note that the IDE might need to be restarted when the file changes.

The suggested way to override compiler options is with the `*.extra_flags`
properties, for example, `compiler.cpp.extra_flags`, `compiler.c.extra_flags`,
and `build.extra_flags`. However, this only works if _platform.txt_ uses those
options. If it does not then there's nothing to override. The current
Teensyduino installation's _platform.txt_ file does not use these options.

Here's how to implement the behaviour:
1. Insert these sections somewhere in _platform.txt_, before the first location
   where these properties will be used:
   ```
   # This can be overridden in boards.txt
   build.extra_flags=

   # These can be overridden in platform.local.txt
   compiler.c.extra_flags=
   compiler.cpp.extra_flags=
   compiler.S.extra_flags=
   ```
2. Insert `{compiler.cpp.extra_flags}` and `{build.extra_flags}` before
   `{includes}` in:
   1. `recipe.preproc.includes`
   2. `recipe.preproc.macros`
   3. `recipe.cpp.o.pattern`
3. Insert `{compiler.c.extra_flags}` and `{build.extra_flags}` before
   `{includes}` in:
   1. `recipe.c.o.pattern`
4. Insert `{compiler.S.extra_flags}` and `{build.extra_flags}` before
   `{includes}` in:
   1. `recipe.S.o.pattern`

Next, create a _platform.local.txt_ file in the same directory as the
_platform.txt_ file and add the options you need. The contents of the properties
are exactly the same as if adding them to the command line. For example, to
enable raw frame support and disable DNS using the macros (the '-D' option
defines a macro):

```
compiler.cpp.extra_flags=-DQNETHERNET_ENABLE_RAW_FRAME_SUPPORT=1 -DLWIP_DNS=0
compiler.c.extra_flags=-DQNETHERNET_ENABLE_RAW_FRAME_SUPPORT=1 -DLWIP_DNS=0
```

Each additional option is simply appended. No commas or quotes are required
unless they would be used for those same options on the command line.
See
[this issue comment](https://github.com/ssilverman/QNEthernet/issues/54#issuecomment-1788100978)
for some incorrect variants.

Note that both properties are needed because _QNEthernet_ contains a mixture of
C and C++ sources. If the extra flags are exactly the same for both properties,
and this is likely the case, one could refer to the other. For example:

```
compiler.cpp.extra_flags=-DQNETHERNET_ENABLE_RAW_FRAME_SUPPORT=1 -DLWIP_DNS=0
compiler.c.extra_flags={compiler.cpp.extra_flags}
```

The properties of most interest are probably the ones in this example. There are
other ones defined in the Arduino AVR version, but those aren't really
needed here.

Lest you think I've forgotten to add it, here're the locations of the files for
the current latest version of the IDE (for Teensy, specifically; the locations
will be different, but should be similar, for other platforms):
* Mac: _~/Library/Arduino15/packages/teensy/hardware/avr/{version}_
* Linux: _~/.arduino15/packages/teensy/hardware/avr/{version}_
* Windows: _%userprofile%\\AppData\\Local\\Arduino15\\packages\\teensy\\hardware\\avr\\{version}_

References:
1. [Additional compiler options - Programming - Arduino Forum](https://forum.arduino.cc/t/additional-compiler-options/631297)
2. [Arduino IDE: Where can I pass defines to the compiler? - IDE 1.x - Arduino Forum](https://forum.arduino.cc/t/arduino-ide-where-can-i-pass-defines-to-the-compiler/680845)
3. [Request for Arduino IDE &quot;extra_flags&quot; support | Teensy Forum](https://forum.pjrc.com/index.php?threads/request-for-arduino-ide-extra_flags-support.72556/)
4. [Platform specification - Arduino CLI](https://arduino.github.io/arduino-cli/latest/platform-specification/)
5. This one started it all &rarr; [RawFrameMonitor example seems to be missing something... · Issue #33 · ssilverman/QNEthernet](https://github.com/ssilverman/QNEthernet/issues/33)
6. [Open the Arduino15 folder &ndash; Arduino Help Center](https://support.arduino.cc/hc/en-us/articles/360018448279-Open-the-Arduino15-folder)
7. [Enabling Raw Frame Support and Promiscuous · Issue #54 · ssilverman/QNEthernet](https://github.com/ssilverman/QNEthernet/issues/54)

### Configuring macros using PlatformIO

Simply add compiler flags to the `build_flags` build option in _platformio.ini_.

For example:

```
build_flags = -DQNETHERNET_ENABLE_RAW_FRAME_SUPPORT=1
```

### Changing lwIP configuration macros in `lwipopts.h`

The `lwipopts.h` file defines certain macros needed by the system. It is
appropriate for the user to alter some of those macros if needed, for example,
to change the number of UDP sockets or IGMP groups.

These macros can either be modified directly in the file (`lwipopts.h`) or from
the command line with a `-D` directive. The ones that can be modified from the
command line are either wrapped in an `#ifndef` block or not defined at all.

Useful macro list; please see further descriptions in `opt.h` and
in `mdns_opts.h`:

| Macro                     | Description                                                |
| ------------------------- | ---------------------------------------------------------- |
| `DNS_MAX_RETRIES`         | Maximum number of DNS retries                              |
| `LWIP_ALTCP`              | `1` to enable application layered TCP (eg. TLS, proxies)   |
| `LWIP_ALTCP_TLS`          | `1` to enable TLS support for ALTCP                        |
| `LWIP_ALTCP_TLS_MBEDTLS`  | `1` to enable the Mbed TLS implementation for ALTCP TLS    |
| `LWIP_DHCP`               | Zero to disable DHCP                                       |
| `LWIP_DNS`                | Zero to disable DNS                                        |
| `LWIP_IGMP`               | Zero to disable IGMP; also disables mDNS by default        |
| `LWIP_LOOPBACK_MAX_PBUFS` | Non-zero to specify loopback queue size                    |
| `LWIP_MDNS_RESPONDER`     | Zero to disable mDNS capabilities                          |
| `LWIP_NETIF_LOOPBACK`     | `1` to enable loopback capabilities                        |
| `LWIP_STATS`              | `1` to enable lwIP stats collection                        |
| `LWIP_STATS_LARGE`        | `1` to use 32-bit stats counters instead of 16-bit         |
| `LWIP_TCP`                | Zero to disable TCP                                        |
| `LWIP_UDP`                | Zero to disable UDP; also disables DHCP and DNS by default |
| `MDNS_MAX_SERVICES`       | Maximum number of mDNS services                            |
| `MEM_LIBC_MALLOC`         | Zero to enable use of lwIP-defined malloc functions        |
| `MEM_SIZE`                | Heap memory size; unused if `MEM_LIBC_MALLOC` is enabled   |
| `MEMP_NUM_IGMP_GROUP`     | Number of multicast groups                                 |
| `MEMP_NUM_TCP_PCB`        | Number of listening TCP sockets                            |
| `MEMP_NUM_TCP_PCB_LISTEN` | Number of TCP sockets                                      |
| `MEMP_NUM_UDP_PCB`        | Number of UDP sockets                                      |

Some extra conditions to keep in mind:
* `MEMP_NUM_IGMP_GROUP`: Count must include 1 for the "all systems" group and 1
  if mDNS is enabled.
* `MEMP_NUM_UDP_PCB`: Count must include one if mDNS is enabled.

## Complete list of features

This section is an attempt to provide a complete list of features in the
_QNEthernet_ library.

1. Compatible with the Arduino-style Ethernet API
2. [Additional functions and features not in the Arduino-style API](#additional-functions-and-features-not-in-the-arduino-style-api)
3. Automatic MAC address detection on the Teensy platform; it's not necessary to
   initialize the library with your own MAC address for that platform
4. A [DNS client](#dnsclient)
5. [mDNS](#mdns) support
6. [Raw Ethernet frame](#raw-ethernet-frames) support
7. [`stdio`](#stdio) output redirection support for `stdout` and `stderr`
8. [VLAN tagging](#how-to-implement-vlan-tagging) support
9. [Zero-length UDP packets](#parsepacket-return-values)
10. [UDP](#udp-receive-buffering) and [raw frame](#raw-frame-receive-buffering)
    receive buffering for when data arrives in bursts that are faster than the
    ability of the program to process them
11. [Listeners](#how-to-use-listeners) to watch link, network interface, and
    address state
12. With some additions:
    1. IPv6
    2. IEEE 1588, Precision Time Protocol (PTP)
    3. TLS and TCP proxies
    4. IEEE 802.3az, Energy-Efficient Ethernet (EEE)
    5. Secure TCP initial sequence numbers (ISNs)
13. [Client shutdown options](#ethernetclient): _close_ (start close process
    without waiting), _closeOutput_ (close just the output side, also called a
    "half-close"), _abort_ (shuts down the connection without going through the
    TCP close process), _stop_ (close and wait)
14. Ability to [fully write](#how-to-write-data-to-connections) data to a
    client connection
15. [Multicast](#how-to-use-multicast) support
16. [Promiscuous mode](#promiscuous-mode)
17. `SO_REUSEADDR` support (see [`EthernetServer`](#ethernetserver)
    and [`EthernetUDP`](#ethernetudp))
18. [`TCP_NODELAY`](#tcp-socket-options) support
19. Configuration via [Configuration macros](#configuration-macros)
20. Non-blocking TCP connections
21. Teensy platform: Internal [Entropy generation](#entropy-generation)
    functions to avoid the _Entropy_ lib dependency; this can be disabled with a
    configuration macro
22. A "random device" satisfying the _UniformRandomBitGenerator_ C++ named
    requirement that provides access to hardware-generated entropy (see
    [The `RandomDevice` _UniformRandomBitGenerator_](#the-randomdevice-uniformrandombitgenerator))
23. Driver support for:
    1. Teensy 4.1
    2. W5500
24. Straightforward to add new Ethernet frame drivers
25. Ability to toggle Nagle's algorithm for TCP
26. Ability to set some IP header fields: differentiated services (DiffServ)
    and TTL
27. Secure TCP initial sequence numbers (ISNs)

## Compatibility with other APIs

This section describes compatibility with other Ethernet APIs. The term
"API unifictation effort" will be used to describe the information from here:
[Guide for Arduino networking library developers](https://github.com/Networking-for-Arduino/Arduino-Networking-API/blob/6370554129b4728bd1deb7ca5d429105c54d8bba/ArduinoNetAPIDev.md)

What follows are explanatory notes for each differing API function from
that link.

`uint8_t* Ethernet.macAddress(uint8_t* mac)`:-\
For MAC address retrieval, see `uint8_t* Ethernet.macAddress()` and
`void Ethernet.macAddress(uint8_t* mac)`. This form isn't implemented because
the return value is ambiguous when the input is NULL.

`Ethernet.begin(...)` functions:-\
Except for the Arduino-defined `begin(mac, timeout)` function, the `begin`
functions are all non-blocking. Regarding parameter order, the "ip, subnet,
gateway, dns" order is more sensible than the "ip, dns, gateway, subnet" order.
Additionally, the default values for the "ip, dns, gateway, subnet" order don't
work well except for very specific networks.

`Ethernet.config(address, dns, gateway, mask)`:-\
There's some ambiguity on how this function is defined with relation to `begin`.

DNS functions:-\
The Arduino API defines `Ethernet.dnsServerIP()` and
`Ethernet.setDnsServerIP(dns)`. The _QNEthernet_ library defines `dnsServerIP()`
and `setDNSServerIP(dns)` for more consistent naming. There are also versions
that take an index parameter for retrieving and setting different DNS addresses.
The unification effort describes inconsistent `setDNS(dns1, dns2)` and
`dnsIP(index)` functions. These are inconsistent in two ways: the names, and the
imbalance between only allowing two to be set but any number to be retrieved. So
as not to add more variants, it was chosen to keep the original names.

`EthernetClient::stop()`:-\
The Arduino version states that this function has a disconnect timeout in the
`setConnectionTimeout(ms)` description. The _QNEthernet_ library version waits
for a timeout and provides a `close()` function that does not wait. There is
also an `abort()` function that aborts the connection with a RST segment.

`EthernetClient::status()`:-\
Returns a value of type `enum tcp_state`, an lwIP type.

`EthernetUDP::parsePacket()`:-\
The _QNEthernet_ library version returns -1 for no packet available and not
zero, because empty UDP packets exist. If zero is returned when no packet is
available then empty packets could not be detected. (A `std::optional` might be
a better choice for the return value here, or even a `hasPacket()` function that
splits out presence from size.)

## Other notes

I'm not 100% percent certain where this library will go, but I want it to be
amazing. It was originally meant to be an alternative to the NativeEthernet/FNET
library on the Teensy 4.1. Now it can be used as an alternative to other
libraries on other platforms.

I'm also not settled on the name.

Input is welcome.

## To do

* Tune lwIP.
* A better API design than the Arduino-defined API.
* Perhaps zero-copy is an option.
* More unit tests.
* I have seen
  `Assertion "tcp_slowtmr: TIME-WAIT pcb->state == TIME-WAIT" failed at line 1442 in src/lwip/tcp.c`
  when sending a large amount of data. Either it's an lwIP bug or I'm doing
  something wrong.
  See: [Re: [lwip-users] Assert "tcp_slowtmr" in tcp.c reached](https://lists.gnu.org/archive/html/lwip-users/2010-02/msg00013.html)
* More examples.
* Fix reduced frame reception when Ethernet is restarted via
  `end()`/`begin(...)`. This is a vexing one.
  See also:
  [Restarting  · Issue #31 · ssilverman/QNEthernet](https://github.com/ssilverman/QNEthernet/issues/31)
* Raw IP.
* Ping.
* Figure out why SYS_TIMEOUT exhaustion occurs sometimes. Is it mDNS?

## Code style

Code style for this project mostly follows the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

Other conventions are adopted from Bjarne Stroustrup's and Herb Sutter's
[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/).
([C++ Core Guidelines repo](https://github.com/isocpp/CppCoreGuidelines))

## References

* [manitou48's original Teensy 4.1 Ethernet code](https://github.com/PaulStoffregen/teensy41_ethernet)
* [Arduino Ethernet Reference](https://docs.arduino.cc/libraries/ethernet/)
* [lwIP Home](https://savannah.nongnu.org/projects/lwip/)
* [Forum _QNEthernet_ announcement thread](https://forum.pjrc.com/index.php?threads/new-lwip-based-ethernet-library-for-teensy-4-1.68066/)
* [lwIP testing by manitou](https://forum.pjrc.com/index.php?threads/teensy-4-1-beta-test.60532/page-5#post-237096)
* [Dan Drown's NTP server and 1588 timestamps](https://forum.pjrc.com/index.php?threads/teensy-4-1-ntp-server.61581/)
* [Dan Drown's modifications to Paul's code](https://github.com/ddrown/teensy41_ethernet)
* [Tino Hernandez's (vjmuzik) FNET-based NativeEthernet library](https://forum.pjrc.com/index.php?threads/t4-1-ethernet-library.60857/)
* [Juraj Andrássy's Arduino Networking API documentation](https://github.com/Networking-for-Arduino/Arduino-Networking-API)

---

Copyright (c) 2021-2025 Shawn Silverman

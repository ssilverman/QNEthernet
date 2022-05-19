<a href="https://www.buymeacoffee.com/ssilverman" title="Donate to this project using Buy Me a Coffee"><img src="https://img.shields.io/badge/buy%20me%20a%20coffee-donate-orange.svg?logo=buy-me-a-coffee&logoColor=FFDD00" alt="Buy Me a Coffee donate button"></a>

# _QNEthernet_, an lwIP-Based Ethernet Library For Teensy 4.1

_Version: 0.15.0-snapshot_

The _QNEthernet_ library provides Arduino-like `Ethernet` functionality for the
Teensy 4.1. While it is mostly the same, there are a few key differences that
don't quite make it a drop-in replacement.

The low-level code is based on code by Paul Stoffregen, here:\
https://github.com/PaulStoffregen/teensy41_ethernet

All the Teensy stuff is under the MIT license, so I'm making this library under
the MIT license as well. Please see the _lwip-info/_ directory for the info
files provided with the lwIP release.

## Table of contents

1. [Differences, assumptions, and notes](#differences-assumptions-and-notes)
2. [Additional functions and features not in the Arduino API](#additional-functions-and-features-not-in-the-arduino-api)
   1. [`Ethernet`](#ethernet)
   2. [`EthernetClient`](#ethernetclient)
      1. [TCP socket options](#tcp-socket-options)
   3. [`EthernetServer`](#ethernetserver)
   4. [`EthernetUDP`](#ethernetudp)
   5. [`EthernetFrame`](#ethernetframe)
   6. [`MDNS`](#mdns)
   7. [`DNSClient`](#dnsclient)
   8. [Print utilities](#print-utilities)
   9. [`IPAddress` operators](#ipaddress-operators)
3. [How to run](#how-to-run)
4. [How to write data to connections](#how-to-write-data-to-connections)
   1. [Write immediacy](#write-immediacy)
5. [A note on the examples](#a-note-on-the-examples)
6. [A survey of how connections (aka `EthernetClient`) work](#a-survey-of-how-connections-aka-ethernetclient-work)
   1. [Connections and link detection](#connections-and-link-detection)
7. [How to use multicast](#how-to-use-multicast)
8. [How to use listeners](#how-to-use-listeners)
9. [mDNS services](#mdns-services)
10. [DNS](#dns)
11. [stdio](#stdio)
12. [Raw Ethernet frames](#raw-ethernet-frames)
    1. [Promiscuous mode](#promiscuous-mode)
13. [How to implement VLAN tagging](#how-to-implement-vlan-tagging)
14. [Other notes](#other-notes)
15. [To do](#to-do)
16. [Code style](#code-style)
17. [References](#references)

## Differences, assumptions, and notes

**Note: Please read the function docs in the relevant header files for
more information.**

This library mostly follows the Arduino Ethernet API, with these differences
and notes:
* Include the `QNEthernet.h` header instead of `Ethernet.h`.
* Ethernet `loop()` is called from `yield()`. The functions that wait for
  timeouts rely on this. This also means that you must use `delay`, `yield()`,
  or `Ethernet.loop()` when waiting on conditions; waiting without calling these
  functions will cause the TCP/IP stack to never refresh. Note that many of the
  I/O functions call `loop()` so that there's less burden on the calling code.
* `EthernetServer::write` functions always return the write size requested. This
  is because different clients may behave differently.
* The examples in https://www.arduino.cc/en/Reference/EthernetServerAccept and
  https://www.arduino.cc/en/Reference/IfEthernetClient directly contradict each
  other with regard to what `operator bool()` means in `EthernetClient`. The
  first example uses it as "already connected", while the second uses it as
  "available to connect". "Connected" is the chosen concept, but different from
  `connected()` in that it doesn't check for unread data.
* All the Arduino-defined `Ethernet.begin` functions that use the MAC address
  are deprecated.
* The following `Ethernet` functions are deprecated and do nothing or return
  some default value:
  * `hardwareStatus()`: Returns `EthernetHardwareStatus::EthernetOtherHardware`
    because zero might be interpreted as "no hardware".
  * `init(uint8_t sspin)`: Does nothing.
  * `maintain()`: Returns zero.
  * `setRetransmissionCount(uint8_t number)`: Does nothing.
  * `setRetransmissionTimeout(uint16_t milliseconds)`: Does nothing.
* The following `Ethernet` functions are deprecated but call
  something equivalent:
  * `MACAddress(uint8_t mac[6])`
  * `setDnsServerIP(const IPAddress &dnsServerIP)`
* The following `EthernetUDP` functions do nothing:
  * `flush()` because it is ill-defined.
* The system starts with the Teensy's actual MAC address. If you want to use
  that address with the deprecated API, you can collect it with
  `Ethernet.macAddress` and then pass it to one of the deprecated `begin`
  functions.
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
  * *src/sys_arch.c*
  * _src/lwipopts.h_ &larr; Use this one for tuning (see _src/lwip/opt.h_ for
    more details).
  * _src/arch/cc.h_
* The main include file, `QNEthernet.h`, in addition to including the
  `Ethernet`, `EthernetFrame`, and `MDNS` instances, also includes the headers
  headers for `EthernetClient`, `EthernetServer`, and `EthernetUDP`.
* Most of the `Ethernet` functions do nothing or return some form of
  empty/nothing/false unless the system has been initialized.

## Additional functions and features not in the Arduino API

_QNEthernet_ defines functions that don't exist in the Arduino API as it's
currently defined. (See:
[Arduino Ethernet library](https://www.arduino.cc/en/Reference/Ethernet))
This section documents those functions.

Features:
* The `read(buf, len)` functions allow a NULL buffer so that the caller can skip
  data without having to read into a buffer.

### `Ethernet`

The `Ethernet` object is the main Ethernet interface.

* `begin()`: Initializes the library and uses the Teensy's internal MAC address.
* `begin(ipaddr, netmask, gw)`: Initializes the library, uses the Teensy's
  internal MAC address, and uses the given parameters for the
  network configuration.
* `broadcastIP()`: Returns the broadcast IP address associated with the current
  local IP and subnet mask.
* `end()`: Shuts down the library, including the Ethernet clocks.
* `hostname()`: Gets the DHCP client hostname. An empty string means that no
  hostname is set. The default is "teensy-lwip".
* `linkState()`: Returns a `bool` indicating the link state.
* `linkSpeed()`: Returns the link speed in Mbps.
* `linkIsFullDuplex()`: Returns whether the link is full duplex (`true`) or half
  duplex (`false`).
* `joinGroup(ip)`: Joins a multicast group.
* `leaveGroup(ip)`: Leaves a multicast group.
* `macAddress(mac)`: Fills the 6-byte `mac` array with the current MAC address.
  Note that the equivalent Arduino function is `MACAddress(mac)`.
* `setDNSServerIP(dnsServerIP)`: Sets the DNS server IP address. Note that the
  equivalent Arduino function is `setDnsServerIP(dnsServerIP)`.
* `setHostname(hostname)`: Sets the DHCP client hostname. The empty string will
  set the hostname to nothing. To use something other than the default at system
  start, call this before calling `begin`.
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
    changed. This might be called before the link is up if a static IP is set.
* `static constexpr int maxMulticastGroups()`: Returns the maximum number of
  multicast groups.
* `static constexpr size_t mtu()`: Returns the MTU.

### `EthernetClient`

* `close()`: Closes a connection, but without waiting. It's similar to `stop()`.
* `closeOutput()`: Shuts down the transmit side of the socket. This is a
  half close operation.
* `writeFully(b)`: Writes a single byte.
* `writeFully(s)`: Writes a string (`const char *`).
* `writeFully(s, size)`: Writes characters (`const char *`).
* `writeFully(buf, size)`: Writes a data buffer (`const uint8_t *`).
* `static constexpr int maxSockets()`: Returns the maximum number of
  TCP connections.

#### TCP socket options

 * `setNoDelay(flag)`: Sets or clears the TCP_NODELAY flag in order to disable
   or enable Nagle's algorithm, respectively. This must be changed for each
   new connection.
 * `isNoDelay()`: Returns whether the TCP_NODELAY flag is set for the current
   connection. Returns false if not connected.

### `EthernetServer`

* `begin(reuse)`: Similar to `begin()`, but the Boolean `reuse` parameter
  controls the SO_REUSEADDR socket option.
* `end()`: Shuts down the server.
* `port()`: Returns the server's port.
* `static constexpr int maxListeners()`: Returns the maximum number of
  TCP listeners.

### `EthernetUDP`

* `begin(localPort, reuse)`: Similar to `begin(localPort)`, but the Boolean
  `reuse` parameter controls the SO_REUSEADDR socket option.
* `beginMulticast(ip, localPort, reuse)`: Similar to
  `beginMulticast(ip, localPort)`, but with a `reuse` parameter, similar to
  the above.
* `data()`: Returns a pointer to the received packet data.
* `localPort()`: Returns the port to which the socket is bound, or zero if it is
  not bound.
* `send(host, port, data, len)`: Sends a packet without having to use
  `beginPacket()`, `write()`, and `endPacket()`. It causes less overhead. The
  host can be either an IP address or a hostname.
* `static constexpr int maxSockets()`: Returns the maximum number of
  UDP sockets.

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
* `data()`: Returns a pointer to the received frame data.
* `endFrame()`: Sends the frame. This returns whether the send was successful. A
  frame must have been started, its data length must be in the range 60-1518,
  and Ethernet must have been initialized. This is similar
  to `EthernetUDP::endPacket()`.
* `parseFrame()`: Checks if a new frame is available. This is similar
  to `EthernetUDP::parseFrame()`.
* `send(frame, len)`: Sends a raw Ethernet frame without the overhead of
  `beginFrame()`/`write()`/`endFrame()`. This is similar
  to `EthernetUDP::send(data, len)`.
* `static constexpr int maxFrameLen()`: Returns the maximum frame length
  including the FCS. Subtract 4 to get the maximum length that can be sent or
  received using this API.
* `static constexpr int minFrameLen()`: Returns the minimum frame length
  including the FCS. Subtract 4 to get the minimum length that can be sent or
  received using this API.

### `MDNS`

The `MDNS` object provides an mDNS API.

* `begin(hostname)`: Starts the mDNS responder and uses the given hostname as
  the name.
* `end()`: Stops the mDNS responder.
* `addService(type, protocol, port)`: Adds a service. The protocol will be set
  to `"_udp"` for anything other than `"_tcp"`. The strings should have a `"_"`
  prefix. Uses the hostname as the service name.
* `addService(type, protocol, port, getTXTFunc)`: Adds a service and associated
  TXT records.
* `removeService(type, protocol, port)`: Removes a service.
* `static constexpr int maxServices()`: Returns the maximum number of
  supported services.

### `DNSClient`

The `DNSClient` class provides an interface to the DNS client.

* `setServer(index, ip)`: Sets a DNS server address.
* `getServer(index)`: Gets a DNS server address.
* `getHostByName(hostname, callback)`: Looks up a host by name and calls the
  callback when there's a result.
* `getHostByName(hostname, ip, timeout)`: Looks up a host by name.
* `static constexpr int maxServers()`: Returns the maximum number of
  DNS servers.

### Print utilities

The `util/PrintUtils.h` file declares some useful output functions. Note that
this file is included when `QNEthernet.h` is included; there's no need to
include it separately.

The functions are in the `qindesign::network::util` namespace, so if you've
already added `using namespace qindesign::network;` to your code, they can be
called with `util::writeMagic()` syntax. Otherwise, they need to be fully
qualified: `qindesign::network::util::writeMagic()`.

1. `writeFully(Print &, buf, size, breakf = nullptr)`: Attempts to completely
   write bytes to the given `Print` object. The optional `breakf` function is
   used as the stopping condition. It returns the number of bytes actually
   written. The return value will be less than the requested size only if
   `breakf` returned `true` before all bytes were written. Note that a NULL
   `breakf` function is assumed to return `false`.

   For example, the `EthernetClient::writeFully()` functions use this and pass
   the "am I disconnected" condition as the `breakf` function.

2. `writeMagic(Print &, mac, breakf = nullptr)`: Writes the payload for a
   [Magic packet](#https://en.wikipedia.org/wiki/Wake-on-LAN#Magic_packet) to
   the given `Print` object. This uses `writeFully()` under the covers and
   passes along the `breakf` function as the stopping condition.

There is also a `Print` decorator for `stdio` output files, `util::StdioPrint`
(assuming the `qindesign::network` namespace is in scope). It provides a `Print`
interface so that it is easy to print `Printable` objects to `stdout` or
`stderr` without having to worry about buffering and the need to flush any
output before printing a `Printable` directly to, say, `Serial`.

### `IPAddress` operators

The core library version of `IPAddress` is missing a `==` operator that can
compare `const IPAddress` values. In addition, it is completely missing a `!=`
operator. Provided in this library are these two operators. They are declared as
follows in the usual namespace:

1. `bool operator==(const IPAddress &a, const IPAddress &b);`
2. `bool operator!=(const IPAddress &a, const IPAddress &b);`

## How to run

This library works with both PlatformIO and Arduino. To use it with Arduino,
here are a few steps to follow:

1. Add `#include <QNEthernet.h>`. Note that this include already includes the
   header for `EthernetUDP`. Some external examples also include `SPI.h`. This
   is not needed unless you're actually using SPI in your program.
2. Below that, add: `using namespace qindesign::network;`
3. In `setup()`, just after initializing `Serial`, set `stdPrint = &Serial`.
   This enables some lwIP output and also `printf` for your own code.
4. You likely don't want or need to set/choose your own MAC address, so just
   call `Ethernet.begin()` with no arguments to use DHCP and the three-argument
   version (IP, subnet mask, gateway) to set your own address. If you really
   want to set your own MAC address, see `setMACAddress(mac)` or one of the
   deprecated `begin` functions that takes a MAC address parameter.
5. There is an `Ethernet.waitForLocalIP(timeout)` convenience function that can
   be used to wait for DHCP to supply an address because `Ethernet.begin()`
   doesn't wait. Try 10 seconds (10000 ms) and see if that works for you.
6. `Ethernet.hardwareStatus()` always returns `EthernetOtherHardware`. This
   means that there is no reason to call this function.
7. Most other things should be the same.

Please see the examples for more things you can do with the API, including:
* Using listeners to watch for network changes,
* Monitoring and sending raw Ethernet frames, and
* Setting up an mDNS service.

## How to write data to connections

I'll start with these statements:
1. **Don't use the `print` functions when writing data to connections.**
2. **Always check the `write` and `print` (and `println` and `printf`)
   return values, retrying if necessary.**
3. Data isn't necessarily sent immediately.

The `write` and `print` functions in the `Print` API all return the number of
bytes actually written. This means that you _must always check the return value,
retrying any missing bytes_ if you want all your data to get sent.

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

The solution is to utilize the raw `write` functions and retry any bytes that
aren't sent. Let's create a `writeFully` function and use that to send the data:

```c++
// Keep writing until all the bytes are sent or the connection
// is closed.
void writeFully(EthernetClient &client, const char *data, int len) {
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

_Note that the library implements `writeFully`; you don't have to roll
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

Let's go back to our original statement about not using the `print` functions.
Their implementation is opaque and they sometimes make assumptions that the data
will be "written fully". For example, Teensyduino's current
`print(const String &)` implementation attempts to send all the bytes and
returns the number of bytes sent, but it doesn't tell you _which_ bytes were
sent. For the string `"12345"`, `print` might send `"12"`, fail to send `"3"`,
and successfully send `"45"`, returning `4`.

Similarly, we have no idea what `print(const Printable &obj)` does because the
`Printable` implementation passed to it is beyond our control. For example,
Teensyduino's `IPAddress::printTo(Print &)` implementation prints the address
without checking the return value of the `print` calls.

Also, most examples I've seen that use any of the `print` functions never check
the return values. Common practice seems to stem from this style of usage.
Network applications work a little differently, and there's no guarantee all the data gets sent.

The `write` functions don't have this problem (unless, of course, there's a
faulty implementation). They attempt to send bytes and return the number of
bytes actually sent.

In summary, my ***strong*** suggestion is to use the `write` functions when
sending network data, checking the return values and acting on them. Or you can
use the library's `writeFully` functions.

See the discussion at:
https://forum.pjrc.com/threads/68389-NativeEthernet-stalling-with-dropped-packets

### Write immediacy

Data isn't necessarily completely sent across the wire after `write` or
`writeFully` calls. Instead, data is merely enqueued until the internal buffer
is full or a timer expires. Now, if the data to send is larger than the internal
TCP buffer then data will be sent and the extra data will be enqueued. In other
words, data is only sent when either the buffer is full or an internal timer
has expired.

To send any buffered data, call `flush()`.

To quote lwIP's `tcp_write()` docs:
> Write data for sending (but does not send it immediately).
>
> It waits in the expectation of more data being sent soon (as
> it can send them more efficiently by combining them together).
> To prompt the system to send data now, call tcp_output() after
> calling tcp_write().

`flush()` is what always calls `tcp_output()` internally. The `write` and
`writeFully` functions only call this when the buffer is full. The suggestion is
to call `flush()` when done sending a "packet" of data, for some definition of
"packet" specific to your application. For example, after sending a web page to
a client or after a chunk of data is ready for the server to process.

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
you've always hoped Teensy library examples could be.

## A survey of how connections (aka `EthernetClient`) work

Hopefully this section disambiguates some details about what each function does:
1. `connected()`: Returns whether connected OR data is still available
   (or both).
2. `operator bool()`: Returns whether connected (at least in _QNEthernet_).
3. `available()`: Returns the amount of data available, whether the connection
   is closed or not.
4. `read`: Reads data if there's data available, whether the connection's closed
   or not.

Connections will be closed automatically if the client shuts down a connection,
and _QNEthernet_ will properly handle the state such that the API behaves as
expected. In addition, if a client closes a connection, any buffered data will
still be available via the client API. If it were up to me, I'd have swapped the
meaning of `operator bool()` and `connected()`, but see the above list as
a guide.

Some options:

1. Keep checking `connected()` (or `operator bool()`) and `available()`/`read`
   to keep reading data. The data will run out when the connection is closed and
   after all the buffers are empty. The calls to `connected()` (or
   `operator bool()`) will indicate connection status (plus data available in
   the case of `connected()` or just connection state in the case of
   `operator bool()`).
2. Same as the above, but without one of the two connection-status calls
   (`connected()` or `operator bool()`). The data will just run out after
   connection-closed and after the buffers are empty.

### Connections and link detection

A link must be present before a connection can be made. Either call
`Ethernet.waitForLink(timeout)` or check the link state before attempting to
connect. Which approach you use will depend on how your code is structured or
intended to be used.

Be aware when using a listener approach to start or stop services that it's
possible, when setting a static IP, for the _address-changed_ callback to be
called before the link is up.

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

Instead of waiting for certain state at system start, for example _link-up_ or
_address-changed_, it's possible to watch for state changes using listeners, and
then act on those state changes. This will make your application more robust and
responsive to state changes during program operation.

The relevant functions are (see the [`Ethernet`](#ethernet) section for further
descriptions):
1. `Ethernet.onLinkState(cb)`
2. `Ethernet.onAddressChanged(cb)`

_Link-state_ occurs when an Ethernet link is detected or lost.

_Address-changed_ events occur when the IP address changes, but its effects are
a little more subtle. When setting an address via DHCP, the link must already be
up in order to receive the information. However, when setting a static IP
address, the event may occur when the link is not yet up. This means that if a
connection is attempted when it is detected that the address is valid, the
attempt will fail.

It is suggested, therefore, that when taking an action based on an
_address-changed_ event, the link state is checked in addition.

Servers, on the other hand, can be brought up even when there's no link.

See: [Connections and link detection](#connections-and-link-detection)

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

The `Ethernet.setDNSServerIP()` function sets the zeroth DNS server.

## stdio

Internally, lwIP uses `printf` for debug output and assertions. The library
defines `_write()` so that `printf` will work for `stdout` and `stderr`. It
sends output to a custom variable, `Print *stdPrint`, that defaults to NULL. To
enable any lwIP output, including assertion failure output, that variable must
be set to something conforming to the `Print` interface. If it is not set,
`printf` will still work, but there will be no output.

For example:
```c++
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial initialization
  }
  qindesign::network::stdPrint = &Serial;
}
```

The side benefit is that user code can use `printf` too.

If your application wants to define its own `_write()` implementation, then the
internal one needs to be declared as weak. To accomplish this, define the
`QNETHERNET_WEAK_WRITE` macro.

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

### Promiscuous mode

It's possible to enable promiscuous mode so that all frames are received, even
ones whose destination MAC address would normally be filtered out by the
Ethernet hardware. To do this, define the `QNETHERNET_PROMISCUOUS_MODE` macro.

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

## Other notes

I'm not 100% percent certain where this library will go, but I want it to be
amazing. It's meant to be an alternative to the NativeEthernet/FNET library,
using a different underlying TCP/IP stack.

I'm also not settled on the name.

Input is welcome.

## To do

* Tune lwIP.
* A better API design than the Arduino-defined API.
* Perhaps zero-copy is an option.
* Make a test suite.
* I have seen
  `Assertion "tcp_slowtmr: TIME-WAIT pcb->state == TIME-WAIT" failed at line 1442 in src/lwip/tcp.c`
  when sending a large amount of data. Either it's an lwIP bug or I'm doing
  something wrong.
  See: https://lists.gnu.org/archive/html/lwip-users/2010-02/msg00013.html
* More examples.

## Code style

Code style for this project mostly follows the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

Other conventions are adopted from Bjarne Stroustrup's and Herb Sutter's
[C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md).

## References

* lwIP testing by manitou:
  https://forum.pjrc.com/threads/60532-Teensy-4-1-Beta-Test?p=237096&viewfull=1#post237096
* Dan Drown's NTP server and 1588 timestamps:
  https://forum.pjrc.com/threads/61581-Teensy-4-1-NTP-server
* Paul Stoffregen's original Teensy 4.1 Ethernet code:
  https://github.com/PaulStoffregen/teensy41_ethernet
* Dan Drown's modifications to Paul's code:
  https://github.com/ddrown/teensy41_ethernet
* Tino Hernandez's (vjmuzik) FNET-based NativeEthernet library:
  https://forum.pjrc.com/threads/60857-T4-1-Ethernet-Library
* [Arduino Ethernet library](https://www.arduino.cc/en/Reference/Ethernet)

---

Copyright (c) 2021-2022 Shawn Silverman

<a href="https://www.buymeacoffee.com/ssilverman" title="Donate to this project using Buy Me a Coffee"><img src="https://img.shields.io/badge/buy%20me%20a%20coffee-donate-orange.svg" alt="Buy Me a Coffee donate button"></a>

# _QNEthernet_, an lwIP-Based Ethernet Library For Teensy 4.1

_Version: 0.7.0-snapshot_

The _QNEthernet_ library provides Arduino-like `Ethernet` functionality for the
Teensy 4.1. While it is mostly the same, there are a few key differences that
don't quite make it a drop-in replacement.

The low-level code is based on code by Paul Stoffregen, here:\
https://github.com/PaulStoffregen/teensy41_ethernet

All the Teensy stuff is under the MIT license, so I'm making this library under
the MIT license as well. Please see the _lwip-info/_ directory for the info
files provided with the lwIP release.

## Differences, assumptions, and notes

This library mostly follows the Arduino Ethernet API, with these differences
and notes:
* Include the `QNEthernet.h` header instead of `Ethernet.h`.
* Ethernet `loop()` is called from `yield()`. The functions that wait for
  timeouts rely on this. This also means that you must use `delay`, `yield()`,
  or `Ethernet.loop()` when waiting on conditions; waiting without calling these
  functions will cause the TCP/IP stack to never refresh. Note that many of the
  I/O functions call `yield()` so that there's less burden on the calling code.
* `EthernetServer` write functions always return the write size requested. This
  is because different clients may behave differently.
* The examples in https://www.arduino.cc/en/Reference/EthernetServerAccept and
  https://www.arduino.cc/en/Reference/IfEthernetClient directly contradict each
  other with regard to what `operator bool()` means in `EthernetClient`. The
  first example uses it as "already connected", while the second uses it as
  "available to connect". "Connected" is the chosen concept, but different from
  `connected()` in that it doesn't check for unread data.
* The following `Ethernet` functions are not defined in the Arduino API:
  * `begin()`
  * `begin(ipaddr, netmask, gw)`
  * `end()`
  * `mtu()`
  * `linkState()`
  * `linkSpeed()`
  * `waitForLocalIP()`
  * The callback-adding functions:
    * `onLinkState(cb)`
    * `onAddressChanged(cb)`
* `EthernetServer::end()` is not in the Arduino API.
* All the Arduino-defined `Ethernet.begin` functions that use the MAC address
  are deprecated.
* The following `Ethernet` functions are deprecated and do nothing or
  return zero:
  * `hardwareStatus()`: returns zero (an `int`) and not `EthernetHardwareStatus`
  * `init(uint8_t sspin)`
  * `maintain()`: returns zero
  * `setMACAddress(uint8_t mac[6])`
  * `setRetransmissionCount(uint8_t number)`
  * `setRetransmissionTimeout(uint16_t milliseconds)`
* The following `Ethernet` functions are deprecated but call
  something equivalent:
  * `MACAddress(uint8_t mac[6])`
  * `setDnsServerIP(const IPAddress &dnsServerIP)`
* The system starts with the Teensy's actual MAC address. If you want to use
  that address with the deprecated API, you can collect it with
  `Ethernet.macAddress` and then pass it to one of the deprecated `begin`
  functions.
* All classes and objects are in the `qindesign::network` namespace. This means
  you'll need to fully qualify any types. To avoid this, you could utilize a `using` directive:
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
* The main include file, `QNEthernet.h`, in addition to including the `Ethernet`
  instance, also includes the headers for `EthernetClient`, `EthernetServer`,
  and `EthernetUDP`.
* All four address-setting functions (IP, subnet mask, gateway, DNS) do nothing
  unless the system has been initialized.

## How to run

This library works with both PlatformIO and Arduino. To use it with Arduino,
here are a few steps to follow:

1. Change `#include <Ethernet.h>` to `#include <QNEthernet.h>`. Note that this
   include already includes the header for EthernetUDP, so you can remove any
   `#include <EthernetUdp.h>`.
2. Just below that, add: `using namespace qindesign::network;`
3. You likely don't want or need to set/choose your own MAC address, so just
   call `Ethernet.begin()` with no arguments. This version uses DHCP. The
   three-argument version (IP, subnet mask, gateway) sets those parameters
   instead of using DHCP. If you really want to set your own MAC address, for
   now, consult the code.
4. There is an `Ethernet.waitForLocalIP(timeout)` convenience function that can
   be used to wait for DHCP to supply an address. Try 10 seconds (10000 ms) and
   see if that works for you.
5. `Ethernet.hardwareStatus()` always returns zero.
6. Most other things should be the same.

## A survey of how connections (aka `EthernetClient`) work

Hopefully this disambiguates some details about what each function does:
1. `connected()`: Returns whether connected OR data is still available
   (or both).
2. `operator bool`: Returns whether connected (at least in _QNEthernet_).
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

## mDNS services

It's possible to register mDNS services. Some notes:
* Similar to `Ethernet`, there is a global `MDNS` object. It too is in the
  `qindesign::network` namespace.
* Registered services disappear after the TTL (currently set to 120 seconds),
  and sometimes earlier. I'm not sure what the cause is yet. (Not receiving
  multicast?) You might find yourself needing to re-announce things every _TTL_
  seconds. This is the purpose of the `MDNS.ttl()` and `MDNS.announce()`
  functions.
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
* After `enet_deinit()` and then restarting, DHCP isn't able to get an address.
* Understand why mDNS requires re-announcements.

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

Copyright (c) 2021 Shawn Silverman

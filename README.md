# _QNEthernet_, a lwIP-Based Ethernet Library For Teensy 4.1

_Version: 0.4.0-snapshot_

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
  timeouts rely on this. This also means that you must use `delay` or `yield()`
  when waiting on conditions; waiting without calling these functions will
  cause the TCP/IP stack to never refresh.
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
  * `mtu()`
  * `waitForLocalIP()`
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
* `Ethernet.linkStatus()` unabashedly returns a `bool` and not
  `EthernetLinkStatus`. You'll have to create your own mapping to an enum of
  this name if you want to use it in the Arduino fashion.
* Files that configure lwIP for our system:
  * src/sys_arch.c
  * src/lwipopts.h &larr; Use this one for tuning
  * src/arch/cc.h
* The main include file, `QNEthernet.h`, in addition to including the `Ethernet`
  instance, also includes the headers for `EthernetClient`, `EthernetServer`,
  and `EthernetUDP`.

## How to run

This library works with both PlatformIO and Arduino. To use it with Arduino,
here are a few steps to follow:

1. Change `#include <Ethernet.h>` to `#include <QNEthernet.h>`. Note that this
   include already includes the header for EthernetUDP, so you can remove any
   `#include <EthernetUdp.h>`.
2. Just below that, add: `using namespace qindesign::network;`
3. You likely don't want or need to set/choose your own MAC address, so just
   call Ethernet.begin() with no arguments. This version uses DHCP. The three-
   argument version (IP, subnet mask, gateway) sets those parameters instead of
   using DHCP. If you really want to set your own MAC address, for now, consult
   the code.
4. There is an `Ethernet.waitForLocalIP(timeout)` convenience function that can
   be used to wait for DHCP to supply an address. Try 10 seconds (10000 ms) and
   see if that works for you.
5. `Ethernet.hardwareStatus()` always returns zero and `Ethernet.linkStatus()`
   returns a `bool` (i.e. not that `EthernetLinkStatus` enum).
6. Most other things should be the same.

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

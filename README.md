# lwIP-Based Ethernet Library For Teensy 4.1

_Version: 0.1.0_

The QNEthernet library provides Arduino-like `Ethernet` functionality for the
Teensy 4.1. While it is mostly the same, there are a few key differences that
don't make it a drop-in replacement.

The low-level code was based on code by Paul Stoffregen, here:\
https://github.com/PaulStoffregen/teensy41_ethernet

All the Teensy stuff is under the MIT license, so I'm making this library under
the MIT license as well.

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
  other with regard to what `operator bool()` means. The first example uses it
  as "already connected", while the second uses it as "available to connect".
* `EthernetClient` can't be copied, only moved, for example with `std::move`.
  If you're getting compiler errors, try this. For example:
  ```c++
  qn::EthernetClient client = server.accept();
  anotherClient = std::move(client);
  ```
  Briefly, the reason behind this is because the connections are managed by the
  client code and not the server code. All the management would need to be done
  in the server code, but this would be in service to making this work with the
  Arduino-style API and I don't love that API and am still thinking about
  underlying design choices.
* All the Arduino-defined `Ethernet.begin` functions that use the MAC address
  are deprecated.
* The following `Ethernet` functions are deprecated and do nothing or
  return zero:
  * `hardwareStatus()`: returns zero
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
* All the classes are in the `qindesign::network` namespace. This means you'll
  need to fully qualify any types. To avoid this, you could utilize a
  `using` directive:
  ```c++
  using namespace qindesign::network;
  EthernetUDP udp;
  ```
  However, this pollutes the current namespace. An alternative is to choose
  something shorter. For example:
  ```c++
  namespace qn = qindesign::network;
  qn::EthernetUDP udp;
  ```
* `Ethernet.linkStatus()` unabashedly returns a `bool` and not
  `EthernetLinkStatus`. You'll have to create your own mapping to an enum of
  this name if you want to use it in the Arduino fashion.
* Files added to lwIP:
  * sys_arch.c
  * lwip/src/include/lwipopts.h &larr; Use this one for tuning
  * lwip/src/include/arch/cc.h
* The main include file, `QNEthernet.h`, in addition to including the `Ethernet`
  instance, also includes the headers for `EthernetClient`, `EthernetServer`,
  and `EthernetUDP`.

## How to run

This library is by no means complete. In fact, I don't think it currently works
with the Arduino IDE. You must use PlatformIO for now.

## Other notes

I'm not 100% percent certain where this library will go, but I want it to be
amazing. It's meant to be an alternative to the NativeEthernet/FNET library,
using a different underlying TCP/IP stack.

I'm also not settled on the name.

Input is welcome.

## To do

* Tune lwIP.
* There's a few hoops the code has to jump through to make client connections
  both standalone and server-managed. I'd like to fix this up and make
  it cleaner.
* See if there's a way to improve DHCP time.
* A better API design than the Arduino-defined API.

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

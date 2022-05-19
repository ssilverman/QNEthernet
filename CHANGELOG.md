# Changelog for the _QNEthernet_ Library

This document details the changes between each release.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
* Added a way to enable promiscuous mode: define the
  `QNETHERNET_PROMISCUOUS_MODE` macro.
* Added a way to remove all the mDNS code: set `LWIP_MDNS_RESPONDER` to `0`
  in `lwipopts.h`.
* Added support for ".local" name lookups.
* Added the ability, in the mDNS client, to specify a service name that's
  different from the host name.

### Changed
* Moved CRC-32 lookup table to PROGMEM.

### Removed
* Removed some unneeded network interfaces:
  * IEEE 802.1D MAC Bridge
  * 6LowPAN
  * PPP
  * SLIP
  * ZigBee Encapsulation Protocol

## [0.14.0]

### Added
* Added a `util::StdioPrint` class, a `Print` decorator for stdio output files.
  It routes `Print` functions to a specific `FILE*`. This exists mainly to make
  it easy to use `Printable` objects without needing a prior call to `fflush()`.
* Added `MDNSClass::maxServices()`.
* Added `operator==()` and `operator!=()` operators for `const IPAddress`. They
  are in the usual namespace. These allow `==` to be used with `const IPAddress`
  values without having to use `const_cast`, and also introduce the completely
  missing `!=` operator.
* Added a way to declare the `_write()` function as weak via a new
  `QNETHERNET_WEAK_WRITE` macro. Defining this macro will cause the function to
  be declared as weak.
* Implemented `EthernetFrameClass::availableForWrite()`.
* Added size limiting to `EthernetFrameClass` write functions.
* New `EthernetClass::waitForLink(timeout)` function that waits for a link to
  be detected.
* Added a way to allow or disallow receiving frames addressed to specific MAC
  addresses: `EthernetClass::setMACAddressAllowed(mac, flag)`

### Changed
* Updated `SNTPClient` example to use `EthernetUDP::send()` instead of
  `beginPacket()`/`write()`/`endPacket()`.
* Updated `PixelPusherServer` example to use the frame average for the
  update period.
* Implemented `EthernetHardwareStatus` enum for the deprecated
  `Ethernet.hardwareStatus()` function. This replaces the zero return value with
  the new non-zero `EthernetOtherHardware`.
* Cleaned up how internal IP addresses are used.
* Changed `_write()` (stdio) to do nothing if the requested length is zero
  because that's what `fwrite()` is specified to do.
* Updated examples to use new `operator!=()` for `IPAddress`.
* Moved lwIP's heap to RAM2 (DMAMEM).
* Updated `EthernetFrame`-related documentation to explain that the API doesn't
  receive any known Ethernet frame types, including IPv4, ARP, and IPv6
  (if enabled).
* Clarified in the examples that `Ethernet.macAddress()` retrieves, not sets.
* Changed `EthernetClass::setMACAddress(mac)` parameter to `const`.
* Moved CRC-32 lookup table to RAM2 (DMAMEM).
* Made const those functions which could be made const.
* Updated examples and README to consider listeners and their relationship with
  a static IP and link detection.

### Fixed

* Fixed `EthernetUDP::send()` function to take the host and port as arguments,
  per its description. There's now two of them: one that takes an `IPAddress`
  and another that takes a `char *` hostname.
* Fixed `enet_output_frame()` to correctly return `false` if Ethernet is
  not initialized.
* Fixed not being able to set the DNS server IP before starting Ethernet.
* Fixed raw frame API to consider any padding bytes.

## [0.13.0]

### Added
* `EthernetFrame` convenience functions that also write the header:
  * `beginFrame(dstAddr, srcAddr, typeOrLen)`
  * `beginVLANFrame(dstAddr, srcAddr, vlanInfo, typeOrLen)`
* `qindesign::network::util` Print utility functions. The `breakf` function
  parameter is used as the stopping condition in `writeFully()`.
  * `writeFully(Print &, buf, size, breakf = nullptr)`
  * `writeMagic(Print &, mac, breakf = nullptr)`
* `enet_deinit()` now gracefully stops any transmission in progress before
  shutting down Ethernet.
* `EthernetClass` functions:
  * `linkIsFullDuplex()`: Returns whether the link is full duplex (`true`) or
    half duplex (`false`).
  * `broadcastIP()`: Returns the broadcast IP address associated with the
    current local IP and subnet mask.
* Functions that return a pointer to the received data:
  * `EthernetUDP::data()`
  * `EthernetFrame::data()`
* `DNSClient::getServer(index)` function for retrieving a specific DNS
  server address.
* `EthernetUDP::localPort()`: Returns the port to which the socket is bound.
* Three new examples:
  1. `IPerfServer`
  2. `OSCPrinter`
  3. `PixelPusherServer`

### Changed
* The `EthernetClient::writeFully()` functions were changed to return the number
  of bytes actually written. These can break early if the connection was closed
  while attempting to send the bytes.
* Changed `EthernetClient::writeFully()` functions to use the new `writeFully()`
  Print utility function.
* Changed the `Ethernet` object to be a reference to a singleton. This matches
  how the `EthernetFrame` object works.
* Changed the `read(buf, len)` functions to allow a NULL buffer so that the
  caller can skip data without having to read into a buffer.
* Moved internal classes and structs into an "internal" namespace to avoid any
  potential contflicts with user declarations.

### Removed
* Removed IEEE 1588 initialization and timer read.

### Fixed
* Fixed `EthernetClient::availableForWrite()` to re-check the state after the
  call to `EthernetClass::loop()`.

## [0.12.0]

### Added
* Added a way to disable and enable Nagle's algorithm. The new functions are
  `EthernetClient::setNoDelay(flag)` and `isNoDelay()`.
* Implemented `EthernetServer::availableForWrite()` as the minimum availability
  of all the connections, or zero if there's no connections.
* New `AppWithListenersTemplate` example.
* Added `EthernetClass::operator bool()` for testing whether Ethernet
  is initialized.
* Added a new way to send and receive raw Ethernet frames. There's a new
  `EthernetFrame` instance (of `EthernetFrameClass`) that is used similarly
  to `EthernetUDP`.
* New `RawFrameMonitor` example.
* New `EthernetUDP::send(data, len)` function for sending a packet without
  having to use `beginPacket()`/`write()`/`endPacket()`. It causes
  less overhead.

### Changed
* Changed `EthernetUDP::flush()` to be a no-op.
* Reduced lwIP's `MEM_SIZE` to 16KiB from 24000.
* Split `MDNSClass::addService()` into two overloaded functions: one with three
  arguments and one with four. No more defaulted TXT record function parameter;
  the three-argument version calls the four-argument version with NULL for
  that function.
* Updated `keywords.txt`.
* Updated `SNTPClient` example: Removed unneeded includes, made the packet
  buffer a global variable, and added setting the RTC and time.
* Changed `EthernetClass::mtu()` to `static size_t`. It was non-static
  and `int`.
* Updated `enet_output_frame(frame, len)` to check if the system is initialized.

### Removed
* Removed `EthernetClass::sendRaw(frame, len)` because there's a new
  `EthernetFrame` API with a `send(frame, len)` function.

### Fixed
* Fixed the length check when sending raw Ethernet frames to exclude the FCS
  field. It checks that the length is in the range 60-1518 instead of 64-1522.
* Fixed `check_link_status()` to check if Ethernet is initialized before trying
  to access the PHY.

## [0.11.0]

### Added
* Implemented `EthernetClass::setMACAddress(mac)`.
* Added `EthernetServer::maxListeners()`, `EthernetClient::maxSockets()`, and
  `EthernetUDP::maxSockets()` so user code doesn't need to guess. These are
  `constexpr` functions that return the compile-time constants from the
  lwIP configuration.
* Added `EthernetServer::port()` for returning the server's port.
* Added `EthernetClass::setHostname(hostname)` and `hostname()` for setting and
  getting the DHCP client option 12 hostname.
* Added `EthernetClass::maxMulticastGroups()` `constexpr` function.
* Added a "Write immediacy" subsection to the README that addresses when data is
  sent over a connection. It's under the "How to write data to
  connections" section.

### Changed
* Changed the default DHCP client option 12 hostname to "teensy-lwip".

### Fixed
* Stop the DHCP client when restarting `Ethernet` (in `begin(ip, mask, gateway)`
  and `setMACAddress(mac)`) to ensure that a static IP won't get overwritten by
  any previously running DHCP client. This also obviates the need to call
  `Ethernet.end()` before re-calling `begin`.

## [0.10.0]

### Added
* Added a way to send raw Ethernet frames. The new function
  is `EthernetClass::sendRaw(frame, len)`.
* Added new sections to the README:
  1. "Sending raw Ethernet frames", and
  2. "How to implement VLAN tagging".
* Added calls to `loop()` in `EthernetClient::connected()`
  and `operator bool()`.
* Added `EthernetUDP::beginMulticast(ip, localPort, reuse)`, where `reuse`
  controls the SO_REUSEADDR socket option.
* Added `EthernetClass::joinGroup(ip)` and `leaveGroup(ip)` for joining and
  leaving a multicast group.
* Added a "How to use multicast" section to the README.

### Changed
* Changed `kMTU` type to be `size_t` everywhere.
* Added `stdPrint` as an `extern` variable to `QNEthernet.h` and moved it to the
  `qindesign::network` namespace.
* Changed transmit data buffers to be 64-byte aligned, for
  "optimal performance".\
  See: IMXRT1060RM_rev2.pdf, "Table 41-38. Enhanced transmit buffer descriptor
  field definitions", page 2186.
* Updated lwIP to v2.1.3.
* Changed `EthernetUDP::beginMulticast` to release resources if joining the
  group failed.
* Increased MEMP_NUM_IGMP_GROUP to 9 to allow 8 multicast groups.

### Removed
* Removed mention of the need to re-announce mDNS and adjusted the
  docs accordingly.

### Fixed
* Changed receive data buffers to be 64-byte aligned.\
  See: IMXRT1060RM_rev2.pdf, "Table 41-36. Receive buffer descriptor field
  definitions", page 2183.
* Changed TA value in ENET_MMFR register to 2, per the chip docs.
* Multicast reception now works. Had to set the ENET_GAUR and ENET_GALR
  registers appropriately.

## [0.9.0]

### Added
* Added example that uses `client.writeFully()` to the "How to write data to
  connections" README section.
* Added `EthernetClient::close()` for closing a connection without waiting. It's
  similar to `stop()`.
* Added `DNSClient` class for interfacing with lwIP's DNS functions.
* Added a "DNS" section to the README.

### Changed
* Renamed the "How to write data to clients" README section to "How to write
  data to connections".
* Increased the maximum number of UDP sockets to 8.
* Updated `EthernetClass`, `EthernetClient`, and `EthernetUDP` to use the new
  `DNSClient` class for DNS lookup and DNS server address setting.

## [0.8.0]

### Added
* Added a check that `Entropy` has already been initialized before calling
  `Entropy.Initialize()`.
* Added a "How to write data to clients" section to the README that addresses
  how to fully send data to clients.
* Added `EthernetClient::writeFully()` functions that might help address
  problems with fully writing data to clients.
* Added a new "Additional functions not in the Arduino API" section to
  the README.
* Added `EthernetClient::closeOutput()` for performing a half close on the
  client connection.

### Changed
* Updated the `ServerWithAddressListener` example. It's more complete and could
  be used as a rudimentary basis for a complete server program.
  1. Added a "Content-Type" header to the response,
  2. It now looks for an empty line before sending the response to the client,
  3. Added the ability to use a static IP,
  4. Added client and shutdown timeouts, and
  5. Added a list to the description at the top describing some additional
     things the program demonstrates.
* In `EthernetClass::end()`, moved setting the DNS to 0 to before DHCP is
  released. This ensures that any address-changed events happen after this.
  i.e. the DNS address will be 0 when an address-changed event happens.

## [0.7.0]

### Added
* The Boolean-valued link state is now `EthernetClass::linkState()`.
* Added a `_write()` definition so that `printf` works and sends its output to
  `Serial`. Parts of lwIP may use `printf`. This directs output to a new
  `Print *stdPrint` variable. It has a default of NULL, so there will be no
  output if not set by user code.
* Now powering down the PHY in `enet_deinit()`.
* Added calls to `loop()` in `EthernetServer::accept()` and `available()` to
  help avoid having to have the caller remember to call loop() if checking
  connectivity in a loop.
* Added a call to `end()` in the `QNMDNS` destructor.
* Added a new externally-available `Print *stdPrint` variable for `printf`
  output, both for lwIP and optionally for user code.
* Added the ability to set the SO_REUSEADDR socket option when listening on a
  port (both TCP and UDP).
* Added four examples and a "note on the examples" section in the README.
  1. FixedWidthServer
  2. LengthWidthServer
  3. ServerWithAddressListener
  4. SNTPClient

### Changed
* `EthernetClass::linkStatus()` now returns an `EthernetLinkStatus` enum. The
  Boolean version is now `EthernetClass::linkState()`.
* The `EthernetLinkStatus` enum is no longer marked as deprecated.
* Updated `EthernetClient` output functions to flush data when the send buffer
  is full and to always call `loop()` before returning. This should obviate the
  need to call `flush()` after writes and the need to call `loop()` if writing
  in a loop. (`flush()` is still useful, however, when you've finished sending a
  "section" of data.)
* Changed `EthernetUDP::parsePacket()` to always call `loop()`.

### Fixed
* Restarting `Ethernet` (via `begin()` or via `end()`/`begin()`) now works
  properly. DHCP can now re-acquire an IP address. Something's weird about
  `EventResponder`. It doesn't look like it's possible to `detach()` then
  `attach()`, or call `attach()` more than once.
* Fixed `QNMDNS` to only call `mdns_resp_init()` once. There's no corresponding
  "deinit" call.

## [0.6.0]

### Added
* Added a new "survey of how connections work" section to the README.
* Added low-level link receive error stats collection.
* Now calling `EthernetClass::loop()` in `EthernetUDP::parsePacket()` when it
  returns zero so that calls in a loop will move the stack forward.
* Added `EthernetLinkStatus` enum (marked as deprecated) for compatibility with
  the Arduino API. Note that `EthernetClass::linkStatus()` is staying as a
  `bool`; comparison with the enumerators will work correctly.
* Now sending a DHCPINFORM message to the network when using a manual
  IP configuration.

### Changed
* Changed `EthernetClass::loop()` to be `static`.
* Changed all the internal "`yield()` to move the stack along" calls to
  `EthernetClass::loop()`. Note that the calls within `while` loops in the
  external API functions were not changed.

### Fixed
* Fixed the driver to add and remove netif ext callback at the correct places.
  This solves the freeze problem when ending Ethernet, however when restarting,
  DHCP isn't able to get an IP address.

## [0.5.0]

### Added
* Added a "Code style" section to the README.
* Added link-status and address-changed callbacks.
* New `EthernetServer::end()` function to stop listening.
* Disabling the PLL before disabling the clock in `enet_deinit()`. This still
  doesn't solve the freeze problem when this function is called.
* New `EthernetClass::linkSpeed()` function, used as `Ethernet.linkSpeed()`.
  It returns the link speed in Mbps.

### Changed
* Changed all the delays to yields because `delay()` just loops and calls
  `yield()` anyway.
* No longer looping when checking the TCP send buffer in the client write
  functions. Instead, just using an `if`.
* The client now takes more opportunities to set the internal connection pointer
  to NULL, meaning the caller doesn't necessarily need to guarantee they call
  `stop()` if other I/O functions are being used to check state or to write,
  for example.
* Moved adding the netif callback to `enet_init()` so that the callback still
  gets an address-changed notification when setting a static address.

### Fixed
* Fixed client functions to also check for connected status.
* Fixed client `read()` and `peek()` to return -1 on no connection.
* Added potential flushing in `EthernetClient::availableForWrite()`. This keeps
  things moving along if it always would return zero.
* A listening server is now correctly added to the internal listening list. This
  fixes `EthernetServer::operator bool()`.

## [0.4.0]

### Added
* This CHANGELOG.
* Instructions in the README for how to use with Arduino.
* Added the ability to add TXT items to mDNS services.
* New `EthernetClass::waitForLocalIP(timeout)` function that waits for a
  DHCP-assigned address.
* Added the ability to re-announce mDNS services. This is useful to prevent the
  entries from disappearing.
* Added mDNS notes to the README.

### Changed
* Updated lwIP to v2.1.3-rc1.
* Moved global objects (`Ethernet` and `MDNS`) into the `qindesign::network`
  namespace.

### Fixed
* UDP multicast address check was checking the wrong byte.
* UDP multicast now IGMP joins the group.

## [0.3.0]

### Changed
* Updated lwIP to the "real" v2.1.2 release.
* The library now works with Arduino.

### Fixed
* `EthernetUDP::endPacket()` was using `nullptr` for the pbuf.
* `EthernetUDP::beginPacket()` was not setting the output port.

## [0.2.0]

### Added
* Small delays in `EthernetClient` output functions to allow data to flush. This
  allows user programs to avoid having to call `yield()` themselves.
* Flushing the output before closing the connection in `EthernetClient::stop()`.
* Global Arduino-style `MDNS` object and a rename of the class to `MDNSClass`.
* `yield()` calls in the `EthernetClient` input functions to allow user programs
  to avoid having to call `yield()` themselves.

### Changed
* Brought `Print::write` functions into scope for Client, Server, and UDP by
  using a `using Print::write` directive.
* New centralized connection management.

### Removed
* Removed all the atomic fences.

### Fixed
* Fixed `EthernetClass::begin()` return value; it was the opposite.

## [0.1.0]

### Added
* Initial release.

---

Copyright (c) 2021-2022 Shawn Silverman

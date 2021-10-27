# Changelog for the _QNEthernet_ Library

This document details the changes between each release.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.10.0]

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
* New `Ethernet::waitForLocalIP(timeout)` function that waits for a
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

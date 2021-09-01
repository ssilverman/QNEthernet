This document details the changes between each release.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.0]

### Added
* This CHANGELOG.
* Instructions in the README for how to use with Arduino.
* Added the ability to add TXT items to mDNS services.
* New `Ethernet::waitForLocalIP(timeout)` function that waits for a
  DHCP-assigned address.
* Added the ability to re-announce mDNS services. This is useful to prevent the
  entries from disappearing.

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
* Removed all the atomic fences.
* Brought `Print::write` functions into scope for Client, Server, and UDP by
  using a `using Print::write` directive.
* New centralized connection management.

### Fixed
* Fixed `EthernetClass::begin()` return value; it was the opposite.

## [0.1.0]

### Added
* Initial release.

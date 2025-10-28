# Changelog for the _QNEthernet_ Library

This document details the changes between each release.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
* Added a way to iterate over the internal TCP connection PCBs.
* Added calls to `Ethernet.loop()` in more places.
* Added setting `errno` to `ENETDOWN` in `EthernetClass` in more places where
  network initialization is checked.

### Changed
* Made `qnethernet_hal_get_system_mac_address(mac)` weak.
* Made `EthernetClass::hostByName(hostname, result)` `const`.
* Made `qnethernet_hal_get_system_mac_address(mac)` weak.
* Improved code style and structure.
* Changed `errno` value in `EthernetClass::joinGroup(ip)` if network is down to
  `ENETDOWN` from `ENOTCONN`.

## [0.32.0]

### Added
* Added `qnethernet_hal_micros()`.
* Added secure TCP initial sequence numbers (ISNs), enabled with the new
  `QNETHERNET_ENABLE_SECURE_TCP_ISN` macro. The default is enabled.

### Changed
* Changed some functions to take a `void*` instead of a `uint8_t*`.
* Changed definition of `TCP_MSS` to be based on `MTU` instead of being
  a constant.
* Updated _LinkWatcher_ example to only print detectable details.

### Fixed
* Restored automatic entropy initialization when including
  `qnethernet/security/RandomDevice.h`. Calling `qnethernet_hal_entropy()` or
  `qnethernet_hal_fill_entropy()` will generate random values again without a
  prior call to `qnethernet_hal_init_entropy()` or `RandomDevice::instance()`.
  This also affects lwIP's internal `LWIP_RAND()`.

## [0.31.0]

### Added
* Added `qnethernet_hal_fill_entropy(buf, size)` for filling a buffer with
  random values.
* Added `EthernetClient::setConnectionTimeoutEnabled(flag)` to enable or disable
  blocking with calls to `connect(...)` and `stop()`. This supersedes calls to
  `connectNoWait(...)` and `close()`. Also added `isConnectionTimeoutEnabled()`.
* Added templated versions of `util::writeFully()` and `util::writeMagic()` that
  use a break function that static_casts a given object to a `bool`.
* Added `EthernetClient::connecting()` for determining if the client is still in
  the process of connecting during a non-blocking connect.

### Changed
* Remove internal uses of `String` from `MDNSClass` and replace them with
  char arrays.
* Updated Mbed TLS version mentions to 2.28.9 from 2.28.8.
* Made `EthernetClient::isNoDelay()` `const`.
* Disallow compilation for Teensyduino < 1.59 because there's no support for
  casting a `const IPAddress` to a `uint32_t` or for equality-comparing them.
* Replaced `EthernetClient::writeFully(const char *, size_t)` and
  `writeFully(const uint8_t *, size_t)` with `writeFully(const void *, size_t)`.
* Updated the file structure to put most sources underneath _src/qnethernet/_.
* Changed `EthernetClient::setConnectionTimeout(timeout)` and
  `connectionTimeout()` to use 32-bit values.
* Renamed `qnethernet_hal_rand()` and `qnethernet_hal_init_rand()` to
  `qnethernet_hal_entropy()` and `qnethernet_hal_init_entropy()`, respectively.
* Changed the non-entropy version of the entropy functions in the HAL to use
  `std::minstd_rand` instead of `std::rand()` and `std::srand()`.
* Changed uses of "www.example.com" to "www.google.com" in the examples
  and tests.

### Fixed
* Now using `(void)` instead of `()` for all C function declarations because
  `()` doesn't mean "no parameters" in C.
* Fixed `driver_unsupported`'s `driver_init(void)` parameters to be empty.

## [0.30.1]

### Fixed
* Fixed wait-for-close for non-altcp connections.

## [0.30.0]

### Added
* Added a _SimpleHTTPClient_ example.
* Added a way to get the driver capabilities:
  `EthernetClass::driverCapabilities()` and `driver_get_capabilities(dc)`.
* Added a way to get the library version: `EthernetClass::libraryVersion()`.
* New `QNETHERNET_DO_LOOP_IN_YIELD` configuration macro for indicating that
  the library should attempt to hook into or override `yield()` to
  call `Ethernet.loop()`.
* New version of `receiveQueueSize()` in `EthernetUDP` and `EthernetFrame` that
  returns the number of unprocessed packets or frames.
* Added `droppedReceiveCount()` and `totalReceiveCount()` to `EthernetUDP`
  and `EthernetFrame`.
* Added `driver_set_link_speed(speed)` and `driver_set_link_full_duplex(flag)`.
* Added `EthernetClass::renewDHCP()`.
* Added `EthernetClass::interfaceName()`.
* Added `setOutgoingTTL(ttl)` and `outgoingTTL()` functions for modifying and
  accessing the TTL field, respectively, in the outgoing IP header, to
  `EthernetClient` and `EthernetUDP`.
* Added `EthernetUDP::receivedTTL()` for retrieving the TTL value of the last
  received packet.
* Added "Compatibility with other APIs" section to the README.

### Changed
* Separated setting the MAC address from driver initialization.
* Changed the netif name to "en0".
* Added macro-gated calls to `Ethernet.loop()` after any `yield()`s in case an
  overridden version doesn't call this. This affects:
  * `DNSClient::getHostByName()`
  * `EthernetClass::waitForLink()`
  * `EthernetClass::waitForLocalIP()`
  * `EthernetClient::connect()`
  * `EthernetClient::stop()`
* Updated example `yield()` implementation notes for
  non-EventResponder versions.
* Changed `setReceiveQueueSize(size)` to `setReceiveQueueCapacity(capacity)`
  and `receiveQueueSize()` to `receiveQueueCapacity()` in both `EthernetUDP`
  and `EthernetFrame`.
* Updated `QNETHERNET_ENABLE_RAW_FRAME_LOOPBACK` behaviour to also check for
  the broadcast MAC addresses.
* Added `FLASHMEM` to some driver functions.
* Changed `EthernetClass` and `MDNSClass` `hostname()` function to return a
  `const char *` instead of a `String`.
* Now always setting the DNS in `EthernetClass::begin(ip, mask, gateway, dns)`,
  even if it's zero.
* Use rename instead of end-then-start when the netif has already been added,
  in `MDNSClass::begin(hostname)`.
* DHCP is started when just the IP address is the "any" address instead of all
  of the IP address, netmask, and gateway.
* `begin(ip, mask, gateway, dns)` now always sets the DNS address.
* `Ethernet.broadcastIP()` now returns 255.255.255.255 if Ethernet is
  not initialized.

### Removed
* Removed `EthernetClass::isLinkStateDetectable()` in favour of the
  driver capabilities.
* Removed `get_uint32(ip)` utility function because a `static_cast<uint32_t>()`
  is sufficient.
* Removed `const IPAddress` equality comparison operators because these exist in
  Teensyduino 1.59.

### Fixed
* Fixed iperf v2 tests by commenting out per-block settings compare.
* Fixed restarting the netif by also bringing the link down.
* Fixed a bug related to closing a TCP socket when using altcp.

## [0.29.1]

### Fixed
* Fixed being able to process more than one incoming frame in a row in
  the driver.

## [0.29.0]

### Added
* Added protected access to the internal `std::FILE*` stream in the
  `StdioPrint` utility class.
* Added more unit tests:
  * test_ethernet:
    * test_server_zero_port
    * test_server_accept
    * test_server_construct_int_port
* Added `printf` format string checking for `Print`-derived classes. As of this
  writing, Teensyduino (1.59) and other platforms don't do compiler checking
  for `Print::printf`.
* Added more support for `errno`. Appropriate functions will set this after
  encountering an error.
* Added tests for the Arduino-API `begin(...)` functions.
* Added a way to utilize an externally-defined driver.
* Added `driver_is_mac_settable()` to the driver API. This checks if the MAC
  address can be set.

### Changed
* Updated and improved _PixelPusherServer_ example.
* Call `qnethernet_hal_get_system_mac_address(mac)` in the unsupported driver's
  `driver_get_system_mac(mac)` implementation. This enables MAC address
  retrieval for more platforms when communication isn't needed; Teensy 4.0,
  for example.
* Turned the internal MAC address into an optional and simplified the `Ethernet`
  constructor. This change should make it easier to initialze a MAC address from
  a custom driver.
* Changed Arduino-API non-DHCP `begin(...)` functions to return `bool`.
* Improved driver logic so that lwIP options can be included in the
  driver headers.
* Improved `OSCPrinter` example to use the UDP data directly.
* Renamed `driver_set_mac_address_allowed()` to
  `driver_set_incoming_mac_address_allowed()`.
* Changed `driver_proc_input(netif)` to return a `pbuf*`.
* Made MTU and max-frame-len dynamic to support drivers that don't know the
  values at compile time.

### Fixed
* Fixed `EthernetServer::port()` to return the system-chosen port if a zero
  value was specified.
* Fixed `EthernetUDP::stop()` to leave any multicast group joined when starting
  to listen on a multicast address.
* Fixed MAC address restore if an Arduino-API non-DHCP `begin(...)` call fails.
* Fixed `EthernetClient::read()` and `peek()` to return -1 instead of 0 when
  there's no internal state.
* Properly initializing multicast filtering so that it happens before
  `igmp_start()`, which sets up the all-systems group.

## [0.28.0]

### Added
* Added raw frame loopback support and a `QNETHERNET_ENABLE_RAW_FRAME_LOOPBACK`
  macro to enable.
* Added a fourth step to the _MbedTLSDemo_ example instructions: modify
  the config.
* New `EthernetFrameClass` functions: `destinationMAC()`, `sourceMAC()`,
  `etherTypeOrLength()`, and `payload()`.
* Consolidated all the hardware abstraction layer (HAL) functions into one
  place: `qnethernet_hal.cpp`.
* New `NullPrint` and `PrintDecorator` utility classes in the
  `qindesign::network::util` namespace.
* Added `driver_is_link_state_detectable()` function to the driver. This is for
  detecting whether the hardware is able to read the link state.
* Added `EthernetClass::isLinkStateDetectable()` to detect whether the driver is
  capable of detecting link state.
* Added `setOutgoingDiffServ(ds)` and `outgoingDiffServ()` functions for
  modifying and accessing the differentiated services (DiffServ) field,
  respectively, in the outgoing IP header, to `EthernetClient`
  and `EthernetUDP`.
* Added `EthernetUDP::receivedDiffServ()` for retrieving the DiffServ value of
  the last received packet.
* Added `EthernetFrameClass::clear()` for clearing the outgoing and
  incoming buffers.

### Changed
* Updated the Mbed TLS version in the README and comments to 2.28.8
  (was 2.28.6).
* Updated `mbedtls_hardware_poll()` in _MbedTLSDemo_ example for
  other platforms.
* Renamed `QNETHERNET_ENABLE_CUSTOM_WRITE` to `QNETHERNET_CUSTOM_WRITE`.
* Improved the _PixelPusherServer_ example.
* The address-changed callback is now called for independent IP address,
  netmask, and gateway changes.
* Improved link function documentation for `linkStatus()`, `linkState()`,
  and `isLinkStateDetectable()`.
* Updated `EthernetClient::setNoDelay(flag)` to return whether successful.
* Add another 2 to `MEMP_NUM_SYS_TIMEOUT` option for mDNS, for a total of an
  additional 8. Timeout exhaustion was still observed with 6. Why 8 and not 7:
  * https://lists.nongnu.org/archive/html/lwip-users/2024-05/msg00000.html
  * https://savannah.nongnu.org/patch/?9523#comment18
* Updated `EthernetClient::connect()` to return a Boolean value. (The function
  signatures don't change; they still return an `int`.) This matches the new
  definition at
  [Ethernet - client.connect()](https://www.arduino.cc/reference/en/libraries/ethernet/client.connect/).
* Changed `EthernetClient::connectNoWait()` return types to `bool`.

### Fixed
* Improved marking of unused parameters.
* Fixed up use of `__has_include()`, per:
  [__has_include (The C Preprocessor)](https://gcc.gnu.org/onlinedocs/cpp/_005f_005fhas_005finclude.html)
* In mDNS, ensure there's at least an empty TXT record, otherwise the SRV record
  doesn't appear.
* Make the alternative `yield()` implementation `extern "C"`.
* Fixed `EthernetUDP` data copy for zero-length outgoing packets. `pbuf_take()`
  considers NULL data an error, so only copy the data if the packet's size is
  not zero.

## [0.27.0]

### Added
* New `QNETHERNET_FLUSH_AFTER_WRITE` configuration macro for flushing after
  every call to `EthernetClient::write()`. This may reduce TCP efficiency.
* Added a W5500 driver.
* Added a new `EthernetHardwareStatus::EthernetTeensy41` enum value.
* New `QNETHERNET_DEFAULT_HOSTNAME` option.

### Changed
* Made it easier to add other low-level drivers.
* Redesigned the driver interface.
* Un-deprecated `EthernetClass::begin(mac, timeout)` and
  `begin(mac, ip, dns, gateway, subnet)`.
* Un-deprecated `EthernetClass::init(sspin)` and added a driver function for
  setting the chip select pin. The type of `sspin` was also changed to `int`.
* Removed dependency on `elapsedMillis`. This might help with compiling for
  other platforms.
* Made the library easier to compile for other platforms.
* Replaced all `#define` guards with `#pragma once`.
* All `QNETHERNET_*` configuration macros can now be defined in a new
  `qnethernet_opts.h` file in addition to the project build.
* Removed test dependencies on Teensy-specific things.
* Renamed _t41_ driver source files to use _teensy41_ in the name instead.
* Changed return type of `EthernetUDP::beginWithReuse()` and
  `beginMulticastWithReuse()` to `bool`.
* Using better randomness for `LWIP_RAND()`.
* Changed attributes to use the C++ style.
* Changed the default hostname to "qnethernet-lwip".
* Renamed `enet_get_mac(mac)` to `enet_get_system_mac(mac)`.

### Fixed
* Added missing `hostByName` to keywords.txt.
* Fixed `enet_output_frame()` to return false if there's no output buffer.
* Fixed `EthernetClient::write(buf, size)` to re-check the state after a call
  to `loop()`.
* Fixed `util::writeMagic()` `mac` parameter to be `const`.
* Fixed `test_ethernet`'s `tearDown()` to remove listeners before calling
  `Ethernet.end()`. This ensures no out-of-scope variables are accessed.
* Updated `StdioPrint` to not use `errno` because stdio doesn't set this.

## [0.26.0]

### Added
* Added `EthernetClient::localIP()`.
* Added `EthernetClass::hostByName(hostname, ip)` convenience function.
* Added `EthernetClass::setDNSServerIP(index, ip)` and `dnsServerIP(index)`.
* Added some support for Mbed TLS v2.x.x. There's four new adapter functions
  for assisting integration (see _src/altcp_tls_adapter.cpp_), included if the
  `QNETHERNET_ALTCP_TLS_ADAPTER` option is set:
  1. `qnethernet_altcp_is_tls`
  2. `qnethernet_altcp_tls_client_cert`
  3. `qnethernet_altcp_tls_server_cert_count`
  4. `qnethernet_altcp_tls_server_cert`
* Added _MbedTLSDemo_ example.
* Added printing the message size in _LengthWidthServer_ example.
* Added `QNETHERNET_ENABLE_ALTCP_DEFAULT_FUNCTIONS`-gated default
  implementations of the altcp interface functions.
* Added `EthernetClass::macAddress()` for returning a pointer to the current
  MAC address.

### Changed
* Updated lwIP to the latest master (5e3268cf).
* Made the driver non-blocking:
  1. TX: if there's no available buffer descriptors (returns ERR_WOULDBLOCK)
  2. Link checks via MDIO
* Updated the tests:
  * Added a 10s connection timeout to `test_client_addr_info()`
  * Added SNTP retries to `test_udp()`
  * Updated and added some messages
* Made `MDNSClass::Service::operator==()` `const`.
* Completely revamped PHY and pin initialization.
* Gated PHY and Ethernet shutdown in `EthernetClass::end()` with a macro; the
  default behaviour is to not execute these blocks. This and the previous are
  part of the quest to figure out why performance drops off a cliff when
  Ethernet is restarted via first calling `end()`.
* Changed return type of `qnethernet_get_allocator` to `bool`.
* Renamed `qnethernet_get_allocator` and `qnethernet_free_allocator` to
  `qnetheret_altcp_get_allocator` and `qnethernet_altcp_free_allocator`,
  respectively.
* Changed `qnethernet_get_allocator` and `qnethernet_free_allocator` `allocator`
  parameter to be a reference.
* Renamed `QNETHERNET_MEMORY_IN_RAM1` to `QNETHERNET_LWIP_MEMORY_IN_RAM1`.
* Updated the _AltcpTemplate_ example.
* Fixed `IPAddress`-related build problems with the new Teensyduino 1.54-beta4.
* Updated the _RawFrameMonitor_ example with information about how to
  disable DHCP.
* Disabled waiting in `EthernetClient::close()` for altcp clients because it's
  not defined.
* Changed configuration macro checks to use value instead of definedness.

### Fixed
* Fixed a `printf` conversion specifier in the _RandomNumbers_ example. This was
  causing a compile warning.
* Fixed location of STATIC_INIT_DECL() for `RandomDevice` by putting it into
  the header. It needs to be in a place where users of the class see it.
* Fixed `EthernetClass::setMACAddress()` for when the interface is up.

## [0.25.0]

### Added
* New "Heap memory use" section in the README that discusses memory
  allocation functions.
* New _RandomNumbers_ example.
* Added a README subsection that talks about the `RandomDevice` class.
* Added `RandomDevice::instance()`.
* Added `EthernetClient::status()` for getting the TCP connection state.

### Changed
* Enabled the `MEM_LIBC_MALLOC` option by default to make use of the system-
  defined malloc functions instead of the lwIP-defined ones.
* Moved around where the netif gets its address set.
* Changed the license to "AGPL-3.0-or-later".
* Made `RandomDevice` constructor and destructor private.
* Improved unit tests.
* Updated lwIP to v2.2.0.

### Removed
* Removed the `extern qindesign::security::RandomDevice randomDevice` instance
  from _QNEthernet.h_ because the way to access the device now is via its
  `instance()` function (the constructor is also private now). (It had been
  added in v0.23.0.)

### Fixed
* Fixed `enet_init()` to always initialize the internal MAC address on
  first init.
* Fixed execution error when running _main.cpp_ (`MAIN_TEST_PROGRAM` macro
  defined) by removing `build_type = debug` from _platformio.ini_.
* Fixed static initialization order for `Ethernet`, `EthernetFrame`, and `MDNS`
  singletons by using the Nifty Counter idiom.

## [0.24.0]

### Added
* The new `QNETHERNET_MEMORY_IN_RAM1` configuration macro indicates that
  lwIP-declared memory should go into RAM1.
* New `EthernetFrameClass::receiveQueueSize()` function.
* New `EthernetUDP::receiveQueueSize()` and `setReceiveQueueSize(size)`
  functions.
* Enabled external definition of macros `LWIP_NETIF_LOOPBACK` and
  `LWIP_LOOPBACK_MAX_PBUFS`.
* Sprinkled some more `Ethernet.loop()` calls where pcb and pbuf
  allocations fail.
* Added `EthernetClass::setLinkState(flag)` for manually setting the link state
  when a link is needed, such as when using the loopback feature. Network
  operations will usually fail unless there's a link.
* Added more unit tests:
  * test_ethernet:
    * test_setLinkState
    * test_udp_receive_queueing
    * test_udp_receive_timestamp
    * test_udp_state
    * test_client_connectNoWait
    * test_client_timeout
    * test_client_state
    * test_server_state
    * test_other_state
  * test_entropy:
    * test_randomDevice
* Added commented-out `LWIP_STATS_LARGE` option to _lwipopts.h_.

### Changed
* Changed memory declaration macro, `LWIP_DECLARE_MEMORY_ALIGNED()`, to use the
  `MEM_ALIGNMENT` value.
* Now calling `shrink_to_fit()` on the UDP and Ethernet frame queues when
  changing their size.
* Add 6 to `MEMP_NUM_SYS_TIMEOUT` option for mDNS instead of 5. Timeout
  exhaustion was still observed with 5.
* There's now a single _lwip_driver.h_ header for interfacing with the stack.
* Changed all `EthernetClass::begin(mac, ...)` functions to be consistent. If
  the MAC address is NULL then the MAC will be set to the internal one. Also,
  if starting Ethernet fails, the MAC address will not be changed.
* Improved Ethernet tests to do proper object destruction when tests fail. The
  Unity test framework makes use of _longjmp_ for failing tests, and that
  doesn't work well with object destructors.
* Unit test updates.
* Made single-argument `EthernetClass` and `EthernetClient` constructors
  explicit.

### Fixed
* Now using the correct name when adding an mDNS service.
* Pre-reserving memory for raw frames and UDP packets prematurely exhausts the
  heap when a larger number of them are reserved in the queue. These buffers are
  no longer reserved; they only grow appropriately when data comes in.
* Fixed closing `EthernetClient`s to remove the connection state if not already
  connected. Restarting an `EthernetClient` via one of the `connectXXX()`
  functions calls `close()` first. If there was no connection, then closing
  never removed the internal connection object, causing a leak.
* Fixed `Ethernet.loop()` to also poll the netif if loopback is enabled. This
  allows loopback to work.
* `EthernetServer::end()` now sets the port to -1.

## [0.23.0]

### Added
* Added `qindesign::security::RandomDevice`, a class that conforms to the
  _UniformRandomBitGenerator_ C++ named requirement. This makes it a little
  easier to generate platform-independent entropy.
* `EthernetUDP::receivedTimestamp()` for retrieving a packet's approximate
  arrival time.
* Added a "`writeFully()` with more break conditions" subsection to the README.
* `EthernetFrame::receivedTimestamp()` for retrieving a frame's approximate
  arrival time.
* `EthernetClient::connectionTimeout()` for getting the current timeout value.

### Changed
* Changed `EthernetUDP::localPort()` to be `const`.
* Changed `entropy_random_range()` to return zero if `EAGAIN`.
* Changed `entropy_random_range()` to use Daniel Lemire's nearly-
  divisionless algorithm.
* Updated lwIP to v2.2.0-rc1.
* Made `StdioPrint` single-argument constructor `explicit`.
* Updated `EthernetClass::linkStatus()` to return `EthernetLinkStatus::Unknown`
  if the hardware hasn't yet been probed.
* Updated `AltcpTemplate` example to print proxy information.
* Updated `EthernetUDP::availableForWrite()` to return the amount remaining
  before hitting the maximum possible payload size.
* Add 5 to `MEMP_NUM_SYS_TIMEOUT` option for mDNS instead of 1.

### Removed
* Removed TTL concept from `MDNSClass`. This enables it to compile with the
  latest lwIP.

### Fixed
* Fixed `SNTP_SET_SYSTEM_TIME_US(sec, us)` definition to set the RTC directly
  because `settimeofday()` doesn't exist here.
* Fixed `AltcpTemplate` example so that it compiles when `LWIP_ALTCP` isn't set.
* Handle nested `altcp_pcb`s when TCP-listening with reuse.

## [0.22.0]

### Added
* `EthernetClass::setDHCPEnabled(flag)` enables or disables the DHCP client. May
  be called either before or after Ethernet has started.
* `EthernetClass::isDHCPEnabled()` returns whether the DHCP client is enabled.
  Valid whether Ethernet has been started or not.
* New `LinkWatcher` example.
* Added support for building for unsupported boards via a new bare lwIP driver.

### Changed
* Limit the number of times `enet_proc_input()` can loop to twice the ring size.
* Limit UDP output size to the maximum possible (65535 - 28(total header)).
* It's now possible to know when adding or removing a MAC address filter failed.
* Make it possible to disable and exclude DHCP, DNS, IGMP, TCP, and UDP.
* Changed `EthernetClass::setMACAddress(mac)` to use the built-in MAC address if
  the given array is NULL.
* Changed `EthernetClass::begin(mac)` to wait for an IP address. The default is
  a 60-second timeout. There's also a new, optional, `timeout` parameter for
  specifying that timeout. This change makes the API match the Arduino
  Ethernet API.
* Renamed `enet_getmac(mac)` to `enet_get_mac(mac)`.
* Better NULL argument checking.
* Simplified `ServerWithListeners` example.
* Changed `enet_init(...)` to return a `bool` for detecting init. failure.

### Fixed
* Fixed how `EthernetClient` functions work when there's a pending connect
  triggered by `connectNoWait()`.
* Fixed how raw frame size limits are checked. Padding is already handled by
  the MAC.
* Fixed compilation if `LWIP_IGMP` is disabled, including making
  `LWIP_MDNS_RESPONDER` dependent on `LWIP_IGMP` (in addition to `LWIP_UDP`).
* Improved `trng_is_started()` by adding an additional check for the "OK to
  stop" bit. It now works at system startup if the clock is already running.

## [0.21.0]

### Added
* Added `EthernetClass::linkIsCrossover()` for checking if a crossover cable
  is detected.
* Added entropy-based random number functions, `entropy_random()` and
  `entropy_random_range(range)`. The second uses an unbiased algorithm.

### Changed
* Renamed TRNG tests to test_entropy.
* Added calling file, line, and function information to
  `LWIP_ASSERT_CORE_LOCKED()`.
* Un-deprecated `EthernetClass::MACAddress(mac)` and `setDnsServerIP(ip)`.
* Optimized byte-swapping by using GCC built-ins.

### Fixed
* Reset the PHY in a much more conservative way. Hopefully this helps with
  restarting Ethernet causing packets to not be received.
* Fixed dhcp struct assignment to be done each time the netif is added. This
  addresses `netif_add()` clearing all the client data.
* Fixed `LWIP_PLATFORM_ASSERT()` to flush stdout's underlying `Print` object.
  This ensures all output for an assertion failure gets sent out before the call
  to `abort()`.
* Fixed the link status values changing after setting the link up.

## [0.20.0]

### Added
* Added `EthernetFrameClass::size()`, for parity with `EthernetUDP`.
* Added internal entropy functions to remove the _Entropy_ library dependency.
* New `QNETHERNET_ENABLE_CUSTOM_WRITE` macro for enabling the inclusion of the
  expanded `stdio` output behaviour.
* Added a bunch of unit tests. These use the Unity test framework from
  within PlatformIO.
* Added sections to the README that describe how to configure compiler options
  for both the Arduino IDE and PlatformIO.
* Added an `AltcpTemplate` example.
* Added a `BroadcastChat` example that implements a simple chat over UDP.

### Changed
* Updated `StdioPrint` adapter to use errors better. `errno` is set for the
  "write error" value and the stdio error state is cleared appropriately when
  the functions detect that "write error" is back to zero.
* Changed default `stdio` output behaviour to use the new system default. (This
  exists as of Teensyduino 1.58-beta4.) See: `QNETHERNET_ENABLE_CUSTOM_WRITE`.
* Added a timeout parameter to the callback version of
  `DNSClient::getHostByName()`. This helps prevent any references from going out
  of scope before the callback is called.
* Changed `EthernetUDP::send()` functions back to returning a Boolean value.
* There's now `Print` objects for each of `stdout` and `stderr`: `stdoutPrint`
  and `stderrPrint`.
* Improved `RawFrameMonitor` and `SNTPClient` examples.
* Changed "tcp" calls to "altcp" calls so that it's easier to add things like
  TLS and proxy support. There's accompanying `qnethernet_get_allocator(...)`
  and `qnethernet_free_allocator(...)` functions that need to be defined by the
  application code if the `LWIP_ALTCP` option is enabled.

### Removed
* `QNETHERNET_WEAK_WRITE` macro in favour of the new way to enable the library's
  internal `_write()` definition. See: `QNETHERNET_ENABLE_CUSTOM_WRITE`.
* Removed `stdPrint` in favour of the new `stdoutPrint` and `stderrPrint`.
* Removed sending a DHCP INFORM message when setting a static IP. It seemed to
  interfere with any first subsequent DHCP requests.

### Fixed
* Fixed `EthernetClass::end()` to call `clearEvent()` before detaching the
  event responder.
* Fixed DNS client to be aware of lookup failures.
* Added set-no-address and link-down calls to `EthernetClass::end()` before
  bringing the interface down. This ensures all the callbacks are called.
* Fixed `EthernetUDP::parsePacket()` to also call `Ethernet.loop()` when there's
  no packet available.
* Increased PHY reset pulse and reset-to-MDIO times. Hopefully this fixes slow
  traffic after restarting the system via `Ethernet.end()`.

## [0.19.0]

### Added
* Added `Ethernet.loop()` calls to the `EthernetClient::write()` functions when
  the send buffer is full.
* Added Ethernet hardware detection to support (well, "non-support") the
  Teensy 4.1 NE.

### Changed
* Updated _lwipopts.h_ to examine `LWIP_MDNS_RESPONDER` when setting
  certain values.
* `Ethernet.loop()` calls are now attached/detached to/from _yield_ in
  `Ethernet.begin(...)`/`end()`.
* Improved pin configurations and comments.
* Disabled `LWIP_STATS` option by default. Saves a little memory.
* Updated Arduino Ethernet API links in _keywords.txt_.
* Only add 1 to `MEMP_NUM_SYS_TIMEOUT` option for mDNS instead of 3.
* Updated examples to use both address and link state on network changes. This
  accommodates when a static IP is used.
* Changed UDP and TCP PCB creation to use an appropriate `ip_addr_t` type
  instead of the unspecified default.
* Changed CRC-32 function for multicast lookup to not use a lookup table. This
  saves 1KiB of flash but makes the calculation about 4.7x slower but still in
  the microsecond range (~0.090µs -> ~0.42µs).
* Moved `EthernetClass`, `EthernetFrameClass`, and `MDNSClass` constructors and
  destructors to `FLASHMEM` (where possible). This saves a little RAM1 space.
* Moved lwIP's memory pools into 4-byte aligned `DMAMEM` (RAM2). This saves
  a lot of RAM1 space, about 27KiB with the current configuration.
* Changed the promiscuous mode macro name from `QNETHERNET_PROMISCUOUS_MODE`
  to `QNETHERNET_ENABLE_PROMISCUOUS_MODE`.
* Changed all the DHCP timeouts in the examples to 15 seconds.
* Changed `EthernetUDP::send()` functions to return an lwIP error code instead
  of a 1-or-0 Boolean value. Zero (`ERR_OK`) means no error. This makes it
  easier to diagnose any problems.

### Fixed
* Reverted how interrupts were being cleared to use assignment instead of OR'ing
  the bits. This seemed to fix an apparent freeze. (See this issue:
  https://github.com/ssilverman/QNEthernet/issues/26)
* Fixed a signedness comparison warning in `OSCPrinter` example.
* Addressed "extra" (`-Wextra`) and pedantic (`-Wpedantic`) warnings.

## [0.18.0]

### Added
* Added a "Notes on ordering and timing" section to the README.
* Added a README section that discusses `EthernetClient::connect()` and its
  return values.
* Added non-blocking TCP connection functions, `connectNoWait()`, that are
  equivalent to the `connect()` functions but don't wait for the connection
  to complete.
* Added `EthernetUDP::beginWithReuse()` and `beginMulticastWithReuse()`
  functions to replace the corresponding begin-with-_reuse_-parameter versions.
* Added printing the link speed and duplex in the IPerfServer example.
* Added an "Asynchronous use is not supported" section to the README.
* New `EthernetClass::onInterfaceStatus(callback)` and `interfaceStatus()`
  functions for tracking the network interface status.
* Added a check to ensure lwIP isn't called from an interrupt context.

### Changed
* Wrapped `LWIP_MDNS_RESPONDER` option in `lwipopts.h` with `#ifndef` and added
  it to the README.
* Made `EthernetClass::loop()` non-static.
* Changed serial output in examples to use CRLF line endings.
* Changed `EthernetClient::connect()` internals to call `close()` instead of
  `stop()` so that any cleanup doesn't block.
* Updated `EthernetClient::connect()` to return some of the error codes defined
  at [Ethernet - client.connect()](https://www.arduino.cc/reference/en/libraries/ethernet/client.connect/).
* Changed `EthernetServer::begin()`-with-Boolean-_reuse_-parameters to be named
  `beginWithReuse()`. This avoids too many overloads with mysterious
  Boolean arguments.
* Changed `EthernetServer::operator bool()` to be `const`.
* Changed `EthernetServer::end()` to return `void` instead of `bool`.
* Changed `MDNSClass::begin(hostname)` and `DNSClient::getHostByName()` to treat
  a NULL hostname as an error; they now explicitly return false in this case.
* Changed `MDNSClass::end()` to return `void` instead of `bool`.
* Changed examples that use `unsigned char` to use `uint8_t` in
  appropriate places.
* `EthernetUDP::begin` functions now call `stop()` if the socket is listening
  and the parameters have changed.
* `MDNSClass::begin(hostname)` now calls `end()` if the responder is running and
  the hostname changed.
* Changed both `EthernetServer` and `EthernetUDP` to disallow copying but
  allow moving.
* Changed raw frame support to be excluded by default. This changed the
  `QNETHERNET_DISABLE_RAW_FRAME_SUPPORT` macro to
  `QNETHERNET_ENABLE_RAW_FRAME_SUPPORT`.
* Changed `tcp_pcb` member accesses to use appropriate TCP API function calls.
  This fixes use of the altcp API.

### Removed
* `EthernetServer` and `EthernetUDP` begin functions that take a Boolean
  `reuse` parameter.

### Fixed
* `EthernetUDP::begin` functions now call `stop()` if there was a bind error.
* Fixed `EthernetClient::setNoDelay(flag)` to actually use the `flag` argument.
  The function was always setting the TCP flag, regardless of the value of
  the argument.
* Fixed printing unknown netif name characters in some debug messages.
* Fixed `EthernetClient::connect()` and `close()` operations to check the
  internal connection object for NULL across `yield()` calls.
* Fixed `lwip_strerr()` buffer size to include the potential sign.
* Don't close the TCP pcb on error since it's already been freed.

## [0.17.0]

### Added
* The library now, by default, puts the RX and TX buffers in RAM2 (DMAMEM).
  This behaviour can be overridden by defining the new
  `QNETHERNET_BUFFERS_IN_RAM1` macro.
* Added separate `stderr` output support with the new `stderrPrint` variable.
  If set to NULL, `stderr` defaults to the usual `stdPrint`.
* Added `MDNSClass::operator bool()` for determining whether mDNS has
  been started.
* Added `MDNSClass::restart()` for when a cable has been disconnected for a
  while and then reconnected.
* Added `EthernetFrameClass::setReceiveQueueSize(size)` for setting the receive
  queue size. This replaces the `QNETHERNET_FRAME_QUEUE_SIZE` macro.
* Added a way to disable raw frame support: define the new
  `QNETHERNET_DISABLE_RAW_FRAME_SUPPORT` macro.
* Added a "Complete list of features" section to the README.
* Added `MDNSClass::hostname()` for returning the hostname of the responder,
  if running.
* Added `EthernetUDP::operator bool()`.
* Added an already-started check to `MDNSClass`.
* New section in the README: "`operator bool()` and `explicit`". It addresses
  problems that may arise with `explicit operator bool()`.
* Added `EthernetClient::connectionId()` for identifying connections across
  possibly multiple `EthernetClient` objects.
* Added `EthernetClass::isDHCPActive()`.

### Changed
* Improved error code messages in `lwip_strerr(err)`. This is used when
  `LWIP_DEBUG` is defined.
* Now shutting down mDNS too in `EthernetClass::end()`.
* Now using overloads instead of default arguments in `EthernetClass`
  and `EthernetUDP`.
* Changed `EthernetClass` and `MDNSClass` `String` functions to take
  `const char *` instead.
* Made all `operator bool()` functions `explicit`.
  See: https://en.cppreference.com/w/cpp/language/implicit_conversion#The_safe_bool_problem
* `MDNSClass::removeService()` now returns `false` instead of `true` if mDNS has
  not been started.
* Enable definition of certain macros in `lwipopts.h` from the command line.
* Changed API uses of `unsigned char` to `uint8_t`, for consistency.

### Removed
* Removed the `QNETHERNET_FRAME_QUEUE_SIZE` macro and replaced it with
  `EthernetFrame.setReceiveQueueSize(size)`.

### Fixed
* Disallow `stdin` in `_write()`.

## [0.16.0]

### Added
* `EthernetUDP::size()`: Returns the total size of the received packet data.
* Added an optional "dns" parameter to the three-parameter Ethernet.begin() that
  defaults to unset. This ensures that the DNS server IP is set before the
  address-changed callback is called.
* Added configurable packet buffering to UDP reception with the new
  `EthernetUDP(queueSize)` constructor. The default and minimum queue size is 1.
* Added configurable frame buffering to raw frame reception with the new
  `QNETHERNET_FRAME_QUEUE_SIZE` macro. Its default is 1.
* Added a new "Configuration macros" section to the README that summarizes all
  the configuration macros.

### Changed
* Changed `EthernetUDP::parsePacket()` to return zero for empty packets and -1
  if nothing is available.

## [0.15.0]

### Added
* Added a way to enable promiscuous mode: define the
  `QNETHERNET_PROMISCUOUS_MODE` macro.
* Added a way to remove all the mDNS code: set `LWIP_MDNS_RESPONDER` to `0`
  in `lwipopts.h`.
* Added support for ".local" name lookups.
* Added the ability, in the mDNS client, to specify a service name that's
  different from the host name.
* Added `EthernetClient::abort()` for killing connections without going through
  the TCP close process.
* New sections in the README:
  * "How to change the number of sockets", and
  * "On connections that hang around after cable disconnect".
* An `EthernetServer` instance can now be created without setting the port.
  There are two new `begin()` functions for setting the port:
  * `begin(port)`
  * `begin(port, reuse)`

### Changed
* Moved CRC-32 lookup table to PROGMEM.
* Changed in `EthernetServer`:
  * `port()` returns an `int32_t` instead of a `uint16_t` so that -1 can
    represent an unset port; non-negative values are still 16-bit quantities
  * `begin(reuse)` now returns a `bool` instead of `void`, indicating success
  * The `EthernetServer` destructor now calls `end()`

### Removed
* Removed some unneeded network interfaces:
  * IEEE 802.1D MAC Bridge
  * 6LowPAN
  * PPP
  * SLIP
  * ZigBee Encapsulation Protocol
* Removed HTTPD options from `lwipopts.h`.

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
* Moved lwIP's heap to RAM2 (DMAMEM) and increased `MEM_SIZE` back to 24000.
* Updated `EthernetFrame`-related documentation to explain that the API doesn't
  receive any known Ethernet frame types, including IPv4, ARP, and IPv6
  (if enabled).
* Clarified in the examples that `Ethernet.macAddress()` retrieves, not sets.
* Changed `EthernetClass::setMACAddress(mac)` parameter to `const`.
* Moved CRC-32 lookup table to RAM2 (DMAMEM).
* Made const those functions which could be made const.
* Renamed `ServerWithAddressListener` example to `ServerWithListeners`.
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
* Added `EthernetClass::isPromiscuousMode()`.

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

Copyright (c) 2021-2025 Shawn Silverman

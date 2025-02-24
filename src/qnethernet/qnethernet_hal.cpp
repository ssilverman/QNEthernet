// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// qnethernet_hal.cpp implements the hardware abstraction layer (HAL).
// This file is part of the QNEthernet library.

// C includes
#include <unistd.h>

#include "qnethernet_opts.h"

// C++ includes
#if QNETHERNET_CUSTOM_WRITE
#include <cerrno>
#endif  // QNETHERNET_CUSTOM_WRITE
#include <cstdint>
#include <cstring>
#include <random>

#include <Arduino.h>  // For Serial, noInterrupts(), interrupts(), millis()
#include <Print.h>

// Processor-specific include
#if defined(TEENSYDUINO)
#if defined(__IMXRT1062__)
#include <imxrt.h>
#elif defined(ARDUINO_TEENSY30) || \
      defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY31) || \
      defined(ARDUINO_TEENSYLC) || \
      defined(ARDUINO_TEENSY35) || \
      defined(ARDUINO_TEENSY36)
#include <kinetis.h>
#endif  // Teensy type
#endif  // defined(TEENSYDUINO)

#include "lwip/arch.h"
#include "lwip/prot/ethernet.h"

// --------------------------------------------------------------------------
//  Time
// --------------------------------------------------------------------------

extern "C" {

// Returns the current time in milliseconds.
[[gnu::weak]]
uint32_t qnethernet_hal_millis(void) {
  return millis();
}

// Returns the current time in microseconds.
[[gnu::weak]]
uint32_t qnethernet_hal_micros(void) {
  return micros();
}

}  // extern "C"

// --------------------------------------------------------------------------
//  stdio
// --------------------------------------------------------------------------

#if QNETHERNET_CUSTOM_WRITE

// The user program can set these to something initialized. For example,
// `&Serial`, after `Serial.begin(speed)`.
namespace qindesign {
namespace network {

Print *volatile stdoutPrint = nullptr;
Print *volatile stderrPrint = nullptr;

}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_CUSTOM_WRITE

extern "C" {

// Gets the Print* for the given file descriptor.
static inline Print *getPrint(const int file) {
  switch (file) {
#if QNETHERNET_CUSTOM_WRITE
    case STDOUT_FILENO:
      return ::qindesign::network::stdoutPrint;
    case STDERR_FILENO:
      return ::qindesign::network::stderrPrint;
#else
    case STDOUT_FILENO:
    case STDERR_FILENO:
      return &Serial;
#endif  // QNETHERNET_CUSTOM_WRITE
    case STDIN_FILENO:
      return nullptr;
    default:
      return reinterpret_cast<Print *>(file);
  }
}

#if QNETHERNET_CUSTOM_WRITE

// Define this function to provide expanded stdio output behaviour. This should
// work for Newlib-based systems.
// See: https://forum.pjrc.com/threads/28473-Quick-Guide-Using-printf()-on-Teensy-ARM
// Note: Can't define as weak by default because we don't know which `_write`
//       would be chosen by the linker, this one or the one defined elsewhere
//       (Print.cpp, for example)
int _write(const int file, const void *const buf, const size_t len) {
  Print *const out = getPrint(file);
  if (out == nullptr) {
    errno = EBADF;
    return -1;
  }

  return out->write(static_cast<const uint8_t *>(buf), len);
}

#endif  // QNETHERNET_CUSTOM_WRITE

// Ensures the Print object is flushed because fflush() just flushes by writing
// to the FILE*. This doesn't necessarily send all the bytes right away. For
// example, Serial/USB output behaves this way.
void qnethernet_hal_stdio_flush(const int file) {
  Print *const p = getPrint(file);
  if (p != nullptr) {
    p->flush();
  }
}

}  // extern "C"

// --------------------------------------------------------------------------
//  Core Locking
// --------------------------------------------------------------------------

extern "C" {

// Asserts if this is called from an interrupt context.
[[gnu::weak]]
void qnethernet_hal_check_core_locking(const char *const file, const int line,
                                       const char *const func) {
  bool inInterruptCtx = false;

#if defined(__arm__)
  uint32_t ipsr;
  __asm__ volatile ("mrs %0, ipsr\n" : "=r" (ipsr) ::);
  inInterruptCtx = (ipsr != 0);
#endif  // defined(__arm__)

  if (inInterruptCtx) {
    printf("%s:%d:%s()\r\n", file, line, func);
    LWIP_PLATFORM_ASSERT("Function called from interrupt context");
  }
}

}  // extern "C"

// --------------------------------------------------------------------------
//  Randomness
// --------------------------------------------------------------------------

// Choose what gets included and called
#if (defined(TEENSYDUINO) && defined(__IMXRT1062__)) && \
    !QNETHERNET_USE_ENTROPY_LIB

#define WHICH_ENTROPY_TYPE 1  // Teensy 4
#include "qnethernet/security/entropy.h"

#elif defined(__has_include)
// https://gcc.gnu.org/onlinedocs/cpp/_005f_005fhas_005finclude.html
#if __has_include(<Entropy.h>)

#define WHICH_ENTROPY_TYPE 2  // Entropy library
#include <Entropy.h>
#endif  // __has_include(<Entropy.h>)

#endif  // Which entropy type

extern "C" {

// Initializes randomness.
[[gnu::weak]] void qnethernet_hal_init_entropy(void);

// Gets 32-bits of entropy for LWIP_RAND() and RandomDevice.
[[gnu::weak]] uint32_t qnethernet_hal_entropy(void);

// Fills a buffer with random values. This will return the number of bytes
// actually filled.
[[gnu::weak]] size_t qnethernet_hal_fill_entropy(void *buf, size_t size);

#if WHICH_ENTROPY_TYPE == 1

void qnethernet_hal_init_entropy(void) {
  if (!trng_is_started()) {
    trng_init();
  }
}

uint32_t qnethernet_hal_entropy(void) {
  return entropy_random();
}

size_t qnethernet_hal_fill_entropy(void *const buf, const size_t size) {
  return trng_data(buf, size);
}

#elif WHICH_ENTROPY_TYPE == 2

void qnethernet_hal_init_entropy(void) {
#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
  // Don't reinitialize
  const bool doEntropyInit =
      ((CCM_CCGR6 & CCM_CCGR6_TRNG(CCM_CCGR_ON_RUNONLY)) !=
       CCM_CCGR6_TRNG(CCM_CCGR_ON_RUNONLY)) ||
      ((TRNG_MCTL & TRNG_MCTL_TSTOP_OK) != 0);
#else
  const bool doEntropyInit = true;
#endif  // defined(TEENSYDUINO) && defined(__IMXRT1062__)
  if (doEntropyInit) {
    Entropy.Initialize();
  }
}

uint32_t qnethernet_hal_entropy(void) {
  return Entropy.random();
}

#else

// Returns a UniformRandomBitGenerator instance.
static std::minstd_rand &urbg_instance() {
  static std::minstd_rand gen;
  return gen;
}

void qnethernet_hal_init_entropy(void) {
  urbg_instance().seed(qnethernet_hal_micros());
}

uint32_t qnethernet_hal_entropy(void) {
  return urbg_instance()();
}

#endif  // Which entropy type

// Multi-byte fill
#if WHICH_ENTROPY_TYPE != 1

size_t qnethernet_hal_fill_entropy(void *const buf, const size_t size) {
  uint8_t *pBuf = buf;

  size_t count = size / 4;
  for (size_t i = 0; i < count; i++) {
    uint32_t r = qnethernet_hal_entropy();
    std::memcpy(pBuf, &r, 4);
    pBuf += 4;
  }

  size_t rem = size % 4;
  if (rem != 0) {
    uint32_t r = qnethernet_hal_entropy();
    std::memcpy(pBuf, &r, rem);
  }

  return size;
}

#endif  // Which entropy type

}  // extern "C"

// --------------------------------------------------------------------------
//  Interrupts
// --------------------------------------------------------------------------

extern "C" {

// Disables interrupts.
[[gnu::weak]]
void qnethernet_hal_disable_interrupts(void) {
  noInterrupts();
}

// Enables interrupts.
[[gnu::weak]]
void qnethernet_hal_enable_interrupts(void) {
  interrupts();
}

}  // extern "C"

// --------------------------------------------------------------------------
//  MAC Address
// --------------------------------------------------------------------------

extern "C" {

#if !defined(TEENSYDUINO)
static const uint8_t kDefaultMACAddress[ETH_HWADDR_LEN] = {
    QNETHERNET_DEFAULT_MAC_ADDRESS,
};
#endif  // !defined(TEENSYDUINO)

// Gets the system MAC address. This will either be some platform-specific value
// or a predefined value.
void qnethernet_hal_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]) {
  if (mac == nullptr) {
    return;
  }

#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
  const uint32_t m1 = HW_OCOTP_MAC1;
  const uint32_t m2 = HW_OCOTP_MAC0;
  mac[0] = m1 >> 8;
  mac[1] = m1 >> 0;
  mac[2] = m2 >> 24;
  mac[3] = m2 >> 16;
  mac[4] = m2 >> 8;
  mac[5] = m2 >> 0;
#elif defined(ARDUINO_TEENSY30) || \
      defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY31) || \
      defined(ARDUINO_TEENSYLC)
  // usb_desc.c:usb_init_serialnumber()
  __disable_irq();
  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  FTFL_FCCOB0 = 0x41;
  FTFL_FCCOB1 = 15;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) {
    // Wait
  }
  uint32_t num = *(uint32_t *)&FTFL_FCCOB7;
  __enable_irq();
  mac[0] = 0x04;
  mac[1] = 0xE9;
  mac[2] = 0xE5;
  mac[3] = num >> 16;
  mac[4] = num >> 8;
  mac[5] = num;
#elif defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY36)
  // usb_desc.c:usb_init_serialnumber()
  __disable_irq();
  kinetis_hsrun_disable();
  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  *(uint32_t *)&FTFL_FCCOB3 = 0x41070000;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) {
    // Wait
  }
  const uint32_t num = *(uint32_t *)&FTFL_FCCOBB;
  kinetis_hsrun_enable();
  __enable_irq();
  mac[0] = 0x04;
  mac[1] = 0xE9;
  mac[2] = 0xE5;
  mac[3] = num >> 16;
  mac[4] = num >> 8;
  mac[5] = num;
#else
  memcpy(mac, kDefaultMACAddress, ETH_HWADDR_LEN);
#endif  // Board type
}

}  // extern "C"

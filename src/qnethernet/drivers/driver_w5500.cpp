// SPDX-FileCopyrightText: (c) 2024-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_w5500.cpp contains the W5500 Ethernet interface implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/lwip_driver.h"

#if defined(QNETHERNET_INTERNAL_DRIVER_W5500)

#include "qnethernet/drivers/driver_w5500_config.h"

// C++ includes
#include <atomic>
#include <cstring>
#include <type_traits>

#include <Arduino.h>
#include <SPI.h>
#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
#include <imxrt.h>
#endif  // defined(TEENSYDUINO) && defined(__IMXRT1062__)

#include "lwip/arch.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/err.h"
#include "lwip/stats.h"
#include "qnethernet/compat/c++11_compat.h"
#include "qnethernet/platforms/pgmspace.h"

#if defined(TEENSYDUINO)
#define DIGITAL_WRITE digitalWriteFast
#else
#define DIGITAL_WRITE digitalWrite
#endif

// --------------------------------------------------------------------------
//  Types
// --------------------------------------------------------------------------

enum class EnetInitStates {
  kStart,                // Unknown hardware
  kNoHardware,           // No hardware
  kNotInitialized,       // There was some error initializing
  kHardwareInitialized,  // The hardware has been initialized
  kInitialized,          // Everything has been initialized
};

// Blocks for register access.
namespace blocks {
  static constexpr uint8_t kCommon   = 0;
  static constexpr uint8_t kSocket   = 1;
  static constexpr uint8_t kSocketTx = 2;
  static constexpr uint8_t kSocketRx = 3;
}  // namespace blocks

static void write_reg(uint16_t addr, uint8_t block, uint8_t v);
static void write_reg_word(uint16_t addr, uint8_t block, uint16_t v);
static void read(uint16_t addr, uint8_t block, void* buf, size_t len);

// Represents a specific register in a specific block.
template <typename T>
struct Reg {
  static_assert((sizeof(T) == 1) || (sizeof(T) == 2),
                "Type T must be 1 or 2 bytes");

  uint16_t addr;
  uint8_t block;

  constexpr Reg(const uint16_t a, const uint8_t b, const uint8_t socket = 0)
      : addr(a),
        block(static_cast<uint8_t>(b + (socket << 2))) {}

  constexpr Reg(const Reg& r, const uint8_t socket = 0)
      : addr(r.addr),
        block(static_cast<uint8_t>((r.block & 0x03) + (socket << 2))) {}

  // Note: C++11-style std::enable_if
  template <typename U = T,
            typename std::enable_if<std::is_same<U, T>::value &&
                                        (sizeof(U) == 1),
                                    bool>::type = true>
  const Reg& operator=(const T v) const {
    write_reg(addr, block, v);
    return *this;
  }

  // Note: C++11-style std::enable_if
  template <typename U = T,
            typename std::enable_if<std::is_same<U, T>::value &&
                                        (sizeof(U) == 2),
                                    bool>::type = true>
  const Reg& operator=(const T v) const {
    write_reg_word(addr, block, v);
    return *this;
  }

  // Reads the N-bit register value in a non-atomic operation.
  ATTRIBUTE_NODISCARD
  explicit operator T() const {
    T r;
    read(addr, block, &r, sizeof(T));
    IF_CONSTEXPR (sizeof(T) == 2) {
      r = ntohs(r);
    }
    return r;
  }

  // Reads the N-bit register value. This calls operator T().
  ATTRIBUTE_NODISCARD
  T operator*() const {
    return operator T();
  }

  // Pre-decrement operator.
  Reg& operator++() {
    ++addr;
    return *this;
  }
};

static constexpr Reg<uint8_t> kMR{0x0000, blocks::kCommon};             // Mode register
static constexpr Reg<uint8_t> kSHAR{0x0009, blocks::kCommon};           // Source Hardware Address Register (1/6)
static constexpr Reg<uint8_t> kPHYCFGR{0x002e, blocks::kCommon};        // PHY configuration
static constexpr Reg<uint8_t> kVERSIONR{0x0039, blocks::kCommon};       // Chip version
static constexpr Reg<uint8_t> kSn_MR{0x0000, blocks::kSocket};          // Socket n Mode
static constexpr Reg<uint8_t> kSn_CR{0x0001, blocks::kSocket};          // Socket n Command
static constexpr Reg<uint8_t> kSn_IR{0x0002, blocks::kSocket};          // Socket n Interrupt
static constexpr Reg<uint8_t> kSn_SR{0x0003, blocks::kSocket};          // Socket n Status
static constexpr Reg<uint8_t> kSn_RXBUF_SIZE{0x001e, blocks::kSocket};  // Socket n RX Buffer Size
static constexpr Reg<uint8_t> kSn_TXBUF_SIZE{0x001f, blocks::kSocket};  // Socket n TX Buffer Size
static constexpr Reg<uint16_t> kSn_TX_FSR{0x0020, blocks::kSocket};     // Socket n TX Free Size (16 bits)
static constexpr Reg<uint16_t> kSn_TX_WR{0x0024, blocks::kSocket};      // Socket n TX Write Pointer (16 bits)
static constexpr Reg<uint16_t> kSn_RX_RSR{0x0026, blocks::kSocket};     // Socket n RX Received Size (16 bits)
static constexpr Reg<uint16_t> kSn_RX_RD{0x0028, blocks::kSocket};      // Socket n RX Read Pointer (16 bits)
static constexpr Reg<uint8_t> kSn_IMR{0x002c, blocks::kSocket};         // Socket n Interrupt Mask Register

// Socket modes.
namespace socketmodes {
  static constexpr uint8_t kMFEN   = (1 << 7);  // MAC Filter Enable in MACRAW mode
  static constexpr uint8_t kBCASTB = (1 << 6);  // Broadcast Blocking in MACRAW and UDP mode
  static constexpr uint8_t kMacraw = 0x04;      // The MACRAW protocol mode
}  // namespace socketmodes

// Socket states.
namespace socketstates {
  static constexpr uint8_t kClosed = 0x00;
  static constexpr uint8_t kMacraw = 0x42;
}  // namespace socketstates

// Socket commands.
namespace socketcommands {
  static constexpr uint8_t kOpen  = 0x01;  // Socket n is initialized and opened according to the protocol
                                           // selected in Sn_MR (P3:P0)
  static constexpr uint8_t kClose = 0x10;  // Close Socket n
  static constexpr uint8_t kSend  = 0x20;  // SEND transmits all the data in the Socket n TX buffer
  static constexpr uint8_t kRecv  = 0x40;  // RECV completes the processing of the received data in
                                           // Socket n RX Buffer by using a RX read pointer register
                                           // (Sn_RX_RD)
}  // namespace socketcommands

// Socket interrupt masks.
namespace socketinterrupts {
  static constexpr uint8_t kSendOk = (1 << 4);  // This is issued when SEND command is completed
  static constexpr uint8_t kRecv   = (1 << 2);  // This is issued whenever data is received from a peer
}  // namespace socketinterrupts

// For using RAII to begin and end a SPI transaction.
class SPITransaction final {
 public:
  explicit SPITransaction() {
    spi.beginTransaction(kSPISettings);
  }

  ~SPITransaction() {
    spi.endTransaction();
  }

  SPITransaction(const SPITransaction&) = delete;
  SPITransaction& operator=(const SPITransaction&) = delete;
};

// --------------------------------------------------------------------------
//  Internal Variables
// --------------------------------------------------------------------------

static constexpr size_t kMTU         = MTU;
static constexpr size_t kMaxFrameLen = MAX_FRAME_LEN;  // Includes the 4-byte FCS (frame check sequence)

static_assert(kMaxFrameLen >= 4, "Max. frame len must be >= 4");

static constexpr uint8_t kControlRWBit = (1 << 2);

static constexpr size_t kInputBufKB = 16;  // In kibibytes

#if !QNETHERNET_BUFFERS_IN_RAM1 && \
    (defined(TEENSYDUINO) && defined(__IMXRT1062__))
#define BUFFER_DMAMEM DMAMEM
#else
#define BUFFER_DMAMEM
#endif  // !QNETHERNET_BUFFERS_IN_RAM1

// Buffers
static uint8_t s_spiBuf[3 + kMaxFrameLen - 4] BUFFER_DMAMEM;  // Exclude the 4-byte FCS
static uint8_t* const s_frameBuf = &s_spiBuf[3];

static struct InputBuf {
  std::array<uint8_t, kInputBufKB * 1024> buf;
  size_t start = 0;  // Where to read from
  size_t size = 0;   // Total size

  // Returns whether this buffer is empty, but not necessarily clear.
  bool empty() const {
    return start >= size;
  }

  // Clears the buffer.
  void clear() {
    start = 0;
    size = 0;
  }

  // Reads a frame length value. This assume the data exists at the given index.
  // This handles conversion from network order.
  uint16_t readFrameLen(const size_t index) const {
    uint16_t v;
    std::memcpy(&v, &buf[index], 2);
    return ntohs(v);
  }
} s_inputBuf BUFFER_DMAMEM;

// Interrupts
static std::atomic_flag s_rxNotAvail = ATOMIC_FLAG_INIT;
static std::atomic_flag s_sending    = ATOMIC_FLAG_INIT;

// Misc. internal state
static EnetInitStates s_initState = EnetInitStates::kStart;
static int s_chipSelectPin = kDefaultCSPin;  // Note: Arduino doesn't specify pin type
#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE
static bool s_macFilteringEnabled = false;  // Whether actually enabled
#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// PHY status, polled
static bool s_linkSpeed10Not100 = false;
static bool s_linkIsFullDuplex  = false;

// Notification data
static bool s_manualLinkState = false;  // True for sticky

// --------------------------------------------------------------------------
//  Internal Functions: Registers
// --------------------------------------------------------------------------

// Initializes the chip-select pin.
ATTRIBUTE_ALWAYS_INLINE
static inline void initCS() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
  pinMode(s_chipSelectPin, OUTPUT);  // Warning: implicit conversion
#pragma GCC diagnostic pop
}

// Asserts or deasserts the chip-select pin.
ATTRIBUTE_ALWAYS_INLINE
static inline void assertCS(const bool flag) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
  DIGITAL_WRITE(s_chipSelectPin, flag ? LOW : HIGH);  // Warning: implicit conversion
#pragma GCC diagnostic pop
}

// Reads bytes starting from the specified register.
static void read(const uint16_t addr, const uint8_t block,
                 void* const buf, const size_t len) {
  s_spiBuf[0] = static_cast<uint8_t>(addr >> 8);
  s_spiBuf[1] = static_cast<uint8_t>(addr);
  s_spiBuf[2] = static_cast<uint8_t>(block << 3);

  // Write zeros during transfer (is this step even necessary?)
  (void)std::memset(buf, 0, len);

  assertCS(true);
  spi.transfer(s_spiBuf, 3);
  spi.transfer(buf, len);
  assertCS(false);
}

// // Writes to the specified register.
// ATTRIBUTE_MAYBE_UNUSED
// static void write(const Reg& reg, void* const buf, const size_t len) {
//   s_spiBuf[0] = reg.addr >> 8;
//   s_spiBuf[1] = reg.addr;
//   s_spiBuf[2] = (reg.block << 3) | kControlRWBit;

//   uint8_t* pBuf = static_cast<uint8_t*>(buf);
//   size_t lenRem = len;

//   size_t index = 3;
//   assertCS(true);
//   do {
//     size_t size = std::min(lenRem, std::size(s_spiBuf) - index);
//     (void)std::memcpy(&s_spiBuf[index], pBuf, size);
//     lenRem -= size;
//     pBuf += size;

//     spi.transfer(s_spiBuf, index + size);
//     index = 0;
//   } while (lenRem > 0);
//   assertCS(false);
// }

// Writes a frame to the specified register. The data starts at &s_spiBuf[3].
static void write_frame(const uint16_t addr, const uint8_t block,
                        const size_t len) {
  s_spiBuf[0] = static_cast<uint8_t>(addr >> 8);
  s_spiBuf[1] = static_cast<uint8_t>(addr);
  s_spiBuf[2] = static_cast<uint8_t>((block << 3) | kControlRWBit);

  assertCS(true);
  spi.transfer(s_spiBuf, len + 3);
  assertCS(false);
}

// Writes to the specified register. The data starts at &s_spiBuf[3].
template <typename T>
static void write_frame(const Reg<T>& reg, const size_t len) {
  write_frame(reg.addr, reg.block, len);
}

// Writes a value to the specified register.
static inline void write_reg(const uint16_t addr, const uint8_t block,
                             const uint8_t v) {
  s_frameBuf[0] = v;
  write_frame(addr, block, 1);
}

// // Reads a 16-bit value, not guaranteeing that the value is stable. Callers may
// // wish to read until there's no change.
// ATTRIBUTE_NODISCARD
// static uint16_t read_reg_word_nonatomic(const Reg& reg) {
//   uint16_t r;
//   read(reg.addr, reg.block, &r, 2);
//   return ntohs(r);
// }

// // Reads a 16-bit value, guaranteeing the results are stable. This loops until
// // the value is stable.
// ATTRIBUTE_MAYBE_UNUSED ATTRIBUTE_NODISCARD
// static uint16_t read_reg_word_atomic(const Reg& reg) {
//   uint16_t v1;
//   uint16_t v2;
//   v1 = read_reg_word_nonatomic(reg);
//   do {
//     std::swap(v1, v2);
//     v1 = read_reg_word_nonatomic(reg);
//   } while (v1 != v2);
//   return v1;
// }

// Reads a 16-bit value. If the value isn't stable, then this will return false.
// Otherwise, this will return true and 'v' will be set to the word.
ATTRIBUTE_NODISCARD
static bool read_reg_word(const Reg<uint16_t>& reg, uint16_t& v) {
  const uint16_t v1 = *reg;
  const uint16_t v2 = *reg;
  if (v1 != v2) {
    return false;
  }
  v = v2;
  return true;
}

static inline void write_reg_word(const uint16_t addr, const uint8_t block,
                                  const uint16_t v) {
  s_frameBuf[0] = static_cast<uint8_t>(v >> 8);
  s_frameBuf[1] = static_cast<uint8_t>(v);
  write_frame(addr, block, 2);
}

// --------------------------------------------------------------------------
//  Internal Functions
// --------------------------------------------------------------------------

static void recv_isr() {
  // Read and clear the interrupts
  const uint8_t ir = []() {
    SPITransaction spiTransaction;
    const uint8_t ir = *kSn_IR;
    kSn_IR = uint8_t{0xff};
    return ir;
  }();

  if ((ir | socketinterrupts::kRecv) != 0) {
    s_rxNotAvail.clear();
  }
  if ((ir | socketinterrupts::kSendOk) != 0) {
    s_sending.clear();
  }
}

// Sends a socket command and ensures it completes.
static void set_socket_command(const uint8_t v) {
  kSn_CR = v;
  while (*kSn_CR != 0) {
    // Wait for Sn_CR to be zero
  }
}

// Soft resets the chip.
ATTRIBUTE_NODISCARD
FLASHMEM static bool soft_reset() {
  int count = 0;

  // Loop up to 20 times
  kMR = uint8_t{0x80};
  do {
    if ((*kMR & 0x80) == 0) {
      return true;
    }
    delay(1);
  } while (++count < 20);

  return false;
}

// Initializes the interface. This sets the init state.
//
// This manages the SPI transaction.
FLASHMEM static void low_level_init() {
  if (s_initState != EnetInitStates::kStart) {
    return;
  }

  // Delay some worst case scenario because Arduino's Ethernet library does
  delay(560);

  initCS();
  assertCS(false);

  spi.begin();

  spi.beginTransaction(kSPISettings);

  if (!soft_reset()) {
    goto low_level_init_nohardware;
  }

  // Register tests (the Arduino Ethernet library does this)
  kMR = uint8_t{0x08};
  if (*kMR != 0x08) {
    goto low_level_init_nohardware;
  }
  kMR = uint8_t{0x10};
  if (*kMR != 0x10) {
    goto low_level_init_nohardware;
  }
  kMR = uint8_t{0x00};
  if (*kMR != 0x00) {
    goto low_level_init_nohardware;
  }

  // Check the version
  if (*kVERSIONR != 4) {
    goto low_level_init_nohardware;
  }

  // Open a MACRAW socket
  // Use 16k buffers

#if QNETHERNET_ENABLE_PROMISCUOUS_MODE || QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
  kSn_MR = socketmodes::kMacraw;
#else
  // Start with MAC filtering enabled until we allow more MAC addresses
  kSn_MR = static_cast<uint8_t>(socketmodes::kMFEN | socketmodes::kMacraw);
  s_macFilteringEnabled = true;
#endif  // QNETHERNET_ENABLE_PROMISCUOUS_MODE || QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

  // Set the others to 0k before setting the first, so the sum won't overflow
  for (uint8_t i = 1; i < 8; ++i) {
    Reg<uint8_t>{kSn_RXBUF_SIZE, i} = uint8_t{0};
    Reg<uint8_t>{kSn_TXBUF_SIZE, i} = uint8_t{0};
  }
  kSn_RXBUF_SIZE = uint8_t{kInputBufKB};
  kSn_TXBUF_SIZE = uint8_t{kInputBufKB};
  IF_CONSTEXPR (kInterruptPin < 0) {
    // Disable the socket interrupts
    kSn_IMR = uint8_t{0};
  } else {
    kSn_IMR = static_cast<uint8_t>(socketinterrupts::kSendOk |
                                   socketinterrupts::kRecv);
    spi.usingInterrupt(kInterruptPin);
    attachInterrupt(kInterruptPin, &recv_isr, FALLING);
  }
  set_socket_command(socketcommands::kOpen);
  if (*kSn_SR != socketstates::kMacraw) {
    s_initState = EnetInitStates::kNotInitialized;
  } else {
    s_initState = EnetInitStates::kHardwareInitialized;
  }

  spi.endTransaction();
  return;

low_level_init_nohardware:
  spi.endTransaction();
  spi.end();
  s_initState = EnetInitStates::kNoHardware;
}

// Sends a frame. This uses data already in s_frameBuf.
ATTRIBUTE_NODISCARD
static err_t send_frame(const size_t len) {
  if (len == 0) {
    return ERR_OK;
  }

  IF_CONSTEXPR (kInterruptPin >= 0) {
    while (s_sending.test_and_set()) {
      // Wait until not sending
    }
  }

  // Check for space in the transmit buffer
  const uint16_t txSize = []() {
    uint16_t w;
    while (!read_reg_word(kSn_TX_FSR, w)) {
      // Wait for valid read
      // TODO: Limit count?
    }
    return w;
  }();
  if (len > txSize) {
    return ERR_MEM;
  }

  // Check that the socket is open
  if (*kSn_SR == socketstates::kClosed) {
    return ERR_CLSD;
  }

  // Send the data
  const uint16_t ptr = *kSn_TX_WR;
  write_frame(ptr, blocks::kSocketTx, len);
  kSn_TX_WR = static_cast<uint16_t>(ptr + len);
  set_socket_command(socketcommands::kSend);

  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

// Checks the current link status.
static void check_link_status(struct netif* const netif) {
  static uint8_t is_link_up = false;

  const uint8_t status = *kPHYCFGR;

  is_link_up = ((status & 0x01) != 0);

  // Watch for changes
  if (netif_is_link_up(netif) != is_link_up) {
    if (is_link_up) {
      s_linkIsFullDuplex  = ((status & 0x04) != 0);
      s_linkSpeed10Not100 = ((status & 0x02) == 0);

      netif_set_link_up(netif);
    } else {
      if (!s_manualLinkState) {
        netif_set_link_down(netif);
      }
    }
  }
}

// --------------------------------------------------------------------------
//  Driver Interface
// --------------------------------------------------------------------------

extern "C" {

FLASHMEM void driver_get_capabilities(struct DriverCapabilities* const dc) {
  dc->isMACSettable                = true;
  dc->isLinkStateDetectable        = true;
  dc->isLinkSpeedDetectable        = true;
  dc->isLinkSpeedSettable          = false;
  dc->isLinkFullDuplexDetectable   = true;
  dc->isLinkFullDuplexSettable     = false;
  dc->isLinkCrossoverDetectable    = false;
  dc->isAutoNegotiationRestartable = false;
}

bool driver_is_unknown(void) {
  return s_initState == EnetInitStates::kStart;
}

void qnethernet_hal_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]);

void driver_get_system_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  qnethernet_hal_get_system_mac_address(mac);
}

bool driver_get_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  switch (s_initState) {
    case EnetInitStates::kHardwareInitialized:
      ATTRIBUTE_FALLTHROUGH;
    case EnetInitStates::kInitialized:
      break;
    default:
      return false;
  }

  SPITransaction spiTransaction;

  auto reg = kSHAR;
  for (int i = 0; i < ETH_HWADDR_LEN; ++i) {
    mac[i] = *reg;
    ++reg;
  }

  return true;
}

bool driver_set_mac(const uint8_t mac[ETH_HWADDR_LEN]) {
  switch (s_initState) {
    case EnetInitStates::kHardwareInitialized:
      ATTRIBUTE_FALLTHROUGH;
    case EnetInitStates::kInitialized:
      break;
    default:
      return false;
  }

  (void)std::memcpy(s_frameBuf, mac, ETH_HWADDR_LEN);

  SPITransaction spiTransaction;
  write_frame(kSHAR, ETH_HWADDR_LEN);

  return true;
}

bool driver_has_hardware(void) {
  switch (s_initState) {
    case EnetInitStates::kHardwareInitialized:
      ATTRIBUTE_FALLTHROUGH;
    case EnetInitStates::kInitialized:
      ATTRIBUTE_FALLTHROUGH;
    case EnetInitStates::kNotInitialized:
      return true;
    case EnetInitStates::kNoHardware:
      return false;
    default:
      break;
  }
  low_level_init();
  return (s_initState != EnetInitStates::kNoHardware);
}

FLASHMEM void driver_set_chip_select_pin(const int pin) {
  if (pin < 0) {
    s_chipSelectPin = kDefaultCSPin;
  } else {
    s_chipSelectPin = pin;
  }
}

FLASHMEM bool driver_init(void) {
  if (s_initState == EnetInitStates::kInitialized) {
    return true;
  }

  // Clear the flags
  (void)s_rxNotAvail.test_and_set();
  s_sending.clear();

  // Set the chip's MAC address
  low_level_init();
  if (s_initState != EnetInitStates::kHardwareInitialized) {
    return false;
  }

  s_initState = EnetInitStates::kInitialized;

  return true;
}

FLASHMEM void driver_deinit(void) {
  switch (s_initState) {
    case EnetInitStates::kStart:
      ATTRIBUTE_FALLTHROUGH;
    case EnetInitStates::kNoHardware:
      return;
    default:
      break;
  }

  // Detach the interrupt
  IF_CONSTEXPR (kInterruptPin >= 0) {
    detachInterrupt(kInterruptPin);
  }

  // Close the socket
  SPITransaction spiTransaction;
  set_socket_command(socketcommands::kClose);

  spi.end();
  s_initState = EnetInitStates::kStart;
}

struct pbuf* driver_proc_input(struct netif* const netif, const int counter) {
  LWIP_UNUSED_ARG(netif);
  LWIP_UNUSED_ARG(counter);

  if (s_initState != EnetInitStates::kInitialized) {
    return NULL;
  }

  // Check if we have buffered or received data, and if not, read some in
  const bool doCheck = s_inputBuf.empty() &&
                       ((kInterruptPin < 0) || !s_rxNotAvail.test_and_set());

  if (doCheck) {
    SPITransaction spiTransaction;

    const uint16_t rxSize = []() -> uint16_t {
      uint16_t w;
      if (!read_reg_word(kSn_RX_RSR, w)) {
        return 0;
      }
      return w;
    }();
    if (rxSize < 2) {
      return NULL;
    }

    // [MACRAW Application Note?](https://forum.wiznet.io/t/topic/979/3)

    const uint16_t ptr = *kSn_RX_RD;

    // Read all the data
    read(ptr, blocks::kSocketRx, s_inputBuf.buf.data(), rxSize);
    s_inputBuf.clear();

    // Scan the data for valid frames
    size_t index = 0;
    bool doSocketReceive = true;  // Whether to notify the chip of received data
                                  // We don't if we've reset the socket
    while (index + 2 <= rxSize) {  // Account for a 2-byte frame length
      s_inputBuf.size = index;

      const uint16_t frameLen = s_inputBuf.readFrameLen(index);
      // The frame length includes its 2-byte self

      // Check for bad data
      if (frameLen < 2) {
        // This is unexpected
        LINK_STATS_INC(link.lenerr);

        // Recommendation is to close and then re-open the socket
        set_socket_command(socketcommands::kClose);
        set_socket_command(socketcommands::kOpen);
        if (*kSn_SR != socketstates::kMacraw) {
          s_initState = EnetInitStates::kNotInitialized;
        }

        s_inputBuf.start = 0;
        // Don't set the size so we can keep any received valid frames
        doSocketReceive = false;
        break;
      }

      // Watch for the end
      if ((index + frameLen) > rxSize) {
        // We've read all we can read
        break;
      }

      index += frameLen;
    }

    // Tell the chip we've read some data
    if (doSocketReceive && (s_inputBuf.size != 0)) {
      kSn_RX_RD = static_cast<uint16_t>(ptr + s_inputBuf.size);
      set_socket_command(socketcommands::kRecv);
    }
  }

  // Check for buffered data
  if (s_inputBuf.empty()) {
    s_inputBuf.clear();
    return NULL;
  }

  // At this point, we can assume data in s_inputBuf is correct

  uint16_t frameLen;

  // Loop until a valid frame or end of the buffer
  do {
    // Read the frame length
    frameLen = s_inputBuf.readFrameLen(s_inputBuf.start);
    // The frame length includes its 2-byte self

    LINK_STATS_INC(link.recv);

    if ((size_t{frameLen} - 2) <= (kMaxFrameLen - 4)) {  // Exclude the 4-byte FCS
      break;
    }

    LINK_STATS_INC(link.drop);
    s_inputBuf.start += frameLen;
  } while (!s_inputBuf.empty());

  // No valid frames
  if (s_inputBuf.empty()) {
    s_inputBuf.clear();
    return NULL;
  }

  // Process the frame
  struct pbuf* const p = pbuf_alloc(PBUF_RAW, frameLen - 2, PBUF_POOL);
  if (p == nullptr) {
    LINK_STATS_INC(link.drop);
    LINK_STATS_INC(link.memerr);
  } else {
    LWIP_ASSERT("Expected space for pbuf fill",
                pbuf_take(p, &s_inputBuf.buf.data()[s_inputBuf.start + 2],
                          p->tot_len) == ERR_OK);
    s_inputBuf.start += frameLen;
  }
  return p;
}

void driver_poll(struct netif* const netif) {
  SPITransaction spiTransaction;
  check_link_status(netif);
}

int driver_link_speed(void) {
  return s_linkSpeed10Not100 ? 10 : 100;
}

bool driver_link_set_speed(const int speed) {
  LWIP_UNUSED_ARG(speed);
  return false;
}

bool driver_link_is_full_duplex(void) {
  return s_linkIsFullDuplex;
}

bool driver_link_set_full_duplex(const bool flag) {
  LWIP_UNUSED_ARG(flag);
  return false;
}

bool driver_link_is_crossover(void) {
  return false;
}

// Outputs data from the MAC.
err_t driver_output(struct pbuf* const p) {
  if (s_initState != EnetInitStates::kInitialized) {
    return ERR_IF;
  }

#if ETH_PAD_SIZE
  LWIP_ASSERT("Expected removed ETH_PAD_SIZE header",
              pbuf_remove_header(p, ETH_PAD_SIZE) == 0);
#endif  // ETH_PAD_SIZE

  // Shouldn't need this check:
  // if (p->tot_len > kMaxFrameLen - 4) {  // Exclude the 4-byte FCS
  //   LINK_STATS_INC(link.drop);
  //   LINK_STATS_INC(link.lenerr);
  //   return ERR_BUF;
  // }

  const uint16_t copied = pbuf_copy_partial(p, s_frameBuf, p->tot_len, 0);
  if (copied == 0) {
    LINK_STATS_INC(link.drop);
    LINK_STATS_INC(link.err);
    return ERR_BUF;
  }

  SPITransaction spiTransaction;
  return send_frame(p->tot_len);
}

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
bool driver_output_frame(const void* const frame, const size_t len) {
  if (s_initState != EnetInitStates::kInitialized) {
    return false;
  }

  (void)std::memcpy(s_frameBuf, frame, len);

  SPITransaction spiTransaction;
  return send_frame(len);
}
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

bool driver_set_incoming_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN],
                                             const bool allow) {
  if (mac == nullptr) {
    return false;
  }

  // It appears MAC filtering still allows multicast destinations through, so
  // don't disable filtering for those (LSb of first byte is 1)
  if (allow && ((mac[0] & 0x01) == 0) && s_macFilteringEnabled) {
    SPITransaction spiTransaction;

    // Allow all MACs now
    uint8_t r = *kSn_MR;
    if ((r & socketmodes::kMFEN) != 0) {
      r &= static_cast<uint8_t>(~socketmodes::kMFEN);
      kSn_MR = r;
    }
    s_macFilteringEnabled = false;
    // It appears we don't need to reopen the socket here
  }

  return allow;  // Can allow but never disallow
}

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// --------------------------------------------------------------------------
//  Notifications from Upper Layers
// --------------------------------------------------------------------------

void driver_notify_manual_link_state(const bool flag) {
  s_manualLinkState = flag;
}

// --------------------------------------------------------------------------
//  Link Functions
// --------------------------------------------------------------------------

void driver_restart_auto_negotiation() {
}

}  // extern "C"

#endif  // QNETHERNET_INTERNAL_DRIVER_W5500

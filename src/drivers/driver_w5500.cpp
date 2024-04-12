// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_w5500.cpp contains the W5500 Ethernet interface implementation.
// This file is part of the QNEthernet library.

#include "lwip_driver.h"

#if defined(QNETHERNET_INTERNAL_DRIVER_W5500)

#include "driver_w5500_config.h"

// C++ includes
#include <cstdint>
#include <cstring>
#include <type_traits>

#include <Arduino.h>  // For pinMode() and digitalWrite()
#include <SPI.h>
#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
#include <imxrt.h>
#endif  // defined(TEENSYDUINO) && defined(__IMXRT1062__)

#include "lwip/def.h"
#include "lwip/err.h"
#include "lwip/stats.h"

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
static void read(uint16_t addr, uint8_t block, void *buf, size_t len);

// Represents a specific register in a specific block.
template <typename T>
struct Reg {
  static_assert(sizeof(T) == 1 || sizeof(T) == 2);

  uint16_t addr;
  uint8_t block;

  constexpr Reg(uint16_t a, uint8_t b, uint8_t socket = 0)
      : addr(a),
        block(b + (socket << 2)) {}

  constexpr Reg(const Reg &r, uint8_t socket = 0)
      : addr(r.addr),
        block((r.block & 0x03) + (socket << 2)) {}

  template <typename U,
            typename std::enable_if<!std::is_same<U, T>::value, bool>::type = true>
  const Reg &operator=(U v) const {
    return operator=(static_cast<T>(v));
  }

  template <typename U = T,
            typename std::enable_if<sizeof(U) == 1, bool>::type = true>
  const Reg &operator=(T v) const {
    write_reg(addr, block, v);
    return *this;
  }

  template <typename U = T,
            typename std::enable_if<sizeof(U) == 2, bool>::type = true>
  const Reg &operator=(T v) const {
    write_reg_word(addr, block, v);
    return *this;
  }

  // Reads the N-bit register value in a non-atomic operation.
  explicit operator T() const {
    T r;
    read(addr, block, &r, sizeof(T));
    if /*constexpr*/ (sizeof(T) == 2) {
      r = ntohs(r);
    }
    return r;
  }

  // Reads the N-bit register value. This calls operator T().
  T operator*() const {
    return operator T();
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

// --------------------------------------------------------------------------
//  Internal Variables
// --------------------------------------------------------------------------

static constexpr uint8_t kControlRWBit = (1 << 2);

#if !QNETHERNET_BUFFERS_IN_RAM1 && \
    (defined(TEENSYDUINO) && defined(__IMXRT1062__))
#define BUFFER_DMAMEM DMAMEM
#else
#define BUFFER_DMAMEM
#endif  // !QNETHERNET_BUFFERS_IN_RAM1

// Buffers
static uint8_t s_spiBuf[3 + MAX_FRAME_LEN - 4] BUFFER_DMAMEM;  // Exclude the 4-byte FCS
static uint8_t *const s_frameBuf = &s_spiBuf[3];
static uint8_t s_inputBuf[16 * 1024] BUFFER_DMAMEM;

// Misc. internal state
static EnetInitStates s_initState = EnetInitStates::kStart;
static int s_chipSelectPin = kDefaultCSPin;
#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE
static bool s_macFilteringEnabled = false;  // Whether actually enabled
#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// PHY status, polled
static bool s_linkSpeed10Not100 = false;
static bool s_linkIsFullDuplex  = false;

// --------------------------------------------------------------------------
//  Internal Functions: Registers
// --------------------------------------------------------------------------

// Reads bytes starting from the specified register.
static void read(uint16_t addr, uint8_t block, void *buf, size_t len) {
  s_spiBuf[0] = addr >> 8;
  s_spiBuf[1] = addr;
  s_spiBuf[2] = block << 3;

  // Write zeros during transfer (is this step even necessary?)
  std::memset(buf, 0, len);

  spi.beginTransaction(kSPISettings);
  digitalWrite(s_chipSelectPin, LOW);
  spi.transfer(s_spiBuf, 3);
  spi.transfer(buf, len);
  digitalWrite(s_chipSelectPin, HIGH);
  spi.endTransaction();
}

// // Writes to the specified register.
// [[maybe_unused]]
// static void write(const Reg &reg, uint8_t *buf, size_t len) {
//   s_spiBuf[0] = reg.addr >> 8;
//   s_spiBuf[1] = reg.addr;
//   s_spiBuf[2] = (reg.block << 3) | kControlRWBit;

//   spi.beginTransaction(kSPISettings);
//   digitalWrite(s_chipSelectPin, LOW);

//   size_t index = 3;
//   do {
//     size_t size = std::min(len, std::size(s_spiBuf) - index);
//     std::memcpy(&s_spiBuf[index], buf, size);
//     len -= size;
//     buf += size;

//     spi.transfer(s_spiBuf, index + size);
//     index = 0;
//   } while (len > 0);

//   digitalWrite(s_chipSelectPin, HIGH);
//   spi.endTransaction();
// }

// Writes a frame to the specified register. The data starts at &s_spiBuf[3].
static void write_frame(uint16_t addr, uint8_t block, size_t len) {
  s_spiBuf[0] = addr >> 8;
  s_spiBuf[1] = addr;
  s_spiBuf[2] = (block << 3) | kControlRWBit;

  spi.beginTransaction(kSPISettings);
  digitalWrite(s_chipSelectPin, LOW);
  spi.transfer(s_spiBuf, len + 3);
  digitalWrite(s_chipSelectPin, HIGH);
  spi.endTransaction();
}

// Writes to the specified register. The data starts at &s_spiBuf[3].
template <typename T>
static void write_frame(const Reg<T> &reg, size_t len) {
  write_frame(reg.addr, reg.block, len);
}

// Writes a value to the specified register.
static inline void write_reg(uint16_t addr, uint8_t block, uint8_t v) {
  s_frameBuf[0] = v;
  write_frame(addr, block, 1);
}

// // Reads a 16-bit value, not guaranteeing that the value is stable. Callers may
// // wish to read until there's no change.
// static uint16_t read_reg_word_nonatomic(const Reg &reg) {
//   uint16_t r;
//   read(reg.addr, reg.block, &r, 2);
//   return ntohs(r);
// }

// // Reads a 16-bit value, guaranteeing the results are stable. This loops until
// // the value is stable.
// [[maybe_unused]]
// static uint16_t read_reg_word_atomic(const Reg &reg) {
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
static bool read_reg_word(const Reg<uint16_t> &reg, uint16_t &v) {
  uint16_t v1 = *reg;
  uint16_t v2 = *reg;
  if (v1 != v2) {
    return false;
  }
  v = v2;
  return true;
}

static inline void write_reg_word(uint16_t addr, uint8_t block, uint16_t v) {
  s_frameBuf[0] = v >> 8;
  s_frameBuf[1] = v;
  write_frame(addr, block, 2);
}

// --------------------------------------------------------------------------
//  Internal Functions
// --------------------------------------------------------------------------

// Sends a socket command and ensures it completes.
static void set_socket_command(uint8_t v) {
  kSn_CR = v;
  while (*kSn_CR != 0) {
    // Wait for Sn_CR to be zero
  }
}

// Soft resets the chip.
static bool soft_reset() {
  int count = 0;

  // Loop up to 20 times
  kMR = 0x80;
  do {
    if ((*kMR & 0x80) == 0) {
      return true;
    }
    delay(1);
  } while (++count < 20);

  return false;
}

// Initializes the interface. This sets the init state.
static void low_level_init() {
  if (s_initState != EnetInitStates::kStart) {
    return;
  }

  // Delay some worst case scenario because Arduino's Ethernet library does
  delay(560);

  pinMode(s_chipSelectPin, OUTPUT);
  spi.begin();

  if (!soft_reset()) {
    goto low_level_init_nohardware;
  }

  // Register tests (the Arduino Ethernet library does this)
  kMR = 0x08;
  if (*kMR != 0x08) {
    goto low_level_init_nohardware;
  }
  kMR = 0x10;
  if (*kMR != 0x10) {
    goto low_level_init_nohardware;
  }
  kMR = 0x00;
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
  kSn_MR = socketmodes::kMFEN | socketmodes::kMacraw;
  s_macFilteringEnabled = true;
#endif  // QNETHERNET_ENABLE_PROMISCUOUS_MODE || QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

  kSn_RXBUF_SIZE = 16;
  kSn_TXBUF_SIZE = 16;
  // Set the others to 0k
  for (uint8_t i = 1; i < 8; i++) {
    Reg<uint8_t>{kSn_RXBUF_SIZE, i} = 0;
    Reg<uint8_t>{kSn_TXBUF_SIZE, i} = 0;
  }
  if /*constexpr*/ (!kSocketInterruptsEnabled) {
    // Disable the socket interrupts
    kSn_IMR = 0;
  } else {
    kSn_IMR = socketinterrupts::kSendOk | socketinterrupts::kRecv;
  }
  set_socket_command(socketcommands::kOpen);
  if (*kSn_SR != socketstates::kMacraw) {
    s_initState = EnetInitStates::kNotInitialized;
    return;
  }

  s_initState = EnetInitStates::kHardwareInitialized;
  return;

low_level_init_nohardware:
  spi.end();
  s_initState = EnetInitStates::kNoHardware;
}

// Sends a frame. This uses data already in s_frameBuf.
static err_t send_frame(size_t len) {
  if (len == 0) {
    return ERR_OK;
  }

  // Check for space in the transmit buffer
  uint16_t size;
  if (!read_reg_word(kSn_TX_FSR, size)) {
    return ERR_WOULDBLOCK;
  }
  if (size < len) {
    return ERR_MEM;
  }

  // Check that the socket is open
  if (*kSn_SR == socketstates::kClosed) {
    return ERR_CLSD;
  }

  // Send the data
  uint16_t ptr = *kSn_TX_WR;
  write_frame(ptr, blocks::kSocketTx, len);
  kSn_TX_WR = ptr + len;
  set_socket_command(socketcommands::kSend);
  if /*constexpr*/ (kSocketInterruptsEnabled) {
    // TODO: See if there's a way to make this non-blocking
    while ((*kSn_IR & socketinterrupts::kSendOk) == 0) {
      // Wait for the interrupt
    }
    kSn_IR = socketinterrupts::kSendOk;  // Clear it
  }

  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

// Checks the current link status.
static void check_link_status(struct netif *netif) {
  static uint8_t is_link_up = false;

  uint8_t status = *kPHYCFGR;

  is_link_up = ((status & 0x01) != 0);

  // Watch for changes
  if (netif_is_link_up(netif) != is_link_up) {
    if (is_link_up) {
      s_linkIsFullDuplex  = ((status & 0x04) != 0);
      s_linkSpeed10Not100 = ((status & 0x02) == 0);

      netif_set_link_up(netif);
    } else {
      netif_set_link_down(netif);
    }
  }
}

// --------------------------------------------------------------------------
//  Driver Interface
// --------------------------------------------------------------------------

extern "C" {

bool driver_is_unknown() {
  return s_initState == EnetInitStates::kStart;
}

bool driver_is_link_state_detectable() {
  return true;
}

void qnethernet_hal_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]);

void driver_get_system_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  qnethernet_hal_get_system_mac_address(mac);
}

void driver_set_mac(const uint8_t mac[ETH_HWADDR_LEN]) {
  switch (s_initState) {
    case EnetInitStates::kHardwareInitialized:
    case EnetInitStates::kInitialized:
      break;
    default:
      return;
  }

  std::memcpy(s_frameBuf, mac, ETH_HWADDR_LEN);
  write_frame(kSHAR, ETH_HWADDR_LEN);
}

bool driver_has_hardware() {
  switch (s_initState) {
    case EnetInitStates::kHardwareInitialized:
    case EnetInitStates::kInitialized:
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

void driver_set_chip_select_pin(int pin) {
  if (pin < 0) {
    pin = kDefaultCSPin;
  }
  s_chipSelectPin = pin;
}

bool driver_init(const uint8_t mac[ETH_HWADDR_LEN]) {
  if (s_initState == EnetInitStates::kInitialized) {
    return true;
  }

  // Set the chip's MAC address
  low_level_init();
  if (s_initState != EnetInitStates::kHardwareInitialized) {
    return false;
  }
  driver_set_mac(mac);

  s_initState = EnetInitStates::kInitialized;

  return true;
}

void driver_deinit() {
  switch (s_initState) {
    case EnetInitStates::kStart:
    case EnetInitStates::kNoHardware:
      return;
    default:
      break;
  }

  // Close the socket
  set_socket_command(socketcommands::kClose);

  spi.end();
  s_initState = EnetInitStates::kStart;
}

void driver_proc_input(struct netif *netif) {
  if (s_initState != EnetInitStates::kInitialized) {
    return;
  }

  uint16_t size;
  if (!read_reg_word(kSn_RX_RSR, size)) {
    return;
  }
  if (size == 0) {
    // TODO: Do we need to process the size < 2 case?
    return;
  }

  // [MACRAW Application Note?](https://forum.wiznet.io/t/topic/979/3)

  uint16_t ptr = *kSn_RX_RD;

  // Read the frame length
  uint16_t frameLen;
  read(ptr, blocks::kSocketRx, &frameLen, 2);
  frameLen = ntohs(frameLen);
  if (frameLen < 2 || size < frameLen) {
    LINK_STATS_INC(link.lenerr);

    // Recommendation is to close and then re-open the socket
    set_socket_command(socketcommands::kClose);
    set_socket_command(socketcommands::kOpen);
    if (*kSn_SR != socketstates::kMacraw) {
      s_initState = EnetInitStates::kNotInitialized;
      return;
    }
    return;
  }
  frameLen -= 2;
  ptr += 2;

  LINK_STATS_INC(link.recv);

  if (frameLen > MAX_FRAME_LEN - 4) {  // Exclude the 4-byte FCS
    LINK_STATS_INC(link.drop);
  } else {
    read(ptr, blocks::kSocketRx, s_inputBuf, frameLen);
  }
  kSn_RX_RD = ptr + frameLen;
  set_socket_command(socketcommands::kRecv);
  if /*constexpr*/ (kSocketInterruptsEnabled) {
    if (frameLen + 2 == size) {
      kSn_IR = socketinterrupts::kRecv;  // Clear the RECV interrupt
    }
  }

  if (frameLen > MAX_FRAME_LEN - 4) {  // Exclude the 4-byte FCS
    return;
  }

  // Process the frame
  struct pbuf *p = pbuf_alloc(PBUF_RAW, frameLen, PBUF_POOL);
  if (p == nullptr) {
    LINK_STATS_INC(link.drop);
    LINK_STATS_INC(link.memerr);
  } else {
    pbuf_take(p, s_inputBuf, p->tot_len);
    if (netif->input(p, netif) != ERR_OK) {
      pbuf_free(p);
    }
  }

  // Process only a single frame because the whole RX buffer might have partial
  // frames, it seems
}

void driver_poll(struct netif *netif) {
  check_link_status(netif);
}

int driver_link_speed() {
  return s_linkSpeed10Not100 ? 10 : 100;
}

bool driver_link_is_full_duplex() {
  return s_linkIsFullDuplex;
}

bool driver_link_is_crossover() {
  return false;
}

// Outputs data from the MAC.
err_t driver_output(struct pbuf *p) {
  if (s_initState != EnetInitStates::kInitialized) {
    return ERR_IF;
  }

#if ETH_PAD_SIZE
  pbuf_remove_header(p, ETH_PAD_SIZE);
#endif  // ETH_PAD_SIZE

  // Shouldn't need this check:
  // if (p->tot_len > MAX_FRAME_LEN - 4) {  // Exclude the 4-byte FCS
  //   LINK_STATS_INC(link.drop);
  //   LINK_STATS_INC(link.lenerr);
  //   return ERR_BUF;
  // }

  uint16_t copied = pbuf_copy_partial(p, s_frameBuf, p->tot_len, 0);
  if (copied == 0) {
    LINK_STATS_INC(link.drop);
    LINK_STATS_INC(link.err);
    return ERR_BUF;
  }

  return send_frame(p->tot_len);
}

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
bool driver_output_frame(const uint8_t *frame, size_t len) {
  if (s_initState != EnetInitStates::kInitialized) {
    return false;
  }

  std::memcpy(s_frameBuf, frame, len);
  return send_frame(len);
}
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

bool driver_set_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN],
                                    bool allow) {
  if (mac == nullptr) {
    return false;
  }

  // It appears MAC filtering still allows multicast destinations through, so
  // don't disable filtering for those (LSb of first byte is 1)
  if (allow && ((mac[0] & 0x01) == 0) && s_macFilteringEnabled) {
    // Allow all MACs now
    uint8_t r = *kSn_MR;
    if ((r & socketmodes::kMFEN) != 0) {
      r &= ~socketmodes::kMFEN;
      kSn_MR = r;
    }
    s_macFilteringEnabled = false;
    // It appears we don't need to reopen the socket here
  }

  return allow;  // Can allow but never disallow
}

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

}  // extern "C"

#endif  // QNETHERNET_INTERNAL_DRIVER_W5500

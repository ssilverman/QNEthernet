// Define Teensy 4.1 Ethernet functions.
// Based on code from Paul Stoffregen: https://github.com/PaulStoffregen/teensy41_ethernet
// (c) 2021 Shawn Silverman

#ifndef LWIP_T41_H_
#define LWIP_T41_H_

#ifdef ARDUINO_TEENSY41

#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*rx_frame_fn)(void *);
typedef void (*tx_timestamp_fn)(uint32_t);

// Get the Ethernet MAC address.
void enet_getmac(uint8_t *mac);

// Initialize Ethernet.
void enet_init(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);

// Sets the receive callback function.
void enet_set_receive_callback(rx_frame_fn rx_cb);

// Sets the timestamp callback function.
void enet_set_tx_timestamp_callback(tx_timestamp_fn tx_cb);

// Get the next chunk of input data.
struct pbuf *enet_rx_next();

// Process one chunk of input data.
void enet_input(struct pbuf *p_frame);

// Process any Ethernet input.
void enet_proc_input(void);

// Poll Ethernet link status.
void enet_poll();

// Timestamp the next transmitted packet.
void enet_txTimestampNextPacket();

// Read the IEEE1588 timer.
uint32_t read_1588_timer();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ARDUINO_TEENSY41

#endif  // LWIP_T41_H_

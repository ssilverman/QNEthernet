// Define Teensy 4.1 Ethernet functions.
// Based on code from Paul Stoffregen: https://github.com/PaulStoffregen/teensy41_ethernet
// (c) 2021 Shawn Silverman

#ifndef LWIP_T41_H_
#define LWIP_T41_H_

#ifdef ARDUINO_TEENSY41

#include <stdint.h>
#include <lwip/ip_addr.h>
#include <lwip/prot/ethernet.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*rx_frame_fn)(void *);

// Get the Ethernet MAC address.
void enet_getmac(uint8_t *mac);

// Initialize Ethernet. This does not set the interface to "up".
//
// This may be called more than once, but if the MAC address has changed then
// the interface is first removed and then re-added.
//
// It is suggested to initialize the random number generator with srand before
// calling this.
void enet_init(const uint8_t macaddr[ETH_HWADDR_LEN],
               const ip_addr_t *ipaddr,
               const ip_addr_t *netmask,
               const ip_addr_t *gw);

// Disable Ethernet.
void enet_deinit();

// // Sets the receive callback function.
// void enet_set_receive_callback(rx_frame_fn rx_cb);

// // Get the next chunk of input data.
// struct pbuf *enet_rx_next();

// // Process one chunk of input data.
// void enet_input(struct pbuf *p_frame);

// Process any Ethernet input. This is meant to be called often by the
// main loop.
void enet_proc_input(void);

// Poll Ethernet link status.
void enet_poll();

// Read the IEEE1588 timer.
uint32_t read_1588_timer();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ARDUINO_TEENSY41

#endif  // LWIP_T41_H_

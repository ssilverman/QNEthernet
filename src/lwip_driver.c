// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// lwip_driver.c implements Ethernet interface functions.
// This file is part of the QNEthernet library.

#include "lwip_driver.h"

// C includes
#include <string.h>

#include "lwip/autoip.h"
#include "lwip/dhcp.h"
#include "lwip/etharp.h"
#include "lwip/init.h"
#include "lwip/prot/ieee.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"

// --------------------------------------------------------------------------
//  Internal Variables
// --------------------------------------------------------------------------

// Current MAC address.
static uint8_t s_mac[ETH_HWADDR_LEN];

// netif state
static struct netif s_netif = { .name = {'e', '0'} };
static bool s_isNetifAdded  = false;
NETIF_DECLARE_EXT_CALLBACK(netif_callback)/*;*/

// Structs for avoiding memory allocation
#if LWIP_DHCP
static struct dhcp s_dhcp;
#endif  // LWIP_DHCP
#if LWIP_AUTOIP
static struct autoip s_autoip;
#endif  // LWIP_AUTOIP

// --------------------------------------------------------------------------
//  Internal Functions
// --------------------------------------------------------------------------

// Outputs the given pbuf to the driver.
static err_t link_output(struct netif *netif, struct pbuf *p) {
  LWIP_UNUSED_ARG(netif);

  if (p == NULL) {
    return ERR_ARG;
  }

  return driver_output(p);
}

// Initializes the netif.
static err_t init_netif(struct netif *netif) {
  if (netif == NULL) {
    return ERR_ARG;
  }

  netif->linkoutput = link_output;
#if LWIP_IPV4
  netif->output     = etharp_output;
#endif  // LWIP_IPV4
  netif->mtu        = driver_get_mtu();
  netif->flags = 0
                 | NETIF_FLAG_BROADCAST
#if LWIP_IPV4
                 | NETIF_FLAG_ETHARP
#endif  // LWIP_IPV4
                 | NETIF_FLAG_ETHERNET
#if LWIP_IGMP
                 | NETIF_FLAG_IGMP
#endif  // LWIP_IGMP
                 ;

  memcpy(netif->hwaddr, s_mac, ETH_HWADDR_LEN);
  netif->hwaddr_len = ETH_HWADDR_LEN;

#if LWIP_NETIF_HOSTNAME
  netif_set_hostname(netif, NULL);
#endif  // LWIP_NETIF_HOSTNAME

  return ERR_OK;
}

#if LWIP_IGMP && !QNETHERNET_ENABLE_PROMISCUOUS_MODE
// Multicast filter for letting the hardware know which packets to let in.
static err_t multicast_filter(struct netif *netif, const ip4_addr_t *group,
                              enum netif_mac_filter_action action) {
  LWIP_UNUSED_ARG(netif);

  bool retval = true;
  switch (action) {
    case NETIF_ADD_MAC_FILTER:
      retval = enet_join_group(group);
      break;
    case NETIF_DEL_MAC_FILTER:
      retval = enet_leave_group(group);
      break;
    default:
      break;
  }
  return retval ? ERR_OK : ERR_USE;
      // ERR_USE seems like the best fit of the choices
      // Next best seems to be ERR_IF
}
#endif  // LWIP_IGMP && !QNETHERNET_ENABLE_PROMISCUOUS_MODE

#if QNETHERNET_INTERNAL_END_STOPS_ALL
// Removes the current netif, if any.
static void remove_netif() {
  if (s_isNetifAdded) {
    netif_set_default(NULL);
    netif_remove(&s_netif);
    netif_remove_ext_callback(&netif_callback);
    s_isNetifAdded = false;
  }
}
#endif  // QNETHERNET_INTERNAL_END_STOPS_ALL

// --------------------------------------------------------------------------
//  Public Interface
// --------------------------------------------------------------------------

struct netif *enet_netif() {
  return &s_netif;
}

void enet_get_system_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  if (mac != NULL) {
    driver_get_system_mac(mac);
  }
}

// This only uses the callback if the interface has not been added.
bool enet_init(const uint8_t mac[ETH_HWADDR_LEN],
               netif_ext_callback_fn callback) {
  // Sanitize the inputs
  uint8_t m[ETH_HWADDR_LEN];
  if (mac == NULL) {
    driver_get_system_mac(m);
    mac = m;
  }

  // Only execute the following code once
  static bool isFirstInit = true;
  if (isFirstInit) {
    lwip_init();
    isFirstInit = false;
  } else if (memcmp(s_mac, mac, ETH_HWADDR_LEN) != 0) {
    // First test if the MAC address has changed
    // If it's changed then remove the interface and start again

    // MAC address has changed

    // Remove any previous configuration
    // remove_netif();
    // TODO: For some reason, remove_netif() prevents further operation
  }

  memcpy(s_mac, mac, ETH_HWADDR_LEN);

  if (!driver_init(s_mac)) {
    return false;
  }

  if (!s_isNetifAdded) {
    netif_add_ext_callback(&netif_callback, callback);
    if (netif_add_noaddr(&s_netif, NULL, init_netif, ethernet_input) == NULL) {
      netif_remove_ext_callback(&netif_callback);
      return false;
    }
    netif_set_default(&s_netif);
    s_isNetifAdded = true;

    // netif_add() clears these, so re-set them
#if LWIP_DHCP
    dhcp_set_struct(&s_netif, &s_dhcp);
#endif  // LWIP_DHCP
#if LWIP_AUTOIP
    autoip_set_struct(&s_netif, &s_autoip);
#endif  // LWIP_AUTOIP

#if LWIP_IGMP && !QNETHERNET_ENABLE_PROMISCUOUS_MODE
    // Multicast filtering, to allow desired multicast packets in
    netif_set_igmp_mac_filter(&s_netif, &multicast_filter);
#endif  // LWIP_IGMP && !QNETHERNET_ENABLE_PROMISCUOUS_MODE
  } else {
    // Just set the MAC address

    memcpy(s_netif.hwaddr, s_mac, ETH_HWADDR_LEN);
    s_netif.hwaddr_len = ETH_HWADDR_LEN;

    driver_set_mac(s_mac);
  }

  return true;
}

void enet_deinit() {
  // Restore state
  memset(s_mac, 0, sizeof(s_mac));

  // Something about stopping Ethernet and the PHY kills performance if Ethernet
  // is restarted after calling end(), so gate the following two blocks with a
  // macro for now

#if QNETHERNET_INTERNAL_END_STOPS_ALL
  remove_netif();  // TODO: This also causes issues (see notes in enet_init())
#endif  // QNETHERNET_INTERNAL_END_STOPS_ALL

  driver_deinit();
}

void enet_proc_input() {
  driver_proc_input(&s_netif);
}

void enet_poll() {
  sys_check_timeouts();
  driver_poll(&s_netif);
}

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
bool enet_output_frame(const uint8_t *frame, size_t len) {
  if (frame == NULL || len < (6 + 6 + 2)) {  // dst + src + len/type
    return false;
  }

  // Check length depending on VLAN
  if (frame[12] == (uint8_t)(ETHTYPE_VLAN >> 8) &&
      frame[13] == (uint8_t)(ETHTYPE_VLAN)) {
    if (len < (6 + 6 + 2 + 2 + 2)) {  // dst + src + VLAN tag + VLAN info + len/type
      return false;
    }
    if (len > driver_get_max_frame_len() - 4) {  // Don't include 4-byte FCS
      return false;
    }
  } else {
    if (len > driver_get_max_frame_len() - 4 - 4) {  // Don't include 4-byte FCS and VLAN
      return false;
    }
  }

#if QNETHERNET_ENABLE_RAW_FRAME_LOOPBACK
  // Check for a loopback frame
  if (memcmp(frame, s_mac, 6) == 0) {
    struct pbuf *p = pbuf_alloc(PBUF_RAW, len + ETH_PAD_SIZE, PBUF_POOL);
    if (p) {
      pbuf_take_at(p, frame, len, ETH_PAD_SIZE);
      if (s_netif.input(p, &s_netif) != ERR_OK) {
        pbuf_free(p);
      }
    }
    // TODO: Collect stats?
    return true;
  }
#endif  // QNETHERNET_ENABLE_RAW_FRAME_LOOPBACK

  return driver_output_frame(frame, len);
}
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE && LWIP_IPV4

// Joins or leaves a multicast group. The flag should be true to join and false
// to leave. This returns whether successful.
static bool enet_join_notleave_group(const ip4_addr_t *group, bool flag) {
  if (group == NULL) {
    return false;
  }

  // Multicast MAC address.
  static uint8_t multicastMAC[ETH_HWADDR_LEN] = {
      LL_IP4_MULTICAST_ADDR_0,
      LL_IP4_MULTICAST_ADDR_1,
      LL_IP4_MULTICAST_ADDR_2,
      0,
      0,
      0,
  };

  multicastMAC[3] = ip4_addr2(group) & 0x7f;
  multicastMAC[4] = ip4_addr3(group);
  multicastMAC[5] = ip4_addr4(group);

  return driver_set_mac_address_allowed(multicastMAC, flag);
}

bool enet_join_group(const ip4_addr_t *group) {
  return enet_join_notleave_group(group, true);
}

bool enet_leave_group(const ip4_addr_t *group) {
  return enet_join_notleave_group(group, false);
}

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE && LWIP_IPV4

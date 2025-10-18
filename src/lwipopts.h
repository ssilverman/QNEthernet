// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// lwipopts.h is the lwIP configuration.
// This file is part of the QNEthernet library.

#pragma once

#include "qnethernet/driver_select.h"
#include "qnethernet_opts.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// NO SYS
#define NO_SYS                 1  /* 0 */
// #define NO_SYS_NO_TIMERS       0
// #define LWIP_TIMERS            1
// #define LWIP_TIMERS_CUSTOM     0
// #define MEMCPY(dst, src, len)  memcpy(dst, src, len)
// #define SMEMCPY(dst, src, len) memcpy(dst, src, len)
// #define MEMMOVE(dst, src, len) memmove(dst, src, len)

// Core locking
// #define LWIP_MPU_COMPATIBLE           0
// #define LWIP_TCPIP_CORE_LOCKING       1
// #define LWIP_TCPIP_CORE_LOCKING_INPUT 0
#define SYS_LIGHTWEIGHT_PROT          0  /* 1 */
#define LWIP_ASSERT_CORE_LOCKED() \
  qnethernet_hal_check_core_locking(__FILE__, __LINE__, __func__)
void qnethernet_hal_check_core_locking(const char *file, int line,
                                       const char *func);

// Memory options
#ifndef MEM_LIBC_MALLOC
#define MEM_LIBC_MALLOC                        1  /* 0 */
#endif  // !MEM_LIBC_MALLOC
// #define MEM_CUSTOM_ALLOCATOR                   0  /* opt.h sets to 1 if MEM_LIBC_MALLOC */
// #define MEM_CUSTOM_FREE                        free
// #define MEM_CUSTOM_MALLOC                      malloc
// #define MEM_CUSTOM_CALLOC                      calloc
// #define MEMP_MEM_MALLOC                        0
// #define MEMP_MEM_INIT                          0
#define MEM_ALIGNMENT                          4  /* 1 */
#ifndef MEM_SIZE
// Note: MEM_SIZE is not used if MEM_LIBC_MALLOC is enabled
#define MEM_SIZE                               24000  /* 1600 */
#endif  // !MEM_SIZE
// #define MEMP_OVERFLOW_CHECK                    0
// #define MEMP_SANITY_CHECK                      0
// #define MEM_OVERFLOW_CHECK                     0
// #define MEM_SANITY_CHECK                       0
// #define MEM_USE_POOLS                          0
// #define MEM_USE_POOLS_TRY_BIGGER_POOL          0
// #define MEMP_USE_CUSTOM_POOLS                  0
// #define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 0

// Internal Memory Pool Sizes
// #define MEMP_NUM_PBUF                      16
// #define MEMP_NUM_RAW_PCB                   4
#ifndef MEMP_NUM_UDP_PCB
// Increment MEMP_NUM_UDP_PCB by 1 for mDNS, if needed:
#define MEMP_NUM_UDP_PCB                   8  /* 4 */
#endif  // !MEMP_NUM_UDP_PCB
#ifndef MEMP_NUM_TCP_PCB
#define MEMP_NUM_TCP_PCB                   8  /* 5 */
#endif  // !MEMP_NUM_TCP_PCB
#ifndef MEMP_NUM_TCP_PCB_LISTEN
// #define MEMP_NUM_TCP_PCB_LISTEN            8
#endif  // !MEMP_NUM_TCP_PCB_LISTEN
// #define MEMP_NUM_TCP_SEG                   16
// #define MEMP_NUM_ALTCP_PCB                 MEMP_NUM_TCP_PCB
// #define MEMP_NUM_REASSDATA                 5
// #define MEMP_NUM_FRAG_PBUF                 15
// #define MEMP_NUM_ARP_QUEUE                 30
#ifndef MEMP_NUM_IGMP_GROUP
#define MEMP_NUM_IGMP_GROUP                9  /* 8 */
#endif  // !MEMP_NUM_IGMP_GROUP
/* #define LWIP_NUM_SYS_TIMEOUT_INTERNAL                                \
   (LWIP_TCP + IP_REASSEMBLY + LWIP_ARP + (2*LWIP_DHCP) + LWIP_ACD + \
    LWIP_IGMP + LWIP_DNS + PPP_NUM_TIMEOUTS +                        \
    (LWIP_IPV6*(1 + LWIP_IPV6_REASS + LWIP_IPV6_MLD + LWIP_IPV6_DHCP6)))*/
#if !defined(LWIP_MDNS_RESPONDER) || LWIP_MDNS_RESPONDER
// Increment MEMP_NUM_SYS_TIMEOUT by 8 for mDNS
// Refs:
// * https://lists.nongnu.org/archive/html/lwip-users/2024-05/msg00000.html
// * https://savannah.nongnu.org/patch/?9523#comment18
#define MEMP_NUM_SYS_TIMEOUT               ((LWIP_NUM_SYS_TIMEOUT_INTERNAL) + (8))  /* LWIP_NUM_SYS_TIMEOUT_INTERNAL */
#else
// #define MEMP_NUM_SYS_TIMEOUT               LWIP_NUM_SYS_TIMEOUT_INTERNAL
#endif  // !defined(LWIP_MDNS_RESPONDER) || LWIP_MDNS_RESPONDER
// #define MEMP_NUM_NETBUF                    2
// #define MEMP_NUM_NETCONN                   4
// #define MEMP_NUM_SELECT_CB                 4
// #define MEMP_NUM_TCPIP_MSG_API             8
// #define MEMP_NUM_TCPIP_MSG_INPKT           8
// #define MEMP_NUM_NETDB                     1
// #define MEMP_NUM_LOCALHOSTLIST             1
// #define PBUF_POOL_SIZE                     16
// #define MEMP_NUM_API_MSG                   MEMP_NUM_TCPIP_MSG_API
// #define MEMP_NUM_DNS_API_MSG               MEMP_NUM_TCPIP_MSG_API
// #define MEMP_NUM_SOCKET_SETGETSOCKOPT_DATA MEMP_NUM_TCPIP_MSG_API
// #define MEMP_NUM_NETIFAPI_MSG              MEMP_NUM_TCPIP_MSG_API

// ARP options
#ifndef LWIP_ARP
#define LWIP_ARP                      (LWIP_IPV4)  /* 1 */
#endif  // LWIP_ARP
// #define ARP_TABLE_SIZE                10
// #define ARP_MAXAGE                    300
// #define ARP_QUEUEING                  0
// #define ARP_QUEUE_LEN                 3
// #define ETHARP_SUPPORT_VLAN           0
// #define LWIP_VLAN_PCP                 0
#define LWIP_ETHERNET                 1  /* LWIP_ARP */
// #define ETH_PAD_SIZE                  0
// #define ETHARP_SUPPORT_STATIC_ENTRIES 0
// #define ETHARP_TABLE_MATCH_NETIF      !LWIP_SINGLE_NETIF

// IP options
#ifndef LWIP_IPV4
// #define LWIP_IPV4                       1
#endif  // LWIP_IPV4
// #define IP_FORWARD                      0
// #define IP_REASSEMBLY                   1
// #define IP_FRAG                         1
// #define IP_OPTIONS_ALLOWED              1
// #define IP_REASS_MAXAGE                 15
// #define IP_REASS_MAX_PBUFS              10
// #define IP_DEFAULT_TTL                  255
// #define IP_SOF_BROADCAST                0
// #define IP_SOF_BROADCAST_RECV           0
// #define IP_FORWARD_ALLOW_TX_ON_RX_NETIF 0

// ICMP options
#ifndef LWIP_ICMP
#define LWIP_ICMP           (LWIP_IPV4)  /* 1 */
#endif  // LWIP_ICMP
// #define ICMP_TTL            IP_DEFAULT_TTL
// #define LWIP_BROADCAST_PING 0
// #define LWIP_MULTICAST_PING 0

// RAW options
// #define LWIP_RAW 0
// #define RAW_TTL  IP_DEFAULT_TTL

// DHCP options
#ifndef LWIP_DHCP
#define LWIP_DHCP                       ((LWIP_IPV4) && (LWIP_UDP))  /* 0 */
#endif  // !LWIP_DHCP
#ifndef LWIP_DHCP_DOES_ACD_CHECK
#define LWIP_DHCP_DOES_ACD_CHECK        0  /* LWIP_DHCP */
#endif  // !LWIP_DHCP_DOES_ACD_CHECK
// #define LWIP_DHCP_BOOTP_FILE            0
// #define LWIP_DHCP_GET_NTP_SRV           0
// #define LWIP_DHCP_MAX_NTP_SERVERS       1
// #define LWIP_DHCP_MAX_DNS_SERVERS       DNS_MAX_SERVERS
// #define LWIP_DHCP_DISCOVER_ADD_HOSTNAME 1

// AUTOIP options
#if !defined(LWIP_MDNS_RESPONDER) || LWIP_MDNS_RESPONDER
// Add both for mDNS:
#ifndef LWIP_AUTOIP
#define LWIP_AUTOIP                 (LWIP_IPV4)  /* 0 */
#endif  // LWIP_AUTOIP
#define LWIP_DHCP_AUTOIP_COOP       ((LWIP_DHCP) && (LWIP_AUTOIP))  /* 0 */
#else
// #define LWIP_AUTOIP                 0
// #define LWIP_DHCP_AUTOIP_COOP       0
#endif  // !defined(LWIP_MDNS_RESPONDER) || LWIP_MDNS_RESPONDER
// #define LWIP_DHCP_AUTOIP_COOP_TRIES 9

// ACD options
// #define LWIP_ACD (LWIP_AUTOIP || LWIP_DHCP_DOES_ACD_CHECK)

// SNMP MIB2 support
// #define LWIP_MIB2_CALLBACKS 0

// Multicast options
/* #define LWIP_MULTICAST_TX_OPTIONS \
   ((LWIP_IGMP || LWIP_IPV6_MLD) && (LWIP_UDP || LWIP_RAW))*/

// IGMP options
#ifndef LWIP_IGMP
#define LWIP_IGMP LWIP_IPV4  /* 0 */
#endif  // !LWIP_IGMP

// DNS options
#ifndef LWIP_DNS
#define LWIP_DNS                                (LWIP_UDP)  /* 0 */
#endif  // !LWIP_DNS
// #define DNS_TABLE_SIZE                          4
// #define DNS_MAX_NAME_LENGTH                     256
// #define DNS_MAX_SERVERS                         2
#ifndef DNS_MAX_RETRIES
// #define DNS_MAX_RETRIES                         4
#endif  // !DNS_MAX_RETRIES
// #define DNS_DOES_NAME_CHECK                     1
/* #define LWIP_DNS_SECURE                                                 \
  (LWIP_DNS_SECURE_RAND_XID | LWIP_DNS_SECURE_NO_MULTIPLE_OUTSTANDING | \
   LWIP_DNS_SECURE_RAND_SRC_PORT)*/
// #define LWIP_DNS_SECURE_RAND_XID                1
// #define LWIP_DNS_SECURE_NO_MULTIPLE_OUTSTANDING 2
// #define LWIP_DNS_SECURE_RAND_SRC_PORT           4
// #define DNS_LOCAL_HOSTLIST                      0
// #define DNS_LOCAL_HOSTLIST_IS_DYNAMIC           0
#define LWIP_DNS_SUPPORT_MDNS_QUERIES           1  /* 0 */

// UDP options
#ifndef LWIP_UDP
#define LWIP_UDP             ((LWIP_IPV4) || (LWIP_IPV6))  /* 1 */
#endif  // !LWIP_UDP
// #define LWIP_UDPLITE         0
// #define UDP_TTL              IP_DEFAULT_TTL
// #define LWIP_NETBUF_RECVINFO 0

// TCP options

#ifndef LWIP_TCP
#define LWIP_TCP                   ((LWIP_IPV4) || (LWIP_IPV6))  /* 1 */
#endif  // !LWIP_TCP
// #define TCP_TTL                    IP_DEFAULT_TTL
// #define TCP_MAXRTX                 12
// #define TCP_SYNMAXRTX              6
// #define TCP_QUEUE_OOSEQ            LWIP_TCP
// #define LWIP_TCP_SACK_OUT          0
// #define LWIP_TCP_MAX_SACK_NUM      4
#define TCP_MSS                    ((MTU) - 40)  /* 536 */
// #define TCP_CALCULATE_EFF_SEND_MSS 1
// #define LWIP_TCP_RTO_TIME          3000
#define TCP_SND_BUF                (4 * (TCP_MSS))  /* (2 * TCP_MSS) */
// #define TCP_SND_QUEUELEN           ((4 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))
/* #define TCP_SNDLOWAT \
   LWIP_MIN(LWIP_MAX(((TCP_SND_BUF)/2), (2 * TCP_MSS) + 1), (TCP_SND_BUF) - 1)*/
// #define TCP_SNDQUEUELOWAT LWIP_MAX(((TCP_SND_QUEUELEN)/2), 5)
// #define TCP_OOSEQ_MAX_BYTES        0
// #if TCP_OOSEQ_MAX_BYTES
// #define TCP_OOSEQ_BYTES_LIMIT(pcb) TCP_OOSEQ_MAX_BYTES
// #endif
// #define TCP_OOSEQ_MAX_PBUFS        0
// #if TCP_OOSEQ_MAX_PBUFS
// #define TCP_OOSEQ_PBUFS_LIMIT(pcb) TCP_OOSEQ_MAX_PBUFS
// #endif
// #define TCP_LISTEN_BACKLOG         0
// #define TCP_DEFAULT_LISTEN_BACKLOG 0xff
// #define TCP_OVERSIZE               TCP_MSS
// #define LWIP_TCP_TIMESTAMPS        0
// #define TCP_WND_UPDATE_THRESHOLD   LWIP_MIN((TCP_WND / 4), (TCP_MSS * 4))
// #define LWIP_EVENT_API             0
// #define LWIP_CALLBACK_API          1
// #define LWIP_WND_SCALE             0
// #define TCP_RCV_SCALE              0
// #define LWIP_TCP_PCB_NUM_EXT_ARGS  0
#ifndef LWIP_ALTCP
// #define LWIP_ALTCP                 0
#endif  // !LWIP_ALTCP
#ifndef LWIP_ALTCP_TLS
// #define LWIP_ALTCP_TLS             0
#endif  // !LWIP_ALTCP_TLS
#ifndef TCP_WND
#if LWIP_ALTCP && LWIP_ALTCP_TLS
#define TCP_WND                    (16 * 1024)  /* 16KiB for TLS */
#else
// #define TCP_WND                    (4 * TCP_MSS)
#endif  // LWIP_ALTCP && LWIP_ALTCP_TLS
#endif  // !TCP_WND

// Pbuf options
// #define PBUF_LINK_HLEN                (14 + ETH_PAD_SIZE) or (18 + ETH_PAD_SIZE)
// #define PBUF_LINK_ENCAPSULATION_HLEN  0
// #define PBUF_POOL_BUFSIZE             LWIP_MEM_ALIGN_SIZE(TCP_MSS+PBUF_IP_HLEN+PBUF_TRANSPORT_HLEN+PBUF_LINK_ENCAPSULATION_HLEN+PBUF_LINK_HLEN)
// #define LWIP_PBUF_REF_T               u8_t
#define LWIP_PBUF_CUSTOM_DATA \
  u8_t timestampValid;        \
  struct timespec timestamp;
#define LWIP_PBUF_CUSTOM_DATA_INIT(p) \
  (p)->timestampValid = 0;            \
  (p)->timestamp.tv_sec = 0;          \
  (p)->timestamp.tv_nsec = 0;

// Network Interfaces options
#define LWIP_SINGLE_NETIF              1  /* 0 */
#define LWIP_NETIF_HOSTNAME            1  /* 0 */
// #define LWIP_NETIF_API                 0
// #define LWIP_NETIF_STATUS_CALLBACK     0
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1  /* 0 */
// #define LWIP_NETIF_LINK_CALLBACK       0
// #define LWIP_NETIF_REMOVE_CALLBACK     0
// #define LWIP_NETIF_HWADDRHINT          0
// #define LWIP_NETIF_TX_SINGLE_PBUF      0
#if !defined(LWIP_MDNS_RESPONDER) || LWIP_MDNS_RESPONDER
// Increment LWIP_NUM_NETIF_CLIENT_DATA by 1 for mDNS:
#define LWIP_NUM_NETIF_CLIENT_DATA     1  /* 0 */
#else
// #define LWIP_NUM_NETIF_CLIENT_DATA     0
#endif  // !defined(LWIP_MDNS_RESPONDER) || LWIP_MDNS_RESPONDER

// LOOPIF options
// #define LWIP_HAVE_LOOPIF (LWIP_NETIF_LOOPBACK && !LWIP_SINGLE_NETIF)
// #define LWIP_LOOPIF_MULTICAST              0
#ifndef LWIP_NETIF_LOOPBACK
// #define LWIP_NETIF_LOOPBACK                0
#endif  // !LWIP_NETIF_LOOPBACK
#ifndef LWIP_LOOPBACK_MAX_PBUFS
// #define LWIP_LOOPBACK_MAX_PBUFS            0
#endif  // !LWIP_LOOPBACK_MAX_PBUFS
// #define LWIP_NETIF_LOOPBACK_MULTITHREADING (!NO_SYS)

// Thread options
/*
#define TCPIP_THREAD_NAME         "tcpip_thread"
#define TCPIP_THREAD_STACKSIZE    0
#define TCPIP_THREAD_PRIO         1
#define TCPIP_MBOX_SIZE           0
#define LWIP_TCPIP_THREAD_ALIVE()
#define SLIPIF_THREAD_NAME        "slipif_loop"
#define SLIPIF_THREAD_STACKSIZE   0
#define SLIPIF_THREAD_PRIO        1
#define DEFAULT_THREAD_NAME       "lwIP"
#define DEFAULT_THREAD_STACKSIZE  0
#define DEFAULT_THREAD_PRIO       1
#define DEFAULT_RAW_RECVMBOX_SIZE 0
#define DEFAULT_UDP_RECVMBOX_SIZE 0
#define DEFAULT_TCP_RECVMBOX_SIZE 0
#define DEFAULT_ACCEPTMBOX_SIZE   0
*/

// Sequential layer options
#define LWIP_NETCONN                0  /* 1 */
// #define LWIP_TCPIP_TIMEOUT          0
// #define LWIP_NETCONN_SEM_PER_THREAD 0
// #define LWIP_NETCONN_FULLDUPLEX     0

// Socket options
#define LWIP_SOCKET                       0  /* 1 */
// #define LWIP_COMPAT_SOCKETS               1
// #define LWIP_POSIX_SOCKETS_IO_NAMES       1
// #define LWIP_SOCKET_OFFSET                0
// #define LWIP_SOCKET_EXTERNAL_HEADERS      0
// #define LWIP_TCP_KEEPALIVE                0
// #define LWIP_SO_SNDTIMEO                  0
// #define LWIP_SO_RCVTIMEO                  0
// #define LWIP_SO_SNDRCVTIMEO_NONSTANDARD   0
// #define LWIP_SO_RCVBUF                    0
// #define LWIP_SO_LINGER                    0
// #define RECV_BUFSIZE_DEFAULT              INT_MAX
// #define LWIP_TCP_CLOSE_TIMEOUT_MS_DEFAULT 20000
#define SO_REUSE                          1  /* 0 */
// #define SO_REUSE_RXTOALL                  0
// #define LWIP_FIONREAD_LINUXMODE           0
// #define LWIP_SOCKET_SELECT                1
// #define LWIP_SOCKET_POLL                  1

// Statistics options
#ifndef LWIP_STATS
#define LWIP_STATS         0  /* 1 */
#endif  // !LWIP_STATS
#ifndef LWIP_STATS_LARGE
// #define LWIP_STATS_LARGE   0
#endif  // !LWIP_STATS_LARGE
// #define LWIP_STATS_DISPLAY 0
// #define LINK_STATS         1
// #define ETHARP_STATS       (LWIP_ARP)
// #define IP_STATS           1
// #define IPFRAG_STATS       (IP_REASSEMBLY || IP_FRAG)
// #define ICMP_STATS         1
// #define IGMP_STATS         (LWIP_IGMP)
// #define UDP_STATS          (LWIP_UDP)
// #define TCP_STATS          (LWIP_TCP)
// #define MEM_STATS          ((MEM_CUSTOM_ALLOCATOR == 0) && (MEM_USE_POOLS == 0))
// #define MEMP_STATS         (MEMP_MEM_MALLOC == 0)
// #define SYS_STATS          (NO_SYS == 0)
// #define IP6_STATS          (LWIP_IPV6)
// #define ICMP6_STATS        (LWIP_IPV6 && LWIP_ICMP6)
// #define IP6_FRAG_STATS     (LWIP_IPV6 && (LWIP_IPV6_FRAG || LWIP_IPV6_REASS))
// #define MLD6_STATS         (LWIP_IPV6 && LWIP_IPV6_MLD)
// #define ND6_STATS          (LWIP_IPV6)
// #define MIB2_STATS         0

// Checksum options
// #define LWIP_CHECKSUM_CTRL_PER_NETIF 0
// #define CHECKSUM_GEN_IP              1
// #define CHECKSUM_GEN_UDP             1
// #define CHECKSUM_GEN_TCP             1
// #define CHECKSUM_GEN_ICMP            1
// #define CHECKSUM_GEN_ICMP6           1
// #define CHECKSUM_CHECK_IP            1
// #define CHECKSUM_CHECK_UDP           1
// #define CHECKSUM_CHECK_TCP           1
// #define CHECKSUM_CHECK_ICMP          1
// #define CHECKSUM_CHECK_ICMP6         1
// #define LWIP_CHECKSUM_ON_COPY        0

// IPv6 options
#ifndef LWIP_IPV6
// #define LWIP_IPV6                       0
#endif  // !LWIP_IPV6
// #define IPV6_REASS_MAXAGE               60
// #define LWIP_IPV6_SCOPES                (LWIP_IPV6 && !LWIP_SINGLE_NETIF)
// #define LWIP_IPV6_SCOPES_DEBUG          0
// #define LWIP_IPV6_NUM_ADDRESSES         3
// #define LWIP_IPV6_FORWARD               0
// #define LWIP_IPV6_FRAG                  1
// #define LWIP_IPV6_REASS                 LWIP_IPV6
// #define LWIP_IPV6_SEND_ROUTER_SOLICIT   LWIP_IPV6
// #define LWIP_IPV6_AUTOCONFIG            LWIP_IPV6
// #define LWIP_IPV6_ADDRESS_LIFETIMES     LWIP_IPV6_AUTOCONFIG
// #define LWIP_IPV6_DUP_DETECT_ATTEMPTS   1
// #define LWIP_ICMP6                      LWIP_IPV6
// #define LWIP_ICMP6_DATASIZE             0
// #define LWIP_ICMP6_HL                   255
// #define LWIP_IPV6_MLD                   LWIP_IPV6
// #define MEMP_NUM_MLD6_GROUP             4
// #define LWIP_ND6_QUEUEING               LWIP_IPV6
// #define MEMP_NUM_ND6_QUEUE              20
// #define LWIP_ND6_NUM_NEIGHBORS          10
// #define LWIP_ND6_NUM_DESTINATIONS       10
// #define LWIP_ND6_NUM_PREFIXES           5
// #define LWIP_ND6_NUM_ROUTERS            3
// #define LWIP_ND6_MAX_MULTICAST_SOLICIT  3
// #define LWIP_ND6_MAX_UNICAST_SOLICIT    3
// #define LWIP_ND6_MAX_ANYCAST_DELAY_TIME 1000
// #define LWIP_ND6_MAX_NEIGHBOR_ADVERTISEMENT  3
// #define LWIP_ND6_REACHABLE_TIME         30000
// #define LWIP_ND6_RETRANS_TIMER          1000
// #define LWIP_ND6_DELAY_FIRST_PROBE_TIME 5000
// #define LWIP_ND6_ALLOW_RA_UPDATES       1
// #define LWIP_ND6_TCP_REACHABILITY_HINTS 1
// #define LWIP_ND6_RDNSS_MAX_DNS_SERVERS  0
// #define LWIP_IPV6_DHCP6                 0
// #define LWIP_IPV6_DHCP6_STATEFUL        0
// #define LWIP_IPV6_DHCP6_STATELESS       LWIP_IPV6_DHCP6
// #define LWIP_DHCP6_GET_NTP_SRV          0
// #define LWIP_DHCP6_MAX_NTP_SERVERS      1
// #define LWIP_DHCP6_MAX_DNS_SERVERS      DNS_MAX_SERVERS

// Hook options
#define LWIP_HOOK_FILENAME "qnethernet/lwip_hooks.h"
// #define LWIP_HOOK_TCP_ISN(local_ip, local_port, remote_ip, remote_port)
// #define LWIP_HOOK_TCP_INPACKET_PCB(pcb, hdr, optlen, opt1len, opt2, p)
// #define LWIP_HOOK_TCP_OUT_TCPOPT_LENGTH(pcb, internal_len)
// #define LWIP_HOOK_TCP_OUT_ADD_TCPOPTS(p, hdr, pcb, opts)
// #define LWIP_HOOK_IP4_INPUT(pbuf, input_netif)
// #define LWIP_HOOK_IP4_ROUTE()
// #define LWIP_HOOK_IP4_ROUTE_SRC(src, dest)
// #define LWIP_HOOK_IP4_CANFORWARD(src, dest)
// #define LWIP_HOOK_ETHARP_GET_GW(netif, dest)
// #define LWIP_HOOK_IP6_INPUT(pbuf, input_netif)
// #define LWIP_HOOK_IP6_ROUTE(src, dest)
// #define LWIP_HOOK_ND6_GET_GW(netif, dest)
// #define LWIP_HOOK_VLAN_CHECK(netif, eth_hdr, vlan_hdr)
// #define LWIP_HOOK_VLAN_SET(netif, p, src, dst, eth_type)
// #define LWIP_HOOK_MEMP_AVAILABLE(memp_t_type)
// #define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(pbuf, netif)
// #define LWIP_HOOK_DHCP_APPEND_OPTIONS(netif, dhcp, state, msg, msg_type, options_len_ptr)
// #define LWIP_HOOK_DHCP_PARSE_OPTION(netif, dhcp, state, msg, msg_type, option, len, pbuf, offset)
// #define LWIP_HOOK_DHCP6_APPEND_OPTIONS(netif, dhcp6, state, msg, msg_type, options_len_ptr, max_len)
// #define LWIP_HOOK_SOCKETS_SETSOCKOPT(s, sock, level, optname, optval, optlen, err)
// #define LWIP_HOOK_SOCKETS_GETSOCKOPT(s, sock, level, optname, optval, optlen, err)
// #define LWIP_HOOK_NETCONN_EXTERNAL_RESOLVE(name, addr, addrtype, err)

// Debugging options
// #define LWIP_DEBUG
// #define LWIP_DBG_MIN_LEVEL LWIP_DBG_LEVEL_ALL
// #define LWIP_DBG_TYPES_ON  LWIP_DBG_ON
// #define ETHARP_DEBUG       LWIP_DBG_OFF
// #define NETIF_DEBUG        LWIP_DBG_OFF
// #define PBUF_DEBUG         LWIP_DBG_OFF
// #define API_LIB_DEBUG      LWIP_DBG_OFF
// #define API_MSG_DEBUG      LWIP_DBG_OFF
// #define SOCKETS_DEBUG      LWIP_DBG_OFF
// #define ICMP_DEBUG         LWIP_DBG_OFF
// #define IGMP_DEBUG         LWIP_DBG_OFF
// #define INET_DEBUG         LWIP_DBG_OFF
// #define IP_DEBUG           LWIP_DBG_OFF
// #define IP_REASS_DEBUG     LWIP_DBG_OFF
// #define RAW_DEBUG          LWIP_DBG_OFF
// #define MEM_DEBUG          LWIP_DBG_OFF
// #define MEMP_DEBUG         LWIP_DBG_OFF
// #define SYS_DEBUG          LWIP_DBG_OFF
// #define TIMERS_DEBUG       LWIP_DBG_OFF
// #define TCP_DEBUG          LWIP_DBG_OFF
// #define TCP_INPUT_DEBUG    LWIP_DBG_OFF
// #define TCP_FR_DEBUG       LWIP_DBG_OFF
// #define TCP_RTO_DEBUG      LWIP_DBG_OFF
// #define TCP_CWND_DEBUG     LWIP_DBG_OFF
// #define TCP_WND_DEBUG      LWIP_DBG_OFF
// #define TCP_OUTPUT_DEBUG   LWIP_DBG_OFF
// #define TCP_RST_DEBUG      LWIP_DBG_OFF
// #define TCP_QLEN_DEBUG     LWIP_DBG_OFF
// #define UDP_DEBUG          LWIP_DBG_OFF
// #define TCPIP_DEBUG        LWIP_DBG_OFF
// #define SLIP_DEBUG         LWIP_DBG_OFF
// #define DHCP_DEBUG         LWIP_DBG_OFF
// #define AUTOIP_DEBUG       LWIP_DBG_OFF
// #define ACD_DEBUG          LWIP_DBG_OFF
// #define DNS_DEBUG          LWIP_DBG_OFF
// #define IP6_DEBUG          LWIP_DBG_OFF
// #define DHCP6_DEBUG        LWIP_DBG_OFF
// #define LWIP_TESTMODE      0

// Performance tracking options
// #define LWIP_PERF 0

// HTTPD options (httpd_opts.h)
// #define LWIP_HTTPD_DYNAMIC_HEADERS 0

// TFTP options (tftp_opts.h)
#define TFTP_MAX_FILENAME_LEN 512  /* 20 */

// SNTP options (sntp_opts.h)
#define SNTP_SERVER_DNS     1  /* 0 */
#define SNTP_CHECK_RESPONSE 3  /* 0 */
#define SNTP_UPDATE_DELAY   600000  /* 3600000 */

#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
#include <imxrt.h>
#include <sys/time.h>
#define SNTP_SET_SYSTEM_TIME_US(sec, us)                    \
  do {                                                      \
    /* Assume 'sec' and 'us' have the proper range */       \
    u32_t hi = (sec) >> 17;                                 \
    u32_t lo = ((sec) << 15) | ((us) << 9)/15625;           \
                                                            \
    /* Code similar to teensy4 core's rtc_set(t) */         \
    /* This version sets the microseconds too    */         \
                                                            \
    /* Stop the RTC */                                      \
    SNVS_HPCR &= ~(SNVS_HPCR_RTC_EN | SNVS_HPCR_HP_TS);     \
    while (SNVS_HPCR & SNVS_HPCR_RTC_EN) ;  /* Wait */      \
    /* Stop the SRTC */                                     \
    SNVS_LPCR &= ~SNVS_LPCR_SRTC_ENV;                       \
    while (SNVS_LPCR & SNVS_LPCR_SRTC_ENV) ;  /* Wait */    \
    /* Set the SRTC */                                      \
    SNVS_LPSRTCLR = lo;                                     \
    SNVS_LPSRTCMR = hi;                                     \
    /* Start the SRTC */                                    \
    SNVS_LPCR |= SNVS_LPCR_SRTC_ENV;                        \
    while (!(SNVS_LPCR & SNVS_LPCR_SRTC_ENV)) ;  /* Wait */ \
    /* Start the RTC and sync it to the SRTC */             \
    SNVS_HPCR |= SNVS_HPCR_RTC_EN | SNVS_HPCR_HP_TS;        \
  } while (0)
#define SNTP_GET_SYSTEM_TIME(sec, us) \
  do {                                \
    struct timeval tv;                \
    gettimeofday(&tv, NULL);          \
    (sec) = tv.tv_sec;                \
    (us) = tv.tv_usec;                \
  } while (0)  /* do { (sec) = 0; (us) = 0; } while(0) */
#endif  // defined(TEENSYDUINO) && defined(__IMXRT1062__)

// MDNS options (mdns_opts.h)
#ifndef LWIP_MDNS_RESPONDER
#define LWIP_MDNS_RESPONDER (LWIP_UDP) && (LWIP_IGMP)  /* 0 */
// If you change LWIP_MDNS_RESPONDER to zero here then:
// 1. Reduce MEMP_NUM_SYS_TIMEOUT by 8
// 2. Change LWIP_AUTOIP and LWIP_DHCP_AUTOIP_COOP to 0
// 3. Reduce LWIP_NUM_NETIF_CLIENT_DATA by 1
// 4. Optionally reduce MEMP_NUM_UDP_PCB by 1
#endif  // !LWIP_MDNS_RESPONDER
// #define MDNS_RESP_USENETIF_EXTCALLBACK LWIP_NETIF_EXT_STATUS_CALLBACK
#ifndef MDNS_MAX_SERVICES
#define MDNS_MAX_SERVICES   3  /* 1 */
#endif  // !MDNS_MAX_SERVICES
// #define MDNS_DEBUG          LWIP_DBG_OFF

// Mbed TLS options
// See lwip/apps/altcp_tls_mbedtls_opts.h for descriptions
#ifndef LWIP_ALTCP_TLS_MBEDTLS
// #define LWIP_ALTCP_TLS_MBEDTLS                       0
#endif  // !LWIP_ALTCP_TLS_MBEDTLS
// #define ALTCP_MBEDTLS_DEBUG                          LWIP_DBG_OFF
// #define ALTCP_MBEDTLS_LIB_DEBUG                      LWIP_DBG_OFF
// #define ALTCP_MBEDTLS_LIB_DEBUG_LEVEL_MIN            0
// #define ALTCP_MBEDTLS_USE_SESSION_CACHE              0
// #define ALTCP_MBEDTLS_SESSION_CACHE_SIZE             30
// #define ALTCP_MBEDTLS_SESSION_CACHE_TIMEOUT_SECONDS  (60 * 60)
// #define ALTCP_MBEDTLS_USE_SESSION_TICKETS            0
// #define ALTCP_MBEDTLS_SESSION_TICKET_CIPHER          MBEDTLS_CIPHER_AES_256_GCM
// #define ALTCP_MBEDTLS_SESSION_TICKET_TIMEOUT_SECONDS (60 * 60 * 24)
// #define ALTCP_MBEDTLS_AUTHMODE                       MBEDTLS_SSL_VERIFY_OPTIONAL

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

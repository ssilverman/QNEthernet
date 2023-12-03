// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// tls_template.c implements the minimum possible in order to get the
// project to compile with the LWIP_ALTCP and LWIP_ALTCP_TLS
// options set. If these are defined somewhere else (such as in an
// included library) then remove or comment these out. (They're
// actually marked as "weak" to save you the trouble, but feel free to
// remove them.)
//
// This file is part of the QNEthernet library.

#include <lwip/altcp.h>
#include <lwip/opt.h>

#if LWIP_ALTCP && LWIP_ALTCP_TLS && !LWIP_ALTCP_TLS_MBEDTLS

struct altcp_tls_config {
};

__attribute__((weak))
void altcp_tls_free_config(struct altcp_tls_config *conf) {
}

__attribute__((weak))
struct altcp_pcb *altcp_tls_wrap(struct altcp_tls_config *config,
                                 struct altcp_pcb *inner_pcb) {
  return NULL;
}

#endif  // LWIP_ALTCP && LWIP_ALTCP_TLS && !LWIP_ALTCP_TLS_MBEDTLS
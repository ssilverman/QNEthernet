// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// tls_template.c implements the minimum possible in order to get the
// project to compile with the LWIP_ALTCP and LWIP_ALTCP_TLS
// options set.
//
// This file is part of the QNEthernet library.

#include <lwip/altcp.h>
#include <lwip/opt.h>

#if LWIP_ALTCP && LWIP_ALTCP_TLS

struct altcp_tls_config {
};

void altcp_tls_free_config(struct altcp_tls_config *conf) {
}

struct altcp_pcb *altcp_tls_wrap(struct altcp_tls_config *config,
                                 struct altcp_pcb *inner_pcb) {
  return NULL;
}

#endif  // LWIP_ALTCP && LWIP_ALTCP_TLS

// SPDX-FileCopyrightText: (c) 2023-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// tls_template.c implements the minimum possible in order to get the
// project to compile with the LWIP_ALTCP and LWIP_ALTCP_TLS
// options set. If these are defined somewhere else (such as in an
// included library) then remove or comment these out. (They're
// actually marked as "weak" to save you the trouble, but feel free to
// remove them.)
//
// This file is part of the QNEthernet library.

// C includes
#include <stddef.h>

#include <lwip/altcp.h>
#include <lwip/arch.h>
#include <lwip/err.h>
#include <lwip/opt.h>
#include <qnethernet_opts.h>

#if LWIP_ALTCP && LWIP_ALTCP_TLS

#if !LWIP_ALTCP_TLS_MBEDTLS
// mbedtls would define these

struct altcp_tls_config {
};

void altcp_tls_free_config(struct altcp_tls_config *conf) {
  LWIP_UNUSED_ARG(conf);
}

struct altcp_pcb *altcp_tls_wrap(struct altcp_tls_config *config,
                                 struct altcp_pcb *inner_pcb) {
  LWIP_UNUSED_ARG(config);
  LWIP_UNUSED_ARG(inner_pcb);
  return NULL;
}

#if QNETHERNET_ALTCP_TLS_ADAPTER
// QNEthernet's altcp_tls_adapter needs these

struct altcp_tls_config *altcp_tls_create_config_server(u8_t cert_count) {
  LWIP_UNUSED_ARG(cert_count);
  return NULL;
}

err_t altcp_tls_config_server_add_privkey_cert(
    struct altcp_tls_config *config,
    const u8_t *privkey,      size_t privkey_len,
    const u8_t *privkey_pass, size_t privkey_pass_len,
    const u8_t *cert,         size_t cert_len) {
  LWIP_UNUSED_ARG(config);
  LWIP_UNUSED_ARG(privkey);
  LWIP_UNUSED_ARG(privkey_len);
  LWIP_UNUSED_ARG(privkey_pass);
  LWIP_UNUSED_ARG(privkey_pass_len);
  LWIP_UNUSED_ARG(cert);
  LWIP_UNUSED_ARG(cert_len);
  return ERR_OK;
}

struct altcp_tls_config *altcp_tls_create_config_client(const u8_t *cert,
                                                        size_t cert_len) {
  LWIP_UNUSED_ARG(cert);
  LWIP_UNUSED_ARG(cert_len);
  return NULL;
}

#endif  // QNETHERNET_ALTCP_TLS_ADAPTER

#else
// mbedtls needs these

int mbedtls_hardware_poll(void *data,
                          unsigned char *output, size_t len, size_t *olen) {
  // A real implementation would fill in random data
  LWIP_UNUSED_ARG(data);
  LWIP_UNUSED_ARG(output);
  LWIP_UNUSED_ARG(len);
  LWIP_UNUSED_ARG(olen);
  return 0;
}

#endif  // !LWIP_ALTCP_TLS_MBEDTLS

#endif  // LWIP_ALTCP && LWIP_ALTCP_TLS

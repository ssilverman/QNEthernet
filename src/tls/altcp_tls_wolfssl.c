// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// altcp_tls_wolfssl.c implements the functions defined in altcp_tls.h.
// This file is part of the QNEthernet library.

#include "lwip/opt.h"

#if LWIP_ALTCP && LWIP_ALTCP_TLS/* && QNETHERNET_ENABLE_WOLFSSL*/

#include <stdbool.h>
#include <string.h>

#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/ssl.h>

#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/err.h"
#include "lwip/priv/altcp_priv.h"

struct altcp_tls_config {
  WOLFSSL_METHOD *method;
  const u8_t *ca;
  size_t      ca_len;
  const u8_t *privkey;
  size_t      privkey_len;
  const u8_t *privkey_pass;
  size_t      privkey_pass_len;
  const u8_t *cert;
  size_t      cert_len;
};

struct altcp_wolfssl_state {
  WOLFSSL_CTX *ctx;
  WOLFSSL     *ssl;
};

struct altcp_tls_config *altcp_tls_create_config_server(uint8_t cert_count) {
  LWIP_UNUSED_ARG(cert_count);

  struct altcp_tls_config *config =
      mem_calloc(1, sizeof(struct altcp_tls_config));
  if (config == NULL) {
    return NULL;
  }
  config->method = wolfTLS_server_method();
  if (config->method == NULL) {
    mem_free(config);
    return NULL;
  }

  return config;
}

err_t altcp_tls_config_server_add_privkey_cert(
    struct altcp_tls_config *config,
    const u8_t *privkey, size_t privkey_len,
    const u8_t *privkey_pass, size_t privkey_pass_len,
    const u8_t *cert, size_t cert_len) {
  config->privkey          = privkey;
  config->privkey_len      = privkey_len;
  config->privkey_pass     = privkey_pass;
  config->privkey_pass_len = privkey_pass_len
  config->cert             = cert;
  config->cert_len         = cert_len;

  return ERR_OK;
}

struct altcp_tls_config *altcp_tls_create_config_server_privkey_cert(
    const u8_t *privkey, size_t privkey_len,
    const u8_t *privkey_pass, size_t privkey_pass_len,
    const u8_t *cert, size_t cert_len) {
  struct altcp_tls_config *config = altcp_tls_create_config_server(0);
  if (config == NULL) {
    return NULL;
  }
  config->privkey          = privkey;
  config->privkey_len      = privkey_len;
  config->privkey_pass     = privkey_pass;
  config->privkey_pass_len = privkey_pass_len
  config->cert             = cert;
  config->cert_len         = cert_len;
}

struct altcp_tls_config *altcp_tls_create_config_client(const u8_t *cert,
                                                        size_t cert_len) {
  struct altcp_tls_config *config =
      mem_calloc(1, sizeof(struct altcp_tls_config));
  if (config == NULL) {
    return NULL;
  }

  config->method = wolfTLS_client_method();
  if (config->method == NULL) {
    mem_free(config);
    return NULL;
  }

  config->cert     = cert;
  config->cert_len = cert_len;

  return config;
}

struct altcp_tls_config *altcp_tls_create_config_client_2wayauth(
    const u8_t *ca, size_t ca_len,
    const u8_t *privkey, size_t privkey_len,
    const u8_t *privkey_pass, size_t privkey_pass_len,
    const u8_t *cert, size_t cert_len) {
  struct altcp_tls_config *config =
      altcp_tls_create_config_client(cert, cert_len);
  if (config == NULL) {
    return NULL;
  }

  config->ca               = ca;
  config->ca_len           = ca_len;
  config->privkey          = privkey;
  config->privkey_len      = privkey_len;
  config->privkey_pass     = privkey_pass;
  config->privkey_pass_len = privkey_pass_len;
  config->cert             = cert;
  config->cert_len         = cert_len;

  return config;
}

void altcp_tls_free_config(struct altcp_tls_config *conf) {
  if (conf != NULL) {
    mem_free(conf);
  }
}

void altcp_tls_free_entropy(void) {

}

static const struct altcp_functions altcp_wolfssl_functions;

static int altcp_wolfssl_passwd_cb(char *passwd, int sz, int rw,
                                   void *userdata) {
  if (sz < 0) {  // Protect against a huge size_t
    return 0;
  }
  struct altcp_tls_config *config = (struct altcp_tls_config *)userdata;
  strncpy(passwd, (char *)config->privkey_pass, sz);
  return config->privkey_pass_len;
}

struct altcp_pcb *altcp_tls_wrap(struct altcp_tls_config *config,
                                 struct altcp_pcb *inner_pcb) {
  if (config == NULL || inner_pcb == NULL) {
    return NULL;
  }

  struct altcp_wolfssl_state *state = NULL;
  struct altcp_pcb *ret             = NULL;
  WOLFSSL_CTX *ctx                  = NULL;
  WOLFSSL *ssl                      = NULL;

  state = mem_calloc(1, sizeof(struct altcp_wolfssl_state));
  if (state == NULL) {
    goto altcp_tls_wrap_cleanup;
  }

  ret = altcp_alloc();
  if (ret == NULL) {
    goto altcp_tls_wrap_cleanup;
  }

  static bool initted = false;
  if (!initted) {
    wolfSSL_Init();
    initted = true;
  }

  ctx = wolfSSL_CTX_new((WOLFSSL_METHOD *)config);
  if (ctx == NULL) {
    goto altcp_tls_wrap_cleanup;
  }
  ssl = wolfSSL_new(ctx);
  if (ssl == NULL) {
    goto altcp_tls_wrap_cleanup;
  }

  // TODO: Check lengths because types are different
  if (config->ca != NULL) {
    wolfSSL_CTX_use_certificate_buffer(ctx, config->ca, config->ca_len,
                                       CA_TYPE);
  }
  if (config->privkey != NULL) {
    wolfSSL_CTX_use_certificate_buffer(ctx,
                                       config->privkey, config->privkey_len,
                                       PRIVATEKEY_TYPE);
    if (config->privkey_pass != NULL) {
      wolfSSL_CTX_set_default_passwd_cb(ctx, &altcp_wolfssl_passwd_cb);
      wolfSSL_CTX_set_default_passwd_cb_userdata(ctx, config);
    }
  }
  if (config->cert != NULL) {
    wolfSSL_CTX_use_certificate_buffer(ctx, config->cert, config->cert_len,
                                       CERT_TYPE);
  }

  state->ctx = ctx;
  state->ssl = ssl;

  ret->fns        = &altcp_wolfssl_functions;
  ret->inner_conn = inner_pcb;
  ret->state      = state;

  return ret;

altcp_tls_wrap_cleanup:
  if (ssl != NULL) {
    wolfSSL_free(ssl);
  }
  if (ctx != NULL) {
    wolfSSL_CTX_free(ctx);
  }
  if (ret != NULL) {
    altcp_free(ret);
  }
  if (state != NULL) {
    mem_free(state);
  }
  return NULL;
}

void *altcp_tls_context(struct altcp_pcb *conn) {
  if (conn) {
    return conn->state;
  }
  return NULL;
}

// --------------------------------------------------------------------------
//  altcp_pcb Functions
// --------------------------------------------------------------------------

static void altcp_wolfssl_abort(struct altcp_pcb *conn) {
  // TODO
  if (conn && conn->inner_conn) {
    altcp_abort(conn->inner_conn);
  }
}

static err_t altcp_wolfssl_write(struct altcp_pcb *conn,
                                 const void *dataptr, u16_t len,
                                 u8_t apiflags) {
  if (wolfSSL_write(NULL, dataptr, len) != len) {
    return ERR_MEM;
  }
  return ERR_OK;
}

static void altcp_wolfssl_dealloc(struct altcp_pcb *conn) {
  if (conn == NULL) {
    return;
  }
  struct altcp_wolfssl_state *state = conn->state;
  if (state != NULL) {
    if (state->ssl != NULL) {
      wolfSSL_free(state->ssl);
    }
    if (state->ctx != NULL) {
      wolfSSL_CTX_free(state->ctx);
    }
    mem_free(state);
    conn->state = NULL;
  }
}

static const struct altcp_functions altcp_wolfssl_functions = {
    NULL,
    NULL,
    &altcp_default_bind,
    NULL,
    NULL,
    &altcp_wolfssl_abort,  // No altcp_default_abort
    NULL,
    &altcp_default_shutdown,
    &altcp_wolfssl_write,
    &altcp_default_output,
    NULL,
    NULL,
    &altcp_default_sndqueuelen,
    &altcp_default_nagle_disable,
    &altcp_default_nagle_enable,
    &altcp_default_nagle_disabled,
    &altcp_default_setprio,
    &altcp_wolfssl_dealloc,
    &altcp_default_get_tcp_addrinfo,
    &altcp_default_get_ip,
    &altcp_default_get_port,
#if LWIP_TCP_KEEPALIVE
    &altcp_default_keepalive_disable,
    &altcp_default_keepalive_enable,
#endif
#ifdef LWIP_DEBUG
    &altcp_default_dbg_get_tcp_state,
#endif
};

#endif  // LWIP_ALTCP && LWIP_ALTCP_TLS

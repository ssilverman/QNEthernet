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
#include "lwip_t41.h"

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
  // Context
  WOLFSSL_CTX *ctx;
  WOLFSSL     *ssl;

  // Incoming data
  struct pbuf *pbuf;
  int          read;  // The number of bytes read from pbuf
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
  config->privkey_pass_len = privkey_pass_len;
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

// --------------------------------------------------------------------------
//  Inner Callback Functions
// --------------------------------------------------------------------------

static err_t altcp_wolfssl_inner_recv(void *arg, struct altcp_pcb *inner_conn,
                                      struct pbuf *p, err_t err) {
  if (arg == NULL || inner_conn == NULL) {
    return ERR_ARG;
  }

  struct altcp_pcb *conn = (struct altcp_pcb *)arg;
  struct altcp_wolfssl_state *state = (struct altcp_wolfssl_state *)conn->state;

  LWIP_ASSERT("pcb mismatch", conn->inner_conn == inner_conn);

  // Some error or already closed
  if (err != ERR_OK || state == NULL) {
    if (p != NULL) {
      pbuf_free(p);
    }
    if (err != ERR_CLSD && err != ERR_ABRT) {
      if (altcp_close(conn) != ERR_OK) {
        altcp_abort(conn);
        // Note: If closing conn, then the implementation MUST abort inner_conn
        return ERR_ABRT;
      }
    }
    return ERR_OK;
  }

  // Closed by remote
  if (p == NULL) {
    altcp_close(conn);
    return ERR_OK;
  }

  // Append data
  if (state->pbuf == NULL) {
    state->pbuf = p;
  } else {
    if (state->pbuf != p) {
      tcp_recved(inner_conn, p->tot_len);
      pbuf_cat(state->pbuf, p);
    }
  }

  return ERR_OK;
}

static err_t altcp_wolfssl_inner_sent(void *arg, struct altcp_pcb *inner_conn,
                                      u16_t len) {
  if (arg == NULL || inner_conn == NULL) {
    return ERR_ARG;
  }
  return ERR_OK;
}

static err_t altcp_wolfssl_inner_err(void *arg, err_t err) {
  if (arg == NULL) {
    return ERR_ARG;
  }
  struct altcp_pcb *conn = arg;  // TODO
  conn->inner_conn = NULL;  // This has already been freed, according to tcp_err()
  if (conn->err) {
    conn->err(conn->arg, err);
  }
  altcp_free(conn);
}

// --------------------------------------------------------------------------
//  wolfSSL Callbacks
// --------------------------------------------------------------------------

static int altcp_wolfssl_passwd_cb(char *passwd, int sz, int rw,
                                   void *userdata) {
  if (sz < 0) {  // Protect against a huge size_t
    return 0;
  }
  struct altcp_tls_config *config = (struct altcp_tls_config *)userdata;
  strncpy(passwd, (char *)config->privkey_pass, sz);
  return config->privkey_pass_len;
}

static int altcp_wolfssl_recv_cb(WOLFSSL *ssl, char *buf, int sz, void *ctx) {
  if (sz <= 0) {
    return 0;
  }

  struct altcp_pcb *inner_conn = (struct altcp_pcb *)ctx;  // Inner pcb

  if (inner_conn == NULL) {
    return WOLFSSL_CBIO_ERR_GENERAL;
  }
  if (inner_conn->state == NULL) {
    return WOLFSSL_CBIO_ERR_CONN_CLOSE;
  }

  struct altcp_wolfssl_state *state =
      (struct altcp_wolfssl_state *)inner_conn->state;
  struct pbuf *p = state->pbuf;
  if (p == NULL || sz > p->tot_len) {
    // An alternative is to return what's available and call Ethernet.loop()
    return WOLFSSL_CBIO_ERR_WANT_READ;
  }

  struct pbuf *pHead = p;
  int read = 0;

  while (p != NULL) {
    int toRead = LWIP_MIN(p->len - state->read, sz);
    if (read + toRead > sz) {  // Sanity check
      return WOLFSSL_CBIO_ERR_GENERAL;
    }
    XMEMCPY(&buf[read], &((const char *)p->payload)[state->read], toRead);
    state->read += toRead;
    if (state->read >= p->len) {
      state->pbuf = p->next;
      p = state->pbuf;
      state->read = 0;
    }
    read += toRead;

    if (read >= sz) {
      // Free all until the current pbuf
      if (p != NULL) {
        pbuf_ref(p);
      }
      pbuf_free(pHead);
    }
  }

  return read;
}

static int altcp_wolfssl_send_cb(WOLFSSL *ssl, char *buf, int sz, void *ctx) {
  if (sz <= 0) {
    return 0;
  }

  struct altcp_pcb *inner_conn = (struct altcp_pcb *)ctx;  // Inner pcb

  if (inner_conn == NULL) {
    return WOLFSSL_CBIO_ERR_GENERAL;
  }
  if (inner_conn->state == NULL) {
    return WOLFSSL_CBIO_ERR_CONN_CLOSE;
  }

  size_t size = sz;
  size_t sent = 0;

  while (size > 0) {
    u16_t sndBufSize = altcp_sndbuf(inner_conn);
    if (sndBufSize == 0) {
      // TODO: Need input processing here?
      enet_proc_input();
      if (sent > 0) {
        return sent;
      }
      return WOLFSSL_CBIO_ERR_WANT_WRITE;
    }
    u16_t toWrite = LWIP_MIN(size, sndBufSize);
    switch (altcp_write(inner_conn, buf, toWrite, TCP_WRITE_FLAG_COPY)) {
      case ERR_OK:
        sent += toWrite;
        size -= toWrite;
        break;
      case ERR_MEM:
        // TODO: Need input processing here?
        enet_proc_input();
        if (sent > 0) {
          return sent;
        }
        return WOLFSSL_CBIO_ERR_WANT_WRITE;
      default:
        return WOLFSSL_CBIO_ERR_GENERAL;
    }
  }
}

// --------------------------------------------------------------------------
//  More altcp Function Implementations
// --------------------------------------------------------------------------

static const struct altcp_functions altcp_wolfssl_functions;

struct altcp_pcb *altcp_tls_wrap(struct altcp_tls_config *config,
                                 struct altcp_pcb *inner_pcb) {
  if (config == NULL || inner_pcb == NULL) {
    return NULL;
  }

  struct altcp_wolfssl_state *state = NULL;
  struct altcp_pcb *pcb             = NULL;
  WOLFSSL_CTX *ctx                  = NULL;
  WOLFSSL *ssl                      = NULL;

  state = mem_calloc(1, sizeof(struct altcp_wolfssl_state));
  if (state == NULL) {
    goto altcp_tls_wrap_cleanup;
  }

  pcb = altcp_alloc();
  if (pcb == NULL) {
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

  // wolfSSL_SetIO_LwIP(ssl, inner_pcb, NULL, NULL, NULL);
  wolfSSL_CTX_SetIORecv(ctx, &altcp_wolfssl_recv_cb);
  wolfSSL_SetIOReadCtx(ssl, inner_pcb);
  wolfSSL_CTX_SetIOSend(ctx, &altcp_wolfssl_send_cb);
  wolfSSL_SetIOWriteCtx(ssl, inner_pcb);

  state->ctx = ctx;
  state->ssl = ssl;

  altcp_arg(inner_pcb, pcb);
  altcp_recv(inner_pcb, altcp_wolfssl_inner_recv);
  altcp_sent(inner_pcb, altcp_wolfssl_inner_sent);
  altcp_err(inner_pcb, altcp_wolfssl_inner_err);
  pcb->fns        = &altcp_wolfssl_functions;
  pcb->inner_conn = inner_pcb;
  pcb->state      = state;

  return pcb;

altcp_tls_wrap_cleanup:
  if (ssl != NULL) {
    wolfSSL_free(ssl);
  }
  if (ctx != NULL) {
    wolfSSL_CTX_free(ctx);
  }
  if (pcb != NULL) {
    altcp_free(pcb);
  }
  if (state != NULL) {
    mem_free(state);
  }
  return NULL;
}

void *altcp_tls_context(struct altcp_pcb *conn) {
  if (conn != NULL) {
    return conn->state;
  }
  return NULL;
}

// --------------------------------------------------------------------------
//  altcp_pcb Functions
// --------------------------------------------------------------------------

static err_t altcp_wolfssl_inner_poll(void *arg, struct altcp_pcb *conn) {
  if (conn) {

  }
}

static void altcp_wolfssl_set_poll(struct altcp_pcb *conn, u8_t interval) {
  if (conn) {
    altcp_poll(conn, &altcp_wolfssl_inner_poll, interval);
  }
}

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
    &altcp_wolfssl_set_poll,
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

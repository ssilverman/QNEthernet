// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// altcp_tls_wolfssl.c implements the functions defined in altcp_tls.h.
// This file is part of the QNEthernet library.

#include "lwip/opt.h"

#if LWIP_ALTCP && LWIP_ALTCP_TLS/* && QNETHERNET_ENABLE_WOLFSSL*/

#include <string.h>

#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/ssl.h>

#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/err.h"
#include "lwip/priv/altcp_priv.h"
#include "lwip_t41.h"

// --------------------------------------------------------------------------
//  Structure Definitions
// --------------------------------------------------------------------------

// Holds information about one certificate.
struct altcp_tls_cert {
  const u8_t *privkey;
  size_t      privkey_len;
  const u8_t *privkey_pass;
  size_t      privkey_pass_len;
  const u8_t *cert;
  size_t      cert_len;
};

// Holds configuration for one connection.
struct altcp_tls_config {
  WOLFSSL_METHOD *method;

  struct altcp_tls_cert *cert_list;
  size_t                 cert_list_size;
  size_t                 cert_list_capacity;

  const u8_t *ca;
  size_t      ca_len;
};

// Holds the state for one connection.
struct altcp_wolfssl_state {
  struct altcp_tls_config *config;

  // Context
  WOLFSSL_CTX *ctx;
  WOLFSSL     *ssl;

  // Incoming data
  struct pbuf *pbuf;
  int          read;  // The number of bytes read from pbuf
  struct pbuf *pbuf_upper;  // Decoded data
  int          read_upper;  // Number of bytes read from pbuf_upper
};

// Creates a generic config. If this returns NULL then the method will have
// been freed.
static struct altcp_tls_config *altcp_tls_create_config(WOLFSSL_METHOD *method,
                                                        uint8_t cert_count) {
  if (method == NULL) {
    return NULL;
  }

  // The config itself
  struct altcp_tls_config *config =
      mem_calloc(1, sizeof(struct altcp_tls_config));
  if (config == NULL) {
    goto altcp_tls_create_config_cleanup;
  }

  config->method = method;

  // Cert list
  if (cert_count > 0) {
    config->cert_list = mem_calloc(cert_count, sizeof(struct altcp_tls_cert));
    if (config->cert_list == NULL) {
      goto altcp_tls_create_config_cleanup;
    }
    config->cert_list_capacity = cert_count;
  }
  // Note: All other elements are zero because of calloc

  return config;

altcp_tls_create_config_cleanup:
  if (config != NULL) {
    if (config->cert_list != NULL) {
      mem_free(config->cert_list);
    }
    mem_free(config);
  }
  XFREE(method, NULL, DYNAMIC_TYPE_METHOD);
  return NULL;
}

// --------------------------------------------------------------------------
//  altcp_tls.h Definitions
// --------------------------------------------------------------------------

struct altcp_tls_config *altcp_tls_create_config_server(uint8_t cert_count) {
  return altcp_tls_create_config(wolfTLS_server_method(), cert_count);
}

err_t altcp_tls_config_server_add_privkey_cert(
    struct altcp_tls_config *config,
    const u8_t *privkey, size_t privkey_len,
    const u8_t *privkey_pass, size_t privkey_pass_len,
    const u8_t *cert, size_t cert_len) {

  if (config == NULL) {
    return ERR_VAL;
  }

  if (config->cert_list_size >= config->cert_list_capacity) {
    return ERR_MEM;
  }

  struct altcp_tls_cert *c = &config->cert_list[config->cert_list_size];
  c->privkey          = privkey;
  c->privkey_len      = privkey_len;
  c->privkey_pass     = privkey_pass;
  c->privkey_pass_len = privkey_pass_len;
  c->cert             = cert;
  c->cert_len         = cert_len;
  config->cert_list_size++;

  return ERR_OK;
}

struct altcp_tls_config *altcp_tls_create_config_server_privkey_cert(
    const u8_t *privkey, size_t privkey_len,
    const u8_t *privkey_pass, size_t privkey_pass_len,
    const u8_t *cert, size_t cert_len) {
  struct altcp_tls_config *config = altcp_tls_create_config_server(1);
  if (config == NULL) {
    return NULL;
  }
  err_t err = altcp_tls_config_server_add_privkey_cert(
      config,
      privkey, privkey_len,
      privkey_pass, privkey_pass_len,
      cert, cert_len);
  if (err != ERR_OK) {
    altcp_tls_free_config(config);
    return NULL;
  }
  return config;
}

struct altcp_tls_config *altcp_tls_create_config_client(const u8_t *cert,
                                                        size_t cert_len) {
  struct altcp_tls_config *config =
      altcp_tls_create_config(wolfTLS_client_method(), 1);
  if (config == NULL) {
    return NULL;
  }

  struct altcp_tls_cert *c = &config->cert_list[0];
  c->cert     = cert;
  c->cert_len = cert_len;
  config->cert_list_size = 1;

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

  config->ca     = ca;
  config->ca_len = ca_len;

  struct altcp_tls_cert *c = &config->cert_list[0];
  c->privkey          = privkey;
  c->privkey_len      = privkey_len;
  c->privkey_pass     = privkey_pass;
  c->privkey_pass_len = privkey_pass_len;

  return config;
}

void altcp_tls_free_config(struct altcp_tls_config *conf) {
  if (conf == NULL) {
    return;
  }
  if (conf->method != NULL) {
    XFREE(conf->method, NULL, DYNAMIC_TYPE_METHOD);
  }
  if (conf->cert_list != NULL) {
    mem_free(conf->cert_list);
  }
  mem_free(conf);
}

void altcp_tls_free_entropy(void) {

}

// --------------------------------------------------------------------------
//  Inner Callback Functions
// --------------------------------------------------------------------------

// Closes or aborts the given connection.
static err_t closeOrAbort(struct altcp_pcb *conn, err_t err) {
  if (err != ERR_CLSD && err != ERR_ABRT) {
    if (altcp_close(conn) != ERR_OK) {
      altcp_abort(conn);
      // Note: If closing conn, then the implementation MUST abort inner_conn
      return ERR_ABRT;
    }
  }
  return ERR_OK;
}

static err_t altcp_wolfssl_inner_recv(void *arg, struct altcp_pcb *inner_conn,
                                      struct pbuf *p, err_t err) {
  if (arg == NULL || inner_conn == NULL) {
    return ERR_VAL;
  }

  struct altcp_pcb *conn = (struct altcp_pcb *)arg;
  struct altcp_wolfssl_state *state = (struct altcp_wolfssl_state *)conn->state;

  LWIP_ASSERT("pcb mismatch", conn->inner_conn == inner_conn);

  // Some error or already closed
  if (err != ERR_OK || state == NULL) {
    if (p != NULL) {
      pbuf_free(p);
    }
    if (state == NULL) {
      if (conn->err) {
        conn->err(conn->arg, ERR_CLSD);
      }
      return closeOrAbort(inner_conn, err);
    } else {
      if (conn->err) {
        conn->err(conn->arg, err);
      }
      return closeOrAbort(conn, err);
    }
  }

  // Closed by remote
  if (p == NULL) {
    // TODO: More state checking?
    if (conn->recv) {
      return conn->recv(conn->arg, conn, NULL, ERR_OK);
    }
    return closeOrAbort(conn, err);
  }

  // Append data
  if (state->pbuf == NULL) {
    state->pbuf = p;
  } else {
    if (state->pbuf != p) {
      altcp_recved(inner_conn, p->tot_len);
      pbuf_cat(state->pbuf, p);
    }
  }

  do {
    // Allocate a full-sized unchained PBUF_POOL: this is for RX!
    struct pbuf *buf = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE, PBUF_POOL);
    if (buf == NULL) {
      return ERR_OK;
    }

    int ret = wolfSSL_read(state->ssl, buf->payload, PBUF_POOL_BUFSIZE);
    if (ret > 0) {
      LWIP_ASSERT("bogus receive length", ret <= PBUF_POOL_BUFSIZE);
      if (conn->recv) {
        pbuf_realloc(buf, ret);
        switch (conn->recv(conn->arg, conn, buf, ERR_OK)) {
          case ERR_OK:
            return ERR_OK;
          case ERR_ABRT:
            return ERR_ABRT;
          default:
            pbuf_free(buf);
        }
      } else {
        pbuf_free(buf);
      }
      continue;
    }

    // Check for closed
    if (ret == 0) {

    }
  } while (true);

  return ERR_OK;
}

static err_t altcp_wolfssl_inner_sent(void *arg, struct altcp_pcb *inner_conn,
                                      u16_t len) {
  struct altcp_pcb *conn = (struct altcp_pcb *)arg;

  if (conn != NULL) {
    LWIP_ASSERT("pcb mismatch", conn->inner_conn == inner_conn);

    if (conn->sent) {
      return conn->sent(conn->arg, conn, len);
    }
  }

  return ERR_OK;
}

static void altcp_wolfssl_inner_err(void *arg, err_t err) {
  if (arg == NULL) {
    return;
  }

  struct altcp_pcb *conn = arg;  // TODO
  conn->inner_conn = NULL;  // This has already been freed, according to tcp_err()
  if (conn->err) {
    conn->err(conn->arg, err);
  }
  altcp_free(conn);
}

// TODO
static err_t altcp_wolfssl_inner_connected(void *arg,
                                           struct altcp_pcb *inner_conn,
                                           err_t err) {
  if (arg == NULL) {
    return ERR_VAL;
  }

  struct altcp_pcb *conn = (struct altcp_pcb *)arg;
  struct altcp_wolfssl_state *state = (struct altcp_wolfssl_state *)conn->state;

  LWIP_ASSERT("pcb mismatch", conn->inner_conn == inner_conn);

  if (state == NULL) {
    if (conn->err) {
      conn->err(conn->arg, ERR_CLSD);
    }
    return closeOrAbort(inner_conn, err);
  }

  if (err != ERR_OK) {
    if (conn->connected) {
      return conn->connected(conn->arg, conn, err);
    }
    return closeOrAbort(conn, err);
  }

  switch (wolfSSL_connect(state->ssl)) {
    case WOLFSSL_SUCCESS:
      return ERR_OK;
    default:
      return closeOrAbort(conn, err);
  }
}

static err_t altcp_wolfssl_inner_accept(void *arg,
                                        struct altcp_pcb *accepted_conn,
                                        err_t err) {
  struct altcp_pcb *listen_conn = (struct altcp_pcb *)arg;

  if (listen_conn == NULL || listen_conn->accept == NULL ||
      listen_conn->state == NULL) {
    return ERR_ARG;
  }

  // Create a new altcp_pcb to pass to the next 'accept' callback
  struct altcp_pcb *new_conn = altcp_alloc();
  if (new_conn == NULL) {
    return ERR_MEM;
  }

  struct altcp_wolfssl_state *state = listen_conn->state;
  err_t setupErr = altcp_wolfssl_setup(state->config, new_conn, accepted_conn);
  if (setupErr != ERR_OK) {
    altcp_free(new_conn);
    return setupErr;
  }
  return listen_conn->accept(listen_conn->arg, new_conn, err);
}

// --------------------------------------------------------------------------
//  wolfSSL Callbacks
// --------------------------------------------------------------------------

static int altcp_wolfssl_passwd_cb(char *passwd, int sz, int rw,
                                   void *userdata) {
  if (sz < 0 || userdata == NULL) {  // Protect against a huge size_t
    return -1;
  }

  struct altcp_tls_cert *cert = (struct altcp_tls_cert *)userdata;
  if (cert->privkey_pass_len >= sz) {  // sz includes the NUL
    return -1;
  }

  memcpy(passwd, cert->privkey_pass, cert->privkey_pass_len);
  return cert->privkey_pass_len;
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

  return sent;
}

// --------------------------------------------------------------------------
//  Setup Functions
// --------------------------------------------------------------------------

static const struct altcp_functions altcp_wolfssl_functions;
static int ctxCount = 0;

static void altcp_wolfssl_remove_callbacks(struct tcp_pcb *inner_conn) {
  altcp_arg(inner_conn, NULL);
  altcp_recv(inner_conn, NULL);
  altcp_sent(inner_conn, NULL);
  altcp_err(inner_conn, NULL);
  altcp_poll(inner_conn, NULL, inner_conn->pollinterval);
}

static void altcp_tcp_setup_callbacks(struct altcp_pcb *conn,
                                      struct altcp_pcb *inner_conn) {
  altcp_arg(inner_conn, conn);
  altcp_recv(inner_conn, &altcp_wolfssl_inner_recv);
  altcp_sent(inner_conn, &altcp_wolfssl_inner_sent);
  altcp_err(inner_conn, &altcp_wolfssl_inner_err);
  // altcp_poll is set when interval is set by application
  // listen is set differently
}

static err_t altcp_wolfssl_setup(struct altcp_tls_config *config,
                                 struct altcp_pcb *conn,
                                 struct altcp_pcb *inner_conn) {
  WOLFSSL_CTX *ctx                  = NULL;
  WOLFSSL *ssl                      = NULL;
  struct altcp_wolfssl_state *state = NULL;
  struct altcp_pcb *pcb             = NULL;

  err_t ret = ERR_OK;

  // Start wolfSSL and check the certificates

  if (ctxCount == 0) {
    wolfSSL_Init();
  }

  WOLFSSL_METHOD *method = config->method;
  config->method = NULL;  // The context now manages this
  ctx = wolfSSL_CTX_new(method);
  if (ctx == NULL) {
    if (ctxCount == 0) {
      wolfSSL_Cleanup();
    }
    ret = ERR_MEM;
    goto altcp_wolfssl_setup_cleanup;
  }
  ctxCount++;

  if (config->ca != NULL) {
    int err = wolfSSL_CTX_load_verify_buffer(ctx, config->ca, config->ca_len,
                                             WOLFSSL_FILETYPE_PEM);
    LWIP_ERROR("Bad CA\r\n", err == WOLFSSL_SUCCESS,
               ret = ERR_ARG; goto altcp_wolfssl_setup_cleanup);
  }
  if (config->cert_list_size > 0) {
    struct altcp_tls_cert *cert = &config->cert_list[0];
    if (cert->cert != NULL) {
      int err = wolfSSL_CTX_use_certificate_buffer(
          ctx,
          cert->cert, cert->cert_len,
          WOLFSSL_FILETYPE_PEM);
      LWIP_ERROR("Bad certificate\r\n", err == WOLFSSL_SUCCESS,
                 ret = ERR_ARG; goto altcp_wolfssl_setup_cleanup);
    }
    if (config->cert_list[0].privkey != NULL) {
      int err = wolfSSL_CTX_use_PrivateKey_buffer(
          ctx,
          cert->privkey, cert->privkey_len,
          WOLFSSL_FILETYPE_PEM);
      LWIP_ERROR(("Bad private key\r\n"), err == WOLFSSL_SUCCESS,
                 ret = ERR_ARG; goto altcp_wolfssl_setup_cleanup);

      if (cert->privkey_pass != NULL && cert->privkey_pass_len > 0) {
        wolfSSL_CTX_set_default_passwd_cb(ctx, &altcp_wolfssl_passwd_cb);
        wolfSSL_CTX_set_default_passwd_cb_userdata(ctx, cert);
      }
    }
  }

  ssl = wolfSSL_new(ctx);
  if (ssl == NULL) {
    ret = ERR_MEM;
    goto altcp_wolfssl_setup_cleanup;
  }

  state = mem_calloc(1, sizeof(struct altcp_wolfssl_state));
  if (state == NULL) {
    ret = ERR_MEM;
    goto altcp_wolfssl_setup_cleanup;
  }
  state->config = config;

  // wolfSSL_SetIO_LwIP(ssl, inner_pcb, NULL, NULL, NULL);
  wolfSSL_CTX_SetIORecv(ctx, &altcp_wolfssl_recv_cb);
  wolfSSL_SetIOReadCtx(ssl, inner_conn);
  wolfSSL_CTX_SetIOSend(ctx, &altcp_wolfssl_send_cb);
  wolfSSL_SetIOWriteCtx(ssl, inner_conn);

  state->ctx = ctx;
  state->ssl = ssl;

  altcp_tcp_setup_callbacks(conn, inner_conn);
  conn->fns        = &altcp_wolfssl_functions;
  conn->inner_conn = inner_conn;
  conn->state      = state;

  return ERR_OK;

altcp_wolfssl_setup_cleanup:
  if (state != NULL) {
    mem_free(state);
  }
  if (ssl != NULL) {
    wolfSSL_free(ssl);
  }
  if (ctx != NULL) {
    wolfSSL_CTX_free(ctx);
    ctxCount--;
    if (ctxCount == 0) {
      wolfSSL_Cleanup();
    }
  }
  return ret;
}

// --------------------------------------------------------------------------
//  More altcp Function Implementations
// --------------------------------------------------------------------------

struct altcp_pcb *altcp_tls_wrap(struct altcp_tls_config *config,
                                 struct altcp_pcb *inner_pcb) {
  if (config == NULL || inner_pcb == NULL) {
    return NULL;
  }

  struct altcp_pcb *pcb = altcp_alloc();
  if (pcb != NULL) {
    if (altcp_wolfssl_setup(config, pcb, inner_pcb) != ERR_OK) {
      altcp_free(pcb);
      pcb = NULL;
    }
  }
  return pcb;
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

static err_t altcp_wolfssl_inner_poll(void *arg, struct altcp_pcb *inner_conn) {
  struct altcp_pcb *conn = (struct altcp_pcb *)arg;
  if (conn != NULL) {
    LWIP_ASSERT("pcb mismatch", conn->inner_conn == inner_conn);
    if (conn->state != NULL) {
      altcp_wolfssl_
    }
    if (conn->poll) {
      return conn->poll(conn->arg, conn);
    }
  }
  return ERR_OK;
}

static void altcp_wolfssl_set_poll(struct altcp_pcb *conn, u8_t interval) {
  if (conn != NULL) {
    altcp_poll(conn, altcp_wolfssl_inner_poll, interval);
  }
}

static err_t altcp_wolfssl_connect(struct altcp_pcb *conn,
                                   const ip_addr_t *ipaddr, u16_t port,
                                   altcp_connected_fn connected) {
  if (conn == NULL) {
    return ERR_VAL;
  }

  conn->connected = connected;
  return altcp_connect(conn->inner_conn, ipaddr, port,
                       &altcp_wolfssl_inner_connected);
}

static struct altcp_pcb *altcp_wolfssl_listen(struct altcp_pcb *conn,
                                              u8_t backlog, err_t *err) {
  if (conn == NULL) {
    return NULL;
  }

  struct altcp_pcb *lpcb =
      altcp_listen_with_backlog_and_err(conn->inner_conn, backlog, err);
  if (lpcb != NULL) {
    conn->inner_conn = lpcb;
    altcp_accept(lpcb, &altcp_wolfssl_inner_accept);
    return conn;
  }
  return NULL;
}

static void altcp_wolfssl_abort(struct altcp_pcb *conn) {
  // TODO
  if (conn != NULL && conn->inner_conn != NULL) {
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

static err_t altcp_wolfssl_close(struct altcp_pcb *conn) {
  if (conn == NULL) {
    return ERR_VAL;
  }

  struct altcp_pcb *inner_conn = conn->inner_conn;
  if (inner_conn != NULL) {
    altcp_poll_fn oldpoll = inner_conn->poll;
    altcp_wolfssl_remove_callbacks(conn->inner_conn);
    err_t err = altcp_close(conn->inner_conn);
    if (err != ERR_OK) {
      // Not closed, set up all callbacks again
      altcp_wolfssl_setup_callbacks(conn, inner_conn);

      // Poll callback is not included in the above
      altcp_poll(inner_conn, oldpoll, inner_conn->pollinterval);

      return err;
    }
    conn->inner_conn = NULL;
  }
  altcp_free(conn);
  return ERR_OK;
}

static void altcp_wolfssl_dealloc(struct altcp_pcb *conn) {
  if (conn == NULL) {
    return;
  }

  struct altcp_wolfssl_state *state = conn->state;
  if (state != NULL) {
    if (state->pbuf != NULL) {
      pbuf_free(state->pbuf);
    }
    if (state->ssl != NULL) {
      wolfSSL_free(state->ssl);
    }
    if (state->ctx != NULL) {
      wolfSSL_CTX_free(state->ctx);
      ctxCount--;
      if (ctxCount == 0) {
        wolfSSL_Cleanup();
      }
    }
    mem_free(state);
    conn->state = NULL;
  }
}

static const struct altcp_functions altcp_wolfssl_functions = {
    &altcp_wolfssl_set_poll,
    &altcp_default_recved,  //
    &altcp_default_bind,
    &altcp_wolfssl_connect,
    &altcp_wolfssl_listen,
    &altcp_wolfssl_abort,  // No altcp_default_abort
    &altcp_wolfssl_close,  // close
    &altcp_default_shutdown,
    &altcp_wolfssl_write,
    &altcp_default_output,
    &altcp_default_mss,  //
    &altcp_default_sndbuf,  //
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

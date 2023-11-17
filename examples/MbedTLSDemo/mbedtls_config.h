// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mbedtls_config.h defines the project's Mbed TLS configuration. It is an
// example only and not used here.
//
// This file is part of the QNEthernet library.

#ifndef PROJECT_MBEDTLS_CONFIG_H_
#define PROJECT_MBEDTLS_CONFIG_H_

/**
 * System support
 *
 * This section sets system specific settings.
 */

#define MBEDTLS_HAVE_ASM
//#define MBEDTLS_NO_UDBL_DIVISION
//#define MBEDTLS_NO_64BIT_MULTIPLICATION
//#define MBEDTLS_HAVE_SSE2
#define MBEDTLS_HAVE_TIME
#define MBEDTLS_HAVE_TIME_DATE
//#define MBEDTLS_PLATFORM_MEMORY
//#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
//#define MBEDTLS_PLATFORM_EXIT_ALT
//#define MBEDTLS_PLATFORM_TIME_ALT
//#define MBEDTLS_PLATFORM_FPRINTF_ALT
//#define MBEDTLS_PLATFORM_PRINTF_ALT
//#define MBEDTLS_PLATFORM_SNPRINTF_ALT
//#define MBEDTLS_PLATFORM_VSNPRINTF_ALT
//#define MBEDTLS_PLATFORM_NV_SEED_ALT
//#define MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT
//#define MBEDTLS_PLATFORM_GMTIME_R_ALT
//#define MBEDTLS_PLATFORM_ZEROIZE_ALT
//#define MBEDTLS_DEPRECATED_WARNING
//#define MBEDTLS_DEPRECATED_REMOVED
//#define MBEDTLS_CHECK_PARAMS
//#define MBEDTLS_CHECK_PARAMS_ASSERT

/**
 * Mbed TLS feature support
 *
 * This section sets support for features that are or are not needed
 * within the modules that are enabled.
 */

//#define MBEDTLS_TIMING_ALT
//#define MBEDTLS_AES_ALT
//#define MBEDTLS_ARC4_ALT
//#define MBEDTLS_ARIA_ALT
//#define MBEDTLS_BLOWFISH_ALT
//#define MBEDTLS_CAMELLIA_ALT
//#define MBEDTLS_CCM_ALT
//#define MBEDTLS_CHACHA20_ALT
//#define MBEDTLS_CHACHAPOLY_ALT
//#define MBEDTLS_CMAC_ALT
//#define MBEDTLS_DES_ALT
//#define MBEDTLS_DHM_ALT
//#define MBEDTLS_ECJPAKE_ALT
//#define MBEDTLS_GCM_ALT
//#define MBEDTLS_NIST_KW_ALT
//#define MBEDTLS_MD2_ALT
//#define MBEDTLS_MD4_ALT
//#define MBEDTLS_MD5_ALT
//#define MBEDTLS_POLY1305_ALT
//#define MBEDTLS_RIPEMD160_ALT
//#define MBEDTLS_RSA_ALT
//#define MBEDTLS_SHA1_ALT
//#define MBEDTLS_SHA256_ALT
//#define MBEDTLS_SHA512_ALT
//#define MBEDTLS_XTEA_ALT
//#define MBEDTLS_ECP_ALT
//#define MBEDTLS_MD2_PROCESS_ALT
//#define MBEDTLS_MD4_PROCESS_ALT
//#define MBEDTLS_MD5_PROCESS_ALT
//#define MBEDTLS_RIPEMD160_PROCESS_ALT
//#define MBEDTLS_SHA1_PROCESS_ALT
//#define MBEDTLS_SHA256_PROCESS_ALT
//#define MBEDTLS_SHA512_PROCESS_ALT
//#define MBEDTLS_DES_SETKEY_ALT
//#define MBEDTLS_DES_CRYPT_ECB_ALT
//#define MBEDTLS_DES3_CRYPT_ECB_ALT
//#define MBEDTLS_AES_SETKEY_ENC_ALT
//#define MBEDTLS_AES_SETKEY_DEC_ALT
//#define MBEDTLS_AES_ENCRYPT_ALT
//#define MBEDTLS_AES_DECRYPT_ALT
//#define MBEDTLS_ECDH_GEN_PUBLIC_ALT
//#define MBEDTLS_ECDH_COMPUTE_SHARED_ALT
//#define MBEDTLS_ECDSA_VERIFY_ALT
//#define MBEDTLS_ECDSA_SIGN_ALT
//#define MBEDTLS_ECDSA_GENKEY_ALT
//#define MBEDTLS_ECP_INTERNAL_ALT
//#define MBEDTLS_ECP_NO_FALLBACK
//#define MBEDTLS_ECP_RANDOMIZE_JAC_ALT
//#define MBEDTLS_ECP_ADD_MIXED_ALT
//#define MBEDTLS_ECP_DOUBLE_JAC_ALT
//#define MBEDTLS_ECP_NORMALIZE_JAC_MANY_ALT
//#define MBEDTLS_ECP_NORMALIZE_JAC_ALT
//#define MBEDTLS_ECP_DOUBLE_ADD_MXZ_ALT
//#define MBEDTLS_ECP_RANDOMIZE_MXZ_ALT
//#define MBEDTLS_ECP_NORMALIZE_MXZ_ALT
//#define MBEDTLS_TEST_NULL_ENTROPY
#define MBEDTLS_ENTROPY_HARDWARE_ALT
//#define MBEDTLS_AES_ROM_TABLES
//#define MBEDTLS_AES_FEWER_TABLES
//#define MBEDTLS_CAMELLIA_SMALL_MEMORY
//#define MBEDTLS_CHECK_RETURN_WARNING
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_CFB
#define MBEDTLS_CIPHER_MODE_CTR
#define MBEDTLS_CIPHER_MODE_OFB
#define MBEDTLS_CIPHER_MODE_XTS
//#define MBEDTLS_CIPHER_NULL_CIPHER
#define MBEDTLS_CIPHER_PADDING_PKCS7
#define MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS
#define MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN
#define MBEDTLS_CIPHER_PADDING_ZEROS
//#define MBEDTLS_CTR_DRBG_USE_128_BIT_KEY
//#define MBEDTLS_ENABLE_WEAK_CIPHERSUITES
#define MBEDTLS_REMOVE_ARC4_CIPHERSUITES
#define MBEDTLS_REMOVE_3DES_CIPHERSUITES
//#define MBEDTLS_ECDH_VARIANT_EVEREST_ENABLED
#define MBEDTLS_ECP_DP_SECP192R1_ENABLED
#define MBEDTLS_ECP_DP_SECP224R1_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
#define MBEDTLS_ECP_DP_SECP521R1_ENABLED
#define MBEDTLS_ECP_DP_SECP192K1_ENABLED
#define MBEDTLS_ECP_DP_SECP224K1_ENABLED
#define MBEDTLS_ECP_DP_SECP256K1_ENABLED
#define MBEDTLS_ECP_DP_BP256R1_ENABLED
#define MBEDTLS_ECP_DP_BP384R1_ENABLED
#define MBEDTLS_ECP_DP_BP512R1_ENABLED
#define MBEDTLS_ECP_DP_CURVE25519_ENABLED
#define MBEDTLS_ECP_DP_CURVE448_ENABLED
#define MBEDTLS_ECP_NIST_OPTIM
//#define MBEDTLS_ECP_NO_INTERNAL_RNG
//#define MBEDTLS_ECP_RESTARTABLE
#define MBEDTLS_ECDH_LEGACY_CONTEXT
#define MBEDTLS_ECDSA_DETERMINISTIC
#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#define MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED
#define MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED
#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_PK_PARSE_EC_EXTENDED
#define MBEDTLS_ERROR_STRERROR_DUMMY
#define MBEDTLS_GENPRIME
// #define MBEDTLS_FS_IO
//#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_NO_PLATFORM_ENTROPY
//#define MBEDTLS_ENTROPY_FORCE_SHA256
//#define MBEDTLS_ENTROPY_NV_SEED
//#define MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER
//#define MBEDTLS_MEMORY_DEBUG
//#define MBEDTLS_MEMORY_BACKTRACE
#define MBEDTLS_PK_RSA_ALT_SUPPORT
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_PKCS1_V21
//#define MBEDTLS_PSA_CRYPTO_BUILTIN_KEYS
//#define MBEDTLS_PSA_CRYPTO_CLIENT
//#define MBEDTLS_PSA_CRYPTO_DRIVERS
//#define MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG
//#define MBEDTLS_PSA_CRYPTO_SPM
//#define MBEDTLS_PSA_INJECT_ENTROPY
//#define MBEDTLS_RSA_NO_CRT
#define MBEDTLS_SELF_TEST
//#define MBEDTLS_SHA256_SMALLER
//#define MBEDTLS_SHA512_SMALLER
//#define MBEDTLS_SHA512_NO_SHA384
#define MBEDTLS_SSL_ALL_ALERT_MESSAGES
#define MBEDTLS_SSL_RECORD_CHECKING
//#define MBEDTLS_SSL_DTLS_CONNECTION_ID
//#define MBEDTLS_SSL_ASYNC_PRIVATE
#define MBEDTLS_SSL_CONTEXT_SERIALIZATION
//#define MBEDTLS_SSL_DEBUG_ALL
#define MBEDTLS_SSL_ENCRYPT_THEN_MAC
#define MBEDTLS_SSL_EXTENDED_MASTER_SECRET
#define MBEDTLS_SSL_FALLBACK_SCSV
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
//#define MBEDTLS_SSL_HW_RECORD_ACCEL
// #define MBEDTLS_SSL_CBC_RECORD_SPLITTING
#define MBEDTLS_SSL_RENEGOTIATION
//#define MBEDTLS_SSL_SRV_SUPPORT_SSLV2_CLIENT_HELLO
//#define MBEDTLS_SSL_SRV_RESPECT_CLIENT_PREFERENCE
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
//#define MBEDTLS_SSL_PROTO_SSL3
// #define MBEDTLS_SSL_PROTO_TLS1
// #define MBEDTLS_SSL_PROTO_TLS1_1
#define MBEDTLS_SSL_PROTO_TLS1_2
//#define MBEDTLS_SSL_PROTO_TLS1_3_EXPERIMENTAL
// #define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_SSL_ALPN
// #define MBEDTLS_SSL_DTLS_ANTI_REPLAY
// #define MBEDTLS_SSL_DTLS_HELLO_VERIFY
//#define MBEDTLS_SSL_DTLS_SRTP
// #define MBEDTLS_SSL_DTLS_CLIENT_PORT_REUSE
// #define MBEDTLS_SSL_DTLS_BADMAC_LIMIT
#define MBEDTLS_SSL_SESSION_TICKETS
#define MBEDTLS_SSL_EXPORT_KEYS
#define MBEDTLS_SSL_SERVER_NAME_INDICATION
#define MBEDTLS_SSL_TRUNCATED_HMAC
//#define MBEDTLS_SSL_TRUNCATED_HMAC_COMPAT
//#define MBEDTLS_SSL_VARIABLE_BUFFER_LENGTH
//#define MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_KEY_EXCHANGE
//#define MBEDTLS_TEST_CONSTANT_FLOW_MEMSAN
//#define MBEDTLS_TEST_CONSTANT_FLOW_VALGRIND
//#define MBEDTLS_TEST_HOOKS
//#define MBEDTLS_THREADING_ALT
//#define MBEDTLS_THREADING_PTHREAD
//#define MBEDTLS_USE_PSA_CRYPTO
//#define MBEDTLS_PSA_CRYPTO_CONFIG
#define MBEDTLS_VERSION_FEATURES
//#define MBEDTLS_X509_ALLOW_EXTENSIONS_NON_V3
//#define MBEDTLS_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION
//#define MBEDTLS_X509_TRUSTED_CERTIFICATE_CALLBACK
#define MBEDTLS_X509_CHECK_KEY_USAGE
#define MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE
#define MBEDTLS_X509_RSASSA_PSS_SUPPORT
//#define MBEDTLS_ZLIB_SUPPORT

/**
 * Mbed TLS modules
 *
 * This section enables or disables entire modules in Mbed TLS
 */

#define MBEDTLS_AESNI_C
#define MBEDTLS_AES_C
// #define MBEDTLS_ARC4_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_BLOWFISH_C
#define MBEDTLS_CAMELLIA_C
// #define MBEDTLS_ARIA_C
#define MBEDTLS_CCM_C
#define MBEDTLS_CERTS_C
#define MBEDTLS_CHACHA20_C
#define MBEDTLS_CHACHAPOLY_C
#define MBEDTLS_CIPHER_C
// #define MBEDTLS_CMAC_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_DEBUG_C
// #define MBEDTLS_DES_C
#define MBEDTLS_DHM_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
// #define MBEDTLS_ECJPAKE_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_ERROR_C
#define MBEDTLS_GCM_C
// #define MBEDTLS_HAVEGE_C
#define MBEDTLS_HKDF_C
#define MBEDTLS_HMAC_DRBG_C
// #define MBEDTLS_NIST_KW_C
#define MBEDTLS_MD_C
//#define MBEDTLS_MD2_C
//#define MBEDTLS_MD4_C
#define MBEDTLS_MD5_C
//#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
// #define MBEDTLS_NET_C
#define MBEDTLS_OID_C
#define MBEDTLS_PADLOCK_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PEM_WRITE_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C
#define MBEDTLS_PKCS5_C
//#define MBEDTLS_PKCS11_C
#define MBEDTLS_PKCS12_C
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_POLY1305_C
#define MBEDTLS_PSA_CRYPTO_C
//#define MBEDTLS_PSA_CRYPTO_SE_C
// #define MBEDTLS_PSA_CRYPTO_STORAGE_C
// #define MBEDTLS_PSA_ITS_FILE_C
#define MBEDTLS_RIPEMD160_C
#define MBEDTLS_RSA_C
#define MBEDTLS_SHA1_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA512_C
#define MBEDTLS_SSL_CACHE_C
#define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_SSL_TICKET_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_TLS_C
//#define MBEDTLS_THREADING_C
// #define MBEDTLS_TIMING_C
#define MBEDTLS_VERSION_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_CRL_PARSE_C
#define MBEDTLS_X509_CSR_PARSE_C
#define MBEDTLS_X509_CREATE_C
#define MBEDTLS_X509_CRT_WRITE_C
#define MBEDTLS_X509_CSR_WRITE_C
#define MBEDTLS_XTEA_C

/**
 * General configuration options
 *
 * This section contains Mbed TLS build settings that are not associated
 * with a particular module.
 */

//#define MBEDTLS_CONFIG_FILE "mbedtls/config.h"
//#define MBEDTLS_USER_CONFIG_FILE "/dev/null"
//#define MBEDTLS_PSA_CRYPTO_CONFIG_FILE "psa/crypto_config.h"
//#define MBEDTLS_PSA_CRYPTO_USER_CONFIG_FILE "/dev/null"

/**
 * Module configuration options
 *
 * This section allows for the setting of module specific sizes and
 * configuration options. The default values are already present in the
 * relevant header files and should suffice for the regular use cases.
 *
 * Our advice is to enable options and change their values here
 * only if you have a good reason and know the consequences.
 */

//#define MBEDTLS_MPI_WINDOW_SIZE            2 /**< Maximum window size used. */
//#define MBEDTLS_MPI_MAX_SIZE            1024 /**< Maximum number of bytes for usable MPIs. */
//#define MBEDTLS_CTR_DRBG_ENTROPY_LEN               48 /**< Amount of entropy used per seed by default (48 with SHA-512, 32 with SHA-256) */
//#define MBEDTLS_CTR_DRBG_RESEED_INTERVAL        10000 /**< Interval before reseed is performed by default */
//#define MBEDTLS_CTR_DRBG_MAX_INPUT                256 /**< Maximum number of additional input bytes */
//#define MBEDTLS_CTR_DRBG_MAX_REQUEST             1024 /**< Maximum number of requested bytes per call */
//#define MBEDTLS_CTR_DRBG_MAX_SEED_INPUT           384 /**< Maximum size of (re)seed buffer */
//#define MBEDTLS_HMAC_DRBG_RESEED_INTERVAL   10000 /**< Interval before reseed is performed by default */
//#define MBEDTLS_HMAC_DRBG_MAX_INPUT           256 /**< Maximum number of additional input bytes */
//#define MBEDTLS_HMAC_DRBG_MAX_REQUEST        1024 /**< Maximum number of requested bytes per call */
//#define MBEDTLS_HMAC_DRBG_MAX_SEED_INPUT      384 /**< Maximum size of (re)seed buffer */
//#define MBEDTLS_ECP_MAX_BITS             521 /**< Maximum bit size of groups. Normally determined automatically from the configured curves. */
//#define MBEDTLS_ECP_WINDOW_SIZE            4 /**< Maximum window size used */
//#define MBEDTLS_ECP_FIXED_POINT_OPTIM      1 /**< Enable fixed-point speed-up */
//#define MBEDTLS_ENTROPY_MAX_SOURCES                20 /**< Maximum number of sources supported */
//#define MBEDTLS_ENTROPY_MAX_GATHER                128 /**< Maximum amount requested from entropy sources */
//#define MBEDTLS_ENTROPY_MIN_HARDWARE               32 /**< Default minimum number of bytes required for the hardware entropy source mbedtls_hardware_poll() before entropy is released */
//#define MBEDTLS_MEMORY_ALIGN_MULTIPLE      4 /**< Align on multiples of this value */
//#define MBEDTLS_PLATFORM_STD_MEM_HDR   <stdlib.h> /**< Header to include if MBEDTLS_PLATFORM_NO_STD_FUNCTIONS is defined. Don't define if no header is needed. */
//#define MBEDTLS_PLATFORM_STD_CALLOC        calloc
//#define MBEDTLS_PLATFORM_STD_FREE            free
//#define MBEDTLS_PLATFORM_STD_EXIT            exit /**< Default exit to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_TIME            time /**< Default time to use, can be undefined. MBEDTLS_HAVE_TIME must be enabled */
//#define MBEDTLS_PLATFORM_STD_FPRINTF      fprintf /**< Default fprintf to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_PRINTF        printf /**< Default printf to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_SNPRINTF    snprintf /**< Default snprintf to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_EXIT_SUCCESS       0 /**< Default exit value to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_EXIT_FAILURE       1 /**< Default exit value to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_NV_SEED_READ   mbedtls_platform_std_nv_seed_read /**< Default nv_seed_read function to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_NV_SEED_WRITE  mbedtls_platform_std_nv_seed_write /**< Default nv_seed_write function to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_NV_SEED_FILE  "seedfile" /**< Seed file to read/write with default implementation */
//#define MBEDTLS_PLATFORM_CALLOC_MACRO        calloc /**< Default allocator macro to use, can be undefined. See MBEDTLS_PLATFORM_STD_CALLOC for requirements. */
//#define MBEDTLS_PLATFORM_FREE_MACRO            free /**< Default free macro to use, can be undefined. See MBEDTLS_PLATFORM_STD_FREE for requirements. */
//#define MBEDTLS_PLATFORM_EXIT_MACRO            exit /**< Default exit macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_TIME_MACRO            time /**< Default time macro to use, can be undefined. MBEDTLS_HAVE_TIME must be enabled */
//#define MBEDTLS_PLATFORM_TIME_TYPE_MACRO       time_t /**< Default time macro to use, can be undefined. MBEDTLS_HAVE_TIME must be enabled */
//#define MBEDTLS_PLATFORM_FPRINTF_MACRO      fprintf /**< Default fprintf macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_PRINTF_MACRO        printf /**< Default printf macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_SNPRINTF_MACRO    snprintf /**< Default snprintf macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_VSNPRINTF_MACRO    vsnprintf /**< Default vsnprintf macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_NV_SEED_READ_MACRO   mbedtls_platform_std_nv_seed_read /**< Default nv_seed_read function to use, can be undefined */
//#define MBEDTLS_PLATFORM_NV_SEED_WRITE_MACRO  mbedtls_platform_std_nv_seed_write /**< Default nv_seed_write function to use, can be undefined */
//#define MBEDTLS_PARAM_FAILED( cond )               assert( cond )
//#define MBEDTLS_CHECK_RETURN __attribute__((__warn_unused_result__))
//#define MBEDTLS_IGNORE_RETURN( result ) ((void) !(result))
//#define MBEDTLS_PSA_HMAC_DRBG_MD_TYPE MBEDTLS_MD_SHA256
//#define MBEDTLS_PSA_KEY_SLOT_COUNT 32
//#define MBEDTLS_SSL_CACHE_DEFAULT_TIMEOUT       86400 /**< 1 day  */
//#define MBEDTLS_SSL_CACHE_DEFAULT_MAX_ENTRIES      50 /**< Maximum entries in cache */
//#define MBEDTLS_SSL_MAX_CONTENT_LEN             16384
//#define MBEDTLS_SSL_IN_CONTENT_LEN              16384
//#define MBEDTLS_SSL_CID_IN_LEN_MAX 32
//#define MBEDTLS_SSL_CID_OUT_LEN_MAX 32
//#define MBEDTLS_SSL_CID_PADDING_GRANULARITY 16
//#define MBEDTLS_SSL_TLS1_3_PADDING_GRANULARITY 1
//#define MBEDTLS_SSL_OUT_CONTENT_LEN             16384
//#define MBEDTLS_SSL_DTLS_MAX_BUFFERING             32768
//#define MBEDTLS_SSL_DEFAULT_TICKET_LIFETIME     86400 /**< Lifetime of session tickets (if enabled) */
//#define MBEDTLS_PSK_MAX_LEN               32 /**< Max size of TLS pre-shared keys, in bytes (default 256 bits) */
//#define MBEDTLS_SSL_COOKIE_TIMEOUT        60 /**< Default expiration delay of DTLS cookies, in seconds if HAVE_TIME, or in number of cookies issued */
//#define MBEDTLS_TLS_EXT_CID                        254
//#define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
//#define MBEDTLS_X509_MAX_INTERMEDIATE_CA   8   /**< Maximum number of intermediate CAs in a verification chain. */
//#define MBEDTLS_X509_MAX_FILE_PATH_LEN     512 /**< Maximum length of a path/filename string in bytes including the null terminator character ('\0'). */

/**
 * Target any application specific configurations
 *
 * Allow user to override any previous default.
 */

#if defined(MBEDTLS_USER_CONFIG_FILE)
#include MBEDTLS_USER_CONFIG_FILE
#endif

#if defined(MBEDTLS_PSA_CRYPTO_CONFIG)
#include "mbedtls/config_psa.h"
#endif

#include "mbedtls/check_config.h"

#endif  // PROJECT_MBEDTLS_CONFIG_H_

// SPDX-FileCopyrightText: (c) 2023-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// sample_mbedtls_config.h is a sample Mbed TLS configuration. It contains a
// suggested minimal set of options. It is an example only and not used here.
//
// This file is part of the QNEthernet library.

/*
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
//#define MBEDTLS_PLATFORM_SETBUF_ALT
//#define MBEDTLS_PLATFORM_EXIT_ALT
//#define MBEDTLS_PLATFORM_TIME_ALT
//#define MBEDTLS_PLATFORM_FPRINTF_ALT
//#define MBEDTLS_PLATFORM_PRINTF_ALT
//#define MBEDTLS_PLATFORM_SNPRINTF_ALT
//#define MBEDTLS_PLATFORM_VSNPRINTF_ALT
//#define MBEDTLS_PLATFORM_NV_SEED_ALT
//#define MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT
#define MBEDTLS_PLATFORM_MS_TIME_ALT  /* Default: undefined */
//#define MBEDTLS_PLATFORM_GMTIME_R_ALT
//#define MBEDTLS_PLATFORM_ZEROIZE_ALT
//#define MBEDTLS_DEPRECATED_WARNING
#define MBEDTLS_DEPRECATED_REMOVED  /* Default: undefined */

/*
 * Mbed TLS feature support
 *
 * This section sets support for features that are or are not needed
 * within the modules that are enabled.
 */

//#define MBEDTLS_TIMING_ALT
//#define MBEDTLS_AES_ALT
//#define MBEDTLS_ARIA_ALT
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
//#define MBEDTLS_MD5_ALT
//#define MBEDTLS_POLY1305_ALT
//#define MBEDTLS_RIPEMD160_ALT
//#define MBEDTLS_RSA_ALT
//#define MBEDTLS_SHA1_ALT
//#define MBEDTLS_SHA256_ALT
//#define MBEDTLS_SHA512_ALT
//#define MBEDTLS_ECP_ALT
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
#define MBEDTLS_ENTROPY_HARDWARE_ALT  /* Default: undefined */
//#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_AES_FEWER_TABLES  /* Default: undefined */
//#define MBEDTLS_AES_ONLY_128_BIT_KEY_LENGTH
//#define MBEDTLS_AES_USE_HARDWARE_ONLY
//#define MBEDTLS_CAMELLIA_SMALL_MEMORY
//#define MBEDTLS_CHECK_RETURN_WARNING
// #define MBEDTLS_CIPHER_MODE_CBC  /* Default: defined */
// #define MBEDTLS_CIPHER_MODE_CFB  /* Default: defined */
// #define MBEDTLS_CIPHER_MODE_CTR  /* Default: defined */
// #define MBEDTLS_CIPHER_MODE_OFB  /* Default: defined */
// #define MBEDTLS_CIPHER_MODE_XTS  /* Default: defined */
//#define MBEDTLS_CIPHER_NULL_CIPHER
// #define MBEDTLS_CIPHER_PADDING_PKCS7  /* Default: defined */
// #define MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS  /* Default: defined */
// #define MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN  /* Default: defined */
// #define MBEDTLS_CIPHER_PADDING_ZEROS  /* Default: defined */
//#define MBEDTLS_CTR_DRBG_USE_128_BIT_KEY
//#define MBEDTLS_ECDH_VARIANT_EVEREST_ENABLED
// #define MBEDTLS_ECP_DP_SECP192R1_ENABLED  /* Default: defined */
// #define MBEDTLS_ECP_DP_SECP224R1_ENABLED  /* Default: defined */
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
#define MBEDTLS_ECP_DP_SECP521R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP192K1_ENABLED  /* Default: defined */
// #define MBEDTLS_ECP_DP_SECP224K1_ENABLED  /* Default: defined */
// #define MBEDTLS_ECP_DP_SECP256K1_ENABLED  /* Default: defined */
// #define MBEDTLS_ECP_DP_BP256R1_ENABLED  /* Default: defined */
// #define MBEDTLS_ECP_DP_BP384R1_ENABLED  /* Default: defined */
// #define MBEDTLS_ECP_DP_BP512R1_ENABLED  /* Default: defined */
#define MBEDTLS_ECP_DP_CURVE25519_ENABLED
#define MBEDTLS_ECP_DP_CURVE448_ENABLED
// #define MBEDTLS_ECP_NIST_OPTIM  /* Default: defined */
//#define MBEDTLS_ECP_RESTARTABLE
//#define MBEDTLS_ECP_WITH_MPI_UINT
// #define MBEDTLS_ECDSA_DETERMINISTIC  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED  /* Default: defined */
// #define MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED  /* Default: defined */
//#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
// #define MBEDTLS_PK_PARSE_EC_EXTENDED  /* Default: defined */
// #define MBEDTLS_PK_PARSE_EC_COMPRESSED  /* Default: defined */
#define MBEDTLS_ERROR_STRERROR_DUMMY
// #define MBEDTLS_GENPRIME  /* Default: defined */
// #define MBEDTLS_FS_IO  /* Default: defined */
//#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_NO_PLATFORM_ENTROPY  /* Default: undefined */
//#define MBEDTLS_ENTROPY_FORCE_SHA256
//#define MBEDTLS_ENTROPY_NV_SEED
//#define MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER
//#define MBEDTLS_MEMORY_DEBUG
//#define MBEDTLS_MEMORY_BACKTRACE
// #define MBEDTLS_PK_RSA_ALT_SUPPORT  /* Default: defined */
// #define MBEDTLS_PKCS1_V15  /* Default: defined */
#define MBEDTLS_PKCS1_V21
//#define MBEDTLS_PSA_CRYPTO_BUILTIN_KEYS
//#define MBEDTLS_PSA_CRYPTO_CLIENT
//#define MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG
//#define MBEDTLS_PSA_CRYPTO_SPM
#define MBEDTLS_PSA_KEY_STORE_DYNAMIC
//#define MBEDTLS_PSA_P256M_DRIVER_ENABLED
//#define MBEDTLS_PSA_INJECT_ENTROPY
//#define MBEDTLS_PSA_ASSUME_EXCLUSIVE_BUFFERS
//#define MBEDTLS_RSA_NO_CRT
// #define MBEDTLS_SELF_TEST  /* Default: defined */
//#define MBEDTLS_SHA256_SMALLER
//#define MBEDTLS_SHA512_SMALLER
#define MBEDTLS_SSL_ALL_ALERT_MESSAGES
#define MBEDTLS_SSL_DTLS_CONNECTION_ID
#define MBEDTLS_SSL_DTLS_CONNECTION_ID_COMPAT 0
//#define MBEDTLS_SSL_ASYNC_PRIVATE
// #define MBEDTLS_SSL_CONTEXT_SERIALIZATION  /* Default: defined */
//#define MBEDTLS_SSL_DEBUG_ALL
#define MBEDTLS_SSL_ENCRYPT_THEN_MAC
#define MBEDTLS_SSL_EXTENDED_MASTER_SECRET
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
// #define MBEDTLS_SSL_RENEGOTIATION  /* Default: defined */
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
//#define MBEDTLS_SSL_RECORD_SIZE_LIMIT
// #define MBEDTLS_SSL_PROTO_TLS1_2  /* Default: defined */
#define MBEDTLS_SSL_PROTO_TLS1_3
#define MBEDTLS_SSL_TLS1_3_COMPATIBILITY_MODE
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_ENABLED
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_EPHEMERAL_ENABLED
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_EPHEMERAL_ENABLED
//#define MBEDTLS_SSL_EARLY_DATA
// #define MBEDTLS_SSL_PROTO_DTLS  /* Default: defined */
// #define MBEDTLS_SSL_ALPN  /* Default: defined */
#define MBEDTLS_SSL_DTLS_ANTI_REPLAY
#define MBEDTLS_SSL_DTLS_HELLO_VERIFY
//#define MBEDTLS_SSL_DTLS_SRTP
#define MBEDTLS_SSL_DTLS_CLIENT_PORT_REUSE
#define MBEDTLS_SSL_SESSION_TICKETS
#define MBEDTLS_SSL_SERVER_NAME_INDICATION
//#define MBEDTLS_SSL_VARIABLE_BUFFER_LENGTH
//#define MBEDTLS_TEST_CONSTANT_FLOW_MEMSAN
//#define MBEDTLS_TEST_CONSTANT_FLOW_VALGRIND
//#define MBEDTLS_TEST_HOOKS
//#define MBEDTLS_THREADING_ALT
//#define MBEDTLS_THREADING_PTHREAD
//#define MBEDTLS_USE_PSA_CRYPTO
// #define MBEDTLS_PSA_CRYPTO_CONFIG
#define MBEDTLS_VERSION_FEATURES
//#define MBEDTLS_X509_TRUSTED_CERTIFICATE_CALLBACK
//#define MBEDTLS_X509_REMOVE_INFO
#define MBEDTLS_X509_RSASSA_PSS_SUPPORT

/*
 * Mbed TLS modules
 *
 * This section enables or disables entire modules in Mbed TLS
 */

#define MBEDTLS_AESNI_C
#define MBEDTLS_AESCE_C
#define MBEDTLS_AES_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BASE64_C
//#define MBEDTLS_BLOCK_CIPHER_NO_DECRYPT
#define MBEDTLS_BIGNUM_C
// #define MBEDTLS_CAMELLIA_C  /* Default: defined */
// #define MBEDTLS_ARIA_C  /* Default: defined */
#define MBEDTLS_CCM_C
#define MBEDTLS_CHACHA20_C
#define MBEDTLS_CHACHAPOLY_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CMAC_C
#define MBEDTLS_CTR_DRBG_C
// #define MBEDTLS_DEBUG_C  /* Default: defined */
// #define MBEDTLS_DES_C  /* Default: defined */
// #define MBEDTLS_DHM_C  /* Default: defined */
#define MBEDTLS_ECDH_C
// #define MBEDTLS_ECDSA_C  /* Default: defined */
// #define MBEDTLS_ECJPAKE_C  /* Default: defined */
#define MBEDTLS_ECP_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_ERROR_C
#define MBEDTLS_GCM_C
//#define MBEDTLS_GCM_LARGE_TABLE
#define MBEDTLS_HKDF_C
// #define MBEDTLS_HMAC_DRBG_C  /* Default: defined */
// #define MBEDTLS_LMS_C  /* Default: defined */
//#define MBEDTLS_LMS_PRIVATE
#define MBEDTLS_NIST_KW_C
#define MBEDTLS_MD_C
// #define MBEDTLS_MD5_C  /* Default: defined */
//#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
// #define MBEDTLS_NET_C  /* Default: defined */
#define MBEDTLS_OID_C
#define MBEDTLS_PADLOCK_C
#define MBEDTLS_PEM_PARSE_C
// #define MBEDTLS_PEM_WRITE_C  /* Default: defined */
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
// #define MBEDTLS_PK_WRITE_C  /* Default: defined */
// #define MBEDTLS_PKCS5_C  /* Default: defined */
// #define MBEDTLS_PKCS7_C  /* Default: defined */
#define MBEDTLS_PKCS12_C
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_POLY1305_C
#define MBEDTLS_PSA_CRYPTO_C
//#define MBEDTLS_PSA_CRYPTO_SE_C
// #define MBEDTLS_PSA_CRYPTO_STORAGE_C  /* Default: defined */
// #define MBEDTLS_PSA_ITS_FILE_C  /* Default: defined */
//#define MBEDTLS_PSA_STATIC_KEY_SLOTS
// #define MBEDTLS_RIPEMD160_C  /* Default: defined */
#define MBEDTLS_RSA_C
// #define MBEDTLS_SHA1_C  /* Default: defined */
// #define MBEDTLS_SHA224_C  /* Default: defined */
#define MBEDTLS_SHA256_C
//#define MBEDTLS_SHA256_USE_ARMV8_A_CRYPTO_IF_PRESENT
//#define MBEDTLS_SHA256_USE_A64_CRYPTO_IF_PRESENT
//#define MBEDTLS_SHA256_USE_ARMV8_A_CRYPTO_ONLY
//#define MBEDTLS_SHA256_USE_A64_CRYPTO_ONLY
#define MBEDTLS_SHA384_C
#define MBEDTLS_SHA512_C
// #define MBEDTLS_SHA3_C  /* Default: defined */
//#define MBEDTLS_SHA512_USE_A64_CRYPTO_IF_PRESENT
//#define MBEDTLS_SHA512_USE_A64_CRYPTO_ONLY
#define MBEDTLS_SSL_CACHE_C
#define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_SSL_TICKET_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_TLS_C
//#define MBEDTLS_THREADING_C
// #define MBEDTLS_TIMING_C  /* Default: defined */
#define MBEDTLS_VERSION_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
// #define MBEDTLS_X509_CRL_PARSE_C  /* Default: defined */
// #define MBEDTLS_X509_CSR_PARSE_C  /* Default: defined */
// #define MBEDTLS_X509_CREATE_C  /* Default: defined */
// #define MBEDTLS_X509_CRT_WRITE_C  /* Default: defined */
// #define MBEDTLS_X509_CSR_WRITE_C  /* Default: defined */

/*
 * General configuration options
 *
 * This section contains Mbed TLS build settings that are not associated
 * with a particular module.
 */

//#define MBEDTLS_CONFIG_FILE "mbedtls/mbedtls_config.h"
//#define MBEDTLS_USER_CONFIG_FILE "/dev/null"
//#define MBEDTLS_PSA_CRYPTO_CONFIG_FILE "psa/crypto_config.h"
//#define MBEDTLS_PSA_CRYPTO_USER_CONFIG_FILE "/dev/null"
//#define MBEDTLS_PSA_CRYPTO_PLATFORM_FILE "psa/crypto_platform_alt.h"
//#define MBEDTLS_PSA_CRYPTO_STRUCT_FILE "psa/crypto_struct_alt.h"

/*
 * Module configuration options
 *
 * This section allows for the setting of module specific sizes and
 * configuration options. The default values are already present in the
 * relevant header files and should suffice for the regular use cases.
 *
 * Our advice is to enable options and change their values here
 * only if you have a good reason and know the consequences.
 */

/* MPI / BIGNUM options */
//#define MBEDTLS_MPI_WINDOW_SIZE            2 /**< Maximum window size used. */
//#define MBEDTLS_MPI_MAX_SIZE            1024 /**< Maximum number of bytes for usable MPIs. */

/* CTR_DRBG options */
//#define MBEDTLS_CTR_DRBG_ENTROPY_LEN               48 /**< Amount of entropy used per seed by default (48 with SHA-512, 32 with SHA-256) */
//#define MBEDTLS_CTR_DRBG_RESEED_INTERVAL        10000 /**< Interval before reseed is performed by default */
//#define MBEDTLS_CTR_DRBG_MAX_INPUT                256 /**< Maximum number of additional input bytes */
//#define MBEDTLS_CTR_DRBG_MAX_REQUEST             1024 /**< Maximum number of requested bytes per call */
//#define MBEDTLS_CTR_DRBG_MAX_SEED_INPUT           384 /**< Maximum size of (re)seed buffer */

/* HMAC_DRBG options */
//#define MBEDTLS_HMAC_DRBG_RESEED_INTERVAL   10000 /**< Interval before reseed is performed by default */
//#define MBEDTLS_HMAC_DRBG_MAX_INPUT           256 /**< Maximum number of additional input bytes */
//#define MBEDTLS_HMAC_DRBG_MAX_REQUEST        1024 /**< Maximum number of requested bytes per call */
//#define MBEDTLS_HMAC_DRBG_MAX_SEED_INPUT      384 /**< Maximum size of (re)seed buffer */

/* ECP options */
//#define MBEDTLS_ECP_WINDOW_SIZE            4 /**< Maximum window size used */
//#define MBEDTLS_ECP_FIXED_POINT_OPTIM      1 /**< Enable fixed-point speed-up */

/* Entropy options */
//#define MBEDTLS_ENTROPY_MAX_SOURCES                20 /**< Maximum number of sources supported */
//#define MBEDTLS_ENTROPY_MAX_GATHER                128 /**< Maximum amount requested from entropy sources */
//#define MBEDTLS_ENTROPY_MIN_HARDWARE               32 /**< Default minimum number of bytes required for the hardware entropy source mbedtls_hardware_poll() before entropy is released */

/* Memory buffer allocator options */
//#define MBEDTLS_MEMORY_ALIGN_MULTIPLE      4 /**< Align on multiples of this value */

/* Platform options */
//#define MBEDTLS_PLATFORM_STD_MEM_HDR   <stdlib.h> /**< Header to include if MBEDTLS_PLATFORM_NO_STD_FUNCTIONS is defined. Don't define if no header is needed. */
//#define MBEDTLS_PLATFORM_STD_CALLOC        calloc
//#define MBEDTLS_PLATFORM_STD_FREE            free
//#define MBEDTLS_PLATFORM_STD_SETBUF      setbuf /**< Default setbuf to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_EXIT            exit /**< Default exit to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_TIME            time /**< Default time to use, can be undefined. MBEDTLS_HAVE_TIME must be enabled */
//#define MBEDTLS_PLATFORM_STD_FPRINTF      fprintf /**< Default fprintf to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_PRINTF        printf /**< Default printf to use, can be undefined */
/* Note: your snprintf must correctly zero-terminate the buffer! */
//#define MBEDTLS_PLATFORM_STD_SNPRINTF    snprintf /**< Default snprintf to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_EXIT_SUCCESS       0 /**< Default exit value to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_EXIT_FAILURE       1 /**< Default exit value to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_NV_SEED_READ   mbedtls_platform_std_nv_seed_read /**< Default nv_seed_read function to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_NV_SEED_WRITE  mbedtls_platform_std_nv_seed_write /**< Default nv_seed_write function to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_NV_SEED_FILE  "seedfile" /**< Seed file to read/write with default implementation */

/* To use the following function macros, MBEDTLS_PLATFORM_C must be enabled. */
/* MBEDTLS_PLATFORM_XXX_MACRO and MBEDTLS_PLATFORM_XXX_ALT cannot both be defined */
//#define MBEDTLS_PLATFORM_CALLOC_MACRO        calloc /**< Default allocator macro to use, can be undefined. See MBEDTLS_PLATFORM_STD_CALLOC for requirements. */
//#define MBEDTLS_PLATFORM_FREE_MACRO            free /**< Default free macro to use, can be undefined. See MBEDTLS_PLATFORM_STD_FREE for requirements. */
//#define MBEDTLS_PLATFORM_EXIT_MACRO            exit /**< Default exit macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_SETBUF_MACRO      setbuf /**< Default setbuf macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_TIME_MACRO            time /**< Default time macro to use, can be undefined. MBEDTLS_HAVE_TIME must be enabled */
//#define MBEDTLS_PLATFORM_TIME_TYPE_MACRO       time_t /**< Default time macro to use, can be undefined. MBEDTLS_HAVE_TIME must be enabled */
//#define MBEDTLS_PLATFORM_FPRINTF_MACRO      fprintf /**< Default fprintf macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_PRINTF_MACRO        printf /**< Default printf macro to use, can be undefined */
/* Note: your snprintf must correctly zero-terminate the buffer! */
//#define MBEDTLS_PLATFORM_SNPRINTF_MACRO    snprintf /**< Default snprintf macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_VSNPRINTF_MACRO    vsnprintf /**< Default vsnprintf macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_NV_SEED_READ_MACRO   mbedtls_platform_std_nv_seed_read /**< Default nv_seed_read function to use, can be undefined */
//#define MBEDTLS_PLATFORM_NV_SEED_WRITE_MACRO  mbedtls_platform_std_nv_seed_write /**< Default nv_seed_write function to use, can be undefined */
//#define MBEDTLS_PLATFORM_MS_TIME_TYPE_MACRO   int64_t //#define MBEDTLS_PLATFORM_MS_TIME_TYPE_MACRO   int64_t /**< Default milliseconds time macro to use, can be undefined. MBEDTLS_HAVE_TIME must be enabled. It must be signed, and at least 64 bits. If it is changed from the default, MBEDTLS_PRINTF_MS_TIME must be updated to match.*/
//#define MBEDTLS_PRINTF_MS_TIME    PRId64 /**< Default fmt for printf. That's avoid compiler warning if mbedtls_ms_time_t is redefined */

//#define MBEDTLS_CHECK_RETURN __attribute__((__warn_unused_result__))
//#define MBEDTLS_IGNORE_RETURN( result ) ((void) !(result))

/* PSA options */
//#define MBEDTLS_PSA_HMAC_DRBG_MD_TYPE MBEDTLS_MD_SHA256
//#define MBEDTLS_PSA_KEY_SLOT_COUNT 32
//#define MBEDTLS_PSA_STATIC_KEY_SLOT_BUFFER_SIZE       256

/* RSA OPTIONS */
//#define MBEDTLS_RSA_GEN_KEY_MIN_BITS            1024 /**<  Minimum RSA key size that can be generated in bits (Minimum possible value is 128 bits) */

/* SSL Cache options */
//#define MBEDTLS_SSL_CACHE_DEFAULT_TIMEOUT       86400 /**< 1 day  */
//#define MBEDTLS_SSL_CACHE_DEFAULT_MAX_ENTRIES      50 /**< Maximum entries in cache */

/* SSL options */
//#define MBEDTLS_SSL_IN_CONTENT_LEN              16384
//#define MBEDTLS_SSL_CID_IN_LEN_MAX 32
//#define MBEDTLS_SSL_CID_OUT_LEN_MAX 32
//#define MBEDTLS_SSL_CID_TLS1_3_PADDING_GRANULARITY 16
//#define MBEDTLS_SSL_OUT_CONTENT_LEN             16384
//#define MBEDTLS_SSL_DTLS_MAX_BUFFERING             32768

//#define MBEDTLS_PSK_MAX_LEN               32 /**< Max size of TLS pre-shared keys, in bytes (default 256 or 384 bits) */
//#define MBEDTLS_SSL_COOKIE_TIMEOUT        60 /**< Default expiration delay of DTLS cookies, in seconds if HAVE_TIME, or in number of cookies issued */

//#define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
#define MBEDTLS_SSL_CIPHERSUITES             \
    MBEDTLS_TLS1_3_AES_128_GCM_SHA256,       \
    MBEDTLS_TLS1_3_AES_256_GCM_SHA384,       \
    MBEDTLS_TLS1_3_CHACHA20_POLY1305_SHA256, \
    MBEDTLS_TLS1_3_AES_128_CCM_SHA256,       \
    MBEDTLS_TLS1_3_AES_128_CCM_8_SHA256
//#define MBEDTLS_SSL_MAX_EARLY_DATA_SIZE        1024
//#define MBEDTLS_SSL_TLS1_3_TICKET_AGE_TOLERANCE 6000
//#define MBEDTLS_SSL_TLS1_3_TICKET_NONCE_LENGTH 32
//#define MBEDTLS_SSL_TLS1_3_DEFAULT_NEW_SESSION_TICKETS 1

/* X509 options */
//#define MBEDTLS_X509_MAX_INTERMEDIATE_CA   8   /**< Maximum number of intermediate CAs in a verification chain. */
//#define MBEDTLS_X509_MAX_FILE_PATH_LEN     512 /**< Maximum length of a path/filename string in bytes including the null terminator character ('\0'). */

/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2016 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Ruslan Osmanov <osmanov@php.net>                             |
   +----------------------------------------------------------------------+
*/
#include "../src/common.h"
#include "../src/util.h"
#include "../src/priv.h"

#include "ssl_context.h"

#ifndef HAVE_EVENT_OPENSSL_LIB
# error "HAVE_EVENT_OPENSSL_LIB undefined"
#endif

/* {{{ Private */

/* {{{ verify_callback */
static int verify_callback(int ok, X509_STORE_CTX *ctx)
{
	SSL                     *ssl;
	int                      err;
	php_event_ssl_context_t *ectx;

	ssl  = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	ectx = (php_event_ssl_context_t *) SSL_get_ex_data(ssl, php_event_ssl_data_index);

	PHP_EVENT_ASSERT(ectx);

	X509_STORE_CTX_get_current_cert(ctx);
	err      = X509_STORE_CTX_get_error(ctx);

	if (!ok && (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT || err == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN)
			&& ectx->allow_self_signed) {
		ok = 1;
	}

	return ok;
}
/* }}} */

/* {{{ passwd_callback */
static int passwd_callback(char *buf, int num, int verify, void *data)
{
    HashTable  *ht  = (HashTable *) data;
    zval      **val = NULL;

	if (zend_hash_index_find(ht, PHP_EVENT_OPT_PASSPHRASE,
				(void **) &val) == SUCCESS) {
        if (Z_STRLEN_PP(val) < num - 1) {
            memcpy(buf, Z_STRVAL_PP(val), Z_STRLEN_PP(val) + 1);
            return Z_STRLEN_PP(val);
        }
    }

    return 0;
}
/* }}} */

/* {{{ set_ca */
static zend_always_inline void set_ca(SSL_CTX *ctx, const char *cafile, const char *capath TSRMLS_DC) {
	if (!SSL_CTX_load_verify_locations(ctx, cafile, capath)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Unable to set verify locations `%s' `%s'",
				cafile, capath);
	}
	if (cafile) {
		SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(cafile));
	}
}
/* }}} */

/* {{{ set_ciphers */
static zend_always_inline void set_ciphers(SSL_CTX *ctx, const char *cipher_list TSRMLS_DC)
{
	if (SSL_CTX_set_cipher_list(ctx, cipher_list) != 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed setting cipher list: `%s'", cipher_list);
	}
}
/* }}} */

/* {{{ _php_event_ssl_ctx_set_private_key */
int _php_event_ssl_ctx_set_private_key(SSL_CTX *ctx, const char *private_key TSRMLS_DC)
{
	if (private_key) {
		char resolved_path_buff_pk[MAXPATHLEN];

		if (VCWD_REALPATH(private_key, resolved_path_buff_pk)) {
			if (SSL_CTX_use_PrivateKey_file(ctx, resolved_path_buff_pk, SSL_FILETYPE_PEM) != 1) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Unable to set private key file `%s'",
						resolved_path_buff_pk);
				return -1;
			}

			return 0;
		}
	}

	return -1;
}
/* }}} */

/* {{{ _php_event_ssl_ctx_set_local_cert */
int _php_event_ssl_ctx_set_local_cert(SSL_CTX *ctx, const char *certfile, const char *private_key TSRMLS_DC)
{
	char resolved_path_buff[MAXPATHLEN];

	if (VCWD_REALPATH(certfile, resolved_path_buff)) {
		if (SSL_CTX_use_certificate_chain_file(ctx, resolved_path_buff) != 1) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"SSL_CTX_use_certificate_chain_file failed, file: `%s'", certfile);
			return -1;
		}

		if (private_key) {
			if (_php_event_ssl_ctx_set_private_key(ctx, private_key TSRMLS_CC)) {
				return -1;
			}
		} else {
			if (SSL_CTX_use_PrivateKey_file(ctx, resolved_path_buff, SSL_FILETYPE_PEM) != 1) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Unable to set private key file `%s'",
						resolved_path_buff);
				return -1;
			}
		}
	}

	return 0;
}
/* }}} */

/* {{{ set_ssl_ctx_options */
static inline void set_ssl_ctx_options(php_event_ssl_context_t *ectx TSRMLS_DC)
{
	SSL_CTX      *ctx         = ectx->ctx;
	HashTable    *ht          = ectx->ht;
	HashPosition  pos         = 0;
	zend_bool     got_ciphers = 0;
	int           verify_mode = SSL_VERIFY_NONE;
	char         *cafile      = NULL;
	char         *capath      = NULL;

	for (zend_hash_internal_pointer_reset_ex(ht, &pos);
			zend_hash_has_more_elements_ex(ht, &pos) == SUCCESS;
			zend_hash_move_forward_ex(ht, &pos)) {
		char   *key;
		uint    keylen;
		ulong   idx;
		int     type;
		zval  **ppzval;

		type = zend_hash_get_current_key_ex(ht, &key, &keylen,
				&idx, 0, &pos);
		if (type != HASH_KEY_IS_LONG) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Invalid option `%s'", key);
			continue;
		}

		if (zend_hash_get_current_data_ex(ht, (void **) &ppzval, &pos) == FAILURE) {
			continue;
		}

		switch (idx) {
			case PHP_EVENT_OPT_LOCAL_CERT: {
				zval **ppz_private_key;
				convert_to_string_ex(ppzval);

				if (zend_hash_index_find(ht, PHP_EVENT_OPT_LOCAL_PK,
						(void **) &ppz_private_key) == SUCCESS) {
					_php_event_ssl_ctx_set_local_cert(ctx, Z_STRVAL_PP(ppzval), Z_STRVAL_PP(ppz_private_key) TSRMLS_CC);
				} else {
					_php_event_ssl_ctx_set_local_cert(ctx, Z_STRVAL_PP(ppzval), NULL TSRMLS_CC);
				}
				break;
			}
			case PHP_EVENT_OPT_LOCAL_PK:
				/* Skip. SSL_CTX_use_PrivateKey_file is applied in "local_cert". */
				break;
			case PHP_EVENT_OPT_PASSPHRASE:
				convert_to_string_ex(ppzval);
				SSL_CTX_set_default_passwd_cb_userdata(ctx, ht);
				SSL_CTX_set_default_passwd_cb(ctx, passwd_callback);
				break;
			case PHP_EVENT_OPT_CA_FILE:
				convert_to_string_ex(ppzval);
				cafile = Z_STRVAL_PP(ppzval);
				break;
			case PHP_EVENT_OPT_CA_PATH:
				convert_to_string_ex(ppzval);
				capath = Z_STRVAL_PP(ppzval);
				break;
#ifdef HAVE_SSL2
			case PHP_EVENT_OPT_NO_SSLv2:
# if OPENSSL_VERSION_NUMBER >= 0x10100000L
				php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
						"OPT_NO_SSLv2 is deprecated, "
						"use EventSslContext::setMinProtoVersion instead. "
						"Setting minimal protocol version to 0");
				/* There is no constant for SSL2 in OpenSSL 1.1.0 */
				SSL_CTX_set_min_proto_version(ctx, 0);
# else
				if (zval_is_true(*ppzval)) {
					SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
				} else {
					SSL_CTX_clear_options(ctx, SSL_OP_NO_SSLv2);
				}
# endif
				break;
#endif
#ifdef HAVE_SSL3
			case PHP_EVENT_OPT_NO_SSLv3:
# if OPENSSL_VERSION_NUMBER >= 0x10100000L
				php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
						"OPT_NO_SSLv3 is deprecated, "
						"use EventSslContext::setMinProtoVersion instead. "
						"Setting minimal protocol version to %d",
						zend_is_true(*ppzval) ? SSL3_VERSION : 0);
				SSL_CTX_set_min_proto_version(ctx,
						zend_is_true(*ppzval) ? SSL3_VERSION : 0);
# else
				if (zval_is_true(*ppzval)) {
					SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3);
				} else {
					SSL_CTX_clear_options(ctx, SSL_OP_NO_SSLv3);
				}
# endif
				break;
#endif
			case PHP_EVENT_OPT_NO_TLSv1:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
				php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
						"OPT_NO_TLSv1 is deprecated, "
						"use EventSslContext::setMinProtoVersion instead. "
						"Setting minimal protocol version to %d",
						zend_is_true(*ppzval) ? TLS1_VERSION : 0);
				SSL_CTX_set_min_proto_version(ctx,
						zend_is_true(*ppzval) ? TLS1_VERSION : 0);
#else
				if (zval_is_true(*ppzval)) {
					SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1);
				} else {
					SSL_CTX_clear_options(ctx, SSL_OP_NO_TLSv1);
				}
#endif
				break;
#ifdef SSL_OP_NO_TLSv1_1
			case PHP_EVENT_OPT_NO_TLSv1_1:
# if OPENSSL_VERSION_NUMBER >= 0x10100000L
				php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
						"OPT_NO_TLSv1_1 is deprecated, "
						"use EventSslContext::setMinProtoVersion instead. "
						"Setting minimal protocol version to %d",
						zend_is_true(*ppzval) ? TLS1_1_VERSION : 0);
				SSL_CTX_set_min_proto_version(ctx,
						zend_is_true(*ppzval) ? TLS1_1_VERSION : 0);
# else
				if (zval_is_true(*ppzval)) {
					SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_1);
				} else {
					SSL_CTX_clear_options(ctx, SSL_OP_NO_TLSv1_1);
				}
# endif
				break;
#endif
#ifdef SSL_OP_NO_TLSv1_2
			case PHP_EVENT_OPT_NO_TLSv1_2:
# if OPENSSL_VERSION_NUMBER >= 0x10100000L
				php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
						"OPT_NO_TLSv1_2 is deprecated, "
						"use EventSslContext::setMinProtoVersion instead. "
						"Setting minimal protocol version to %d",
						zend_is_true(*ppzval) ? TLS1_2_VERSION : 0);
				SSL_CTX_set_min_proto_version(ctx,
						zend_is_true(*ppzval) ? TLS1_2_VERSION : 0);
# else
				if (zval_is_true(*ppzval)) {
					SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_2);
				} else {
					SSL_CTX_clear_options(ctx, SSL_OP_NO_TLSv1_2);
				}
# endif
				break;
#endif
			case PHP_EVENT_OPT_CIPHER_SERVER_PREFERENCE:
				if (zval_is_true(*ppzval)) {
					SSL_CTX_set_options(ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
				} else {
					SSL_CTX_clear_options(ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
				}
				break;
			case PHP_EVENT_OPT_ALLOW_SELF_SIGNED:
				ectx->allow_self_signed = (zend_bool) zval_is_true(*ppzval);
				break;
			case PHP_EVENT_OPT_VERIFY_PEER:
				if (zval_is_true(*ppzval)) {
					verify_mode |= SSL_VERIFY_PEER;
				} else {
					verify_mode &= ~SSL_VERIFY_PEER;
				}
				break;
			case PHP_EVENT_OPT_VERIFY_DEPTH:
				convert_to_long_ex(ppzval);
				SSL_CTX_set_verify_depth(ctx, Z_LVAL_PP(ppzval));
				break;
			case PHP_EVENT_OPT_CIPHERS:
				got_ciphers = 1;
				convert_to_string_ex(ppzval);
				set_ciphers(ctx, Z_STRVAL_PP(ppzval) TSRMLS_CC);
				break;
			case PHP_EVENT_OPT_REQUIRE_CLIENT_CERT:
				if (zend_is_true(*ppzval)) {
					verify_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
				}
				break;
			case PHP_EVENT_OPT_VERIFY_CLIENT_ONCE:
				if (zend_is_true(*ppzval)) {
					verify_mode |= SSL_VERIFY_CLIENT_ONCE;
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Unknown option %ld", idx);
		}
	}

	SSL_CTX_set_verify(ctx, verify_mode, verify_callback);

	if (got_ciphers == 0) {
		set_ciphers(ctx, "DEFAULT" TSRMLS_CC);
	}

	if (cafile || capath) {
		/* We have to disable this flag, because CA file/path provides
		 * a "whitelist" of specific certificates which will pass even if self-signed.
		 * We can't have allow_self_signed enabled, because in this case verify_callback
		 * accepts *any* self-signed certificate.
		 */
		ectx->allow_self_signed = 0;

		set_ca(ctx, cafile, capath TSRMLS_CC);
	}
}
/* }}} */

/* {{{ get_ssl_method */
static zend_always_inline SSL_METHOD *get_ssl_method(long in_method TSRMLS_DC)
{
	SSL_METHOD *method;

	switch (in_method) {
		case PHP_EVENT_SSLv2_CLIENT_METHOD:
#ifndef HAVE_SSL2
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"SSLv2 support is not compiled into the "
					"OpenSSL library PHP is linked against");
			return NULL;
#else
			method = (SSL_METHOD *) SSLv2_client_method();
			break;
#endif

		case PHP_EVENT_SSLv3_CLIENT_METHOD:
#ifndef HAVE_SSL3
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"SSLv3 support is not compiled into the "
					"OpenSSL library PHP is linked against");
			return NULL;
#elif OPENSSL_VERSION_NUMBER >= 0x10100000L
			php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
					"SSLv3_CLIENT_METHOD is deprecated, "
					"use TLS_CLIENT_METHOD instead");
			method = (SSL_METHOD *) TLS_client_method();
			break;
#else
			method = (SSL_METHOD *) SSLv3_client_method();
			break;
#endif

		case PHP_EVENT_SSLv23_CLIENT_METHOD:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			method = (SSL_METHOD *) TLS_client_method();
#else
			method = (SSL_METHOD *) SSLv23_client_method();
#endif
			break;

		case PHP_EVENT_TLS_CLIENT_METHOD:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			method = (SSL_METHOD *) TLS_client_method();
#else
			method = (SSL_METHOD *) TLSv1_client_method();
#endif
			break;

		case PHP_EVENT_SSLv2_SERVER_METHOD:
#ifndef HAVE_SSL2
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"SSLv2 support is not compiled into the "
					"OpenSSL library PHP is linked against");
			return NULL;
#else
			method = (SSL_METHOD *) SSLv2_server_method();
			break;
#endif

		case PHP_EVENT_SSLv3_SERVER_METHOD:
#ifndef HAVE_SSL3
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"SSLv3 support is not compiled into the "
					"OpenSSL library PHP is linked against");
			return NULL;
#elif OPENSSL_VERSION_NUMBER >= 0x10100000L
			method = (SSL_METHOD *) TLS_server_method();
			break;
#else
			method = (SSL_METHOD *) SSLv3_server_method();
			break;
#endif

		case PHP_EVENT_SSLv23_SERVER_METHOD:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			method = (SSL_METHOD *) TLS_server_method();
#else
			method = (SSL_METHOD *) SSLv23_server_method();
#endif
			break;

		case PHP_EVENT_TLS_SERVER_METHOD:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			method = (SSL_METHOD *) TLS_server_method();
#else
			method = (SSL_METHOD *) TLSv1_server_method();
#endif
			break;

		case PHP_EVENT_TLSv11_CLIENT_METHOD:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
					"TLSv11_CLIENT_METHOD is deprecated, "
					"use TLS_CLIENT_METHOD instead");
			method = (SSL_METHOD *) TLS_client_method();
			break;
#elif defined(SSL_OP_NO_TLSv1_1)
			method = (SSL_METHOD *) TLSv1_1_client_method();
			break;
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"TLSv1_1 support is not compiled into the "
					"OpenSSL library PHP is linked against");
			return NULL;
#endif

		case PHP_EVENT_TLSv11_SERVER_METHOD:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
					"TLSv11_SERVER_METHOD is deprecated, "
					"use TLS_SERVER_METHOD instead");
			method = (SSL_METHOD *) TLS_server_method();
			break;
#elif defined(SSL_OP_NO_TLSv1_1)
			method = (SSL_METHOD *) TLSv1_1_server_method();
			break;
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"TLSv1_1 support is not compiled into the "
					"OpenSSL library PHP is linked against");
			return NULL;
#endif

		case PHP_EVENT_TLSv12_CLIENT_METHOD:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
					"TLSv12_CLIENT_METHOD is deprecated, "
					"use TLS_CLIENT_METHOD instead");
			method = (SSL_METHOD *) TLS_client_method();
			break;
#elif defined(SSL_OP_NO_TLSv1_2)
			method = (SSL_METHOD *) TLSv1_2_client_method();
			break;
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"TLSv1_2 support is not compiled into the "
					"OpenSSL library PHP is linked against");
			return NULL;
#endif

		case PHP_EVENT_TLSv12_SERVER_METHOD:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			php_error_docref(NULL TSRMLS_CC, E_DEPRECATED,
					"TLSv12_SERVER_METHOD is deprecated, "
					"use TLS_SERVER_METHOD instead");
			method = (SSL_METHOD *) TLS_server_method();
			break;
#elif defined(SSL_OP_NO_TLSv1_2)
			method = (SSL_METHOD *) TLSv1_2_server_method();
			break;
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"TLSv1_2 support is not compiled into the "
					"OpenSSL library PHP is linked against");
			return NULL;
#endif

		default:
			return NULL;
	}

	return method;
}
/* }}} */

/* Private }}} */


/* {{{ proto EventSslContext EventSslContext::__construct(int method, array options);
 *
 * Creates SSL context holding pointer to SSL_CTX.
 * method parameter is one of EventSslContext::*_METHOD constants.
 * options parameter is an associative array of SSL context options */
PHP_METHOD(EventSslContext, __construct)
{
	php_event_ssl_context_t *ectx;
	HashTable               *ht_options;
	long                     in_method;
	SSL_METHOD              *method;
	SSL_CTX                 *ctx;
	long                     options    = SSL_OP_ALL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lh",
				&in_method, &ht_options) == FAILURE) {
		return;
	}

	method = get_ssl_method(in_method TSRMLS_CC);
	if (method == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"Invalid method passed: %ld", in_method);
		return;
	}

	ctx = SSL_CTX_new(method);
	if (ctx == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Creation of a new SSL_CTX object failed");
		return;
	}

	PHP_EVENT_FETCH_SSL_CONTEXT(ectx, getThis());
	ectx->ctx = ctx;

	ALLOC_HASHTABLE(ectx->ht);
	if (zend_hash_init_ex(ectx->ht, zend_hash_num_elements(ht_options), NULL, ZVAL_PTR_DTOR, 0, 0) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to allocate hashtable for options");
		FREE_HASHTABLE(ectx->ht);
		return;
	}
	zend_hash_copy(ectx->ht, ht_options, (copy_ctor_func_t) zval_add_ref,
			(void *) NULL, sizeof(zval *));

	SSL_CTX_set_options(ectx->ctx, options);
	set_ssl_ctx_options(ectx TSRMLS_CC);

	/* Issue #20 */
	SSL_CTX_set_session_id_context(ectx->ctx, (unsigned char *)(void *)ectx->ctx, sizeof(ectx->ctx));
}
/* }}} */


#if OPENSSL_VERSION_NUMBER >= 0x10100000L
/*{{{ proto bool EventSslContext::setMinProtoVersion(int proto);
 *
 * Sets minimum supported protocol version for the SSL context.
 */
PHP_METHOD(EventSslContext, setMinProtoVersion)
{
	php_event_ssl_context_t *ectx;
	long                     proto;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &proto) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_SSL_CONTEXT(ectx, getThis());

	if (!SSL_CTX_set_min_proto_version(ectx->ctx, proto)) {
		RETVAL_FALSE;
	}
	RETVAL_TRUE;
} /*}}}*/

/*{{{ proto bool EventSslContext::setMaxProtoVersion(int proto);
 *
 * Sets max supported protocol version for the SSL context.
 */
PHP_METHOD(EventSslContext, setMaxProtoVersion)
{
	php_event_ssl_context_t *ectx;
	long                     proto;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &proto) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_SSL_CONTEXT(ectx, getThis());

	if (!SSL_CTX_set_max_proto_version(ectx->ctx, proto)) {
		RETVAL_FALSE;
	}
	RETVAL_TRUE;
} /*}}}*/
#endif /* OpenSSL version >= 1.1.0 */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */

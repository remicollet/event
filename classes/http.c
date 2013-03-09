/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
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
#include "src/common.h"
#include "src/util.h"
#include "src/priv.h"

/* {{{ proto EventHttp EventHttp::__construct(EventBase base);
 *
 * Creates new http server object.
 */
PHP_METHOD(EventHttp, __construct)
{
	zval             *zbase;
	php_event_base_t *b;
	php_event_http_t *http;
	struct evhttp    *http_ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbase, php_event_base_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	PHP_EVENT_FETCH_HTTP(http, getThis());

	http_ptr = evhttp_new(b->base);
	if (!http_ptr) {
		return;
	}
	http->ptr = http_ptr;

	http->base = zbase;
	Z_ADDREF_P(zbase);

	http->stream_id = -1;
}
/* }}} */

/* {{{ proto bool EventHttp::accept(mixed socket);
 *
 * Makes an HTTP server accept connections on the specified socket stream or resource.
 * The socket should be ready to accept connections.
 * Can be called multiple times to accept connections on different sockets. */
PHP_METHOD(EventHttp, accept)
{
	php_event_http_t  *http;
	zval              *zhttp = getThis();
	zval             **ppzfd;
	evutil_socket_t    fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z",
				&ppzfd) == FAILURE) {
		return;
	}

	fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	if (evhttp_accept_socket(http->ptr, fd)) {
		RETURN_FALSE;
	}

#if 0
	http->stream_id = Z_LVAL_P(ppzfd);
	zend_list_addref(Z_LVAL_P(ppzfd));
#endif

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventHttp::bind(string address, int port);
 *
 * Binds an HTTP server on the specified address and port.
 * Can be called multiple times to bind the same http server to multiple different ports. */
PHP_METHOD(EventHttp, bind)
{
	zval              *zhttp = getThis();
	php_event_http_t  *http;
	char *address;
	int address_len;
	long port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl",
				&address, &address_len, &port) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	if (evhttp_bind_socket(http->ptr, address, port)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */

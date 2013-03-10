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

/* {{{ Private */

/* {{{ _http_callback */
static void _http_callback(struct evhttp_request *req, void *arg)
{
	php_event_http_t *http = (php_event_http_t *) arg;
	PHP_EVENT_ASSERT(http);

	php_event_http_req_t *http_req;

	zend_fcall_info       *pfci = http->fci;
	zend_fcall_info_cache *pfcc = http->fcc;
	PHP_EVENT_ASSERT(pfci && pfcc);

	TSRMLS_FETCH_FROM_CTX(http->thread_ctx);

	/* Call userspace function according to
	 * proto void callback(EventHttpRequest req, mixed data);*/

	zval  *arg_data = http->data;
	zval  *arg_req;
	zval **args[2];
	zval  *retval_ptr;

	MAKE_STD_ZVAL(arg_req);
	PHP_EVENT_INIT_CLASS_OBJECT(arg_req, php_event_http_req_ce);
	PHP_EVENT_FETCH_HTTP_REQ(http_req, arg_req);
	http_req->ptr = req;
	Z_ADDREF_P(arg_req);
	args[0] = &arg_req;

	if (arg_data) {
		Z_ADDREF_P(arg_data);
	} else {
		ALLOC_INIT_ZVAL(arg_data);
	}
	args[1] = &arg_data;

	pfci->params		 = args;
	pfci->retval_ptr_ptr = &retval_ptr;
	pfci->param_count	 = 2;
	pfci->no_separation  = 1;

    if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS && retval_ptr) {
        zval_ptr_dtor(&retval_ptr);
    } else {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                "An error occurred while invoking the http request callback");
    }

    zval_ptr_dtor(&arg_req);
    zval_ptr_dtor(&arg_data);
}
/* }}} */

/* }}} */

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
	http->fci       = http->fcc      = NULL;
	http->data      = http->gen_data = NULL;
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

/* {{{ proto bool EventHttp::setCallback(string path, callable cb[, mixed arg = NULL]);
 * Set a callback for a specified URI.
 */
PHP_METHOD(EventHttp, setCallback)
{
	zval                  *zhttp    = getThis();
	php_event_http_t      *http;
	char                  *path;
	int                    path_len;
	zend_fcall_info        fci      = empty_fcall_info;
	zend_fcall_info_cache  fcc      = empty_fcall_info_cache;
	zval                  *zarg     = NULL;
	int                    res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sf|z!",
				&path, &path_len, &fci, &fcc, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	res = evhttp_set_cb(http->ptr, path, _http_callback, (void *) http);
	if (res == -2) {
		RETURN_FALSE;
	}
	if (res == -1) { // the callback existed already
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"The callback already exists");
		RETURN_FALSE;
	}

	if (http->data) {
		zval_ptr_dtor(&http->data);
	}
	if (zarg) {
		Z_ADDREF_P(zarg);
	}
	http->data = zarg;

	/* 
	 * XXX We should set up individual user functions for every path!!!
	 */

	PHP_EVENT_COPY_FCALL_INFO(http->fci, http->fcc, &fci, &fcc);

	TSRMLS_SET_CTX(http->thread_ctx);

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void EventHttp::setAllowedMethods(int methods);
 * Sets the what HTTP methods are supported in requests accepted by this
 * server, and passed to user callbacks.
 *
 * If not supported they will generate a <literal>"405 Method not
 * allowed"</literal> response.
 *
 * By default this includes the following methods: GET, POST, HEAD, PUT, DELETE
 */
PHP_METHOD(EventHttp, setAllowedMethods)
{
	zval             *zhttp   = getThis();
	php_event_http_t *http;
	long              methods;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&methods) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	evhttp_set_allowed_methods(http->ptr, methods);
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

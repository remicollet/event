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

/* {{{ proto EventHttpConnection EventHttpConnection::__construct(EventBase base, EventDnsBase dns_base, string address, int port);
 * */
PHP_METHOD(EventHttpConnection, __construct)
{
	zval                     *zbase;
	php_event_base_t         *b;
	zval                     *zdns_base;
	php_event_dns_base_t     *dnsb;
	char                     *address;
	int                       address_len;
	long                      port;
	php_event_http_conn_t    *evcon;
	struct evhttp_connection *conn;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOsl",
				&zbase, php_event_base_ce, &zdns_base, php_event_dns_base_ce,
				&address, &address_len, &port) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);
	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	PHP_EVENT_FETCH_HTTP_CONN(evcon, getThis());

	conn = evhttp_connection_base_new(b->base, dnsb->dns_base, address, (unsigned short) port);
	if (!conn) {
		return;
	}
	evcon->conn = conn;

	evcon->base = zbase;
	Z_ADDREF_P(zbase);

	evcon->dns_base = zdns_base;
	Z_ADDREF_P(zdns_base);
}
/* }}} */

/* {{{ proto EventBase EventHttpConnection::getBase(void);
 *
 * Get event base associated with the http connection.
 */
PHP_METHOD(EventHttpConnection, getBase)
{
	zval                 *zevcon = getThis();
	php_event_http_conn_t *evcon;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	/*
	 * No sense in this call
	 * base = evhttp_connection_get_base(evcon->con);
	 */

	if (evcon->base) {
		RETURN_ZVAL(evcon->base, 1, 0);
	}

	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto void EventHttpConnection::getPeer(string &address, int &port);
 * Get the remote address and port associated with this connection. */
PHP_METHOD(EventHttpConnection, getPeer)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	zval                  *zaddress;
	zval                  *zport;

	char *address;
	unsigned short port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz",
				&zaddress, &zport) == FAILURE) {
		return;
	}

	if (! (Z_ISREF_P(zaddress) && Z_ISREF_P(zport))) {
		/* Was not passed by reference */
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_get_peer(evcon->conn, &address, &port);

	ZVAL_STRING(zaddress, address, 1);
	ZVAL_LONG(zport, port);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setLocalAddress(string address);
 * Sets the ip address from which http connections are made */
PHP_METHOD(EventHttpConnection, setLocalAddress)
{
	zval                  *zevcon      = getThis();
	php_event_http_conn_t *evcon;
	char                  *address;
	int                    address_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&address, &address_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_local_address(evcon->conn, address);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setLocalPort(int port);
 * Sets the port from which http connections are made */
PHP_METHOD(EventHttpConnection, setLocalPort)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&port) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_local_port(evcon->conn, port);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setTimeout(int timeout);
 */
PHP_METHOD(EventHttpConnection, setTimeout)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   timeout;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&timeout) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_timeout(evcon->conn, timeout);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setMaxHeadersSize(int max_size);
 */
PHP_METHOD(EventHttpConnection, setMaxHeadersSize)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   max_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&max_size) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_max_headers_size(evcon->conn, (ev_ssize_t) max_size);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setMaxBodySize(int max_size);
 */
PHP_METHOD(EventHttpConnection, setMaxBodySize)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   max_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&max_size) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_max_body_size(evcon->conn, (ev_ssize_t) max_size);
}
/* }}} */

/* {{{ proto void EventHttpConnection::setRetries(int retries);
 */
PHP_METHOD(EventHttpConnection, setRetries)
{
	zval                  *zevcon = getThis();
	php_event_http_conn_t *evcon;
	long                   retries;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&retries) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_retries(evcon->conn, retries);
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

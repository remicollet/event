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

/* {{{ proto int EventUtil::getLastSocketErrno([mixed socket = null]);
 *
 * Returns the most recent socket error number(errno). */
PHP_METHOD(EventUtil, getLastSocketErrno)
{
	zval **ppzfd = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|Z!",
				&ppzfd) == FAILURE) {
		return;
	}

	if (ppzfd) {
		evutil_socket_t fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);

		if (fd < 0) {
			RETURN_FALSE;
		}

		RETVAL_LONG(evutil_socket_geterror(fd));
	} else {
		RETVAL_LONG(EVUTIL_SOCKET_ERROR());
	}
}
/* }}} */

/* {{{ proto string EventUtil::getLastSocketError([resource socket = null]);
 *
 * Returns the most recent socket error */
PHP_METHOD(EventUtil, getLastSocketError)
{
	zval **ppzfd = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|Z!",
				&ppzfd) == FAILURE) {
		return;
	}

	if (ppzfd) {
		evutil_socket_t fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);

		if (fd < 0) {
			RETURN_FALSE;
		}

		RETVAL_STRING(evutil_socket_error_to_string(evutil_socket_geterror(fd)), 1);
	} else {
		RETVAL_STRING(evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), 1);
	}
}
/* }}} */

#ifdef HAVE_EVENT_OPENSSL_LIB/* {{{ */
/* {{{ proto bool EventUtil::sslRandPoll(void);
 * 
 * Generates entropy by means of OpenSSL's RAND_poll()
 */
PHP_METHOD(EventUtil, sslRandPoll)
{
	RETVAL_BOOL((zend_bool) RAND_poll());
}
/* }}} */
#endif/* }}} */

/* {{{ proto bool EventUtil::getSocketName(mixed socket, string &address[, int &port]);
 * Retreives the current address to which the <parameter>socket</parameter> is bound.
 *
 * <parameter>socket</parameter> may be a stream or socket resource, or a numeric file descriptor
 * associated with a socket.
 *
 * Returns &true; on success. Otherwise &false;.*/
PHP_METHOD(EventUtil, getSocketName)
{
	zval            **ppzfd;
	zval             *zaddress;
	zval             *zport    = NULL;
	evutil_socket_t   fd;

	php_sockaddr_storage  sa_storage;
	struct sockaddr      *sa         = (struct sockaddr *) &sa_storage;
	socklen_t             sa_len     = sizeof(php_sockaddr_storage);
	long                  port       = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Zz|z",
				&ppzfd, &zaddress, &zport) == FAILURE) {
		return;
	}

	fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	if (getsockname(fd, sa, &sa_len)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Unable to retreive socket name, errno: %d", errno);
		RETURN_FALSE;
	}

	switch (sa->sa_family) {
		case AF_INET:
			{
				struct sockaddr_in *sin = (struct sockaddr_in *) sa;
				char addr[INET_ADDRSTRLEN + 1];

				if (evutil_inet_ntop(sa->sa_family, &sin->sin_addr,
							(void *) &addr, sizeof(addr))) {
					zval_dtor(zaddress);
					ZVAL_STRING(zaddress, addr, 1);

					if (zport != NULL) {
						port = ntohs(sin->sin_port);
					}
				}
			}
			break;
#if HAVE_IPV6 && HAVE_INET_NTOP
		case AF_INET6:
			{
				struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
				char addr6[INET6_ADDRSTRLEN + 1];

				if (evutil_inet_ntop(sa->sa_family, &sin6->sin6_addr,
							(void *) &addr6, sizeof(addr6))) {
					zval_dtor(zaddress);
					ZVAL_STRING(zaddress, addr6, 1);

					if (zport != NULL) {
						port = ntohs(sin6->sin6_port);
					}
				}
			}
			break;
#endif
#ifdef AF_UNIX
		case AF_UNIX:
			{
				struct sockaddr_un *ua = (struct sockaddr_un *) sa;

				zval_dtor(zaddress);
				ZVAL_STRING(zaddress, ua->sun_path, 1);
			}
			break;
#endif
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Unsupported address family: %d", sa->sa_family);
			RETURN_FALSE;
	}

	if (port != -1) {
		zval_dtor(zport);
		ZVAL_LONG(zport, port);
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

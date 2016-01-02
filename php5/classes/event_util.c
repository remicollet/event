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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Zz|z",
				&ppzfd, &zaddress, &zport) == FAILURE) {
		return;
	}

	fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	if (_php_event_getsockname(fd, &zaddress, &zport TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventUtil::setSocketOption(mixed socket, int level, int optname, int|array optval)
   Sets socket options for the socket */
PHP_METHOD(EventUtil, setSocketOption)
{
	zval            **ppzfd    , **zoptval;
	struct linger     lv;
	int               ov;
	int               optlen;
	int               retval;
	struct timeval    tv;
	long              level;
	long              optname;
	void             *opt_ptr;
	HashTable        *opt_ht;
	zval            **l_onoff  , **l_linger;
	zval            **sec      , **usec;
	evutil_socket_t   fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ZllZ",
				&ppzfd, &level, &optname, &zoptval) == FAILURE) {
		return;
	}

	fd = php_event_zval_to_fd(ppzfd TSRMLS_CC);
	if (fd == -1) {
		RETURN_FALSE;
	}

	/*errno = 0;*/

	switch (optname) {
		case SO_LINGER: {
			const char l_onoff_key[]  = "l_onoff";
			const char l_linger_key[] = "l_linger";

			convert_to_array_ex(zoptval);
			opt_ht = HASH_OF(*zoptval);

			if (zend_hash_find(opt_ht, l_onoff_key, sizeof(l_onoff_key), (void **) &l_onoff) == FAILURE) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "no key \"%s\" passed in optval", l_onoff_key);
				RETURN_FALSE;
			}
			if (zend_hash_find(opt_ht, l_linger_key, sizeof(l_linger_key), (void **) &l_linger) == FAILURE) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "no key \"%s\" passed in optval", l_linger_key);
				RETURN_FALSE;
			}

			convert_to_long_ex(l_onoff);
			convert_to_long_ex(l_linger);

			lv.l_onoff  = (unsigned short) Z_LVAL_PP(l_onoff);
			lv.l_linger = (unsigned short) Z_LVAL_PP(l_linger);

			optlen = sizeof(lv);
			opt_ptr = &lv;
			break;
		}

		case SO_RCVTIMEO:
		case SO_SNDTIMEO: {
			const char sec_key[]  = "sec";
			const char usec_key[] = "usec";

			convert_to_array_ex(zoptval);
			opt_ht = HASH_OF(*zoptval);

			if (zend_hash_find(opt_ht, sec_key, sizeof(sec_key), (void **) &sec) == FAILURE) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "no key \"%s\" passed in optval", sec_key);
				RETURN_FALSE;
			}
			if (zend_hash_find(opt_ht, usec_key, sizeof(usec_key), (void **) &usec) == FAILURE) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "no key \"%s\" passed in optval", usec_key);
				RETURN_FALSE;
			}

			convert_to_long_ex(sec);
			convert_to_long_ex(usec);

			tv.tv_sec  = Z_LVAL_PP(sec);
			tv.tv_usec = Z_LVAL_PP(usec);

			optlen  = sizeof(tv);
			opt_ptr = &tv;
			break;
		}

		default:
			convert_to_long_ex(zoptval);
			ov = Z_LVAL_PP(zoptval);

			optlen = sizeof(ov);
			opt_ptr = &ov;
			break;
	}

	retval = setsockopt(fd, level, optname, opt_ptr, optlen);
	if (retval != 0) {
		if (retval != -2) { /* error, but message already emitted */
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Unable to set socket option, errno: %d", errno);
		}

		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventUtil::getSocketFd(mixed socket)
 *    Gets numeric file descriptor of a socket. */
PHP_METHOD(EventUtil, getSocketFd) {
	zval **ppzfd = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z",
				&ppzfd) == FAILURE) {
		return;
	}

	RETVAL_LONG(ppzfd ? php_event_zval_to_fd(ppzfd TSRMLS_CC) : -1);
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

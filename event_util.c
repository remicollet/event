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
#include "common.h"
#include "util.h"
#include "priv.h"

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

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */

/*
   +----------------------------------------------------------------------+
   | PHP Version 8                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2023 The PHP Group                                |
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
#ifndef PHP_WIN32
# include <fcntl.h>
#endif
#include "zend_exceptions.h"

/* {{{ php_event_zval_to_fd
 * Get numeric file descriptor from PHP stream or Socket resource */
php_socket_t php_event_zval_to_fd(zval *pfd)
{
	php_socket_t  file_desc = -1;
	php_stream   *stream;
	FILE *fp = NULL;
#ifdef PHP_EVENT_SOCKETS_SUPPORT
	php_socket   *php_sock;
#endif
	const char* invalid_fd_error = "Invalid file descriptor";

	if (Z_TYPE_P(pfd) == IS_RESOURCE) {
		/* PHP stream or PHP socket resource  */
		if ((stream = (php_stream *)zend_fetch_resource2(Z_RES_P(pfd), NULL, php_file_le_stream(), php_file_le_pstream())) != NULL) {
			if (php_stream_is(stream, PHP_STREAM_IS_MEMORY) || php_stream_is(stream, PHP_STREAM_IS_TEMP)) {
				zend_throw_exception(zend_ce_exception,
						"Cannot fetch file descriptor from memory based stream", 0);
				return -1;
			}

			php_stream_from_zval_no_verify(stream, pfd);

			if (stream == NULL) {
				zend_throw_exception(zend_ce_exception, "Failed obtaining fd", 0);
				return -1;
			}

			/* PHP stream */
			if (php_stream_can_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL,
							(void *) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else if (php_stream_can_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL,
							(void *) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else if (php_stream_can_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_STDIO,
							(void **) &fp, 1) != SUCCESS) {
					return -1;
				}
				file_desc = fileno(fp);
			} else { /* STDIN, STDOUT, STDERR etc. */
				file_desc = Z_LVAL_P(pfd);
			}
		} else {
			zend_throw_exception(zend_ce_exception,
					"valid PHP stream resource expected", 0);
			return -1;
		}
#ifdef PHP_EVENT_SOCKETS_SUPPORT
	} else if (Z_TYPE_P(pfd) == IS_OBJECT && Z_OBJCE_P(pfd) == socket_ce) {
		php_sock = Z_SOCKET_P(pfd);
		if (php_sock->error) {
			if (!php_sock->blocking && php_sock->error == EINPROGRESS) {
#ifdef PHP_EVENT_DEBUG
				php_error_docref(NULL, E_NOTICE, "Operation in progress");
#endif
			} else {
				return -1;
			}
		}
		return php_sock->bsd_socket;
#endif
	} else if (Z_TYPE_P(pfd) == IS_LONG) {
		file_desc = Z_LVAL_P(pfd);
		if (file_desc < 0) {
			zend_throw_exception(zend_ce_exception, invalid_fd_error, 0);
			return -1;
		}
	} else {
		zend_throw_exception(zend_ce_exception, invalid_fd_error, 0);
		return -1;
	}

	if (!ZEND_VALID_SOCKET(file_desc)) {
		zend_throw_exception(zend_ce_exception, invalid_fd_error, 0);
		return -1;
	}

	return file_desc;
}
/* }}} */

int _php_event_getsockname(evutil_socket_t fd, zval *pzaddr, zval *pzport)/*{{{*/
{
	php_sockaddr_storage  sa_storage;
	struct sockaddr      *sa         = (struct sockaddr *)&sa_storage;
	socklen_t             sa_len     = sizeof(php_sockaddr_storage);
	zend_long             port       = -1;

	if (getsockname(fd, sa, &sa_len)) {
		php_error_docref(NULL, E_WARNING,
				"Unable to retreive socket name, errno: %d", errno);
		return FAILURE;
	}

	switch (sa->sa_family) {
		case AF_INET:
			{
				struct sockaddr_in *sin = (struct sockaddr_in *) sa;
				char addr[INET_ADDRSTRLEN + 1];

				if (evutil_inet_ntop(sa->sa_family, &sin->sin_addr,
							(void *) &addr, sizeof(addr))) {
					if (!Z_ISUNDEF_P(pzaddr)) {
						zval_dtor(pzaddr);
					}
					ZVAL_STRING(pzaddr, addr);

					if (pzport != NULL) {
						port = ntohs(sin->sin_port);
					}
				}
			}
			break;
#if HAVE_IPV6
		case AF_INET6:
			{
				struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
				char addr6[INET6_ADDRSTRLEN + 1];

				if (evutil_inet_ntop(sa->sa_family, &sin6->sin6_addr,
							(void *) &addr6, sizeof(addr6))) {
					if (!Z_ISUNDEF_P(pzaddr)) {
						zval_dtor(pzaddr);
					}
					ZVAL_STRING(pzaddr, addr6);

					if (pzport != NULL) {
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

				if (!Z_ISUNDEF_P(pzaddr)) {
					zval_dtor(pzaddr);
				}
				ZVAL_STRING(pzaddr, ua->sun_path);
			}
			break;
#endif
		default:
			php_error_docref(NULL, E_WARNING,
					"Unsupported address family: %d", sa->sa_family);
			return FAILURE;
	}

	if (port != -1) {
		if (pzport && !Z_ISUNDEF_P(pzport)) {
			zval_dtor(pzport);
		}
		ZVAL_LONG(pzport, port);
	}

	return SUCCESS;
}/*}}}*/

void php_event_call_or_break( /*{{{*/
		struct event_base *base,
		zend_fcall_info *fci,
		zend_fcall_info_cache *fcc,
		php_event_break_loop_cleanup cleanup_cb,
		void *cleanup_data
		)
{
	PHP_EVENT_ASSERT(base);
	PHP_EVENT_ASSERT(fci);
	PHP_EVENT_ASSERT(fcc);

	if (zend_call_function(fci, fcc) == SUCCESS) {
		if (!Z_ISUNDEF_P(fci->retval)) {
			zval_ptr_dtor(fci->retval);
		}

	/* Starting from PHP 8.2.0, zend_call_function returns SUCCESS instead of FAILURE
		when the callable throws an exception.
		See https://github.com/php/php-src/commit/485d3acfe6a40e126783b105fde689728fca262a */
#if PHP_VERSION_ID >= 80200

		if (EG(exception)) {
			/* We probably shouldn't generate the warning if the user called exit() */
			if (!zend_is_unwind_exit(EG(exception))) {
				php_error_docref(NULL, E_WARNING, "Breaking the loop due to exception %s", ZSTR_VAL(EG(exception)->ce->name));
			}
			_php_event_break_loop_or_throw(base, cleanup_cb, cleanup_data);
		}
#endif

		return;
	}

#if PHP_VERSION_ID < 80200
	php_error_docref(NULL, E_WARNING, "Failed to invoke callback, breaking the loop");
#endif

	_php_event_break_loop_or_throw(base, cleanup_cb, cleanup_data);
}/*}}}*/

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */

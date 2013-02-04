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

/* {{{ sockaddr_parse
 * Parse in_addr and fill out_arr with IP and port.
 * out_arr must be a pre-allocated empty zend array */
static int sockaddr_parse(const struct sockaddr *in_addr, zval *out_zarr)
{
	char buf[256];
	int  ret      = FAILURE;

	PHP_EVENT_ASSERT(Z_TYPE_P(out_zarr) == IS_ARRAY);

	switch (in_addr->sa_family) {
		case AF_INET:
			if (evutil_inet_ntop(in_addr->sa_family, &((struct sockaddr_in *) in_addr)->sin_addr,
						(void *) &buf, sizeof(buf))) {
				add_next_index_string(out_zarr, (char *)&buf, 1);
				add_next_index_long(out_zarr,
						ntohs(((struct sockaddr_in *) in_addr)->sin_port));

				ret = SUCCESS;
			}
			break;
#if HAVE_IPV6 && HAVE_INET_NTOP
		case AF_INET6:
			if (evutil_inet_ntop(in_addr->sa_family, &((struct sockaddr_in6 *) in_addr)->sin6_addr,
						(void *) &buf, sizeof(buf))) {
				add_next_index_string(out_zarr, (char *)&buf, 1);
				add_next_index_long(out_zarr,
						ntohs(((struct sockaddr_in6 *) in_addr)->sin6_port));

				ret = SUCCESS;
			}
			break;
#endif
#ifdef AF_UNIX
		case AF_UNIX:
			{
				struct sockaddr_un *ua = (struct sockaddr_un *) in_addr;

				if (ua->sun_path[0] == '\0') {
					/* abstract name */
 					zval *tmp;
					int len = strlen(ua->sun_path + 1) + 1;

    				MAKE_STD_ZVAL(tmp);
    				ZVAL_STRINGL(tmp, ua->sun_path, len, 1);
    				Z_STRVAL_P(tmp)[len] = '\0';

    				zend_hash_next_index_insert(Z_ARRVAL_P(out_zarr), &tmp, sizeof(zval *), NULL);
				} else {
					add_next_index_string(out_zarr, ua->sun_path, 1);
				}
			}
			break;
#endif
	}

	return ret;
}
/* }}} */

/* {{{ _php_event_listener_cb */
static void _php_event_listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {
	php_event_listener_t *l = (php_event_listener_t *) ctx;

	PHP_EVENT_ASSERT(l);

	zend_fcall_info       *pfci = l->fci;
	zend_fcall_info_cache *pfcc = l->fcc;

	PHP_EVENT_ASSERT(pfci && pfcc);

	zval  **args[4];
	zval   *arg_fd;
	zval   *arg_address;
	zval   *arg_data     = l->data;
	zval   *retval_ptr;

	php_stream *stream;

	TSRMLS_FETCH_FROM_CTX(l->thread_ctx);

	/* Call user function having proto:
	 * void cb (EventListener $listener, resource $fd, array $address, mixed $data);
	 * $address = array ("IP-address", *server* port)
	 * Note, address contains the server port(not the one user passed to ex.
	 * evconnlistener_new_bind()!
	 */

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		args[0] = &l->self;
		/*Z_ADDREF_P(l->self);*/

		/* Convert the socket created by libevent to PHP stream
	 	 * and save it's resource ID in l->stream_id */

		if (l->stream_id > 0) {
			MAKE_STD_ZVAL(arg_fd);
			ZVAL_RESOURCE(arg_fd, l->stream_id);
			zend_list_addref(l->stream_id);
		} else {
			stream = php_stream_fopen_from_fd(fd, "r", NULL);
			if (stream) {
				MAKE_STD_ZVAL(arg_fd);
				php_stream_to_zval(stream, arg_fd);

				l->stream_id = Z_LVAL_P(arg_fd);
				zend_list_addref(l->stream_id);
			} else {
				ALLOC_INIT_ZVAL(arg_fd);
				l->stream_id = -1;
			}
		}
		args[1] = &arg_fd;

		MAKE_STD_ZVAL(arg_address);
		array_init(arg_address);
		sockaddr_parse(address, arg_address);
		args[2] = &arg_address;

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[3] = &arg_data;

 		/* Prepare callback */
        pfci->params         = args;
        pfci->retval_ptr_ptr = &retval_ptr;
        pfci->param_count    = 4;
        pfci->no_separation  = 1;

        if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_fd);
        zval_ptr_dtor(&arg_address);
        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* {{{ listener_error_cb */
static void listener_error_cb(struct evconnlistener *listener, void *ctx) {
	php_event_listener_t  *l = (php_event_listener_t *) ctx;

	PHP_EVENT_ASSERT(l);

	zend_fcall_info       *pfci = l->fci_err;
	zend_fcall_info_cache *pfcc = l->fcc_err;

	PHP_EVENT_ASSERT(pfci && pfcc);

	zval  **args[2];
	zval   *arg_data     = l->data;
	zval   *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(l->thread_ctx);

	/* Call user function having proto:
	 * void cb (resource $listener, mixed $data); */

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		args[0] = &l->self;

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[1] = &arg_data;

 		/* Prepare callback */
        pfci->params         = args;
        pfci->retval_ptr_ptr = &retval_ptr;
        pfci->param_count    = 2;
        pfci->no_separation  = 1;

        if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* Private }}} */


/* {{{ proto EventListener EventListener::__construct(EventBase base, callable cb, mixed data, int flags, int backlog, mixed target);
 *
 * Creates new connection listener associated with an event base.
 *
 * target parameter may be string, socket resource, or a stream associated with a socket.
 * In case if target is a string, the string will be parsed as network address.
 *
 * Returns resource representing the event connection listener.
 */
PHP_METHOD(EventListener, __construct)
{
	zval                  *zself    = getThis();
	zval                  *zbase;
	php_event_base_t      *base;
	zend_fcall_info        fci      = empty_fcall_info;
	zend_fcall_info_cache  fcc      = empty_fcall_info_cache;
	php_event_listener_t  *l;
	zval                  *zdata    = NULL;
	zval                 **ppztarget;
	long                   flags;
	long                   backlog;
	struct evconnlistener *listener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ofz!llZ",
				&zbase, php_event_base_ce,
				&fci, &fcc, &zdata, &flags, &backlog, &ppztarget) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (Z_TYPE_PP(ppztarget) == IS_STRING) {
		struct sockaddr sa;
		socklen_t       sa_len = sizeof(struct sockaddr);

		if (php_network_parse_network_address_with_port(Z_STRVAL_PP(ppztarget), Z_STRLEN_PP(ppztarget),
					&sa, &sa_len TSRMLS_CC) != SUCCESS) {
			RETURN_FALSE;
		}

		PHP_EVENT_FETCH_LISTENER(l, zself);

		listener = evconnlistener_new_bind(base->base, _php_event_listener_cb,
				(void *) l, flags, backlog, &sa, sa_len);
		if (!listener) {
			return;
		}
		l->listener = listener;
	} else { /* ppztarget is not string */
		evutil_socket_t   fd    = -1;

		/* php_event_zval_to_fd reports error
	 	 * in case if it is not a valid socket resource */
		fd = php_event_zval_to_fd(ppztarget TSRMLS_CC);
		if (fd < 0) {
			return;
		}

		/* Make sure that the socket is in non-blocking mode(libevent's tip) */
		evutil_make_socket_nonblocking(fd);

		PHP_EVENT_FETCH_LISTENER(l, zself);

		listener = evconnlistener_new(base->base, _php_event_listener_cb,
				(void *) l, flags, backlog, fd);
		if (!listener) {
			return;
		}
		l->listener = listener;

#if 0
		/* WARNING! Don't do this, since libevent calls accept() afterwards,
		 * thus producing new file descriptor. The new descriptor is available
		 * in _php_event_listener_cb() callback. */

		if (Z_TYPE_PP(ppztarget) == IS_RESOURCE) {
			/* lval of ppztarget is the resource ID */
			l->stream_id = Z_LVAL_PP(ppztarget);
			zend_list_addref(Z_LVAL_PP(ppztarget));
		} else {
			l->stream_id = -1;
		}
#endif
	}

	if (zdata) {
		l->data = zdata;
		Z_ADDREF_P(zdata);
	}

	PHP_EVENT_COPY_FCALL_INFO(l->fci, l->fcc, &fci, &fcc);

	l->stream_id = -1;

	l->self = zself;

	TSRMLS_SET_CTX(l->thread_ctx);
}
/* }}} */

/* {{{ proto bool EventListener::enable(void);
 * Enable an event connect listener resource */
PHP_METHOD(EventListener, enable)
{
	zval                 *zlistener = getThis();
	php_event_listener_t *l;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (evconnlistener_enable(l->listener)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventListener::disable(void);
 *
 * Disable an event connect listener resource */
PHP_METHOD(EventListener, disable)
{
	zval                 *zlistener = getThis();
	php_event_listener_t *l;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (evconnlistener_disable(l->listener)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void EventListener::setCallback(callable cb[, mixed arg = NULL]);
 *
 * Adjust event connect listener's callback and optionally the callback argument.
 * Both cb and arg may be NULL.
 */
PHP_METHOD(EventListener, setCallback)
{
	php_event_listener_t  *l;
	zval                  *zlistener = getThis();
	zend_fcall_info        fci       = empty_fcall_info;
	zend_fcall_info_cache  fcc       = empty_fcall_info_cache;
	zval                  *zarg      = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!|z!",
				&fci, &fcc, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (ZEND_FCI_INITIALIZED(fci)) {
		PHP_EVENT_FREE_FCALL_INFO(l->fci, l->fcc);
		PHP_EVENT_COPY_FCALL_INFO(l->fci, l->fcc, &fci, &fcc);
	}

	if (zarg) {
		if (l->data) {
			zval_ptr_dtor(&l->data);
		}

		l->data = zarg;
		Z_ADDREF_P(zarg);
	}

	/*
	 * No sense in the following call, since the callback and the pointer
	 * remain the same
	 * evconnlistener_set_cb(l->listener, _php_event_listener_cb, (void *) l);
	 */
}
/* }}} */

/* {{{ proto void EventListener::setErrorCallback(callable cb);
 * Set event listener's error callback
 */
PHP_METHOD(EventListener, setErrorCallback)
{
	zval                  *zlistener = getThis();
	php_event_listener_t  *l;
	zend_fcall_info        fci;
	zend_fcall_info_cache  fcc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f",
				&fci, &fcc) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (ZEND_FCI_INITIALIZED(fci)) {
		PHP_EVENT_FREE_FCALL_INFO(l->fci_err, l->fcc_err);
		PHP_EVENT_COPY_FCALL_INFO(l->fci_err, l->fcc_err, &fci, &fcc);
	}

	/*
	 * No much sense in the following call, since the callback and the pointer
	 * remain the same. However, we have to set it once at least
	 */
	 evconnlistener_set_error_cb(l->listener, listener_error_cb);
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02000300
/* {{{ proto EventBase EventListener::getBase(void);
 * Get event base associated with the connection listener
 */
PHP_METHOD(EventListener, getBase)
{
	php_event_listener_t *l;
	zval                 *zlistener = getThis();
	php_event_base_t     *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	/* base = evconnlistener_get_base(l->listener); */

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_base_ce);
	PHP_EVENT_FETCH_BASE(b, return_value);
	/* Don't do this. It's normal to have refcount = 1 here.
	 * If we got bugs, we most likely free'd an internal buffer somewhere
	 * Z_ADDREF_P(return_value);*/

	b->base = evconnlistener_get_base(l->listener);
	b->internal = 1;
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
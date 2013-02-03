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

/* {{{ bevent_rw_cb
 * Is called from the bufferevent read and write callbacks */
static zend_always_inline void bevent_rw_cb(struct bufferevent *bevent, php_event_bevent_t *bev, zend_fcall_info *pfci, zend_fcall_info_cache *pfcc)
{
	PHP_EVENT_ASSERT(bev);
	PHP_EVENT_ASSERT(bevent == bev->bevent);
	PHP_EVENT_ASSERT(pfci && pfcc);
	PHP_EVENT_ASSERT(bev->self);

	zval  *arg_data = bev->data;
	zval **args[2];
	zval  *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(bev->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */

		if (bev->self) {
			args[0] = &bev->self;
			/*Z_ADDREF_P(bev->self);*/
		}
		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[1] = &arg_data;

		/* Prepare callback */
		pfci->params		 = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count	 = 2;
		pfci->no_separation  = 1;

        if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS
                && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* {{{ bevent_read_cb */
static void bevent_read_cb(struct bufferevent *bevent, void *ptr)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) ptr;

	bevent_rw_cb(bevent, bev, bev->fci_read, bev->fcc_read);
}
/* }}} */

/* {{{ bevent_write_cb */
static void bevent_write_cb(struct bufferevent *bevent, void *ptr)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) ptr;

	bevent_rw_cb(bevent, bev, bev->fci_write, bev->fcc_write);
}
/* }}} */

/* {{{ bevent_event_cb */
static void bevent_event_cb(struct bufferevent *bevent, short events, void *ptr)
{
	php_event_bevent_t    *bev  = (php_event_bevent_t *) ptr;
	zend_fcall_info       *pfci = bev->fci_event;
	zend_fcall_info_cache *pfcc = bev->fcc_event;

	PHP_EVENT_ASSERT(pfci && pfcc);
	PHP_EVENT_ASSERT(bev->bevent == bevent);
	PHP_EVENT_ASSERT(bev->self);

	zval  *arg_data   = bev->data;
	zval  *arg_events;
	zval **args[3];
	zval  *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(bev->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */
		args[0] = &bev->self;
		/*Z_ADDREF_P(bev->self);*/

		MAKE_STD_ZVAL(arg_events);
		ZVAL_LONG(arg_events, events);
		args[1] = &arg_events;

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[2] = &arg_data;

		/* Prepare callback */
		pfci->params		 = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count	 = 3;
		pfci->no_separation  = 1;

        if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS
                && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_events);
        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* Private }}} */


/* {{{ proto EventBufferEvent EventBufferEvent::__construct(EventBase base[, mixed socket = NULL[, int options = 0]]);
 *
 * Create a socket-based buffer event.
 * options is one of EVENT_BEV_OPT_* constants, or 0.
 * Passing NULL to socket parameter means that the socket stream should be created later,
 * e.g. by means of bufferevent_socket_connect().
 *
 * socket parameter may be created as a stream(not necessarily by means of sockets extension)
 *
 * Returns buffer event resource optionally associated with socket resource. */
PHP_METHOD(EventBufferEvent, __construct)
{
	zval                *zself   = getThis();
	zval                *zbase;
	php_event_base_t    *base;
	zval               **ppzfd   = NULL;
	evutil_socket_t      fd;
	long                 options = 0;
	php_event_bevent_t  *bev;
	struct bufferevent  *bevent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|Z!l",
				&zbase, php_event_base_ce, &ppzfd, &options) == FAILURE) {
		return;
	}

	if (ppzfd) {
#ifdef PHP_EVENT_SOCKETS_SUPPORT 
		/* php_event_zval_to_fd reports error
	 	 * in case if it is not a valid socket resource */
		/*fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);*/
		fd = php_event_zval_to_fd(ppzfd TSRMLS_CC);

		if (fd < 0) {
			return;
		}
		/* Make sure that the socket is in non-blocking mode(libevent's tip) */
		evutil_make_socket_nonblocking(fd);
#else
		fd = -1;
#endif
	} else {
 		/* User decided to assign fd later,
 		 * e.g. by means of bufferevent_socket_connect()
 		 * which allocates new socket stream in this case. */
		fd = -1;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	PHP_EVENT_FETCH_BEVENT(bev, zself);

	bevent = bufferevent_socket_new(base->base, fd, options);
	if (bevent == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate bufferevent for socket");
		return;
	}

	bev->bevent = bevent;

	if (ppzfd) {
		/* lval of ppzfd is the resource ID */
		bev->stream_id = Z_LVAL_PP(ppzfd);
		zend_list_addref(Z_LVAL_PP(ppzfd));
	} else {
		/* Should be assigned in bufferevent_socket_connect() later
		 * (by means of bufferevent_getfd()) */
		bev->stream_id = -1;
	}

	bev->self = zself;
	Z_ADDREF_P(zself);
}
/* }}} */

/* {{{ proto void EventBufferEvent::free(void); */
PHP_METHOD(EventBufferEvent, free)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (bev->bevent) {
		bufferevent_free(bev->bevent);
		bev->bevent = 0;

		/* Do it once */
		Z_DELREF_P(zbevent);
		/*zval_ptr_dtor(&zbevent);*/
	}
}
/* }}} */

/* {{{ proto array EventBufferEvent::createPair(EventBase base[, int options = 0]);
 *
 * options is one of EVENT_BEV_OPT_* constants, or 0.
 *
 * Returns array of two EventBufferEvent objects connected to each other.
 * All the usual options are supported, except for EVENT_BEV_OPT_CLOSE_ON_FREE,
 * which has no effect, and EVENT_BEV_OPT_DEFER_CALLBACKS, which is always on.
 */
PHP_METHOD(EventBufferEvent, createPair)
{
	zval               *zbase;
	php_event_base_t   *base;
	long                options        = 0;
	zval               *zbev[2];
	php_event_bevent_t *b[2];
	struct bufferevent *bevent_pair[2];
	int                 i;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|l",
				&zbase, php_event_base_ce, &options) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (bufferevent_pair_new(base->base, options, bevent_pair)) {
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i = 0; i < 2; i++) {
		MAKE_STD_ZVAL(zbev[i]);
		PHP_EVENT_INIT_CLASS_OBJECT(zbev[i], php_event_bevent_ce);
		PHP_EVENT_FETCH_BEVENT(b[i], zbev[i]);

		b[i]->bevent    = bevent_pair[i];
		b[i]->stream_id = -1;

		add_next_index_zval(return_value, zbev[i]);
	}
}
/* }}} */

/* {{{ proto bool EventBufferEvent::connect(string addr[, bool sync_resolve = FALSE]);
 *
 * Connect buffer event's socket to given address(optionally with port).  The
 * function available since libevent 2.0.2-alpha.
 *
 * This function doesn't require sockets support. If socket is not assigned to
 * the bufferevent, this function allocates a socket stream and makes it
 * non-blocking internally.
 *
 * If sync_resolve parameter is TRUE, the function tries to resolve the
 * hostname within addr *syncronously*(!).  Otherwise addr parameter expected
 * to be an IP address with optional port number. Recognized formats are:
 *
 *    [IPv6Address]:port
 *    [IPv6Address]
 *    IPv6Address
 *    IPv4Address:port
 *    IPv4Address
 * 
 * To resolve DNS names asyncronously, use
 * bufferevent_socket_connect_hostname() function.
 */
PHP_METHOD(EventBufferEvent, connect)
{
	php_event_bevent_t *bev;
	zval               *zbevent      = getThis();
	char               *addr;
	int                 addr_len;
	struct sockaddr     sa;
	socklen_t           sa_len       = sizeof(struct sockaddr);
	zend_bool           sync_resolve = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b",
				&addr, &addr_len, &sync_resolve) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (sync_resolve) {
		/* The PHP API *syncronously* resolves hostname, if it doesn't look
		 * like IP(v4/v6) */
		if (php_network_parse_network_address_with_port(addr, addr_len, &sa, &sa_len TSRMLS_CC) != SUCCESS) {
			/* The function reports errors, if necessary */
			RETURN_FALSE;
		}
	} else {
		/* Numeric addresses only. Don't try to resolve hostname. */
		if (evutil_parse_sockaddr_port(addr, &sa, (int *) &sa_len)) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR,
					"Failed parsing address: the address is not well-formed, "
					"or the port is out of range");
			RETURN_FALSE;
		}
	}

	/* bufferevent_socket_connect() allocates a socket stream internally, if we
	 * didn't provide the file descriptor to the bufferevent before, e.g. with
	 * bufferevent_socket_new() */
	if (bufferevent_socket_connect(bev->bevent, &sa, sa_len)) {
		RETURN_FALSE;
	}

	bev->stream_id = bufferevent_getfd(bev->bevent);

	PHP_EVENT_ASSERT(bev->stream_id >= 0);

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::connectHost(resource dns_base, string hostname, int port[, int family = EVENT_AF_UNSPEC]);
 *
 * Resolves the DNS name hostname, looking for addresses of type
 * family(EVENT_AF_* constants). If the name resolution fails, it invokes the
 * event callback with an error event. If it succeeds, it launches a connection
 * attempt just as bufferevent_socket_connect would.
 *
 * dns_base is optional. May be NULL, or a resource created with
 * event_dns_base_new()(requires --with-event-extra configure option).
 * For asyncronous hostname resolving pass a valid event dns base resource.
 * Otherwise the hostname resolving will block.
 * Recognized hostname formats are:
 * www.example.com (hostname) 1.2.3.4 (ipv4address) ::1 (ipv6address) [::1] ([ipv6address])
 */
PHP_METHOD(EventBufferEvent, connectHost)
{
#if LIBEVENT_VERSION_NUMBER < 0x02000300
	PHP_EVENT_LIBEVENT_VERSION_REQUIRED(bufferevent_socket_connect_hostname, 2.0.3-alpha);
	RETVAL_FALSE;
#else
	php_event_bevent_t *bev;
	zval               *zbevent      = getThis();
	zval               *zdns_base    = NULL;
	char               *hostname;
	int                 hostname_len;
	long                port;
	long                family;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O!sl|l",
				&zdns_base, php_event_dns_base_ce, &hostname, &hostname_len,
				&port, &family) == FAILURE) {
		return;
	}
	
	if (family & ~(AF_INET | AF_INET6 | AF_UNSPEC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid address family specified");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	/* bufferevent_socket_connect() allocates a socket stream internally, if we
	 * didn't provide the file descriptor to the bufferevent before, e.g. with
	 * bufferevent_socket_new() */

#if HAVE_EVENT_EXTRA_LIB
	php_event_dns_base_t *dnsb;

	if (zdns_base) {
		PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);
	}

	if (bufferevent_socket_connect_hostname(bev->bevent,
				(zdns_base ? dnsb->dns_base : NULL),
				family, hostname, port)) {
# ifdef PHP_EVENT_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s",
				evutil_gai_strerror(bufferevent_socket_get_dns_error(bev->bevent)));
# endif
		RETURN_FALSE;
	}
#else /* don't HAVE_EVENT_EXTRA_LIB */
	if (bufferevent_socket_connect_hostname(bev->bevent,
				NULL,
				family, hostname, port)) {
# ifdef PHP_EVENT_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s",
				evutil_gai_strerror(bufferevent_socket_get_dns_error(bev->bevent)));
# endif
		RETURN_FALSE;
	}
#endif

	bev->stream_id = bufferevent_getfd(bev->bevent);

	/*
	It may not work with evdns
	if (bev->stream_id < 0) {
		RETURN_FALSE;
	}
	 PHP_EVENT_ASSERT(bev->stream_id >= 0);
	*/

	RETVAL_TRUE;
#endif
}
/* }}} */

/* {{{ proto string EventBufferEvent::getDnsErrorString(void);
 * Returns string describing the last failed DNS lookup attempt made by
 * bufferevent_socket_connect_hostname(), or an empty string, if no DNS error
 * detected. */
PHP_METHOD(EventBufferEvent, getDnsErrorString)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	int                 err;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	err = bufferevent_socket_get_dns_error(bev->bevent);

	if (err == 0) {
		RETURN_EMPTY_STRING();
	}
	RETVAL_STRING(evutil_gai_strerror(err), 1);
}
/* }}} */

/* {{{ proto void EventBufferEvent::setCallbacks(callable readcb, callable writecb, callable eventcb[, mixed arg = NULL]);
 * Changes one or more of the callbacks of a bufferevent.
 * A callback may be disabled by passing NULL instead of the callable.
 * arg is an argument passed to the callbacks.
 */
PHP_METHOD(EventBufferEvent, setCallbacks)
{
	zval                  *zbevent   = getThis();
	php_event_bevent_t    *bev;
	zend_fcall_info        fci_read;
	zend_fcall_info_cache  fcc_read;
	zend_fcall_info        fci_write;
	zend_fcall_info_cache  fcc_write;
	zend_fcall_info        fci_event;
	zend_fcall_info_cache  fcc_event;
	zval                  *zarg      = NULL;
	bufferevent_data_cb    read_cb;
	bufferevent_data_cb    write_cb;
	bufferevent_event_cb   event_cb;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!f!f!|z!",
				&fci_read, &fcc_read,
				&fci_write, &fcc_write,
				&fci_event, &fcc_event,
				&zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (ZEND_FCI_INITIALIZED(fci_read)) {
		read_cb = bevent_read_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_read, bev->fcc_read);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_read, bev->fcc_read, &fci_read, &fcc_read);
	} else {
		read_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_write)) {
		write_cb = bevent_write_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_write, bev->fcc_write);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_write, bev->fcc_write, &fci_write, &fcc_write);
	} else {
		write_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_event)) {
		event_cb = bevent_event_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_event, bev->fcc_event);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_event, bev->fcc_event, &fci_event, &fcc_event);
	} else {
		event_cb = NULL;
	}

	if (zarg) {
		Z_ADDREF_P(zarg);
		bev->data = zarg;
	}

	TSRMLS_SET_CTX(bev->thread_ctx);

	bufferevent_setcb(bev->bevent, read_cb, write_cb, event_cb, (void *) bev);
}
/* }}} */

/* {{{ proto void EventBufferEvent::enable(int events);
 * Enable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_METHOD(EventBufferEvent, enable)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	long                events;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&events) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	bufferevent_enable(bev->bevent, events);
}
/* }}} */

/* {{{ proto void EventBufferEvent::disable(int events);
 * Disable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_METHOD(EventBufferEvent,disable)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	long                events;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&events) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	bufferevent_disable(bev->bevent, events);
}
/* }}} */

/* {{{ proto int EventBufferEvent::getEnabled(void);
 * Returns bitmask of events currently enabled on the buffer event. */
PHP_METHOD(EventBufferEvent, getEnabled)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	RETVAL_LONG(bufferevent_get_enabled(bev->bevent));
}
/* }}} */

/* {{{ proto EventBuffer EventBufferEvent::getInput(void);
 *
 * Returns an input EventBuffer object associated with the buffer event */
PHP_METHOD(EventBufferEvent, getInput)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	PHP_EVENT_FETCH_BUFFER(b, return_value);
	/* Don't do this. It's normal to have refcount = 1 here.
	 * If we got bugs, we most likely free'd an internal buffer somewhere
	 * Z_ADDREF_P(return_value);*/

	b->buf      = bufferevent_get_input(bev->bevent);
	b->internal = 1;
}
/* }}} */

/* {{{ proto EventBuffer EventBufferEvent::getOutput(void);
 *
 * Returns an output EventBuffer object associated with the buffer event */
PHP_METHOD(EventBufferEvent, getOutput)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	PHP_EVENT_FETCH_BUFFER(b, return_value);
	/* Don't do this. It's normal to have refcount = 1 here.
	 * If we got bugs, we most likely free'd an internal buffer somewhere
	 * Z_ADDREF_P(return_value);*/

	b->buf      = bufferevent_get_output(bev->bevent);
	b->internal = 1;
}
/* }}} */

/* {{{ proto void EventBufferEvent::setWatermark(int events, int lowmark, int highmark);
 * Adjusts the read watermarks, the write watermarks, or both, of a single bufferevent. */
PHP_METHOD(EventBufferEvent, setWatermark)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	long                events;
	long                lowmark;
	long                highmark;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll",
				&events, &lowmark, &highmark) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	bufferevent_setwatermark(bev->bevent, events, (size_t) lowmark, (size_t) highmark);
}
/* }}} */

/* {{{ proto bool EventBufferEvent::write(string data);
 * Adds `data' to a buffe revent's output buffer. */
PHP_METHOD(EventBufferEvent, write)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	zval               *zdata;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
				&zdata) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	convert_to_string(zdata);

	if (bufferevent_write(bev->bevent, Z_STRVAL_P(zdata), Z_STRLEN_P(zdata))) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::writeBuffer(EventBuffer buf);
 * Adds contents of the entire buffer to a buffer event's output buffer. */
PHP_METHOD(EventBufferEvent, writeBuffer)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (bufferevent_write_buffer(bev->bevent, b->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventBufferEvent::read(string &data, int size);
 * Removes up to size bytes from the input buffer, storing them into the memory at data.
 *
 * Returns the number of bytes actually removed.  */
PHP_METHOD(EventBufferEvent, read)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	zval               *zdata;
	long                size;
	char               *data;
	long                ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl",
				&zdata, &size) == FAILURE) {
		return;
	}

	if (!Z_ISREF_P(zdata)) {
		/* Was not passed by reference */
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	data = safe_emalloc(size, sizeof(char), 1);

	ret = bufferevent_read(bev->bevent, data, size);

	if (ret > 0) {
		convert_to_string(zdata);
		zval_dtor(zdata);
		Z_STRVAL_P(zdata) = estrndup(data, ret);
		Z_STRLEN_P(zdata) = ret;
	}

	efree(data);

	RETVAL_LONG(ret);
}
/* }}} */

/* {{{ proto bool EventBufferEvent::readBuffer(EventBuffer buf);
 * Drains the entire contents of the input buffer and places them into buf; it returns 0 on success and -1 on failure. */
PHP_METHOD(EventBufferEvent, readBuffer)
{
	zval               *zbevent = getThis();
	php_event_bevent_t *bev;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (bufferevent_read_buffer(bev->bevent, b->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::setPriority(int priority);
 * Assign a priority to a bufferevent.
 * Only supported for socket bufferevents. */
PHP_METHOD(EventBufferEvent, setPriority)
{
	zval               *zbevent  = getThis();
	php_event_bevent_t *bev;
	long                priority;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&priority) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (bufferevent_priority_set(bev->bevent, priority)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBufferEvent::setTimeouts(double timeout_read, double timeout_write);
 * Set the read and write timeout for a bufferevent. */
PHP_METHOD(EventBufferEvent, setTimeouts)
{
	zval               *zbevent       = getThis();
	php_event_bevent_t *bev;
	double              timeout_read;
	double              timeout_write;
	struct timeval      tv_read;
	struct timeval      tv_write;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll",
				&timeout_read, &timeout_write) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	PHP_EVENT_TIMEVAL_SET(tv_read, timeout_read);
	PHP_EVENT_TIMEVAL_SET(tv_write, timeout_write);

	if (bufferevent_set_timeouts(bev->bevent, &tv_read, &tv_write)) {
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

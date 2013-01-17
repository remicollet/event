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

extern const zend_function_entry event_functions[];

/*
ZEND_DECLARE_MODULE_GLOBALS(event)
*/

/* True global resources - no need for thread safety here */
/* Represents an event returned by event_new */
static int le_event;
static int le_event_base;
/* Represents the config returned by event_config_new */
static int le_event_config;
/* Represents bufferevent returned by ex. bufferevent_socket_new() */
static int le_event_bevent;
/* Represents php_event_buffer_t(struct evbuffer)*/
static int le_event_buffer;

#if HAVE_EVENT_EXTRA_LIB
static int le_event_dns_base;
static int le_event_listener;
#endif

static const zend_module_dep event_deps[] = {
	ZEND_MOD_OPTIONAL("sockets")
	{NULL, NULL, NULL}
};

/* {{{ event_module_entry */
zend_module_entry event_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
	STANDARD_MODULE_HEADER_EX,
	NULL,
	event_deps,
#elif ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"event",
	event_functions,
	PHP_MINIT(event),
	PHP_MSHUTDOWN(event),
	NULL,
	NULL,
	PHP_MINFO(event),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_EVENT_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_EVENT
ZEND_GET_MODULE(event)
#endif


#define PHP_EVENT_FETCH_BASE(base, zbase) \
	ZEND_FETCH_RESOURCE((base), php_event_base_t *, &(zbase), -1, PHP_EVENT_BASE_RES_NAME, le_event_base)

#define PHP_EVENT_FETCH_EVENT(event, zevent) \
	ZEND_FETCH_RESOURCE((event), php_event_t *, &(zevent), -1, PHP_EVENT_RES_NAME, le_event)

#define PHP_EVENT_FETCH_CONFIG(cfg, zcfg) \
	ZEND_FETCH_RESOURCE((cfg), php_event_config_t *, &(zcfg), -1, PHP_EVENT_CONFIG_RES_NAME, le_event_config)

#define PHP_EVENT_FETCH_BEVENT(b, zb) \
	ZEND_FETCH_RESOURCE((b), php_event_bevent_t *, &(zb), -1, PHP_EVENT_BEVENT_RES_NAME, le_event_bevent)

#define PHP_EVENT_FETCH_BUFFER(b, zb) \
	ZEND_FETCH_RESOURCE((b), php_event_buffer_t *, &(zb), -1, PHP_EVENT_BUFFER_RES_NAME, le_event_buffer)

#define PHP_EVENT_FETCH_DNS_BASE(b, zb) \
	ZEND_FETCH_RESOURCE((b), php_event_dns_base_t *, &(zb), -1, PHP_EVENT_DNS_BASE_RES_NAME, le_event_dns_base)

#define PHP_EVENT_FETCH_LISTENER(b, zb) \
	ZEND_FETCH_RESOURCE((b), php_event_listener_t *, &(zb), -1, PHP_EVENT_LISTENER_RES_NAME, le_event_listener)

#define PHP_EVENT_TIMEVAL_SET(tv, t)                     \
        do {                                             \
            tv.tv_sec  = (long) t;                       \
            tv.tv_usec = (long) ((t - tv.tv_sec) * 1e6); \
        } while (0)

#define PHP_EVENT_TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)

#define PHP_EVENT_SOCKETS_REQUIRED_NORET                                       \
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "`sockets' extension required. " \
            "If you have `sockets' installed, rebuild `event' extension")

#define PHP_EVENT_SOCKETS_REQUIRED_RET                                         \
    PHP_EVENT_SOCKETS_REQUIRED_NORET;                                          \
    RETURN_FALSE



/* {{{ Private functions */

/* {{{ fatal_error_cb
 * Is called when Libevent detects a non-recoverable internal error. */
static void fatal_error_cb(int err)
{
	TSRMLS_FETCH();

	php_error_docref(NULL TSRMLS_CC, E_ERROR,
			"libevent detected a non-recoverable internal error, code: %d", err);
}
/* }}} */


#if LIBEVENT_VERSION_NUMBER < 0x02001900
# define PHP_EVENT_LOG_CONST(name) _ ## name
#else
# define PHP_EVENT_LOG_CONST(name) name
#endif

/* {{{ log_cb
 * Overrides libevent's default error logging(it logs to stderr) */
static void log_cb(int severity, const char *msg)
{
	/* TSRMLS_FETCH consumes a fair amount of resources.  But a ready-to-use
	 * program shouldn't get any error logs. Nevertheless, we have no other way
	 * to fetch TSRMLS. */
	TSRMLS_FETCH();

	int error_type;

	switch (severity) {
		case PHP_EVENT_LOG_CONST(EVENT_LOG_DEBUG):
			error_type = E_STRICT;
		case PHP_EVENT_LOG_CONST(EVENT_LOG_MSG):
			error_type = E_NOTICE;
		case PHP_EVENT_LOG_CONST(EVENT_LOG_WARN):
			error_type = E_WARNING;
		case PHP_EVENT_LOG_CONST(EVENT_LOG_ERR):
			error_type = E_ERROR;
		default:
			error_type = E_NOTICE;
	}

	php_error_docref(NULL TSRMLS_CC, error_type, "%s", msg);
}
/* }}} */

/* {{{ _php_event_sockaddr_parse
 * Parse in_addr and fill out_arr with IP and port.
 * out_arr must be a pre-allocated empty zend array */
static int _php_event_sockaddr_parse(const struct sockaddr *in_addr, zval *out_zarr)
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

/* {{{ zval_to_fd
 * Get numeric file descriptor from PHP stream or Socket resource */
static php_socket_t zval_to_fd(zval **ppfd TSRMLS_DC)
{
	php_socket_t  file_desc = -1;
	php_stream   *stream;
#ifdef PHP_EVENT_SOCKETS_SUPPORT 
	php_socket   *php_sock;
#endif

	if (Z_TYPE_PP(ppfd) == IS_RESOURCE) {
		/* PHP stream or PHP socket resource  */
		if (ZEND_FETCH_RESOURCE_NO_RETURN(stream, php_stream *, ppfd, -1, NULL, php_file_le_stream())) {
			php_stream_from_zval_no_verify(stream, ppfd);

			if (stream == NULL) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed obtaining fd");
				return -1;
			}

			/* PHP stream */
			if (php_stream_can_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else if (php_stream_can_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_FD,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else if (php_stream_can_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_INTERNAL,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else { /* STDIN, STDOUT, STDERR etc. */
				file_desc = Z_LVAL_P(*ppfd);
			}
		} else {
			/* PHP socket resource */
#ifdef PHP_EVENT_SOCKETS_SUPPORT
			if (ZEND_FETCH_RESOURCE_NO_RETURN(php_sock, php_socket *,ppfd, -1, NULL, php_sockets_le_socket())) {
				if (php_sock->error) {
					if (!php_sock->blocking && php_sock->error == EINPROGRESS) {
#ifdef PHP_EVENT_DEBUG
						php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Operation in progress");
#endif
					} else
						return -1;
				}

				return php_sock->bsd_socket;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"either valid PHP stream or valid PHP socket resource expected");
			}
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"valid PHP stream resource expected");
#endif
			return -1;
		}
	} else if (Z_TYPE_PP(ppfd) == IS_LONG) {
		/* Numeric fd */
		file_desc = Z_LVAL_PP(ppfd);
		if (file_desc < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid file descriptor passed");
			return -1;
		}
	} else {
		/* Invalid fd */
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid file descriptor passed");
		return -1;
	}

	return file_desc;
}
/* }}} */

#ifdef PHP_EVENT_SOCKETS_SUPPORT 
/* {{{ sockets_zval_to_fd 
 * Get numeric file descriptor from PHP stream or Socket resource */
static php_socket_t sockets_zval_to_fd(zval **ppfd TSRMLS_DC)
{
	php_socket *php_sock;

	if (ZEND_FETCH_RESOURCE_NO_RETURN(php_sock, php_socket *, ppfd, -1, NULL, php_sockets_le_socket())) {
		if (php_sock->error) {
			if (!php_sock->blocking && php_sock->error == EINPROGRESS) {
				/* Skip, no messages, since we should provide non-blocking
				 * sockets to bufferevent functions */
			} else {
				return -1;
			}
		}

		return php_sock->bsd_socket;
	} 

	php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"valid PHP socket resource expected");
	return -1;
}
/* }}} */
#endif

/* {{{ zval_to_signum */
static zend_always_inline evutil_socket_t zval_to_signum(zval **ppzfd)
{
	evutil_socket_t fd;

	convert_to_long_ex(ppzfd);

	fd = Z_LVAL_PP(ppzfd);

	if (fd < 0 || fd >= NSIG) {
		return -1;
	}

	return fd;
}
/* }}} */

/* {{{ is_pending 
Don't allow for pending or active event
See http://www.wangafu.net/~nickm/libevent-book/Ref4_event.html */
static zend_always_inline zend_bool is_pending(const struct event *e)
{
	return event_pending(e, EV_READ | EV_WRITE | EV_SIGNAL | EV_TIMEOUT, NULL);
}
/* }}} */

/* {{{ timer_is_pending 
Whether timer event is pending */
static zend_always_inline zend_bool timer_is_pending(const struct event *e)
{
	return evtimer_pending(e, NULL);
}
/* }}} */

/* {{{ event_cb */
static void event_cb(evutil_socket_t fd, short what, void *arg)
{
	php_event_t *e = (php_event_t *) arg;

	PHP_EVENT_ASSERT(e);
	PHP_EVENT_ASSERT(e->fci && e->fcc);

	zend_fcall_info     *pfci       = e->fci;
	zval                *arg_data   = e->data;
	zval                *arg_fd;
	zval                *arg_what;
	zval               **args[3];
	zval                *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(e->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback arguments */
		MAKE_STD_ZVAL(arg_fd);
		if (what & EV_SIGNAL) {
			ZVAL_LONG(arg_fd, fd);
		} else if (e->stream_id >= 0) {
			ZVAL_RESOURCE(arg_fd, e->stream_id);
			zend_list_addref(e->stream_id);
		} else {
			ZVAL_NULL(arg_fd);
		}
		args[0] = &arg_fd;

		MAKE_STD_ZVAL(arg_what);
		args[1] = &arg_what;
		ZVAL_LONG(arg_what, what);

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[2] = &arg_data;

 		/* Prepare callback */
        pfci->params         = args;
        pfci->retval_ptr_ptr = &retval_ptr;
        pfci->param_count    = 3;
        pfci->no_separation  = 1;

        if (zend_call_function(pfci, e->fcc TSRMLS_CC) == SUCCESS
                && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_fd);
        zval_ptr_dtor(&arg_what);
        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* {{{ timer_cb */
static void timer_cb(evutil_socket_t fd, short what, void *arg)
{
	php_event_t *e = (php_event_t *) arg;

	PHP_EVENT_ASSERT(e);
	PHP_EVENT_ASSERT(what & EV_TIMEOUT);
	PHP_EVENT_ASSERT(e->fci && e->fcc);

	zend_fcall_info     *pfci       = e->fci;
	zval                *arg_data   = e->data;
	zval               **args[1];
	zval                *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(e->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback arg */
		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[0] = &arg_data;

		/* Prepare callback */
		pfci->params		 = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count	 = 1;
		pfci->no_separation  = 1;

        if (zend_call_function(pfci, e->fcc TSRMLS_CC) == SUCCESS
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

/* {{{ signal_cb */
static void signal_cb(evutil_socket_t signum, short what, void *arg)
{
	php_event_t *e = (php_event_t *) arg;

	PHP_EVENT_ASSERT(e);
	PHP_EVENT_ASSERT(what & EV_SIGNAL);
	PHP_EVENT_ASSERT(e->fci && e->fcc);

	zend_fcall_info     *pfci       = e->fci;
	zval                *arg_data   = e->data;
	zval                *arg_signum;
	zval               **args[2];
	zval                *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(e->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */
		MAKE_STD_ZVAL(arg_signum);
		ZVAL_LONG(arg_signum, signum);
		args[0] = &arg_signum;

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

        if (zend_call_function(pfci, e->fcc TSRMLS_CC) == SUCCESS
                && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_data);
        zval_ptr_dtor(&arg_signum);
	}
}
/* }}} */

/* {{{ bevent_rw_cb
 * Is called from the bufferevent read and write callbacks */
static zend_always_inline void bevent_rw_cb(struct bufferevent *bevent, php_event_bevent_t *bev, zend_fcall_info *pfci, zend_fcall_info_cache *pfcc)
{
	PHP_EVENT_ASSERT(bev);
	PHP_EVENT_ASSERT(pfci && pfcc);

	zval  *arg_data   = bev->data;
	zval  *arg_bevent;
	zval **args[2];
	zval  *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(bev->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */
		MAKE_STD_ZVAL(arg_bevent);

		PHP_EVENT_ASSERT(bev->rsrc_id > 0);

		if (bev->rsrc_id > 0) {
			ZVAL_RESOURCE(arg_bevent, bev->rsrc_id);
			zend_list_addref(bev->rsrc_id);
		} else {
			ZVAL_NULL(arg_bevent);
		}
		args[0] = &arg_bevent;

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

        zval_ptr_dtor(&arg_bevent);
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

	zval  *arg_data   = bev->data;
	zval  *arg_bevent;
	zval  *arg_events;
	zval **args[3];
	zval  *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(bev->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */
		MAKE_STD_ZVAL(arg_bevent);

		PHP_EVENT_ASSERT(bev->rsrc_id > 0);

		if (bev->rsrc_id >= 0) {
			ZVAL_RESOURCE(arg_bevent, bev->rsrc_id);
			zend_list_addref(bev->rsrc_id);
		} else {
			ZVAL_NULL(arg_bevent);
		}
		args[0] = &arg_bevent;

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

        zval_ptr_dtor(&arg_bevent);
        zval_ptr_dtor(&arg_events);
        zval_ptr_dtor(&arg_data);
	}

}
/* }}} */

/* {{{ _php_event_listener_cb */
static void _php_event_listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {

	php_event_listener_t  *l = (php_event_listener_t *) ctx;

	PHP_EVENT_ASSERT(l);

	zend_fcall_info       *pfci = l->fci;
	zend_fcall_info_cache *pfcc = l->fcc;

	PHP_EVENT_ASSERT(pfci && pfcc);

	zval  **args[4];
	zval   *arg_listener;
	zval   *arg_fd;
	zval   *arg_address;
	zval   *arg_data     = l->data;
	zval   *retval_ptr;

	php_stream *stream;

	TSRMLS_FETCH_FROM_CTX(l->thread_ctx);

	/* Call user function having proto:
	 * void cb (resource $listener, resource $fd, array $address, mixed $data);
	 * $address = array ("IP-address", *server* port)
	 * Note, address contains the server port(not the one user passed to ex.
	 * evconnlistener_new_bind()!
	 */

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		MAKE_STD_ZVAL(arg_listener);
		ZVAL_RESOURCE(arg_listener, l->rsrc_id);
		zend_list_addref(l->rsrc_id);
		args[0] = &arg_listener;

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
				l->stream_id = -1;
				ALLOC_INIT_ZVAL(arg_fd);
			}
		}
		args[1] = &arg_fd;

		MAKE_STD_ZVAL(arg_address);
		array_init(arg_address);
		_php_event_sockaddr_parse(address, arg_address);
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

        zval_ptr_dtor(&arg_listener);
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
	zval   *arg_listener;
	zval   *arg_data     = l->data;
	zval   *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(l->thread_ctx);

	/* Call user function having proto:
	 * void cb (resource $listener, mixed $data); */

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		MAKE_STD_ZVAL(arg_listener);
		ZVAL_RESOURCE(arg_listener, l->rsrc_id);
		zend_list_addref(l->rsrc_id);
		args[0] = &arg_listener;

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

        zval_ptr_dtor(&arg_listener);
        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* {{{ php_event_dtor */
static void php_event_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) rsrc->ptr;

	if (e) {
		if (e->data) {
			zval_ptr_dtor(&e->data);
		}

		PHP_EVENT_FREE_FCALL_INFO(e->fci, e->fcc);

		if (e->stream_id >= 0) { /* stdin fd == 0 */
			zend_list_delete(e->stream_id);
		}
		event_del(e->event);
		event_free(e->event);
		efree(e);
	}
}
/* }}} */

/* {{{ php_event_base_dtor */
static void php_event_base_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	/* TODO: what if events bound to the event_base are not destroyed? */
	php_event_base_t *b= (php_event_base_t *) rsrc->ptr;

	if (b) {
		event_base_free(b->base);
		efree(b);
	}
}
/* }}} */

/* {{{ php_event_config_dtor */
static void php_event_config_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	struct event_config *cfg = (struct event_config *) rsrc->ptr;

	event_config_free(cfg);
}
/* }}} */

/* {{{ php_event_bevent_dtor */
static void php_event_bevent_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_bevent_t *b = (php_event_bevent_t *) rsrc->ptr;

	if (b->data) {
		zval_ptr_dtor(&b->data);
	}

	PHP_EVENT_FREE_FCALL_INFO(b->fci_read,  b->fcc_read);
	PHP_EVENT_FREE_FCALL_INFO(b->fci_write, b->fcc_write);
	PHP_EVENT_FREE_FCALL_INFO(b->fci_event, b->fcc_event);

	if (b->stream_id >= 0) { /* stdin fd == 0 */
		zend_list_delete(b->stream_id);
	}

	bufferevent_free(b->bevent);

	efree(b);
}
/* }}} */

/* {{{ php_event_buffer_dtor*/
static void php_event_buffer_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) rsrc->ptr;

	if (b) {
		evbuffer_free(b->buf);
		efree(b);
	}
}
/* }}} */

/* {{{ php_event_dns_base_dtor */
static void php_event_dns_base_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_dns_base_t *dnsb = (php_event_dns_base_t *) rsrc->ptr;

	if (dnsb) {
		/* Setting fail_requests to 1 makes all in-flight requests get
	 	 * their callbacks invoked with a canceled error code before it
	 	 * frees the base*/
		evdns_base_free(dnsb->dns_base, 1);

		efree(dnsb);
	}
}
/* }}} */

/* {{{ php_event_listener_dtor */
static void php_event_listener_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_listener_t *l = (php_event_listener_t *) rsrc->ptr;

	if (l) {
		if (l->stream_id >= 0) {
			zend_list_delete(l->stream_id);
		}

		if (l->base_id) {
			zend_list_delete(l->base_id);
		}

		if (l->data) {
			zval_ptr_dtor(&l->data);
		}

		PHP_EVENT_FREE_FCALL_INFO(l->fci, l->fcc);
		PHP_EVENT_FREE_FCALL_INFO(l->fci_err, l->fcc_err);

		evconnlistener_free(l->listener);

		efree(l);
	}
}
/* }}} */

/* Private functions }}} */



#define PHP_EVENT_REG_CONST_LONG(name, real_name) \
    REGISTER_LONG_CONSTANT(#name, real_name, CONST_CS | CONST_PERSISTENT);

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(event)
{
	le_event          = zend_register_list_destructors_ex(php_event_dtor,          NULL, PHP_EVENT_RES_NAME,          module_number);
	le_event_base     = zend_register_list_destructors_ex(php_event_base_dtor,     NULL, PHP_EVENT_BASE_RES_NAME,     module_number);
	le_event_config   = zend_register_list_destructors_ex(php_event_config_dtor,   NULL, PHP_EVENT_CONFIG_RES_NAME,   module_number);
	le_event_bevent   = zend_register_list_destructors_ex(php_event_bevent_dtor,   NULL, PHP_EVENT_BEVENT_RES_NAME,   module_number);
	le_event_buffer   = zend_register_list_destructors_ex(php_event_buffer_dtor,   NULL, PHP_EVENT_BUFFER_RES_NAME,   module_number);
	le_event_dns_base = zend_register_list_destructors_ex(php_event_dns_base_dtor, NULL, PHP_EVENT_DNS_BASE_RES_NAME, module_number);
	le_event_listener = zend_register_list_destructors_ex(php_event_listener_dtor, NULL, PHP_EVENT_LISTENER_RES_NAME, module_number);

	/* Loop flags */
	PHP_EVENT_REG_CONST_LONG(EVENT_LOOP_ONCE,     EVLOOP_ONCE);
	PHP_EVENT_REG_CONST_LONG(EVENT_LOOP_NONBLOCK, EVLOOP_NONBLOCK);

	/* Event flags */
	PHP_EVENT_REG_CONST_LONG(EVENT_ET,      EV_ET);
	PHP_EVENT_REG_CONST_LONG(EVENT_PERSIST, EV_PERSIST);
	PHP_EVENT_REG_CONST_LONG(EVENT_READ,    EV_READ);
	PHP_EVENT_REG_CONST_LONG(EVENT_WRITE,   EV_WRITE);
	PHP_EVENT_REG_CONST_LONG(EVENT_SIGNAL,  EV_SIGNAL);
	PHP_EVENT_REG_CONST_LONG(EVENT_TIMEOUT, EV_TIMEOUT);

	/* Features of event_base usually passed to event_config_require_features */
	PHP_EVENT_REG_CONST_LONG(EVENT_FEATURE_ET,  EV_FEATURE_ET);
	PHP_EVENT_REG_CONST_LONG(EVENT_FEATURE_O1,  EV_FEATURE_O1);
	PHP_EVENT_REG_CONST_LONG(EVENT_FEATURE_FDS, EV_FEATURE_FDS);

	/* Run-time flags of event base usually passed to event_config_set_flag */
	PHP_EVENT_REG_CONST_LONG(EVENT_BASE_FLAG_NOLOCK,               EVENT_BASE_FLAG_NOLOCK);
	PHP_EVENT_REG_CONST_LONG(EVENT_BASE_FLAG_STARTUP_IOCP,         EVENT_BASE_FLAG_STARTUP_IOCP);
	PHP_EVENT_REG_CONST_LONG(EVENT_BASE_FLAG_NO_CACHE_TIME,        EVENT_BASE_FLAG_NO_CACHE_TIME);
	PHP_EVENT_REG_CONST_LONG(EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);
#ifdef EVENT_BASE_FLAG_IGNORE_ENV
	PHP_EVENT_REG_CONST_LONG(EVENT_BASE_FLAG_IGNORE_ENV,           EVENT_BASE_FLAG_IGNORE_ENV);
#endif
#ifdef EVENT_BASE_FLAG_PRECISE_TIMER
	PHP_EVENT_REG_CONST_LONG(EVENT_BASE_FLAG_PRECISE_TIMER,        EVENT_BASE_FLAG_PRECISE_TIMER);
#endif

	/* Buffer event flags */
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_EVENT_READING,   BEV_EVENT_READING);
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_EVENT_WRITING,   BEV_EVENT_WRITING);
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_EVENT_EOF,       BEV_EVENT_EOF);
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_EVENT_ERROR,     BEV_EVENT_ERROR);
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_EVENT_TIMEOUT,   BEV_EVENT_TIMEOUT);
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_EVENT_CONNECTED, BEV_EVENT_CONNECTED);

	/* Option flags for bufferevents */
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_OPT_CLOSE_ON_FREE,    BEV_OPT_CLOSE_ON_FREE);
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_OPT_THREADSAFE,       BEV_OPT_THREADSAFE);
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_OPT_DEFER_CALLBACKS,  BEV_OPT_DEFER_CALLBACKS);
#if LIBEVENT_VERSION_NUMBER >= 0x02000500
	PHP_EVENT_REG_CONST_LONG(EVENT_BEV_OPT_UNLOCK_CALLBACKS, BEV_OPT_UNLOCK_CALLBACKS);
#endif

	/* Address families */
	PHP_EVENT_REG_CONST_LONG(EVENT_AF_INET,   AF_INET);
	PHP_EVENT_REG_CONST_LONG(EVENT_AF_INET6,  AF_INET6);
	PHP_EVENT_REG_CONST_LONG(EVENT_AF_UNSPEC, AF_UNSPEC);

	/* DNS options */
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTION_SEARCH,      DNS_OPTION_SEARCH);
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTION_NAMESERVERS, DNS_OPTION_NAMESERVERS);
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTION_MISC,        DNS_OPTION_MISC);
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTION_HOSTSFILE,   DNS_OPTION_HOSTSFILE);
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTIONS_ALL,        DNS_OPTIONS_ALL);

#if HAVE_EVENT_EXTRA_LIB
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_LEAVE_SOCKETS_BLOCKING, LEV_OPT_LEAVE_SOCKETS_BLOCKING);
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_CLOSE_ON_FREE,          LEV_OPT_CLOSE_ON_FREE);
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_CLOSE_ON_EXEC,          LEV_OPT_CLOSE_ON_EXEC);
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_REUSEABLE,              LEV_OPT_REUSEABLE);
# if LIBEVENT_VERSION_NUMBER >= 0x02010100
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_DISABLED,               LEV_OPT_DISABLED);
# endif
# if LIBEVENT_VERSION_NUMBER >= 0x02000800
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_THREADSAFE,             LEV_OPT_THREADSAFE);
#endif
# if LIBEVENT_VERSION_NUMBER >= 0x02010100
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_DEFERRED_ACCEPT,        LEV_OPT_DEFERRED_ACCEPT);
# endif
#endif

	/* Handle libevent's error logging more gracefully than it's default
	 * logging to stderr, or calling abort()/exit() */
	event_set_fatal_callback(fatal_error_cb);
	event_set_log_callback(log_cb);
#ifdef PHP_EVENT_DEBUG
	event_enable_debug_mode();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(event)
{
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
	/* libevent_global_shutdown is available since libevent 2.1.0-alpha.
	 *
	 * Make sure that libevent has released all internal library-global data
	 * structures. Don't call any of libevent functions below! */
	libevent_global_shutdown();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(event)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "event support", "enabled");
#ifdef PHP_EVENT_DEBUG 
	php_info_print_table_row(2, "Debug support", "enabled");
#else
	php_info_print_table_row(2, "Debug support", "disabled");
#endif
	php_info_print_table_row(2, "Version", PHP_EVENT_VERSION);
	php_info_print_table_end();
}
/* }}} */

/* {{{ API functions */


/* {{{ proto resource evtimer_new(resource base, callable cb[, zval arg = NULL]);
 * Creates new event */
PHP_FUNCTION(evtimer_new)
{
	zval                  *zbase;
	php_event_base_t      *b;
	zend_fcall_info        fci   = empty_fcall_info;
	zend_fcall_info_cache  fcc   = empty_fcall_info_cache;
	zval                  *arg   = NULL;
	php_event_t           *e;
	struct event          *event;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rf|z",
				&zbase, &fci, &fcc, &arg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	e = emalloc(sizeof(php_event_t));
	memset(e, 0, sizeof(php_event_t));

	event = evtimer_new(b->base, timer_cb, (void *) e);
	if (!event) {
		efree(e);
		RETURN_FALSE;
	}

	e->event = event;

	if (arg) {
		Z_ADDREF_P(arg);
	}
	e->data = arg;

	PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);

	TSRMLS_SET_CTX(e->thread_ctx);

	e->stream_id = -1; /* stdin fd = 0 */

	ZEND_REGISTER_RESOURCE(return_value, e, le_event);
}
/* }}} */

/* {{{ proto bool evtimer_set(resource event, resource base, callable cb[, zval arg = NULL]);
 * Re-configures timer event.
 * Note, this function doesn't invoke obsolete libevent's event_set. It calls event_assign instead. */
PHP_FUNCTION(evtimer_set)
{
	zval                  *zbase;
	php_event_base_t      *b;
	zval                  *zevent;
	php_event_t           *e;
	zend_fcall_info        fci    = empty_fcall_info;
	zend_fcall_info_cache  fcc    = empty_fcall_info_cache;
	zval                  *arg    = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrf|z!",
				&zevent, &zbase, &fci, &fcc, &arg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (timer_is_pending(e->event)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't modify pending timer");
		RETURN_FALSE;
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (ZEND_FCI_INITIALIZED(fci)) {
		if (e->fci && ZEND_FCI_INITIALIZED(*e->fci)) {
			PHP_EVENT_FREE_FCALL_INFO(e->fci, e->fcc);
		}

		PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);
	}

	if (arg) {
		if (e->data) {
			zval_ptr_dtor(&e->data);
		}
		e->data = arg;
		Z_ADDREF_P(arg);
	}

	e->stream_id = -1; /* stdin fd = 0 */

    if (evtimer_assign(e->event, b->base, timer_cb, (void *) e)) {
    	RETURN_FALSE;
    }
    RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evtimer_pending(resource event);
 * Detect whether timer event is pending or scheduled. */
PHP_FUNCTION(evtimer_pending)
{
	zval        *zevent;
	php_event_t *e;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (timer_is_pending(e->event)) {
		RETURN_TRUE;
	}
	RETVAL_FALSE;
	
}
/* }}} */



/* {{{ proto resource evsignal_new(resource base, int signum, callable cb[, zval arg = NULL]);
 * Creates new signal event */
PHP_FUNCTION(evsignal_new)
{
	zval                  *zbase;
	php_event_base_t      *b;
	long                   signum;
	zend_fcall_info        fci    = empty_fcall_info;
	zend_fcall_info_cache  fcc    = empty_fcall_info_cache;
	zval                  *arg    = NULL;
	php_event_t           *e;
	struct event          *event;
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlf|z",
				&zbase, &signum, &fci, &fcc, &arg) == FAILURE) {
		return;
	}
	
	if (signum < 0 || signum >= NSIG) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid signal passed");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	e = emalloc(sizeof(php_event_t));
	memset(e, 0, sizeof(php_event_t));

	event = evsignal_new(b->base, signum, signal_cb, (void *) e);
	if (!event) {
		efree(e);
		RETURN_FALSE;
	}

	e->event = event;

	if (arg) {
		Z_ADDREF_P(arg);
	}
	e->data = arg;

	PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);

	TSRMLS_SET_CTX(e->thread_ctx);

	e->stream_id = -1; /* stdin fd = 0 */

	ZEND_REGISTER_RESOURCE(return_value, e, le_event);
}
/* }}} */


/* {{{ proto resource event_new(resource base, mixed fd, int what, callable cb[, zval arg = NULL]);
 * Creates new event */
PHP_FUNCTION(event_new)
{
	zval                   *zbase;
	php_event_base_t       *b;
	zval                  **ppzfd;
	evutil_socket_t         fd;
	long                    what;
	zend_fcall_info         fci     = empty_fcall_info;
	zend_fcall_info_cache   fcc     = empty_fcall_info_cache;
	zval                   *arg     = NULL;
	php_event_t            *e;
	struct event           *event;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZlf|z",
				&zbase, &ppzfd, &what, &fci, &fcc, &arg) == FAILURE) {
		return;
	}

	if (what & ~(EV_TIMEOUT | EV_READ | EV_WRITE | EV_SIGNAL | EV_PERSIST | EV_ET)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid mask");
		RETURN_FALSE;
	}

	if (what & EV_SIGNAL) {
		fd = zval_to_signum(ppzfd);
		if (fd == -1) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid signal passed");
			RETURN_FALSE;
		}
	} else {
		fd = (evutil_socket_t) zval_to_fd(ppzfd TSRMLS_CC);
		if (fd < 0) {
			RETURN_FALSE;
		}
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	/* TODO: check if a signum bound to different event bases */

	e = emalloc(sizeof(php_event_t));
	memset(e, 0, sizeof(php_event_t));

	event = event_new(b->base, fd, what, event_cb, (void *) e);
	if (!event) {
		efree(e);
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "event_new failed");
		RETURN_FALSE;
	}

	e->event = event;

	if (arg) {
		Z_ADDREF_P(arg);
	}
	e->data = arg;

	PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);

	TSRMLS_SET_CTX(e->thread_ctx);

	if (what & EV_SIGNAL) {
		e->stream_id = -1; /* stdin fd = 0 */
	} else {
		/* lval of ppzfd is the resource ID */
		e->stream_id = Z_LVAL_PP(ppzfd);
		zend_list_addref(Z_LVAL_PP(ppzfd));
	}

	e->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, e, le_event);
}
/* }}} */

/* {{{ proto void event_free(resource event);
 * Free an event resource */
PHP_FUNCTION(event_free)
{
	php_event_t *ev;
	zval        *zevent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(ev, zevent);

	zend_list_delete(ev->rsrc_id);
}
/* }}} */

/* {{{ proto bool event_set(resource event, resource base, mixed fd,[ int what = NULL[, callable cb = NULL[, zval arg = NULL]]]);
 * Re-configures event.
 * Note, this function doesn't invoke obsolete libevent's event_set. It calls event_assign instead.  */
PHP_FUNCTION(event_set)
{
	zval                   *zbase;
	php_event_base_t       *b;
	zval                   *zevent;
	php_event_t            *e;
	zval                  **ppzfd   = NULL;
	evutil_socket_t         fd;
	long                    what    = -1;
	zend_fcall_info         fci     = empty_fcall_info;
	zend_fcall_info_cache   fcc     = empty_fcall_info_cache;
	zval                   *arg     = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrZ!|lfz!",
				&zevent, &zbase, &ppzfd, &what, &fci, &fcc, &arg) == FAILURE) {
		return;
	}

	if (what != -1) {
		if (what & ~(EV_TIMEOUT | EV_READ | EV_WRITE | EV_SIGNAL | EV_PERSIST | EV_ET)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid events mask");
			RETURN_FALSE;
		}

		if (what & EV_SIGNAL) {
			if (zval_to_signum(ppzfd) == -1) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid signal passed");
				RETURN_FALSE;
			}
		} else {
			fd = (evutil_socket_t) zval_to_fd(ppzfd TSRMLS_CC);
			if (fd < 0) {
				RETURN_FALSE;
			}
		}
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (is_pending(e->event)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't modify pending event");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	/* TODO: check if a signum bound to different event bases */

	if (ppzfd) {
		if (what != -1 && what & EV_SIGNAL) {
			e->stream_id = -1; /* stdin fd = 0 */
		} else {
			if (e->stream_id != Z_LVAL_PP(ppzfd)) {
				zend_list_delete(e->stream_id);
				/* lval of ppzfd is the resource ID */
				e->stream_id = Z_LVAL_PP(ppzfd);
				zend_list_addref(Z_LVAL_PP(ppzfd));
			}
		}
	}

	if (ZEND_FCI_INITIALIZED(fci)) {
		PHP_EVENT_FREE_FCALL_INFO(e->fci, e->fcc);
		PHP_EVENT_COPY_FCALL_INFO(e->fci, e->fcc, &fci, &fcc);
	}

	if (arg) {
		if (e->data) {
			zval_ptr_dtor(&e->data);
		}
		e->data = arg;
		Z_ADDREF_P(arg);
	}

	event_get_assignment(e->event, &b->base,
			(ppzfd ? NULL : &fd),
			(short *) (what == -1 ? &what : NULL),
			NULL /* ignore old callback */ ,
			NULL /* ignore old callback argument */);

	if (event_assign(e->event, b->base, fd, what, event_cb, (void *) e)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto array event_get_supported_methods(void);
 * Returns array with of the names of the methods supported in this version of Libevent */
PHP_FUNCTION(event_get_supported_methods)
{
	int i;
	const char **methods;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	methods = event_get_supported_methods();

	if (methods == NULL) {
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i = 0; methods[i] != NULL; ++i) {
		add_next_index_string(return_value, methods[i], 1);
	}
}

/* }}} */

/* {{{ proto bool event_add(resource event[, double timeout]);
 * Make event pending. */
PHP_FUNCTION(event_add)
{
	zval        *zevent;
	php_event_t *e;
	double       timeout = -1;
	int          res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|d",
				&zevent, &timeout) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (timeout == -1) {
		res = event_add(e->event, NULL);
	} else {
		struct timeval tv;
		PHP_EVENT_TIMEVAL_SET(tv, timeout);

		res = event_add(e->event, &tv);
	}

	if (res) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed adding event");
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_del(resource event);
 * Remove an event from the set of monitored events. */
PHP_FUNCTION(event_del)
{
	zval        *zevent;
	php_event_t *e;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (event_del(e->event)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed deletting event");
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010200
/* {{{ proto bool event_remove_timer(resource event);
 * Remove a pending event’s timeout completely without deleting its IO or signal components.
 * Available since libevent 2.1.2-alpha. */
PHP_FUNCTION(event_remove_timer)
{
	zval        *zevent;
	php_event_t *e;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (event_remove_timer(e->event)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed deletting event");
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */
#endif

/* {{{ proto bool event_priority_set(resource event, int priority);
 * Make event pending. */
PHP_FUNCTION(event_priority_set)
{
	zval        *zevent;
	php_event_t *e;
	long         priority;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zevent, &priority) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (event_priority_set(e->event, priority)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to set event priority: %ld", priority);
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_pending(resource event, int flags);
 *  Detect whether event is pending or scheduled. */
PHP_FUNCTION(event_pending)
{
	zval        *zevent;
	php_event_t *e;
	long         flags;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zevent, &flags) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (event_pending(e->event, flags, NULL)) {
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto bool event_reinit(resource base);
 * Re-initialize event base. Should be called after a fork. */
PHP_FUNCTION(event_reinit)
{
	zval             *zbase;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_reinit(b->base)) {
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}
/* }}} */


/* {{{ proto resource event_base_new(void);
 * Returns resource representing new event base */
PHP_FUNCTION(event_base_new)
{
	php_event_base_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	b = emalloc(sizeof(php_event_base_t));
	memset(b, 0, sizeof(php_event_base_t));

	b->base = event_base_new();

	if (b->base) {
		b->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, b, le_event_base);
	} else {
		efree(b);
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto resource event_base_new_with_config(resource config);
 * Creates new event base taking the specified configuration under consideration. */
PHP_FUNCTION(event_base_new_with_config)
{
	php_event_base_t   *b;
	php_event_config_t *cfg;
	zval               *zcfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zcfg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_CONFIG(cfg, zcfg);

	b = emalloc(sizeof(php_event_base_t));
	memset(b, 0, sizeof(php_event_base_t));

	b->base = event_base_new_with_config(cfg);

	if (b->base) {
		b->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, b, le_event_base);
	} else {
		efree(b);
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto event_base_free(resource base);
 * Does nothing! Exists for compatibility with scripts that used libevent ext. */
PHP_FUNCTION(event_base_free)
{
	/* If some day we decide to add support for this func,
	 * we probably should add a zend linked list to the event struct
	 * (zend_llist_*() API) */
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "event_base_free does nothing! "
			"The reason is that there is some overhead with keeping track of "
			"events registered for an event base");
}
/* }}} */

/* {{{ proto string event_base_get_method(resource base);
 * Returns event method in use. */
PHP_FUNCTION(event_base_get_method)
{
	zval             *zbase;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	RETVAL_STRING(event_base_get_method(b->base), 1);
}
/* }}} */

/* {{{ proto int event_base_get_features(resource base);
 * Returns bitmask of features supported. See EVENT_FEATURE_* constants. */
PHP_FUNCTION(event_base_get_features)
{
	zval             *zbase;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase)

	RETVAL_LONG(event_base_get_features(b->base));
}
/* }}} */

/* {{{ proto bool event_base_priority_init(resource base, int n_priorities);
 * Sets number of priorities per event base. Returns &true; on success, otherwise &false; */
PHP_FUNCTION(event_base_priority_init)
{
	zval             *zbase;
	long              n_priorities;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zbase, &n_priorities) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_priority_init(b->base, n_priorities)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_base_loop(resource base[, int flags]);
 * Wait for events to become active, and run their callbacks. */
PHP_FUNCTION(event_base_loop)
{
	zval             *zbase;
	long              flags = -1;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l",
				&zbase, &flags) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	/* Call event_base_dispatch when flags omitted. */
	if (flags == -1) {
		if (event_base_dispatch(b->base) == -1) {
			RETURN_FALSE;
		}
	} else if (event_base_loop(b->base, flags) == -1) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_base_dispatch(resource base);
 * Wait for events to become active, and run their callbacks.
 * The same as event_base_loop with no flags set*/
PHP_FUNCTION(event_base_dispatch)
{
	zval             *zbase;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_dispatch(b->base) == -1) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_base_loopexit(resource base[, double timeout = 0.0]);
 * Tells event_base to stop optionally after given number of seconds. */
PHP_FUNCTION(event_base_loopexit)
{
	zval             *zbase;
	php_event_base_t *b;
	double            timeout = -1;
	int               res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|d",
				&zbase, &timeout) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (timeout == -1) {
		res = event_base_loopexit(b->base, NULL);
	} else {
		struct timeval tv;
		PHP_EVENT_TIMEVAL_SET(tv, timeout);

		res = event_base_loopexit(b->base, &tv);
	}

	if (res) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_base_loopbreak(resource base);
 * Tells event_base to stop. */
PHP_FUNCTION(event_base_loopbreak)
{
	zval             *zbase;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_loopbreak(b->base)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_base_set(resource base, resource event);
 * Associate event base with an event. */
PHP_FUNCTION(event_base_set)
{
	zval             *zbase;
	php_event_base_t *b;
	zval             *zevent;
	php_event_t      *e;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr",
				&zbase, &zevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_EVENT(e, zevent);

	if (is_pending(e->event)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't modify pending event");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_set(b->base, e->event)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_base_got_break(resource base);
 * Checks if the event loop was told to abort immediately by <function>event_loopbreak</function> */
PHP_FUNCTION(event_base_got_break)
{
	zval             *zbase;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_got_break(b->base)) {
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto bool event_base_got_exit(resource base);
 * Checks if the event loop was told to exit by <function>event_loopexit</function> */
PHP_FUNCTION(event_base_got_exit)
{
	zval             *zbase;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_got_exit(b->base)) {
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}
/* }}} */

/* {{{ proto double event_base_gettimeofday_cached(resource base);
 * On success returns the current time(as returned by gettimeofday()), looking
 * at the cached value in 'base' if possible, and calling gettimeofday() or
 * clock_gettime() as appropriate if there is no cached time. On failure
 * returns NULL. */
PHP_FUNCTION(event_base_gettimeofday_cached)
{
	zval                  *zbase;
	php_event_base_t      *b;
	struct timeval         tv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_gettimeofday_cached(b->base, &tv)) {
		RETURN_NULL();
	}
	
	RETVAL_DOUBLE(PHP_EVENT_TIMEVAL_TO_DOUBLE(tv));
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010100
/* {{{ proto bool event_base_update_cache_time(resource base);
 * Updates cache time. Available since libevent 2.1.1-alpha */
PHP_FUNCTION(event_base_update_cache_time)
{
	zval             *zbase;
	php_event_base_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	if (event_base_update_cache_time(b->base)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */
#endif



/* {{{ proto resource event_config_new(void);
 * On success returns a valid resource representing an event configuration
 * which can be passed to <function>event_base_new_with_config</function>. Otherwise returns &false;. */
PHP_FUNCTION(event_config_new)
{
	php_event_config_t *cfg;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	cfg = event_config_new();

	if (cfg) {
		ZEND_REGISTER_RESOURCE(return_value, cfg, le_event_config);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool event_config_avoid_method(resource cfg, string method);
 * Tells libevent to avoid specific event method.
 * See http://www.wangafu.net/~nickm/libevent-book/Ref2_eventbase.html#_creating_an_event_base
 * Returns &true; on success, otherwise &false;.*/
PHP_FUNCTION(event_config_avoid_method)
{
	zval               *zcfg;
	char               *method;
	int                 method_len;
	php_event_config_t *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
				&zcfg, &method, &method_len) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(cfg, php_event_config_t *, &zcfg, -1,
		PHP_EVENT_CONFIG_RES_NAME, le_event_config);

	if (event_config_avoid_method(cfg, method)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_config_require_features(resource cfg, int feature);
 * Enters a required event method feature that the application demands. */
PHP_FUNCTION(event_config_require_features)
{
	zval               *zcfg;
	long                feature;
	php_event_config_t *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zcfg, &feature) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(cfg, php_event_config_t *, &zcfg, -1,
		PHP_EVENT_CONFIG_RES_NAME, le_event_config);

	if (event_config_require_features(cfg, feature)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010000
/* {{{ proto void event_config_set_max_dispatch_interval(resource cfg, int max_interval, int max_callbacks, int min_priority);
 * Prevents priority inversion by limiting how many low-priority event
 * callbacks can be invoked before checking for more high-priority events.
 * Available since libevent 2.1.0-alpha. */
PHP_FUNCTION(event_config_set_max_dispatch_interval)
{
	zval                  *zcfg;
	php_event_timestamp_t  max_interval;
	long                   max_callbacks;
	long                   min_priority;
	php_event_config_t    *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rdll",
				&zcfg, &max_interval, &max_callbacks, min_priority) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(cfg, php_event_config_t *, &zcfg, -1,
		PHP_EVENT_CONFIG_RES_NAME, le_event_config);

	if (max_interval > 0) {
		struct timeval tv;
		PHP_EVENT_TIMEVAL_SET(tv, max_interval);

		event_config_set_max_dispatch_interval(cfg, &tv, max_callbacks, min_priority);
	} else {
		event_config_set_max_dispatch_interval(cfg, NULL, max_callbacks, min_priority);
	}
}
/* }}} */
#endif



/* {{{ proto resource bufferevent_socket_new(resource base[, resource socket = NULL[, int options = 0]]);
 *
 * Create a socket-based bufferevent.
 * options is one of EVENT_BEV_OPT_* constants, or 0.
 * Passing NULL to socket parameter means that the socket stream should be created later,
 * e.g. by means of bufferevent_socket_connect().
 *
 * socket parameter may be created as a stream(not necessarily by means of sockets extension)
 *
 * Returns bufferevent resource optionally associated with socket resource. */
PHP_FUNCTION(bufferevent_socket_new)
{
	zval                *zbase;
	php_event_base_t    *base;
	zval               **ppzfd   = NULL;
	evutil_socket_t      fd;
	long                 options = 0;
	php_event_bevent_t  *bev;
	struct bufferevent  *bevent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|Z!l",
				&zbase, &ppzfd, &options) == FAILURE) {
		return;
	}

	if (ppzfd) {
#ifdef PHP_EVENT_SOCKETS_SUPPORT 
		if (ppzfd) {
			/* sockets_zval_to_fd reports error
	 	 	 * in case if it is not a valid socket resource */
			/*fd = (evutil_socket_t) sockets_zval_to_fd(ppzfd TSRMLS_CC);*/
			fd = zval_to_fd(ppzfd TSRMLS_CC);

		}

		if (fd < 0) {
			RETURN_FALSE;
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

	bev = emalloc(sizeof(php_event_bevent_t));
	memset(bev, 0, sizeof(php_event_bevent_t));

	bevent = bufferevent_socket_new(base->base, fd, options);
	if (bevent == NULL) {
		efree(bev);
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate bufferevent for socket");
		RETURN_FALSE;
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

	/* Make sure base destroyed after the bufferevent */
	zend_list_addref(base->rsrc_id);

	bev->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, bev, le_event_bevent);
	/* XXX Normally I wouldn't do it. But if one creates the resource inside a callback,
	 * particularly called by libevent->event, the resource is destroyed on return automatically.
	 * See examples/listener.php
	 * Maybe add some userspace function like bufferevent_ref() */
	zend_list_addref(bev->rsrc_id);
}
/* }}} */

/* {{{ proto array bufferevent_pair_new(resource base[, int options = 0]);
 *
 * options is one of EVENT_BEV_OPT_* constants, or 0.
 *
 * Returns array of two buffer event resources, each connected to the other.
 * All the usual options are supported, except for EVENT_BEV_OPT_CLOSE_ON_FREE,
 * which has no effect, and EVENT_BEV_OPT_DEFER_CALLBACKS, which is always on.
 */
PHP_FUNCTION(bufferevent_pair_new)
{
	zval               *zbase;
	php_event_base_t   *base;
	long                options        = 0;
	php_event_bevent_t *b[2];
	struct bufferevent *bevent_pair[2];
	int                 i;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l",
				&zbase, &options) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (bufferevent_pair_new(base->base, options, bevent_pair)) {
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i = 0; i < 2; i++) {
		b[i] = emalloc(sizeof(php_event_bevent_t));
		memset(b[i], 0, sizeof(php_event_bevent_t));

		b[i]->bevent    = bevent_pair[i];
		b[i]->stream_id = -1;
		b[i]->rsrc_id   = zend_list_insert(b, le_event_bevent TSRMLS_CC);

		add_next_index_resource(return_value, b[i]->rsrc_id);
	}
}
/* }}} */

/* {{{ proto void bufferevent_free(resource bevent);
 * Free a buffer event resource. */
PHP_FUNCTION(bufferevent_free)
{
	php_event_bevent_t *bev;
	zval               *zbevent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	zend_list_delete(bev->rsrc_id);
}

/* }}} */

/* {{{ proto bool bufferevent_socket_connect(resource bevent, string addr[, bool sync_resolve = FALSE]);
 *
 * Connect bufferevent's socket to given address(optionally with port).  The
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
PHP_FUNCTION(bufferevent_socket_connect)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	char               *addr;
	int                 addr_len;
	struct sockaddr     sa;
	socklen_t           sa_len = sizeof(struct sockaddr);
	zend_bool           sync_resolve = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|b",
				&zbevent, &addr, &addr_len, &sync_resolve) == FAILURE) {
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

/* {{{ proto bool bufferevent_socket_connect_hostname(resource bevent, resource dns_base, string hostname, int port[, int family = EVENT_AF_UNSPEC]);
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
PHP_FUNCTION(bufferevent_socket_connect_hostname)
{
#if LIBEVENT_VERSION_NUMBER < 0x02000300
	PHP_EVENT_LIBEVENT_VERSION_REQUIRED(bufferevent_socket_connect_hostname, 2.0.3-alpha);
	RETVAL_FALSE;
#else
	php_event_bevent_t   *bev;
	zval                 *zbevent;
	zval                 *zdns_base;
	char                 *hostname;
	int                   hostname_len;
	long                  port;
	long                  family;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr!sl|l",
				&zbevent, &zdns_base, &hostname, &hostname_len,
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
		PHP_EVENT_ASSERT(dnsb->rsrc_id);
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
	/*zend_list_addref(dnsb->rsrc_id);*/
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

/* {{{ proto resource bufferevent_socket_get_dns_error_string(resource bevent);
 * Returns string describing the last failed DNS lookup attempt made by
 * bufferevent_socket_connect_hostname(), or an empty string, if no DNS error
 * detected. */
PHP_FUNCTION(bufferevent_socket_get_dns_error)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	int                 err;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
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

/* {{{ proto void bufferevent_setcb(resource bevent, callable readcb, callable writecb, callable eventcb[, mixed arg = NULL]);
 * Changes one or more of the callbacks of a bufferevent.
 * A callback may be disabled by passing NULL instead of the callable.
 * arg is an argument passed to the callbacks.
 */
PHP_FUNCTION(bufferevent_setcb)
{
	php_event_bevent_t    *bev;
	zval                  *zbevent;
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rf!f!f!|z!",
				&zbevent,
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

/* {{{ proto void bufferevent_enable(resource bevent, int events);
 * Enable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_FUNCTION(bufferevent_enable)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	long                events;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zbevent, &events) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	bufferevent_enable(bev->bevent, events);
}
/* }}} */

/* {{{ proto void bufferevent_disable(resource bevent, int events);
 * Disable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_FUNCTION(bufferevent_disable)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	long                events;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zbevent, &events) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	bufferevent_disable(bev->bevent, events);
}
/* }}} */

/* {{{ proto int bufferevent_get_enabled(resource bevent);
 * Returns bitmask of events currently enabled on the buffer event. */
PHP_FUNCTION(bufferevent_get_enabled)
{
	php_event_bevent_t *bev;
	zval               *zbevent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	RETVAL_LONG(bufferevent_get_enabled(bev->bevent));
}
/* }}} */

/* {{{ proto resource bufferevent_get_input(resource bevent);
 * Returns the input event buffer resource */
PHP_FUNCTION(bufferevent_get_input)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	php_event_buffer_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	b = emalloc(sizeof(php_event_buffer_t));
	memset(b, 0, sizeof(php_event_buffer_t));

	b->buf       = bufferevent_get_input(bev->bevent);
	b->rsrc_id   = ZEND_REGISTER_RESOURCE(return_value, b, le_event_buffer);

	zend_list_addref(b->rsrc_id);
}
/* }}} */

/* {{{ proto resource bufferevent_get_output(resource bevent);
 * Returns the output event buffer resource */
PHP_FUNCTION(bufferevent_get_output)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	php_event_buffer_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	b = emalloc(sizeof(php_event_buffer_t));
	memset(b, 0, sizeof(php_event_buffer_t));

	b->buf       = bufferevent_get_output(bev->bevent);
	b->rsrc_id   = ZEND_REGISTER_RESOURCE(return_value, b, le_event_buffer);

	zend_list_addref(b->rsrc_id);
}
/* }}} */

/* {{{ proto void bufferevent_setwatermark(resource bevent, int events, int lowmark, int highmark);
 * Adjusts the read watermarks, the write watermarks, or both, of a single bufferevent. */
PHP_FUNCTION(bufferevent_setwatermark)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	long                events;
	long                lowmark;
	long                highmark;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlll",
				&zbevent, &events, &lowmark, &highmark) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	bufferevent_setwatermark(bev->bevent, events, (size_t) lowmark, (size_t) highmark);
}
/* }}} */

/* {{{ proto bool bufferevent_write(resource bevent, string data);
 * Adds `data' to a bufferevent's output buffer. */
PHP_FUNCTION(bufferevent_write)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	zval               *zdata;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz",
				&zbevent, &zdata) == FAILURE) {
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

/* {{{ proto bool bufferevent_write_buffer(resource bevent, resource buf);
 * Adds contents of the entire buffer to a bufferevent's output buffer. */
PHP_FUNCTION(bufferevent_write_buffer)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr",
				&zbevent, &zbuf) == FAILURE) {
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

/* {{{ proto int bufferevent_read(resource bevent, string &data, int size);
 * Removes up to size bytes from the input buffer, storing them into the memory at data.
 *
 * Returns the number of bytes actually removed.  */
PHP_FUNCTION(bufferevent_read)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	zval               *zdata;
	long                size;
	char               *data;
	long                ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rzl",
				&zbevent, &zdata, &size) == FAILURE) {
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

/* {{{ proto bool bufferevent_read_buffer(resource bevent, resource buf);
 * Drains the entire contents of the input buffer and places them into buf; it returns 0 on success and -1 on failure. */
PHP_FUNCTION(bufferevent_read_buffer)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr",
				&zbevent, &zbuf) == FAILURE) {
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

/* {{{ proto bool bufferevent_priority_set(resource bevent, int priority);
 * Assign a priority to a bufferevent.
 * Only supported for socket bufferevents. */
PHP_FUNCTION(bufferevent_priority_set)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	long                priority;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zbevent, &priority) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (bufferevent_priority_set(bev->bevent, priority)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool bufferevent_set_timeouts(resource bevent, double timeout_read, double timeout_write);
 * Set the read and write timeout for a bufferevent. */
PHP_FUNCTION(bufferevent_set_timeouts)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	double              timeout_read;
	double              timeout_write;
	struct timeval      tv_read;
	struct timeval      tv_write;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll",
				&zbevent, &timeout_read, &timeout_write) == FAILURE) {
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



/* {{{ proto resource evbuffer_new(void);
 * Allocates storage for new event buffer and returns it's resource. */
PHP_FUNCTION(evbuffer_new)
{
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	b = emalloc(sizeof(php_event_buffer_t));
	memset(b, 0, sizeof(php_event_buffer_t));

	b->buf       = evbuffer_new();
	b->rsrc_id   = ZEND_REGISTER_RESOURCE(return_value, b, le_event_buffer);
}
/* }}} */

/* {{{ proto void evbuffer_free(resource buf);
 * Free storage allocated for the event buffer */
PHP_FUNCTION(evbuffer_free)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	zend_list_delete(b->rsrc_id);
}
/* }}} */

/* {{{ proto bool evbuffer_freeze(resource buf, bool at_front);
 * Prevent calls that modify an event buffer from succeeding. */
PHP_FUNCTION(evbuffer_freeze)
{
	php_event_buffer_t *b;
	zval               *zbuf;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rb",
				&zbuf, &at_front) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_freeze(b->buf, at_front)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evbuffer_unfreeze(resource buf, bool at_front);
 * Re-enable calls that modify an evbuffer. */
PHP_FUNCTION(evbuffer_unfreeze)
{
	php_event_buffer_t *b;
	zval               *zbuf;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rb",
				&zbuf, &at_front) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_unfreeze(b->buf, at_front)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto long evbuffer_get_length(resource buf);
 * Returns the total number of bytes stored in the event buffer. */
PHP_FUNCTION(evbuffer_get_length)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	RETVAL_LONG(evbuffer_get_length(b->buf));
}
/* }}} */

/* {{{ proto void evbuffer_lock(resource buf);
 * Acquire the lock on an evbuffer. 
 * Has no effect if locking was not enabled with evbuffer_enable_locking.
 */
PHP_FUNCTION(evbuffer_lock)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_lock(b->buf);
}
/* }}} */

/* {{{ proto void evbuffer_unlock(resource buf);
 * Release the lock on an evbuffer.
 * Has no effect if locking was not enabled with evbuffer_enable_locking.
 */
PHP_FUNCTION(evbuffer_unlock)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_unlock(b->buf);
}
/* }}} */

/* {{{ proto void evbuffer_enable_locking(resource buf);
 *
 * Enable locking on an evbuffer so that it can safely be used by multiple threads at the same time.
 * When locking is enabled, the lock will be held when callbacks are invoked.
 * This could result in deadlock if you aren't careful. Plan accordingly!
 */
PHP_FUNCTION(evbuffer_enable_locking)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_enable_locking(b->buf, NULL);
}
/* }}} */

/* {{{ proto bool evbuffer_add(resource buf, string data); 
 *
 * Append data to the end of an event buffer.
 */
PHP_FUNCTION(evbuffer_add)
{
	php_event_buffer_t   *b;
	zval                 *zbuf;
	zval                **ppzdata;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ",
				&zbuf, &ppzdata) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	convert_to_string_ex(ppzdata);

	if (evbuffer_add(b->buf, (void *) Z_STRVAL_PP(ppzdata), Z_STRLEN_PP(ppzdata))) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evbuffer_add_buffer(resource outbuf, inbuf); 
 * Move all data from one evbuffer into another evbuffer.
 * This is a destructive add. The data from one buffer moves into the other buffer. However, no unnecessary memory copies occur.
 */
PHP_FUNCTION(evbuffer_add_buffer)
{
	php_event_buffer_t *b_out     , *b_in;
	zval               *zbuf_out  , *zbuf_in;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr",
				&zbuf_out, &zbuf_in) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b_out, zbuf_out);
	PHP_EVENT_FETCH_BUFFER(b_in, zbuf_in);

	if (evbuffer_add_buffer(b_out->buf, b_in->buf)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int evbuffer_remove(resource buf, string &data, long max_bytes);
 * Read data from an evbuffer and drain the bytes read.  If more bytes are
 * requested than are available in the evbuffer, we only extract as many bytes
 * as were available.
 *
 * Returns the number of bytes read, or -1 if we can't drain the buffer.
 */
PHP_FUNCTION(evbuffer_remove)
{
	php_event_buffer_t *b;
	zval               *zbuf;
	zval               *zdata;
	long                max_bytes;
	long                ret;
	char               *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rzl",
				&zbuf, &zdata, &max_bytes) == FAILURE) {
		return;
	}

	if (!Z_ISREF_P(zdata)) {
		/* Was not passed by reference */
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	data = emalloc(sizeof(char) * max_bytes + 1);

	ret = evbuffer_remove(b->buf, data, max_bytes);

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


/* {{{ proto int event_socket_get_last_errno([resource socket = null]);
 * Returns the most recent socket error number(errno).
 *
 * Note: if sockets extension was not available at the time when event
 * extension built, the socket parameter is ignored. In this case the function
 * returns the morst recent socket error regardless the argument provided. */
PHP_FUNCTION(event_socket_get_last_errno)
{

	zval **ppzfd = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|Z!",
				&ppzfd) == FAILURE) {
		return;
	}

#ifdef PHP_EVENT_SOCKETS_SUPPORT 
	if (ppzfd) {
		evutil_socket_t fd = (evutil_socket_t) sockets_zval_to_fd(ppzfd TSRMLS_CC);

		if (fd < 0) {
			RETURN_FALSE;
		}

		RETVAL_LONG(evutil_socket_geterror(fd));
	} else {
		RETVAL_LONG(EVUTIL_SOCKET_ERROR());
	}
#else
	RETVAL_LONG(EVUTIL_SOCKET_ERROR());
#endif
}
/* }}} */

/* {{{ proto string event_socket_get_last_error([resource socket = null]);
 * Returns the most recent socket error */
PHP_FUNCTION(event_socket_get_last_error)
{
	zval **ppzfd = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|Z!",
				&ppzfd) == FAILURE) {
		return;
	}

#ifdef PHP_EVENT_SOCKETS_SUPPORT 
	if (ppzfd) {
		evutil_socket_t fd = (evutil_socket_t) sockets_zval_to_fd(ppzfd TSRMLS_CC);

		if (fd < 0) {
			RETURN_FALSE;
		}

		RETVAL_STRING(evutil_socket_error_to_string(evutil_socket_geterror(fd)), 1);
	} else {
		RETVAL_STRING(evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), 1);
	}
#else
	RETVAL_STRING(evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), 1);
#endif
}
/* }}} */


/* API functions END }}} */



#if HAVE_EVENT_EXTRA_LIB
/* {{{ Extra API functions */

/* {{{ proto resource evdns_base_new(resource base, bool initialize);
 *
 * Returns resource representing event dns base.
 *
 * If the initialize argument is true, it tries to configure the DNS base
 * sensibly given your operating system’s default. Otherwise, it leaves the
 * evdns_base empty, with no nameservers or options configured. In the latter
 * case you should configure dns base yourself, e.g. with
 * event_dns_base_resolv_conf_parse() */
PHP_FUNCTION(evdns_base_new)
{
	php_event_base_t     *base;
	zval                 *zbase;
	php_event_dns_base_t *dnsb;
	zend_bool             initialize;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rb",
				&zbase, &initialize) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	dnsb = emalloc(sizeof(php_event_dns_base_t));
	memset(dnsb, 0, sizeof(php_event_dns_base_t));

	dnsb->dns_base = evdns_base_new(base->base, initialize);

	if (dnsb->dns_base) {
		dnsb->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, dnsb, le_event_dns_base);
		PHP_EVENT_ASSERT(dnsb->rsrc_id);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto void evdns_base_free(resource base);
 * Free an evdns base */
PHP_FUNCTION(evdns_base_free)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zdns_base) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	zend_list_delete(dnsb->rsrc_id);
}
/* }}} */

/* {{{ proto bool evdns_base_resolv_conf_parse(resource dns_base, int flags, string filename);
 * Scans the resolv.conf formatted file stored in filename, and read in all the
 * options from it that are listed in flags */
PHP_FUNCTION(evdns_base_resolv_conf_parse)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;
	long                  flags;
	char                 *filename;
	int                   filename_len;
	int                   ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls",
				&zdns_base, &flags, &filename, &filename_len) == FAILURE) {
		return;
	}


	if (flags & ~(DNS_OPTION_NAMESERVERS | DNS_OPTION_SEARCH | DNS_OPTION_MISC
				| DNS_OPTIONS_ALL)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid flags");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	ret = evdns_base_resolv_conf_parse(dnsb->dns_base, flags, filename);

	if (ret) {
		char err[40];

		switch (ret) {
			case 1:
				strcpy(err, "Failed to open file");
				break;
			case 2:
				strcpy(err, "Failed to stat file");
				break;
			case 3:
				strcpy(err, "File too large");
				break;
			case 4:
				strcpy(err, "Out of memory");
				break;
			case 5:
				strcpy(err, "Short read from file");
				break;
			case 6:
				strcpy(err, "No nameservers listed in the file");
				break;
		}

		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", err);
	}

	RETVAL_TRUE;
}
/* }}} */



/* {{{ proto resource evconnlistener_new_bind(resource base, callable cb, mixed data, int flags, int backlog, string addr);
 * Creates new connection listener associated with an event base.
 *
 * Returns resource representing the event connection listener.
 */
PHP_FUNCTION(evconnlistener_new_bind)
{
	zval                  *zbase;
	php_event_base_t      *base;
	zend_fcall_info        fci      = empty_fcall_info;
	zend_fcall_info_cache  fcc      = empty_fcall_info_cache;
	php_event_listener_t  *l;
	zval                  *zdata    = NULL;
	zval                  *zaddr;
	long                   flags;
	long                   backlog;
	struct evconnlistener *listener;
	struct sockaddr        sa;
	socklen_t              sa_len   = sizeof(struct sockaddr);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rfz!llz",
				&zbase, &fci, &fcc, &zdata, &flags, &backlog, &zaddr) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	convert_to_string(zaddr);
	if (php_network_parse_network_address_with_port(Z_STRVAL_P(zaddr), Z_STRLEN_P(zaddr),
				&sa, &sa_len TSRMLS_CC) != SUCCESS) {
		RETURN_FALSE;
	}

	l = emalloc(sizeof(php_event_listener_t));
	memset(l, 0, sizeof(php_event_listener_t));

	listener = evconnlistener_new_bind(base->base, _php_event_listener_cb,
			(void *) l, flags, backlog, &sa, sa_len);
	if (!listener) {
		efree(l);
		RETURN_FALSE;
	}
	l->listener = listener;

	if (zdata) {
		l->data = zdata;
		Z_ADDREF_P(zdata);
	}

	PHP_EVENT_COPY_FCALL_INFO(l->fci, l->fcc, &fci, &fcc);

	l->base_id = Z_LVAL_P(zbase);
	zend_list_addref(l->base_id);

	TSRMLS_SET_CTX(l->thread_ctx);

	l->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, l, le_event_listener);

	PHP_EVENT_ASSERT(l->rsrc_id);
}
/* }}} */

/* {{{ proto resource evconnlistener_new(resource base, callable cb, mixed data, int flags, int backlog, resource stream);
 * Creates new connection listener associated with an event base.
 *
 * stream parameter may be a socket resource, or a stream associated with a socket.
 *
 * Returns resource representing the event connection listener.
 */
PHP_FUNCTION(evconnlistener_new)
{
	zval                   *zbase;
	php_event_base_t       *base;
	zend_fcall_info         fci       = empty_fcall_info;
	zend_fcall_info_cache   fcc       = empty_fcall_info_cache;
	php_event_listener_t   *l;
	zval                   *zdata     = NULL;
	long                    flags;
	long                    backlog;
	zval                  **ppzfd;
	evutil_socket_t         fd        = -1;
	struct evconnlistener  *listener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rfz!llZ",
				&zbase, &fci, &fcc, &zdata, &flags, &backlog,
				&ppzfd) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	/* sockets_zval_to_fd reports error
	 * in case if it is not a valid socket resource */
	fd = zval_to_fd(ppzfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	/* Make sure that the socket is in non-blocking mode(libevent's tip) */
	evutil_make_socket_nonblocking(fd);

	l = emalloc(sizeof(php_event_listener_t));
	memset(l, 0, sizeof(php_event_listener_t));

	listener = evconnlistener_new(base->base, _php_event_listener_cb,
			(void *) l, flags, backlog, fd);
	if (!listener) {
		efree(l);
		RETURN_FALSE;
	}
	l->listener = listener;

	if (zdata) {
		l->data = zdata;
		Z_ADDREF_P(zdata);
	}

	PHP_EVENT_COPY_FCALL_INFO(l->fci, l->fcc, &fci, &fcc);

#if 0
	/* Don't do this, since libevent calls accept() afterwards, thus producing
	 * new file descriptor. The new descriptor is available in
	 * _php_event_listener_cb() callback. */

	if (Z_TYPE_PP(ppzfd) == IS_RESOURCE) {
		/* lval of ppzfd is the resource ID */
		l->stream_id = Z_LVAL_PP(ppzfd);
		zend_list_addref(Z_LVAL_PP(ppzfd));
	} else {
		l->stream_id = -1;
	}
#endif
	l->stream_id = -1;

	l->base_id = Z_LVAL_P(zbase);
	zend_list_addref(l->base_id);

	TSRMLS_SET_CTX(l->thread_ctx);

	l->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, l, le_event_listener);

	PHP_EVENT_ASSERT(l->rsrc_id);
}
/* }}} */


/* {{{ proto void evconnlistener_free(resource listener);
 * Free an event connect listener resource */
PHP_FUNCTION(evconnlistener_free)
{
	php_event_listener_t *l;
	zval                 *zlistener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zlistener) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	zend_list_delete(l->rsrc_id);
}
/* }}} */

/* {{{ proto bool evconnlistener_enable(resource listener);
 * Enable an event connect listener resource */
PHP_FUNCTION(evconnlistener_enable)
{
	php_event_listener_t *l;
	zval                 *zlistener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zlistener) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (evconnlistener_enable(l->listener)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evconnlistener_disable(resource listener);
 * Disable an event connect listener resource */
PHP_FUNCTION(evconnlistener_disable)
{
	php_event_listener_t *l;
	zval                 *zlistener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zlistener) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (evconnlistener_disable(l->listener)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void evconnlistener_set_cb(resource listener, callable cb[, mixed arg = NULL]);
 * Adjust event connect listener's callback and optionally the callback argument.
 * Both cb and arg may be NULL.
 */
PHP_FUNCTION(evconnlistener_set_cb)
{
	php_event_listener_t  *l;
	zval                  *zlistener;
	zend_fcall_info        fci       = empty_fcall_info;
	zend_fcall_info_cache  fcc       = empty_fcall_info_cache;
	zval                  *zarg      = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rf!|z!",
				&zlistener, &fci, &fcc, &zarg) == FAILURE) {
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

		Z_ADDREF_P(zarg);
		l->data = zarg;
	}

	/*
	 * No sense in the following call, since the callback and the pointer
	 * remain the same
	 * evconnlistener_set_cb(l->listener, _php_event_listener_cb, (void *) l);
	 */
}
/* }}} */

/* {{{ proto void evconnlistener_set_error_cb(resource listener, callable cb);
 * Set event listener's error callback
 */
PHP_FUNCTION(evconnlistener_set_error_cb)
{
	php_event_listener_t  *l;
	zval                  *zlistener;
	zend_fcall_info        fci;
	zend_fcall_info_cache  fcc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rf",
				&zlistener, &fci, &fcc) == FAILURE) {
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
/* {{{ proto resource evconnlistener_get_base(resource listener);
 * Get event base associated with the connection listener
 */
PHP_FUNCTION(evconnlistener_get_base)
{
	php_event_listener_t *l;
	zval                 *zlistener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zlistener) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	/*
	   base = evconnlistener_get_base(l->listener);
	*/

	if (l->base_id) {
		/* Make sure base wouldn't be inexpectedly destroyed */
		zend_list_addref(l->base_id);

		RETURN_RESOURCE(l->base_id);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */
#endif


/* Extra API functions END }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */

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

#define PHP_EVENT_TIMEVAL_SET(tv, t)                     \
        do {                                             \
            tv.tv_sec  = (long) t;                       \
            tv.tv_usec = (long) ((t - tv.tv_sec) * 1e6); \
        } while (0)

#define PHP_EVENT_TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)

#define PHP_EVENT_RET_SOCKETS_REQUIRED_NORET                                   \
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "`sockets' extension required. " \
            "If you have `sockets' installed, rebuild `event' extension")      \
#define PHP_EVENT_RET_SOCKETS_REQUIRED                                         \
    PHP_EVENT_RET_SOCKETS_REQUIRED_NORET;                                      \
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
		case EVENT_LOG_DEBUG:
			error_type = E_STRICT;
		case EVENT_LOG_MSG:
			error_type = E_NOTICE;
		case EVENT_LOG_WARN:
			error_type = E_WARNING;
		case EVENT_LOG_ERR:
			error_type = E_ERROR;
		default:
			error_type = E_NOTICE;
	}

	php_error_docref(NULL TSRMLS_CC, error_type, "%s", msg);
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
			/* PHP stream */
			if (php_stream_can_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL)) {
				if (php_stream_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else if (php_stream_can_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_INTERNAL)) {
				if (php_stream_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_INTERNAL,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else { /* STDIN, STDOUT, STDERR etc. */
				php_stream_from_zval_no_verify(stream, ppfd);
				if (stream == NULL) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed obtaining fd");
					return -1;
				}
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

/* {{{ bevent_read_cb */
static void bevent_read_cb(struct bufferevent *bev, void *ptr)
{

}
/* }}} */

/* {{{ bevent_write_cb */
static void bevent_write_cb(struct bufferevent *bev, void *ptr)
{

}
/* }}} */

/* {{{ bevent_event_cb */
static void bevent_event_cb(struct bufferevent *bev, short events, void *ptr)
{

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
		event_free(e->event);
		efree(e);
	}
}
/* }}} */

/* {{{ php_event_base_dtor */
static void php_event_base_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	/* TODO: what if events bound to the event_base are not destroyed? */
	struct event_base *base = (struct event_base *) rsrc->ptr;

	event_base_free(base);
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

/* Private functions }}} */

#define PHP_EVENT_REG_CONST_LONG(name, real_name) \
    REGISTER_LONG_CONSTANT(#name, real_name, CONST_CS | CONST_PERSISTENT);

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(event)
{
	le_event        = zend_register_list_destructors_ex(php_event_dtor, NULL, PHP_EVENT_RES_NAME, module_number);
	le_event_base   = zend_register_list_destructors_ex(php_event_base_dtor, NULL, PHP_EVENT_BASE_RES_NAME, module_number);
	le_event_config = zend_register_list_destructors_ex(php_event_config_dtor, NULL, PHP_EVENT_CONFIG_RES_NAME, module_number);
	le_event_bevent = zend_register_list_destructors_ex(php_event_bevent_dtor, NULL, PHP_EVENT_BEVENT_RES_NAME, module_number);

	/* Loop flags */
	PHP_EVENT_REG_CONST_LONG(EVENT_LOOP_ONCE,     EVLOOP_ONCE);
	PHP_EVENT_REG_CONST_LONG(EVENT_LOOP_NONBLOCK, EVLOOP_NONBLOCK);

	/* Event flags */
	PHP_EVENT_REG_CONST_LONG(EVENT_ET,      EV_ET);
	PHP_EVENT_REG_CONST_LONG(EVENT_PERSIST, EV_PERSIST);
	PHP_EVENT_REG_CONST_LONG(EVENT_READ,    EV_READ);
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


	/* Handle libevent's error logging more gracefully than it's default
	 * logging to stderr, or calling abort()/exit() */
	event_set_fatal_callback(fatal_error_cb);
	event_set_log_callback(log_cb);

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
	php_event_base_t      *base;
	zend_fcall_info        fci   = empty_fcall_info;
	zend_fcall_info_cache  fcc   = empty_fcall_info_cache;
	zval                  *arg   = NULL;
	php_event_t           *e;
	struct event          *event;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rf|z",
				&zbase, &fci, &fcc, &arg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	e = emalloc(sizeof(php_event_t));
	memset(e, 0, sizeof(php_event_t));

	event = evtimer_new(base, timer_cb, (void *) e);
	if (!event) {
		efree(e);
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "evtimer_new failed");
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
	php_event_base_t      *base;
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

	PHP_EVENT_FETCH_BASE(base, zbase);

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

    if (evtimer_assign(e->event, base, timer_cb, (void *) e)) {
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



/* {{{ proto resource event_new(resource base, mixed fd, int what, callable cb[, zval arg = NULL]);
 * Creates new event */
PHP_FUNCTION(event_new)
{
	zval                   *zbase;
	php_event_base_t       *base;
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

	PHP_EVENT_FETCH_BASE(base, zbase);

	/* TODO: check if a signum bound to different event bases */

	e = emalloc(sizeof(php_event_t));
	memset(e, 0, sizeof(php_event_t));

	event = event_new(base, fd, what, event_cb, (void *) e);
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

	ZEND_REGISTER_RESOURCE(return_value, e, le_event);
}
/* }}} */

/* {{{ proto bool event_set(resource event, resource base, mixed fd,[ int what = NULL[, callable cb = NULL[, zval arg = NULL]]]);
 * Re-configures event.
 * Note, this function doesn't invoke obsolete libevent's event_set. It calls event_assign instead.  */
PHP_FUNCTION(event_set)
{
	zval                   *zbase;
	php_event_base_t       *base;
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

	PHP_EVENT_FETCH_BASE(base, zbase);

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

	event_get_assignment(e->event, &base,
			(ppzfd ? NULL : &fd),
			(short *) (what == -1 ? &what : NULL),
			NULL /* ignore old callback */ ,
			NULL /* ignore old callback argument */);

	if (event_assign(e->event, base, fd, what, event_cb, (void *) e)) {
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

/* {{{ proto event_free(resource event);
 * Does nothing! Exists for compatibility with scripts that used libevent ext. */
PHP_FUNCTION(event_free)
{
}
/* }}} */



/* {{{ proto resource event_base_new(void);
 * Returns resource representing new event base */
PHP_FUNCTION(event_base_new)
{
	php_event_base_t *base;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	base = event_base_new();

	if (base) {
		ZEND_REGISTER_RESOURCE(return_value, base, le_event_base);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto resource event_base_new_with_config(resource config);
 * Creates new event base taking the specified configuration under consideration. */
PHP_FUNCTION(event_base_new_with_config)
{
	php_event_base_t   *base;
	php_event_config_t *cfg;
	zval               *zcfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zcfg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_CONFIG(cfg, zcfg);

	base = event_base_new_with_config(cfg);

	if (base) {
		ZEND_REGISTER_RESOURCE(return_value, base, le_event_base);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto string event_base_get_method(resource base);
 * Returns event method in use. */
PHP_FUNCTION(event_base_get_method)
{
	zval             *zbase;
	php_event_base_t *base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	RETVAL_STRING(event_base_get_method(base), 1);
}
/* }}} */

/* {{{ proto int event_base_get_features(resource base);
 * Returns bitmask of features supported. See EVENT_FEATURE_* constants. */
PHP_FUNCTION(event_base_get_features)
{
	zval             *zbase;
	php_event_base_t *base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase)

	RETVAL_LONG(event_base_get_features(base));
}
/* }}} */

/* {{{ proto bool event_base_priority_init(resource base, int n_priorities);
 * Sets number of priorities per event base. Returns &true; on success, otherwise &false; */
PHP_FUNCTION(event_base_priority_init)
{
	zval             *zbase;
	long              n_priorities;
	php_event_base_t *base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zbase, &n_priorities) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (event_base_priority_init(base, n_priorities)) {
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
	php_event_base_t *base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l",
				&zbase, &flags) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	/* Call event_base_dispatch when flags omitted. */
	if (flags == -1) {
		if (event_base_dispatch(base) == -1) {
			RETURN_FALSE;
		}
	} else if (event_base_loop(base, flags) == -1) {
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
	php_event_base_t *base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (event_base_dispatch(base) == -1) {
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
	php_event_base_t *base;
	double            timeout = -1;
	int               res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|d",
				&zbase, &timeout) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (timeout == -1) {
		res = event_base_loopexit(base, NULL);
	} else {
		struct timeval tv;
		PHP_EVENT_TIMEVAL_SET(tv, timeout);

		res = event_base_loopexit(base, &tv);
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
	php_event_base_t *base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (event_base_loopbreak(base)) {
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
	php_event_base_t *base;
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

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (event_base_set(base, e->event)) {
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
	php_event_base_t *base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (event_base_got_break(base)) {
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
	php_event_base_t *base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (event_base_got_exit(base)) {
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
	php_event_base_t      *base;
	struct timeval         tv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (event_base_gettimeofday_cached(base, &tv)) {
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
	php_event_base_t *base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (event_base_update_cache_time(base)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */
#endif

/* {{{ proto event_base_free(resource base);
 * Does nothing! Exists for compatibility with scripts that used libevent ext. */
PHP_FUNCTION(event_base_free)
{
}
/* }}} */



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



/* {{{ proto resource bufferevent_socket_new(resource base[, mixed fd = NULL[, int options = 0]]);
 * Create a socket-based bufferevent.
 * options is one of EVENT_BEV_OPT_* constants, or 0. */
PHP_FUNCTION(bufferevent_socket_new)
{
#ifndef PHP_EVENT_SOCKETS_SUPPORT 
	PHP_EVENT_RET_SOCKETS_REQUIRED;
#else
	zval                *zbase;
	php_event_base_t    *base;
	zval               **ppzfd   = NULL;
	evutil_socket_t      fd;
	long                 options = 0;
	php_event_bevent_t  *b;
	struct bufferevent  *bevent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|Z!l",
				&zbase, &ppzfd, &options) == FAILURE) {
		return;
	}

	if (ppzfd == NULL) {
		/* sockets_zval_to_fd reports error
	 	 * in case if it is not a valid socket resource */
		fd = (evutil_socket_t) sockets_zval_to_fd(ppzfd TSRMLS_CC);
		if (fd < 0) {
			RETURN_FALSE;
		}
		/* Make sure that the socket you provide to bufferevent_socket_new is
		 * in non-blocking mode(libevent's tip). */
		evutil_make_socket_nonblocking(fd);
	} else {
		fd = -1; /* User decided to assign fd later */
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	b = emalloc(sizeof(php_event_bevent_t));
	memset(b, 0, sizeof(php_event_bevent_t));

	bevent = bufferevent_socket_new(base, fd, options);
	if (bevent == NULL) {
		efree(b);
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate bufferevent for socket");
		RETURN_FALSE;
	}

	b->bevent = bevent;

	/* lval of ppzfd is the resource ID */
	b->stream_id = Z_LVAL_PP(ppzfd);
	zend_list_addref(Z_LVAL_PP(ppzfd));

	ZEND_REGISTER_RESOURCE(return_value, b, le_event_bevent);
#endif
}
/* }}} */

/* {{{ proto bool bufferevent_socket_connect(resource bevent, string addr);
 * Connect bufferevent's socket to given address(optionally with port).
 * Recognized address formats:
 *    [IPv6Address]:port
 *    [IPv6Address]
 *    IPv6Address
 *    IPv4Address:port
 *    IPv4Address
 * The function available since libevent 2.0.2-alpha.
 */
PHP_FUNCTION(bufferevent_socket_connect)
{
#ifndef PHP_EVENT_SOCKETS_SUPPORT 
	PHP_EVENT_RET_SOCKETS_REQUIRED;
#elif LIBEVENT_VERSION_NUMBER < 0x02000200
	php_error_docref(NULL TSRMLS_CC, E_ERROR,
			"bufferevent_socket_connect is available since libevent "
			"2.0.2-alpha. Please upgrade libevent distribution");
	RETURN_FALSE;
#else
	php_event_bevent_t *bev;
	zval               *zbevent;
	char               *addr;
	int                 addr_len;
	struct sockaddr     sa;
	int                 sa_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
				&zbevent, &addr, &addr_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (evutil_parse_sockaddr_port(addr, &sa, &sa_len)) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed parsing address: the address is not well-formed, "
				"or the port is out of range");
		RETURN_FALSE;
	}

	if (bufferevent_socket_connect(bev->bevent, &sa, sa_len)) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed starting connection");
		RETURN_FALSE;
	}

	RETVAL_TRUE;
#endif
}
/* }}} */

/* {{{ proto void bufferevent_setcb(resource bevent, callable readcb, callable writecb, callable eventcb[, mixed arg = NULL]);
 * Changes one or more of the callbacks of a bufferevent.
 * A callback may be disabled by passing NULL instead of the callable.
 * arg is an argument passed to the callbacks.
 */
PHP_FUNCTION(bufferevent_setcb)
{
#ifndef PHP_EVENT_SOCKETS_SUPPORT 
	PHP_EVENT_RET_SOCKETS_REQUIRED_NORET;
#else
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
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_read, bev->fcc_read, &fci_read, &fcc_read);
	} else {
		read_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_write)) {
		write_cb = bevent_write_cb;
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_write, bev->fcc_write, &fci_write, &fcc_write);
	} else {
		write_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_event)) {
		event_cb = bevent_event_cb;
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
#endif
}
/* }}} */

/* API functions END }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */

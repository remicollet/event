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

#include "fe.h"
#include "priv.h"

/* {{{ ARGINFO */
ZEND_BEGIN_ARG_INFO(arginfo_event__void, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base_config_1, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base__construct, 0, 0, 0)
	ZEND_ARG_INFO(0, cfg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base_void, 0, 0, 0)
	ZEND_ARG_INFO(0, cfg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base_priority_init, 0, 0, 1)
	ZEND_ARG_INFO(0, n_priorities)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base_loop, 0, 0, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_base_loopexit, 0, 0, 0)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event__construct, 0, 0, 4)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, what)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_set, 0, 0, 2)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, what)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_add, 0, 0, 0)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_config_avoid_method, 0, 0, 1)
	ZEND_ARG_INFO(0, method)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_config_require_features, 0, 0, 1)
	ZEND_ARG_INFO(0, feature)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_priority_set, 0, 0, 1)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_pending, 0, 0, 1)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_reinit, 0, 0, 1)
	ZEND_ARG_INFO(0, base)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evtimer_new, 0, 0, 2)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evtimer_set, 0, 0, 2)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_evsignal_new, 0, 0, 3)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, signum)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();


#if LIBEVENT_VERSION_NUMBER >= 0x02010000
ZEND_BEGIN_ARG_INFO_EX(arginfo_event_config_set_max_dispatch_interval, 0, 0, 3)
	ZEND_ARG_INFO(0, max_interval)
	ZEND_ARG_INFO(0, max_callbacks)
	ZEND_ARG_INFO(0, min_priority)
ZEND_END_ARG_INFO();
#endif


ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent__events, 0, 0, 1)
	ZEND_ARG_INFO(0, events)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent__construct, 0, 0, 1)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_connect, 0, 0, 2)
	ZEND_ARG_INFO(0, addr)
	ZEND_ARG_INFO(0, sync_resolve)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_socket_connect_hostname, 0, 0, 3)
	ZEND_ARG_INFO(0, dns_base)
	ZEND_ARG_INFO(0, hostname)
	ZEND_ARG_INFO(0, port)
	ZEND_ARG_INFO(0, family)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_set_callbacks, 0, 0, 3)
	ZEND_ARG_INFO(0, readcb)
	ZEND_ARG_INFO(0, writecb)
	ZEND_ARG_INFO(0, eventcb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_setwatermark, 0, 0, 3)
	ZEND_ARG_INFO(0, events)
	ZEND_ARG_INFO(0, lowmark)
	ZEND_ARG_INFO(0, highmark)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_write, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_write_buffer, 0, 0, 1)
	ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_read, 0, 0, 2)
	ZEND_ARG_INFO(1, data)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_pair_new, 0, 0, 2)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, events)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_priority_set, 0, 0, 1)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_bufferevent_set_timeouts, 0, 0, 2)
	ZEND_ARG_INFO(0, timeout_read)
	ZEND_ARG_INFO(0, timeout_write)
ZEND_END_ARG_INFO();



ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_freeze, 0, 0, 1)
	ZEND_ARG_INFO(0, at_front)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_add, 0, 0, 1)
	ZEND_ARG_INFO(0, data) 
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_add_buffer, 0, 0, 2)
	ZEND_ARG_INFO(0, outbuf)
	ZEND_ARG_INFO(0, inbuf) 
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evbuffer_remove, 0, 0, 2)
	ZEND_ARG_INFO(1, data)
	ZEND_ARG_INFO(0, max_bytes)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_event_socket_1, 0, 0, 0)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection__construct, 0, 0, 4)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, dns_base)
	ZEND_ARG_INFO(0, address)
	ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_get_peer, 0, 0, 2)
	ZEND_ARG_INFO(1, address)
	ZEND_ARG_INFO(1, port)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_local_address, 0, 0, 1)
	ZEND_ARG_INFO(0, address)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_local_port, 0, 0, 1)
	ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_timeout, 0, 0, 1)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_max_size, 0, 0, 1)
	ZEND_ARG_INFO(0, max_size)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_evhttp_connection_set_retries, 0, 0, 1)
	ZEND_ARG_INFO(0, retries)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http__construct, 0, 0, 1)
	ZEND_ARG_INFO(0, base)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_accept, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_http_bind, 0, 0, 2)
	ZEND_ARG_INFO(0, address)
	ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO();


/* ARGINFO END }}} */


#if HAVE_EVENT_EXTRA_LIB
/* {{{ ARGINFO for extra API */

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns__construct, 0, 0, 2)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, initialize)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_resolv_conf_parse, 0, 0, 2)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_nameserver_ip_add, 0, 0, 1)
	ZEND_ARG_INFO(0, ip)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_load_hosts, 0, 0, 1)
	ZEND_ARG_INFO(0, hosts)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_search_add, 0, 0, 1)
	ZEND_ARG_INFO(0, domain)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_search_ndots_set, 0, 0, 1)
	ZEND_ARG_INFO(0, ndots)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evdns_base_set_option, 0, 0, 2)
	ZEND_ARG_INFO(0, option)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();


ZEND_BEGIN_ARG_INFO_EX(arginfo_evconnlistener__construct, 0, 0, 5)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, backlog)
	ZEND_ARG_INFO(0, stream)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evconnlistener_new_bind, 0, 0, 5)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, backlog)
	ZEND_ARG_INFO(0, addr)
	ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evconnlistener_set_cb, 0, 0, 1)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_evconnlistener_set_error_cb, 0, 0, 1)
	ZEND_ARG_INFO(0, cb)
ZEND_END_ARG_INFO();


/* ARGINFO for extra API END }}} */
#endif


const zend_function_entry php_event_ce_functions[] = {/* {{{ */
	PHP_ME(Event, __construct,         arginfo_event__construct,   ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(Event, set,                 arginfo_event_set,          ZEND_ACC_PUBLIC)
	PHP_ME(Event, getSupportedMethods, arginfo_event__void,        ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(Event, add,                 arginfo_event_add,          ZEND_ACC_PUBLIC)
	PHP_ME(Event, del,                 arginfo_event__void,        ZEND_ACC_PUBLIC)
	PHP_ME(Event, setPriority,         arginfo_event_priority_set, ZEND_ACC_PUBLIC)
	PHP_ME(Event, pending,             arginfo_event_pending,      ZEND_ACC_PUBLIC)
	PHP_ME(Event, reInit,              arginfo_event_reinit,       ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
	PHP_ME(Event, removeTimer, arginfo_event__void, ZEND_ACC_PUBLIC)
#endif

	PHP_ME(Event, timer,        arginfo_evtimer_new,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Event, setTimer,     arginfo_evtimer_set,  ZEND_ACC_PUBLIC)
	PHP_ME(Event, timerPending, arginfo_event__void,  ZEND_ACC_PUBLIC)
	PHP_ME(Event, signal,       arginfo_evsignal_new, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

	PHP_MALIAS(Event, addTimer,  add, arginfo_event_add,   ZEND_ACC_PUBLIC)
	PHP_MALIAS(Event, delTimer,  del, arginfo_event__void, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Event, addSignal, add, arginfo_event_add,   ZEND_ACC_PUBLIC)
	PHP_MALIAS(Event, delSignal, del, arginfo_event__void, ZEND_ACC_PUBLIC)
													  
	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_base_ce_functions[] = {/* {{{ */
	PHP_ME(EventBase, __construct,        arginfo_event_base__construct,    ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventBase, getMethod,          arginfo_event_base_void,          ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, getFeatures,        arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, priorityInit,       arginfo_event_base_priority_init, ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, loop,               arginfo_event_base_loop,          ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, dispatch,           arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, exit,               arginfo_event_base_loopexit,      ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, stop,               arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, gotStop,            arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, gotExit,            arginfo_event__void,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBase, getTimeOfDayCached, arginfo_event__void,              ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
	PHP_ME(EventBase, updateCacheTime, arginfo_event__void, ZEND_ACC_PUBLIC)
#endif
														   
	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_config_ce_functions[] = {/* {{{ */
	PHP_ME(EventConfig, __construct,     arginfo_event__void,                   ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventConfig, avoidMethod,     arginfo_event_config_avoid_method,     ZEND_ACC_PUBLIC)
	PHP_ME(EventConfig, requireFeatures, arginfo_event_config_require_features, ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
	PHP_ME(EventConfig, setMaxDispatchInterval, arginfo_event_config_set_max_dispatch_interval, ZEND_ACC_PUBLIC)
#endif

	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_bevent_ce_functions[] = {/* {{{ */
	PHP_ME(EventBufferEvent, __construct,       arginfo_bufferevent__construct,              ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, connect,           arginfo_bufferevent_connect,                 ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, connectHost,       arginfo_bufferevent_socket_connect_hostname, ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, getDnsErrorString, arginfo_event__void,                         ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, setCallbacks,      arginfo_bufferevent_set_callbacks,           ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, enable,            arginfo_bufferevent__events,                 ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, disable,           arginfo_bufferevent__events,                 ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, getEnabled,        arginfo_event__void,                         ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, getInput,          arginfo_event__void,                         ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, getOutput,         arginfo_event__void,                         ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, setWatermark,      arginfo_bufferevent_setwatermark,            ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, write,             arginfo_bufferevent_write,                   ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, writeBuffer,       arginfo_bufferevent_write_buffer,            ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, read,              arginfo_bufferevent_read,                    ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, readBuffer,        arginfo_bufferevent_write_buffer,            ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, createPair,        arginfo_bufferevent_pair_new,                ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, setPriority,       arginfo_bufferevent_priority_set,            ZEND_ACC_PUBLIC)
	PHP_ME(EventBufferEvent, setTimeouts,       arginfo_bufferevent_set_timeouts,            ZEND_ACC_PUBLIC)

	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_buffer_ce_functions[] = {/* {{{ */
	PHP_ME(EventBuffer, __construct,   arginfo_event__void,         ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventBuffer, freeze,        arginfo_evbuffer_freeze,     ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, unfreeze,      arginfo_evbuffer_freeze,     ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, getLength,     arginfo_event__void,         ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, lock,          arginfo_event__void,         ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, unlock,        arginfo_event__void,         ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, enableLocking, arginfo_event__void,         ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, add,           arginfo_evbuffer_add,        ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, addBuffer,     arginfo_evbuffer_add_buffer, ZEND_ACC_PUBLIC)
	PHP_ME(EventBuffer, remove,        arginfo_evbuffer_remove,     ZEND_ACC_PUBLIC)

	PHP_FE_END
};
/* }}} */

const zend_function_entry php_event_util_ce_functions[] = {/* {{{ */
	PHP_ABSTRACT_ME(EventUtil, __construct, NULL)

	PHP_ME(EventUtil, getLastSocketErrno, arginfo_event_socket_1, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(EventUtil, getLastSocketError, arginfo_event_socket_1, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

	PHP_FE_END
};
/* }}} */

#if HAVE_EVENT_EXTRA_LIB
/* {{{ Extra API */

const zend_function_entry php_event_dns_base_ce_functions[] = {
	PHP_ME(EventDnsBase, __construct,      arginfo_evdns__construct,             ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventDnsBase, parseResolvConf,  arginfo_evdns_resolv_conf_parse,      ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, addNameserverIp,  arginfo_evdns_base_nameserver_ip_add, ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, loadHosts,        arginfo_evdns_base_load_hosts,        ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, clearSearch,      arginfo_event__void,                  ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, addSearch,        arginfo_evdns_base_search_add,        ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, setSearchNdots,   arginfo_evdns_base_search_ndots_set,  ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, setOption,        arginfo_evdns_base_set_option,        ZEND_ACC_PUBLIC)
	PHP_ME(EventDnsBase, countNameservers, arginfo_event__void,                  ZEND_ACC_PUBLIC)

	PHP_FE_END
};

const zend_function_entry php_event_http_conn_ce_functions[] = {
	PHP_ME(EventHttpConnection, __construct,       arginfo_event_evhttp_connection__construct,        ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventHttpConnection, getBase,           arginfo_event__void,                               ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, getPeer,           arginfo_event_evhttp_connection_get_peer,          ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setLocalAddress,   arginfo_event_evhttp_connection_set_local_address, ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setLocalPort,      arginfo_event_evhttp_connection_set_local_port,    ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setTimeout,        arginfo_event_evhttp_connection_set_timeout,       ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setMaxHeadersSize, arginfo_event_evhttp_connection_set_max_size,      ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setMaxBodySize,    arginfo_event_evhttp_connection_set_max_size,      ZEND_ACC_PUBLIC)
	PHP_ME(EventHttpConnection, setRetries,        arginfo_event_evhttp_connection_set_retries,       ZEND_ACC_PUBLIC)

	PHP_FE_END
};

const zend_function_entry php_event_http_ce_functions[] = {
	PHP_ME(EventHttp, __construct, arginfo_event_http__construct, ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventHttp, accept,      arginfo_event_http_accept,     ZEND_ACC_PUBLIC)
	PHP_ME(EventHttp, bind,        arginfo_event_http_bind,       ZEND_ACC_PUBLIC)

	PHP_FE_END
};

const zend_function_entry php_event_listener_ce_functions[] = {
	PHP_ME(EventListener, __construct,      arginfo_evconnlistener__construct,   ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EventListener, enable,           arginfo_event__void,                 ZEND_ACC_PUBLIC)
	PHP_ME(EventListener, disable,          arginfo_event__void,                 ZEND_ACC_PUBLIC)
	PHP_ME(EventListener, setCallback,      arginfo_evconnlistener_set_cb,       ZEND_ACC_PUBLIC)
	PHP_ME(EventListener, setErrorCallback, arginfo_evconnlistener_set_error_cb, ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02000300
	PHP_ME(EventListener, getBase, arginfo_event__void, ZEND_ACC_PUBLIC)
#endif

	PHP_FE_END
};

/* Extra API END}}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */

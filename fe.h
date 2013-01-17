/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2011 The PHP Group                                |
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
#ifndef PHP_EVENT_FE_H
#define PHP_EVENT_FE_H

#include "common.h"

PHP_FUNCTION(event_new);
PHP_FUNCTION(event_set);
PHP_FUNCTION(event_add);
PHP_FUNCTION(event_del);
PHP_FUNCTION(event_remove_timer);
PHP_FUNCTION(event_priority_set);
PHP_FUNCTION(event_pending);
PHP_FUNCTION(event_reinit);
PHP_FUNCTION(event_free);

PHP_FUNCTION(evtimer_new);
PHP_FUNCTION(evtimer_set);
PHP_FUNCTION(evtimer_pending);

PHP_FUNCTION(evsignal_new);

PHP_FUNCTION(event_base_new);
PHP_FUNCTION(event_base_new_with_config);
PHP_FUNCTION(event_base_get_method);
PHP_FUNCTION(event_base_get_features);
PHP_FUNCTION(event_base_priority_init);
PHP_FUNCTION(event_base_loop);
PHP_FUNCTION(event_base_loopexit);
PHP_FUNCTION(event_base_loopbreak);
PHP_FUNCTION(event_base_dispatch);
PHP_FUNCTION(event_base_got_break);
PHP_FUNCTION(event_base_got_exit);
PHP_FUNCTION(event_base_gettimeofday_cached);
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
PHP_FUNCTION(event_base_update_cache_time);
#endif
PHP_FUNCTION(event_base_free);

PHP_FUNCTION(event_get_supported_methods);

PHP_FUNCTION(event_config_new);
PHP_FUNCTION(event_config_avoid_method);
PHP_FUNCTION(event_config_require_features);
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
PHP_FUNCTION(event_config_set_max_dispatch_interval);
#endif

PHP_FUNCTION(bufferevent_socket_new);
PHP_FUNCTION(bufferevent_pair_new);
PHP_FUNCTION(bufferevent_free);
PHP_FUNCTION(bufferevent_socket_connect);
PHP_FUNCTION(bufferevent_socket_connect_hostname);
PHP_FUNCTION(bufferevent_setcb);
PHP_FUNCTION(bufferevent_enable);
PHP_FUNCTION(bufferevent_disable);
PHP_FUNCTION(bufferevent_get_enabled);
PHP_FUNCTION(bufferevent_get_input);
PHP_FUNCTION(bufferevent_get_output);
PHP_FUNCTION(bufferevent_setwatermark);
PHP_FUNCTION(bufferevent_socket_get_dns_error);
PHP_FUNCTION(bufferevent_write);
PHP_FUNCTION(bufferevent_write_buffer);
PHP_FUNCTION(bufferevent_read);
PHP_FUNCTION(bufferevent_read_buffer);
PHP_FUNCTION(bufferevent_priority_set);
PHP_FUNCTION(bufferevent_set_timeouts);

PHP_FUNCTION(evbuffer_new);
PHP_FUNCTION(evbuffer_free);
PHP_FUNCTION(evbuffer_freeze);
PHP_FUNCTION(evbuffer_unfreeze);
PHP_FUNCTION(evbuffer_get_length);
PHP_FUNCTION(evbuffer_lock);
PHP_FUNCTION(evbuffer_unlock);
PHP_FUNCTION(evbuffer_enable_locking);
PHP_FUNCTION(evbuffer_add);
PHP_FUNCTION(evbuffer_remove);
PHP_FUNCTION(evbuffer_add_buffer);

PHP_FUNCTION(event_socket_get_last_errno);
PHP_FUNCTION(event_socket_get_last_error);

#if HAVE_EVENT_EXTRA_LIB
/* {{{ Extra API */

PHP_FUNCTION(evdns_base_new);
PHP_FUNCTION(evdns_base_free);
PHP_FUNCTION(evdns_base_resolv_conf_parse);
PHP_FUNCTION(evdns_base_nameserver_ip_add);
PHP_FUNCTION(evdns_base_load_hosts);
PHP_FUNCTION(evdns_base_search_clear);
PHP_FUNCTION(evdns_base_search_add);
PHP_FUNCTION(evdns_base_search_ndots_set);
PHP_FUNCTION(evdns_base_set_option);
PHP_FUNCTION(evdns_base_count_nameservers);

PHP_FUNCTION(evconnlistener_new);
PHP_FUNCTION(evconnlistener_new_bind);
PHP_FUNCTION(evconnlistener_free);
PHP_FUNCTION(evconnlistener_enable);
PHP_FUNCTION(evconnlistener_disable);
PHP_FUNCTION(evconnlistener_set_cb);
PHP_FUNCTION(evconnlistener_set_error_cb);
#if LIBEVENT_VERSION_NUMBER >= 0x02000300
PHP_FUNCTION(evconnlistener_get_base);
#endif

/* Extra API END }}} */
#endif

#endif /* PHP_EVENT_FE_H */

/* 
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */

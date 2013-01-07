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
PHP_FUNCTION(event_free);

PHP_FUNCTION(evtimer_new);
PHP_FUNCTION(evtimer_set);
PHP_FUNCTION(evtimer_pending);

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
PHP_FUNCTION(bufferevent_free);
PHP_FUNCTION(bufferevent_socket_connect);
PHP_FUNCTION(bufferevent_setcb);
PHP_FUNCTION(bufferevent_enable);
PHP_FUNCTION(bufferevent_disable);
PHP_FUNCTION(bufferevent_get_enabled);
PHP_FUNCTION(bufferevent_set_watermark);
PHP_FUNCTION(bufferevent_socket_get_dns_error);


#if HAVE_EVENT_EXTRA_LIB
/* {{{ Extra API */

PHP_FUNCTION(event_dns_base_new);
PHP_FUNCTION(event_dns_base_free);

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

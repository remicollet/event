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
#ifndef PHP_EVENT_COMMON_H
#define PHP_EVENT_COMMON_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <php_network.h>
#include <php_streams.h>

#include <signal.h>

#include "php_event.h"
#include "structs.h"
#if PHP_VERSION_ID >= 50301 && (HAVE_SOCKETS || defined(COMPILE_DL_SOCKETS))
# include <ext/sockets/php_sockets.h>
# define PHP_EVENT_SOCKETS_SUPPORT
#else
# error "No sockets!"
#endif

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#ifdef HAVE_HAVE_LIBEVENT_EXTRA
# include <event2/dns.h>
# include <event2/http.h>
# include <event2/rpc.h>
# include <event2/tag.h>
#endif

#if !defined(LIBEVENT_VERSION_NUMBER) || LIBEVENT_VERSION_NUMBER < 0x02000100
# error "This version of Libevent is not supported; get 2.0.1-alpha or later."
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

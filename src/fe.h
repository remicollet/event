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

PHP_METHOD(Event, __construct);
PHP_METHOD(Event, free);
PHP_METHOD(Event, set);
PHP_METHOD(Event, add);
PHP_METHOD(Event, del);
PHP_METHOD(Event, removeTimer);
PHP_METHOD(Event, setPriority);
PHP_METHOD(Event, pending);
PHP_METHOD(Event, getSupportedMethods);

PHP_METHOD(Event, timer);
PHP_METHOD(Event, setTimer);
PHP_METHOD(Event, timerPending);

PHP_METHOD(Event, signal);

PHP_METHOD(EventBase, __construct);
PHP_METHOD(EventBase, getMethod);
PHP_METHOD(EventBase, getFeatures);
PHP_METHOD(EventBase, priorityInit);
PHP_METHOD(EventBase, loop);
PHP_METHOD(EventBase, exit);
PHP_METHOD(EventBase, stop);
PHP_METHOD(EventBase, dispatch);
PHP_METHOD(EventBase, gotStop);
PHP_METHOD(EventBase, gotExit);
PHP_METHOD(EventBase, getTimeOfDayCached);
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
PHP_METHOD(EventBase, updateCacheTime);
#endif
PHP_METHOD(EventBase, reInit);

PHP_METHOD(EventConfig, __construct);
PHP_METHOD(EventConfig, avoidMethod);
PHP_METHOD(EventConfig, requireFeatures);
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
PHP_METHOD(EventConfig, setMaxDispatchInterval);
#endif

PHP_METHOD(EventBufferEvent, __construct);
PHP_METHOD(EventBufferEvent, free);
PHP_METHOD(EventBufferEvent, createPair);
PHP_METHOD(EventBufferEvent, connect);
PHP_METHOD(EventBufferEvent, connectHost);
PHP_METHOD(EventBufferEvent, setCallbacks);
PHP_METHOD(EventBufferEvent, enable);
PHP_METHOD(EventBufferEvent, disable);
PHP_METHOD(EventBufferEvent, getEnabled);
PHP_METHOD(EventBufferEvent, getInput);
PHP_METHOD(EventBufferEvent, getOutput);
PHP_METHOD(EventBufferEvent, setWatermark);
PHP_METHOD(EventBufferEvent, getDnsErrorString);
PHP_METHOD(EventBufferEvent, write);
PHP_METHOD(EventBufferEvent, writeBuffer);
PHP_METHOD(EventBufferEvent, read);
PHP_METHOD(EventBufferEvent, readBuffer);
PHP_METHOD(EventBufferEvent, setPriority);
PHP_METHOD(EventBufferEvent, setTimeouts);

PHP_METHOD(EventBuffer, __construct);
PHP_METHOD(EventBuffer, freeze);
PHP_METHOD(EventBuffer, unfreeze);
PHP_METHOD(EventBuffer, getLength);
PHP_METHOD(EventBuffer, lock);
PHP_METHOD(EventBuffer, unlock);
PHP_METHOD(EventBuffer, enableLocking);
PHP_METHOD(EventBuffer, add);
PHP_METHOD(EventBuffer, remove);
PHP_METHOD(EventBuffer, addBuffer);

PHP_METHOD(EventUtil, getLastSocketErrno);
PHP_METHOD(EventUtil, getLastSocketError);

#if HAVE_EVENT_EXTRA_LIB
/* {{{ Extra API */

PHP_METHOD(EventDnsBase, __construct);
PHP_METHOD(EventDnsBase, parseResolvConf);
PHP_METHOD(EventDnsBase, addNameserverIp);
PHP_METHOD(EventDnsBase, loadHosts);
PHP_METHOD(EventDnsBase, clearSearch);
PHP_METHOD(EventDnsBase, addSearch);
PHP_METHOD(EventDnsBase, setSearchNdots);
PHP_METHOD(EventDnsBase, setOption);
PHP_METHOD(EventDnsBase, countNameservers);

PHP_METHOD(EventListener, __construct);
PHP_METHOD(EventListener, enable);
PHP_METHOD(EventListener, disable);
PHP_METHOD(EventListener, setCallback);
PHP_METHOD(EventListener, setErrorCallback);
#if LIBEVENT_VERSION_NUMBER >= 0x02000300
PHP_METHOD(EventListener, getBase);
#endif

PHP_METHOD(EventHttpConnection, __construct);
PHP_METHOD(EventHttpConnection, getBase);
PHP_METHOD(EventHttpConnection, getPeer);
PHP_METHOD(EventHttpConnection, setLocalAddress);
PHP_METHOD(EventHttpConnection, setLocalPort);
PHP_METHOD(EventHttpConnection, setTimeout);
PHP_METHOD(EventHttpConnection, setMaxHeadersSize);
PHP_METHOD(EventHttpConnection, setMaxBodySize);
PHP_METHOD(EventHttpConnection, setRetries);

PHP_METHOD(EventHttp, __construct);
PHP_METHOD(EventHttp, accept);
PHP_METHOD(EventHttp, bind);

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

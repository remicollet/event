/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 2fc5b99790e3e24e20dd70d2ed7b94579a7d65d9 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventConfig___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventConfig___sleep arginfo_class_SomeNs_EventConfig___construct

#define arginfo_class_SomeNs_EventConfig___wakeup arginfo_class_SomeNs_EventConfig___construct

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventConfig_avoidMethod, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, cfg, SomeNs\\EventConfig, 0)
	ZEND_ARG_TYPE_INFO(0, method, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventConfig_requireFeatures, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, feature, IS_LONG, 0)
ZEND_END_ARG_INFO()

#if LIBEVENT_VERSION_NUMBER >= 0x02010000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventConfig_setMaxDispatchInterval, 0, 3, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, max_interval, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, max_callbacks, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, min_priority, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if LIBEVENT_VERSION_NUMBER >= 0x02000201 /* 2.0.2-alpha */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventConfig_setFlags, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventBase___construct, 0, 0, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cfg, SomeNs\\EventConfig, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBase___sleep arginfo_class_SomeNs_EventConfig___construct

#define arginfo_class_SomeNs_EventBase___wakeup arginfo_class_SomeNs_EventConfig___construct

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_getMethod, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_getFeatures, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_priorityInit, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, n_priorities, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_loop, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_dispatch, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_exit, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 0, "0.0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_set, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, event, SomeNs\\Event, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBase_stop arginfo_class_SomeNs_EventBase_dispatch

#define arginfo_class_SomeNs_EventBase_gotStop arginfo_class_SomeNs_EventBase_dispatch

#define arginfo_class_SomeNs_EventBase_gotExit arginfo_class_SomeNs_EventBase_dispatch

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_getTimeOfDayCached, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBase_reInit arginfo_class_SomeNs_EventBase_dispatch

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_free, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

#if LIBEVENT_VERSION_NUMBER >= 0x02010100
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_updateCacheTime, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

#if LIBEVENT_VERSION_NUMBER >= 0x02010200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBase_resume, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_Event___construct, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, what, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_Event_free arginfo_class_SomeNs_EventBase_free

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_Event_set, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, what, IS_LONG, 0, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cb, IS_CALLABLE, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_Event_getSupportedMethods, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_Event_add, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 0, "-1")
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_Event_del arginfo_class_SomeNs_EventBase_dispatch

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_Event_setPriority, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, priority, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_Event_pending, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

#if LIBEVENT_VERSION_NUMBER >= 0x02010200
#define arginfo_class_SomeNs_Event_removeTimer arginfo_class_SomeNs_EventBase_resume
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SomeNs_Event_timer, 0, 2, SomeNs\\Event, 0)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_Event_setTimer, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SomeNs_Event_signal, 0, 3, SomeNs\\Event, 0)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, signum, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_Event_addTimer arginfo_class_SomeNs_Event_add

#define arginfo_class_SomeNs_Event_delTimer arginfo_class_SomeNs_EventBase_dispatch

#define arginfo_class_SomeNs_Event_addSignal arginfo_class_SomeNs_Event_add

#define arginfo_class_SomeNs_Event_delSignal arginfo_class_SomeNs_EventBase_dispatch

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventDnsBase___construct, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, initialize, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventDnsBase_parseResolvConf, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventDnsBase_addNameserverIp, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, ip, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventDnsBase_loadHosts, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, hosts, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventDnsBase_clearSearch, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventDnsBase_addSearch, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventDnsBase_setSearchNdots, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, ndots, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventDnsBase_setOption, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventDnsBase_countNameservers, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventHttpConnection___construct, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_OBJ_INFO(0, dns_base, SomeNs\\EventDnsBase, 1)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, ctx, SomeNs\\EventSslContext, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventHttpConnection___construct, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_OBJ_INFO(0, dns_base, SomeNs\\EventDnsBase, 1)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventHttpConnection___sleep, 0, 0, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpConnection___wakeup arginfo_class_SomeNs_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_SomeNs_EventHttpConnection_getBase, 0, 0, SomeNs\\EventBase, MAY_BE_FALSE)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpConnection_getPeer, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(1, address, IS_MIXED, 1)
	ZEND_ARG_TYPE_INFO(1, port, IS_MIXED, 1)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpConnection_setLocalAddress, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpConnection_setLocalPort, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpConnection_setTimeout, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpConnection_setMaxHeadersSize, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, max_size, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpConnection_setMaxBodySize arginfo_class_SomeNs_EventHttpConnection_setMaxHeadersSize
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpConnection_setRetries, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, retries, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpConnection_makeRequest, 0, 3, _IS_BOOL, 1)
	ZEND_ARG_OBJ_INFO(0, req, SomeNs\\EventHttpRequest, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpConnection_setCloseCallback, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventHttp___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, ctx, SomeNs\\EventSslContext, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventHttp___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttp___sleep arginfo_class_SomeNs_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttp___wakeup arginfo_class_SomeNs_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttp_accept, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttp_bind, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttp_setCallback, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttp_setDefaultCallback, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttp_setAllowedMethods, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, methods, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttp_setMaxBodySize, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttp_setMaxHeadersSize arginfo_class_SomeNs_EventHttp_setMaxBodySize
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttp_setTimeout arginfo_class_SomeNs_EventHttp_setMaxBodySize
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttp_addServerAlias, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, alias, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttp_removeServerAlias arginfo_class_SomeNs_EventHttp_addServerAlias
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventHttpRequest___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest___sleep arginfo_class_SomeNs_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest___wakeup arginfo_class_SomeNs_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_free arginfo_class_SomeNs_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_getCommand arginfo_class_SomeNs_EventDnsBase_countNameservers
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_getHost, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_getUri arginfo_class_SomeNs_EventHttpRequest_getHost
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_getResponseCode arginfo_class_SomeNs_EventDnsBase_countNameservers
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_getInputHeaders, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_getOutputHeaders arginfo_class_SomeNs_EventHttpRequest_getInputHeaders
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_getInputBuffer, 0, 0, SomeNs\\EventBuffer, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_getOutputBuffer arginfo_class_SomeNs_EventHttpRequest_getInputBuffer
#endif

#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02001100
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_getBufferEvent, 0, 0, SomeNs\\EventBufferEvent, 1)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_getConnection, 0, 0, SomeNs\\EventHttpConnection, 1)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_closeConnection arginfo_class_SomeNs_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_sendError, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, error, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, reason, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_sendReply, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, code, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, reason, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, buf, SomeNs\\EventBuffer, 1, "null")
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_sendReplyChunk, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, buf, SomeNs\\EventBuffer, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_sendReplyEnd arginfo_class_SomeNs_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_sendReplyStart, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, code, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, reason, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_cancel arginfo_class_SomeNs_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_addHeader, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventHttpRequest_clearHeaders arginfo_class_SomeNs_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_removeHeader, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventHttpRequest_findHeader, 0, 2, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventListener___construct, 0, 0, 6)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 1)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, backlog, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, target, IS_MIXED, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventListener___sleep arginfo_class_SomeNs_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventListener___wakeup arginfo_class_SomeNs_EventHttpConnection___sleep
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventListener_free arginfo_class_SomeNs_EventDnsBase_clearSearch
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventListener_enable, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventListener_disable arginfo_class_SomeNs_EventListener_enable
#endif

#if HAVE_EVENT_EXTRA_LIB
#define arginfo_class_SomeNs_EventListener_setCallback arginfo_class_SomeNs_EventHttp_setDefaultCallback
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventListener_setErrorCallback, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, cb, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02000300
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SomeNs_EventListener_getBase, 0, 0, SomeNs\\EventBase, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_EVENT_EXTRA_LIB
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventListener_getSocketName, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(1, address, IS_MIXED, 1)
	ZEND_ARG_TYPE_INFO(1, port, IS_MIXED, 1)
ZEND_END_ARG_INFO()
#endif

#define arginfo_class_SomeNs_EventUtil___construct arginfo_class_SomeNs_EventConfig___construct

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_SomeNs_EventUtil_getLastSocketErrno, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, socket, Socket, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_SomeNs_EventUtil_getLastSocketError, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, socket, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventUtil_sslRandPoll, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventUtil_getSocketName, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(1, address, IS_MIXED, 1)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(1, port, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventUtil_getSocketFd, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventUtil_setSocketOption, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, optname, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, optval, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#if defined(PHP_EVENT_SOCKETS_SUPPORT)
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_SomeNs_EventUtil_createSocket, 0, 1, Socket, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#define arginfo_class_SomeNs_EventBuffer___construct arginfo_class_SomeNs_EventConfig___construct

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_freeze, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, at_front, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBuffer_unfreeze arginfo_class_SomeNs_EventBuffer_freeze

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_lock, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, at_front, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBuffer_unlock arginfo_class_SomeNs_EventBuffer_lock

#define arginfo_class_SomeNs_EventBuffer_enableLocking arginfo_class_SomeNs_EventBase_free

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_add, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_read, 0, 1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, max_bytes, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_addBuffer, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, buf, SomeNs\\EventBuffer, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_appendFrom, 0, 2, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, buf, SomeNs\\EventBuffer, 0)
	ZEND_ARG_TYPE_INFO(0, len, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_expand, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, len, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBuffer_prepend arginfo_class_SomeNs_EventBuffer_add

#define arginfo_class_SomeNs_EventBuffer_prependBuffer arginfo_class_SomeNs_EventBuffer_addBuffer

#define arginfo_class_SomeNs_EventBuffer_drain arginfo_class_SomeNs_EventBuffer_expand

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_copyout, 0, 2, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(1, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, max_bytes, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_readLine, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, eol_style, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_SomeNs_EventBuffer_search, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, what, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, end, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_SomeNs_EventBuffer_searchEol, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, eol_style, IS_LONG, 0, "SomeNs\\EventBuffer::EOL_ANY")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBuffer_pullup, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_SomeNs_EventBuffer_write, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, howmuch, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBuffer_readFrom arginfo_class_SomeNs_EventBuffer_write

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_SomeNs_EventBuffer_substr, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, start, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventBufferEvent___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, socket, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, readcb, IS_CALLABLE, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, writecb, IS_CALLABLE, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, eventcb, IS_CALLABLE, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBufferEvent_free arginfo_class_SomeNs_EventBase_free

#define arginfo_class_SomeNs_EventBufferEvent_close arginfo_class_SomeNs_EventBase_free

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_connect, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, addr, IS_STRING, 0)
ZEND_END_ARG_INFO()

#if defined(HAVE_EVENT_EXTRA_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_connectHost, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, dns_base, SomeNs\\EventDnsBase, 1)
	ZEND_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, family, IS_LONG, 0, "SomeNs\\EventUtil::AF_UNSPEC")
ZEND_END_ARG_INFO()
#endif

#if !(defined(HAVE_EVENT_EXTRA_LIB))
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_connectHost, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, unused, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, family, IS_LONG, 0, "SomeNs\\EventUtil::AF_UNSPEC")
ZEND_END_ARG_INFO()
#endif

#define arginfo_class_SomeNs_EventBufferEvent_getDnsErrorString arginfo_class_SomeNs_EventBase_getMethod

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_setCallbacks, 0, 3, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, readcb, IS_CALLABLE, 1)
	ZEND_ARG_TYPE_INFO(0, writecb, IS_CALLABLE, 1)
	ZEND_ARG_TYPE_INFO(0, eventcb, IS_CALLABLE, 1)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_enable, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, events, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBufferEvent_disable arginfo_class_SomeNs_EventBufferEvent_enable

#define arginfo_class_SomeNs_EventBufferEvent_getEnabled arginfo_class_SomeNs_EventBase_getFeatures

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_getInput, 0, 0, SomeNs\\EventBuffer, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBufferEvent_getOutput arginfo_class_SomeNs_EventBufferEvent_getInput

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_setWatermark, 0, 3, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, events, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, lowmark, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, highmark, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBufferEvent_write arginfo_class_SomeNs_EventBuffer_add

#define arginfo_class_SomeNs_EventBufferEvent_writeBuffer arginfo_class_SomeNs_EventBuffer_addBuffer

#define arginfo_class_SomeNs_EventBufferEvent_read arginfo_class_SomeNs_EventBuffer_pullup

#define arginfo_class_SomeNs_EventBufferEvent_readBuffer arginfo_class_SomeNs_EventBuffer_addBuffer

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_SomeNs_EventBufferEvent_createPair, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_class_SomeNs_EventBufferEvent_setPriority arginfo_class_SomeNs_Event_setPriority

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_setTimeouts, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, timeout_read, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, timeout_write, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_createSslFilter, 0, 3, SomeNs\\EventBufferEvent, 0)
	ZEND_ARG_OBJ_INFO(0, unnderlying, SomeNs\\EventBufferEvent, 0)
	ZEND_ARG_OBJ_INFO(0, ctx, SomeNs\\EventSslContext, 0)
	ZEND_ARG_TYPE_INFO(0, state, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_sslSocket, 0, 4, SomeNs\\EventBufferEvent, 0)
	ZEND_ARG_OBJ_INFO(0, base, SomeNs\\EventBase, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO(0, ctx, SomeNs\\EventSslContext, 0)
	ZEND_ARG_TYPE_INFO(0, state, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_sslError, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventBufferEvent_sslRenegotiate, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
#define arginfo_class_SomeNs_EventBufferEvent_sslGetCipherInfo arginfo_class_SomeNs_EventBufferEvent_sslError
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
#define arginfo_class_SomeNs_EventBufferEvent_sslGetCipherName arginfo_class_SomeNs_EventBufferEvent_sslError
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
#define arginfo_class_SomeNs_EventBufferEvent_sslGetCipherVersion arginfo_class_SomeNs_EventBufferEvent_sslError
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
#define arginfo_class_SomeNs_EventBufferEvent_sslGetProtocol arginfo_class_SomeNs_EventBufferEvent_sslError
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SomeNs_EventSslContext___construct, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, method, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SomeNs_EventSslContext_setMinProtoVersion, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, proto, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
#define arginfo_class_SomeNs_EventSslContext_setMaxProtoVersion arginfo_class_SomeNs_EventSslContext_setMinProtoVersion
#endif


ZEND_METHOD(SomeNs_EventConfig, __construct);
ZEND_METHOD(SomeNs_EventConfig, __sleep);
ZEND_METHOD(SomeNs_EventConfig, __wakeup);
ZEND_METHOD(SomeNs_EventConfig, avoidMethod);
ZEND_METHOD(SomeNs_EventConfig, requireFeatures);
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
ZEND_METHOD(SomeNs_EventConfig, setMaxDispatchInterval);
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02000201 /* 2.0.2-alpha */
ZEND_METHOD(SomeNs_EventConfig, setFlags);
#endif
ZEND_METHOD(SomeNs_EventBase, __construct);
ZEND_METHOD(SomeNs_EventBase, __sleep);
ZEND_METHOD(SomeNs_EventBase, __wakeup);
ZEND_METHOD(SomeNs_EventBase, getMethod);
ZEND_METHOD(SomeNs_EventBase, getFeatures);
ZEND_METHOD(SomeNs_EventBase, priorityInit);
ZEND_METHOD(SomeNs_EventBase, loop);
ZEND_METHOD(SomeNs_EventBase, dispatch);
ZEND_METHOD(SomeNs_EventBase, exit);
ZEND_METHOD(SomeNs_EventBase, set);
ZEND_METHOD(SomeNs_EventBase, stop);
ZEND_METHOD(SomeNs_EventBase, gotStop);
ZEND_METHOD(SomeNs_EventBase, gotExit);
ZEND_METHOD(SomeNs_EventBase, getTimeOfDayCached);
ZEND_METHOD(SomeNs_EventBase, reInit);
ZEND_METHOD(SomeNs_EventBase, free);
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
ZEND_METHOD(SomeNs_EventBase, updateCacheTime);
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
ZEND_METHOD(SomeNs_EventBase, resume);
#endif
ZEND_METHOD(SomeNs_Event, __construct);
ZEND_METHOD(SomeNs_Event, free);
ZEND_METHOD(SomeNs_Event, set);
ZEND_METHOD(SomeNs_Event, getSupportedMethods);
ZEND_METHOD(SomeNs_Event, add);
ZEND_METHOD(SomeNs_Event, del);
ZEND_METHOD(SomeNs_Event, setPriority);
ZEND_METHOD(SomeNs_Event, pending);
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
ZEND_METHOD(SomeNs_Event, removeTimer);
#endif
ZEND_METHOD(SomeNs_Event, timer);
ZEND_METHOD(SomeNs_Event, setTimer);
ZEND_METHOD(SomeNs_Event, signal);
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventDnsBase, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventDnsBase, parseResolvConf);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventDnsBase, addNameserverIp);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventDnsBase, loadHosts);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventDnsBase, clearSearch);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventDnsBase, addSearch);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventDnsBase, setSearchNdots);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventDnsBase, setOption);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventDnsBase, countNameservers);
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventHttpConnection, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
ZEND_METHOD(SomeNs_EventHttpConnection, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, __sleep);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, __wakeup);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, getBase);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, getPeer);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, setLocalAddress);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, setLocalPort);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, setTimeout);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, setMaxHeadersSize);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, setMaxBodySize);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, setRetries);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, makeRequest);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpConnection, setCloseCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventHttp, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
ZEND_METHOD(SomeNs_EventHttp, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, __sleep);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, __wakeup);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, accept);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, bind);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, setCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, setDefaultCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, setAllowedMethods);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, setMaxBodySize);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, setMaxHeadersSize);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, setTimeout);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, addServerAlias);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttp, removeServerAlias);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, __sleep);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, __wakeup);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, free);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, getCommand);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, getHost);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, getUri);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, getResponseCode);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, getInputHeaders);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, getOutputHeaders);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, getInputBuffer);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, getOutputBuffer);
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02001100
ZEND_METHOD(SomeNs_EventHttpRequest, getBufferEvent);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, getConnection);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, closeConnection);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, sendError);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, sendReply);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, sendReplyChunk);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, sendReplyEnd);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, sendReplyStart);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, cancel);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, addHeader);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, clearHeaders);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, removeHeader);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventHttpRequest, findHeader);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventListener, __construct);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventListener, __sleep);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventListener, __wakeup);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventListener, free);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventListener, enable);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventListener, disable);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventListener, setCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventListener, setErrorCallback);
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02000300
ZEND_METHOD(SomeNs_EventListener, getBase);
#endif
#if HAVE_EVENT_EXTRA_LIB
ZEND_METHOD(SomeNs_EventListener, getSocketName);
#endif
ZEND_METHOD(SomeNs_EventUtil, __construct);
ZEND_METHOD(SomeNs_EventUtil, getLastSocketErrno);
ZEND_METHOD(SomeNs_EventUtil, getLastSocketError);
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventUtil, sslRandPoll);
#endif
ZEND_METHOD(SomeNs_EventUtil, getSocketName);
ZEND_METHOD(SomeNs_EventUtil, getSocketFd);
ZEND_METHOD(SomeNs_EventUtil, setSocketOption);
#if defined(PHP_EVENT_SOCKETS_SUPPORT)
ZEND_METHOD(SomeNs_EventUtil, createSocket);
#endif
ZEND_METHOD(SomeNs_EventBuffer, __construct);
ZEND_METHOD(SomeNs_EventBuffer, freeze);
ZEND_METHOD(SomeNs_EventBuffer, unfreeze);
ZEND_METHOD(SomeNs_EventBuffer, lock);
ZEND_METHOD(SomeNs_EventBuffer, unlock);
ZEND_METHOD(SomeNs_EventBuffer, enableLocking);
ZEND_METHOD(SomeNs_EventBuffer, add);
ZEND_METHOD(SomeNs_EventBuffer, read);
ZEND_METHOD(SomeNs_EventBuffer, addBuffer);
ZEND_METHOD(SomeNs_EventBuffer, appendFrom);
ZEND_METHOD(SomeNs_EventBuffer, expand);
ZEND_METHOD(SomeNs_EventBuffer, prepend);
ZEND_METHOD(SomeNs_EventBuffer, prependBuffer);
ZEND_METHOD(SomeNs_EventBuffer, drain);
ZEND_METHOD(SomeNs_EventBuffer, copyout);
ZEND_METHOD(SomeNs_EventBuffer, readLine);
ZEND_METHOD(SomeNs_EventBuffer, search);
ZEND_METHOD(SomeNs_EventBuffer, searchEol);
ZEND_METHOD(SomeNs_EventBuffer, pullup);
ZEND_METHOD(SomeNs_EventBuffer, write);
ZEND_METHOD(SomeNs_EventBuffer, readFrom);
ZEND_METHOD(SomeNs_EventBuffer, substr);
ZEND_METHOD(SomeNs_EventBufferEvent, __construct);
ZEND_METHOD(SomeNs_EventBufferEvent, free);
ZEND_METHOD(SomeNs_EventBufferEvent, close);
ZEND_METHOD(SomeNs_EventBufferEvent, connect);
#if defined(HAVE_EVENT_EXTRA_LIB)
ZEND_METHOD(SomeNs_EventBufferEvent, connectHost);
#endif
#if !(defined(HAVE_EVENT_EXTRA_LIB))
ZEND_METHOD(SomeNs_EventBufferEvent, connectHost);
#endif
ZEND_METHOD(SomeNs_EventBufferEvent, getDnsErrorString);
ZEND_METHOD(SomeNs_EventBufferEvent, setCallbacks);
ZEND_METHOD(SomeNs_EventBufferEvent, enable);
ZEND_METHOD(SomeNs_EventBufferEvent, disable);
ZEND_METHOD(SomeNs_EventBufferEvent, getEnabled);
ZEND_METHOD(SomeNs_EventBufferEvent, getInput);
ZEND_METHOD(SomeNs_EventBufferEvent, getOutput);
ZEND_METHOD(SomeNs_EventBufferEvent, setWatermark);
ZEND_METHOD(SomeNs_EventBufferEvent, write);
ZEND_METHOD(SomeNs_EventBufferEvent, writeBuffer);
ZEND_METHOD(SomeNs_EventBufferEvent, read);
ZEND_METHOD(SomeNs_EventBufferEvent, readBuffer);
ZEND_METHOD(SomeNs_EventBufferEvent, createPair);
ZEND_METHOD(SomeNs_EventBufferEvent, setPriority);
ZEND_METHOD(SomeNs_EventBufferEvent, setTimeouts);
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventBufferEvent, createSslFilter);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventBufferEvent, sslSocket);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventBufferEvent, sslError);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventBufferEvent, sslRenegotiate);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventBufferEvent, sslGetCipherInfo);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventBufferEvent, sslGetCipherName);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventBufferEvent, sslGetCipherVersion);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventBufferEvent, sslGetProtocol);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
ZEND_METHOD(SomeNs_EventSslContext, __construct);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
ZEND_METHOD(SomeNs_EventSslContext, setMinProtoVersion);
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
ZEND_METHOD(SomeNs_EventSslContext, setMaxProtoVersion);
#endif


static const zend_function_entry class_SomeNs_EventConfig_methods[] = {
	ZEND_ME(SomeNs_EventConfig, __construct, arginfo_class_SomeNs_EventConfig___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventConfig, __sleep, arginfo_class_SomeNs_EventConfig___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(SomeNs_EventConfig, __wakeup, arginfo_class_SomeNs_EventConfig___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(SomeNs_EventConfig, avoidMethod, arginfo_class_SomeNs_EventConfig_avoidMethod, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(SomeNs_EventConfig, requireFeatures, arginfo_class_SomeNs_EventConfig_requireFeatures, ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
	ZEND_ME(SomeNs_EventConfig, setMaxDispatchInterval, arginfo_class_SomeNs_EventConfig_setMaxDispatchInterval, ZEND_ACC_PUBLIC)
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02000201 /* 2.0.2-alpha */
	ZEND_ME(SomeNs_EventConfig, setFlags, arginfo_class_SomeNs_EventConfig_setFlags, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventBase_methods[] = {
	ZEND_ME(SomeNs_EventBase, __construct, arginfo_class_SomeNs_EventBase___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, __sleep, arginfo_class_SomeNs_EventBase___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(SomeNs_EventBase, __wakeup, arginfo_class_SomeNs_EventBase___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(SomeNs_EventBase, getMethod, arginfo_class_SomeNs_EventBase_getMethod, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, getFeatures, arginfo_class_SomeNs_EventBase_getFeatures, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, priorityInit, arginfo_class_SomeNs_EventBase_priorityInit, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, loop, arginfo_class_SomeNs_EventBase_loop, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, dispatch, arginfo_class_SomeNs_EventBase_dispatch, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, exit, arginfo_class_SomeNs_EventBase_exit, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, set, arginfo_class_SomeNs_EventBase_set, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, stop, arginfo_class_SomeNs_EventBase_stop, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, gotStop, arginfo_class_SomeNs_EventBase_gotStop, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, gotExit, arginfo_class_SomeNs_EventBase_gotExit, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, getTimeOfDayCached, arginfo_class_SomeNs_EventBase_getTimeOfDayCached, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, reInit, arginfo_class_SomeNs_EventBase_reInit, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBase, free, arginfo_class_SomeNs_EventBase_free, ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
	ZEND_ME(SomeNs_EventBase, updateCacheTime, arginfo_class_SomeNs_EventBase_updateCacheTime, ZEND_ACC_PUBLIC)
#endif
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
	ZEND_ME(SomeNs_EventBase, resume, arginfo_class_SomeNs_EventBase_resume, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_Event_methods[] = {
	ZEND_ME(SomeNs_Event, __construct, arginfo_class_SomeNs_Event___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_Event, free, arginfo_class_SomeNs_Event_free, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_Event, set, arginfo_class_SomeNs_Event_set, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_Event, getSupportedMethods, arginfo_class_SomeNs_Event_getSupportedMethods, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(SomeNs_Event, add, arginfo_class_SomeNs_Event_add, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_Event, del, arginfo_class_SomeNs_Event_del, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_Event, setPriority, arginfo_class_SomeNs_Event_setPriority, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_Event, pending, arginfo_class_SomeNs_Event_pending, ZEND_ACC_PUBLIC)
#if LIBEVENT_VERSION_NUMBER >= 0x02010200
	ZEND_ME(SomeNs_Event, removeTimer, arginfo_class_SomeNs_Event_removeTimer, ZEND_ACC_PUBLIC)
#endif
	ZEND_ME(SomeNs_Event, timer, arginfo_class_SomeNs_Event_timer, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(SomeNs_Event, setTimer, arginfo_class_SomeNs_Event_setTimer, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_Event, signal, arginfo_class_SomeNs_Event_signal, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_MALIAS(SomeNs_Event, addTimer, add, arginfo_class_SomeNs_Event_addTimer, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(SomeNs_Event, delTimer, del, arginfo_class_SomeNs_Event_delTimer, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(SomeNs_Event, addSignal, add, arginfo_class_SomeNs_Event_addSignal, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(SomeNs_Event, delSignal, del, arginfo_class_SomeNs_Event_delSignal, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventDnsBase_methods[] = {
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventDnsBase, __construct, arginfo_class_SomeNs_EventDnsBase___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventDnsBase, parseResolvConf, arginfo_class_SomeNs_EventDnsBase_parseResolvConf, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventDnsBase, addNameserverIp, arginfo_class_SomeNs_EventDnsBase_addNameserverIp, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventDnsBase, loadHosts, arginfo_class_SomeNs_EventDnsBase_loadHosts, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventDnsBase, clearSearch, arginfo_class_SomeNs_EventDnsBase_clearSearch, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventDnsBase, addSearch, arginfo_class_SomeNs_EventDnsBase_addSearch, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventDnsBase, setSearchNdots, arginfo_class_SomeNs_EventDnsBase_setSearchNdots, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventDnsBase, setOption, arginfo_class_SomeNs_EventDnsBase_setOption, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventDnsBase, countNameservers, arginfo_class_SomeNs_EventDnsBase_countNameservers, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventHttpConnection_methods[] = {
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventHttpConnection, __construct, arginfo_class_SomeNs_EventHttpConnection___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
	ZEND_ME(SomeNs_EventHttpConnection, __construct, arginfo_class_SomeNs_EventHttpConnection___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, __sleep, arginfo_class_SomeNs_EventHttpConnection___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, __wakeup, arginfo_class_SomeNs_EventHttpConnection___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, getBase, arginfo_class_SomeNs_EventHttpConnection_getBase, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, getPeer, arginfo_class_SomeNs_EventHttpConnection_getPeer, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, setLocalAddress, arginfo_class_SomeNs_EventHttpConnection_setLocalAddress, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, setLocalPort, arginfo_class_SomeNs_EventHttpConnection_setLocalPort, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, setTimeout, arginfo_class_SomeNs_EventHttpConnection_setTimeout, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, setMaxHeadersSize, arginfo_class_SomeNs_EventHttpConnection_setMaxHeadersSize, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, setMaxBodySize, arginfo_class_SomeNs_EventHttpConnection_setMaxBodySize, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, setRetries, arginfo_class_SomeNs_EventHttpConnection_setRetries, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, makeRequest, arginfo_class_SomeNs_EventHttpConnection_makeRequest, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpConnection, setCloseCallback, arginfo_class_SomeNs_EventHttpConnection_setCloseCallback, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventHttp_methods[] = {
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventHttp, __construct, arginfo_class_SomeNs_EventHttp___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB && !(LIBEVENT_VERSION_NUMBER >= 0x02010000 && defined(HAVE_EVENT_OPENSSL_LIB))
	ZEND_ME(SomeNs_EventHttp, __construct, arginfo_class_SomeNs_EventHttp___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, __sleep, arginfo_class_SomeNs_EventHttp___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, __wakeup, arginfo_class_SomeNs_EventHttp___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, accept, arginfo_class_SomeNs_EventHttp_accept, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, bind, arginfo_class_SomeNs_EventHttp_bind, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, setCallback, arginfo_class_SomeNs_EventHttp_setCallback, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, setDefaultCallback, arginfo_class_SomeNs_EventHttp_setDefaultCallback, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, setAllowedMethods, arginfo_class_SomeNs_EventHttp_setAllowedMethods, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, setMaxBodySize, arginfo_class_SomeNs_EventHttp_setMaxBodySize, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, setMaxHeadersSize, arginfo_class_SomeNs_EventHttp_setMaxHeadersSize, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, setTimeout, arginfo_class_SomeNs_EventHttp_setTimeout, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, addServerAlias, arginfo_class_SomeNs_EventHttp_addServerAlias, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttp, removeServerAlias, arginfo_class_SomeNs_EventHttp_removeServerAlias, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventHttpRequest_methods[] = {
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, __construct, arginfo_class_SomeNs_EventHttpRequest___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, __sleep, arginfo_class_SomeNs_EventHttpRequest___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, __wakeup, arginfo_class_SomeNs_EventHttpRequest___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, free, arginfo_class_SomeNs_EventHttpRequest_free, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, getCommand, arginfo_class_SomeNs_EventHttpRequest_getCommand, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, getHost, arginfo_class_SomeNs_EventHttpRequest_getHost, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, getUri, arginfo_class_SomeNs_EventHttpRequest_getUri, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, getResponseCode, arginfo_class_SomeNs_EventHttpRequest_getResponseCode, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, getInputHeaders, arginfo_class_SomeNs_EventHttpRequest_getInputHeaders, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, getOutputHeaders, arginfo_class_SomeNs_EventHttpRequest_getOutputHeaders, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, getInputBuffer, arginfo_class_SomeNs_EventHttpRequest_getInputBuffer, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, getOutputBuffer, arginfo_class_SomeNs_EventHttpRequest_getOutputBuffer, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02001100
	ZEND_ME(SomeNs_EventHttpRequest, getBufferEvent, arginfo_class_SomeNs_EventHttpRequest_getBufferEvent, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, getConnection, arginfo_class_SomeNs_EventHttpRequest_getConnection, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, closeConnection, arginfo_class_SomeNs_EventHttpRequest_closeConnection, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, sendError, arginfo_class_SomeNs_EventHttpRequest_sendError, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, sendReply, arginfo_class_SomeNs_EventHttpRequest_sendReply, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, sendReplyChunk, arginfo_class_SomeNs_EventHttpRequest_sendReplyChunk, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, sendReplyEnd, arginfo_class_SomeNs_EventHttpRequest_sendReplyEnd, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, sendReplyStart, arginfo_class_SomeNs_EventHttpRequest_sendReplyStart, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, cancel, arginfo_class_SomeNs_EventHttpRequest_cancel, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, addHeader, arginfo_class_SomeNs_EventHttpRequest_addHeader, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, clearHeaders, arginfo_class_SomeNs_EventHttpRequest_clearHeaders, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, removeHeader, arginfo_class_SomeNs_EventHttpRequest_removeHeader, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventHttpRequest, findHeader, arginfo_class_SomeNs_EventHttpRequest_findHeader, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventListener_methods[] = {
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventListener, __construct, arginfo_class_SomeNs_EventListener___construct, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventListener, __sleep, arginfo_class_SomeNs_EventListener___sleep, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventListener, __wakeup, arginfo_class_SomeNs_EventListener___wakeup, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventListener, free, arginfo_class_SomeNs_EventListener_free, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventListener, enable, arginfo_class_SomeNs_EventListener_enable, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventListener, disable, arginfo_class_SomeNs_EventListener_disable, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventListener, setCallback, arginfo_class_SomeNs_EventListener_setCallback, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventListener, setErrorCallback, arginfo_class_SomeNs_EventListener_setErrorCallback, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB && LIBEVENT_VERSION_NUMBER >= 0x02000300
	ZEND_ME(SomeNs_EventListener, getBase, arginfo_class_SomeNs_EventListener_getBase, ZEND_ACC_PUBLIC)
#endif
#if HAVE_EVENT_EXTRA_LIB
	ZEND_ME(SomeNs_EventListener, getSocketName, arginfo_class_SomeNs_EventListener_getSocketName, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventUtil_methods[] = {
	ZEND_ME(SomeNs_EventUtil, __construct, arginfo_class_SomeNs_EventUtil___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(SomeNs_EventUtil, getLastSocketErrno, arginfo_class_SomeNs_EventUtil_getLastSocketErrno, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(SomeNs_EventUtil, getLastSocketError, arginfo_class_SomeNs_EventUtil_getLastSocketError, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventUtil, sslRandPoll, arginfo_class_SomeNs_EventUtil_sslRandPoll, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
	ZEND_ME(SomeNs_EventUtil, getSocketName, arginfo_class_SomeNs_EventUtil_getSocketName, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(SomeNs_EventUtil, getSocketFd, arginfo_class_SomeNs_EventUtil_getSocketFd, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(SomeNs_EventUtil, setSocketOption, arginfo_class_SomeNs_EventUtil_setSocketOption, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#if defined(PHP_EVENT_SOCKETS_SUPPORT)
	ZEND_ME(SomeNs_EventUtil, createSocket, arginfo_class_SomeNs_EventUtil_createSocket, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventBuffer_methods[] = {
	ZEND_ME(SomeNs_EventBuffer, __construct, arginfo_class_SomeNs_EventBuffer___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, freeze, arginfo_class_SomeNs_EventBuffer_freeze, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, unfreeze, arginfo_class_SomeNs_EventBuffer_unfreeze, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, lock, arginfo_class_SomeNs_EventBuffer_lock, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, unlock, arginfo_class_SomeNs_EventBuffer_unlock, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, enableLocking, arginfo_class_SomeNs_EventBuffer_enableLocking, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, add, arginfo_class_SomeNs_EventBuffer_add, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, read, arginfo_class_SomeNs_EventBuffer_read, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, addBuffer, arginfo_class_SomeNs_EventBuffer_addBuffer, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, appendFrom, arginfo_class_SomeNs_EventBuffer_appendFrom, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, expand, arginfo_class_SomeNs_EventBuffer_expand, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, prepend, arginfo_class_SomeNs_EventBuffer_prepend, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, prependBuffer, arginfo_class_SomeNs_EventBuffer_prependBuffer, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, drain, arginfo_class_SomeNs_EventBuffer_drain, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, copyout, arginfo_class_SomeNs_EventBuffer_copyout, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, readLine, arginfo_class_SomeNs_EventBuffer_readLine, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, search, arginfo_class_SomeNs_EventBuffer_search, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, searchEol, arginfo_class_SomeNs_EventBuffer_searchEol, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, pullup, arginfo_class_SomeNs_EventBuffer_pullup, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, write, arginfo_class_SomeNs_EventBuffer_write, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, readFrom, arginfo_class_SomeNs_EventBuffer_readFrom, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBuffer, substr, arginfo_class_SomeNs_EventBuffer_substr, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventBufferEvent_methods[] = {
	ZEND_ME(SomeNs_EventBufferEvent, __construct, arginfo_class_SomeNs_EventBufferEvent___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, free, arginfo_class_SomeNs_EventBufferEvent_free, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, close, arginfo_class_SomeNs_EventBufferEvent_close, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, connect, arginfo_class_SomeNs_EventBufferEvent_connect, ZEND_ACC_PUBLIC)
#if defined(HAVE_EVENT_EXTRA_LIB)
	ZEND_ME(SomeNs_EventBufferEvent, connectHost, arginfo_class_SomeNs_EventBufferEvent_connectHost, ZEND_ACC_PUBLIC)
#endif
#if !(defined(HAVE_EVENT_EXTRA_LIB))
	ZEND_ME(SomeNs_EventBufferEvent, connectHost, arginfo_class_SomeNs_EventBufferEvent_connectHost, ZEND_ACC_PUBLIC)
#endif
	ZEND_ME(SomeNs_EventBufferEvent, getDnsErrorString, arginfo_class_SomeNs_EventBufferEvent_getDnsErrorString, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, setCallbacks, arginfo_class_SomeNs_EventBufferEvent_setCallbacks, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, enable, arginfo_class_SomeNs_EventBufferEvent_enable, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, disable, arginfo_class_SomeNs_EventBufferEvent_disable, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, getEnabled, arginfo_class_SomeNs_EventBufferEvent_getEnabled, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, getInput, arginfo_class_SomeNs_EventBufferEvent_getInput, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, getOutput, arginfo_class_SomeNs_EventBufferEvent_getOutput, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, setWatermark, arginfo_class_SomeNs_EventBufferEvent_setWatermark, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, write, arginfo_class_SomeNs_EventBufferEvent_write, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, writeBuffer, arginfo_class_SomeNs_EventBufferEvent_writeBuffer, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, read, arginfo_class_SomeNs_EventBufferEvent_read, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, readBuffer, arginfo_class_SomeNs_EventBufferEvent_readBuffer, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, createPair, arginfo_class_SomeNs_EventBufferEvent_createPair, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(SomeNs_EventBufferEvent, setPriority, arginfo_class_SomeNs_EventBufferEvent_setPriority, ZEND_ACC_PUBLIC)
	ZEND_ME(SomeNs_EventBufferEvent, setTimeouts, arginfo_class_SomeNs_EventBufferEvent_setTimeouts, ZEND_ACC_PUBLIC)
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventBufferEvent, createSslFilter, arginfo_class_SomeNs_EventBufferEvent_createSslFilter, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventBufferEvent, sslSocket, arginfo_class_SomeNs_EventBufferEvent_sslSocket, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventBufferEvent, sslError, arginfo_class_SomeNs_EventBufferEvent_sslError, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventBufferEvent, sslRenegotiate, arginfo_class_SomeNs_EventBufferEvent_sslRenegotiate, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventBufferEvent, sslGetCipherInfo, arginfo_class_SomeNs_EventBufferEvent_sslGetCipherInfo, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventBufferEvent, sslGetCipherName, arginfo_class_SomeNs_EventBufferEvent_sslGetCipherName, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventBufferEvent, sslGetCipherVersion, arginfo_class_SomeNs_EventBufferEvent_sslGetCipherVersion, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventBufferEvent, sslGetProtocol, arginfo_class_SomeNs_EventBufferEvent_sslGetProtocol, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};


static const zend_function_entry class_SomeNs_EventSslContext_methods[] = {
#if defined(HAVE_EVENT_OPENSSL_LIB)
	ZEND_ME(SomeNs_EventSslContext, __construct, arginfo_class_SomeNs_EventSslContext___construct, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
	ZEND_ME(SomeNs_EventSslContext, setMinProtoVersion, arginfo_class_SomeNs_EventSslContext_setMinProtoVersion, ZEND_ACC_PUBLIC)
#endif
#if defined(HAVE_EVENT_OPENSSL_LIB) && OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
	ZEND_ME(SomeNs_EventSslContext, setMaxProtoVersion, arginfo_class_SomeNs_EventSslContext_setMaxProtoVersion, ZEND_ACC_PUBLIC)
#endif
	ZEND_FE_END
};

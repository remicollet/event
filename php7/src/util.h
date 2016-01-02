/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2015 The PHP Group                                |
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
#ifndef PHP_EVENT_UTIL_H
#define PHP_EVENT_UTIL_H

php_socket_t php_event_zval_to_fd(zval *pfd);
int _php_event_getsockname(evutil_socket_t fd, zval **ppzaddress, zval **ppzport);

#define php_event_is_pending(e) \
	event_pending((e), EV_READ | EV_WRITE | EV_SIGNAL | EV_TIMEOUT, NULL)

#define PHP_EVENT_REGISTER_CLASS(name, create_func, ce, ce_functions) \
{                                                                     \
	zend_class_entry tmp_ce;                                          \
	INIT_CLASS_ENTRY(tmp_ce, name, ce_functions);                     \
	tmp_ce.create_object = create_func;                               \
	ce = zend_register_internal_class(&tmp_ce);                       \
}

#define PHP_EVENT_INIT_CLASS_OBJECT(pz, pce) object_init_ex((pz), (pce))

#define REGISTER_EVENT_CLASS_CONST_LONG(pce, const_name, value) \
	zend_declare_class_constant_long((pce), #const_name,        \
			sizeof(#const_name) - 1, (zend_long) value)

#define PHP_EVENT_SET_X_OBJ_HANDLER(x, name) \
	event_ ## x ## _object_handlers = php_event_ ## x ## _ ## name

#define PHP_EVENT_SET_X_OBJ_HANDLERS(x) do { \
	PHP_EVENT_X_OBJ_HANDLERS(x).offset = XtOffsetOf(Z_EVENT_X_OBJ_T(x), zo); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, free_obj); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, read_property); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, write_property); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, get_property_ptr_ptr); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, has_property); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, get_debug_info); \
	PHP_EVENT_SET_X_OBJ_HANDLER(x, get_properties); \
} while (0)

/* php_event_x_fetch_object(zend_object *obj) */
#define Z_EVENT_X_FETCH_OBJ_DECL(x) \
	static zend_always_inline php_event_ ## x ## _t * php_event_ ## x ## _fetch_object(zend_object *obj) { \
		return (EXPECTED(obj) ? (php_event_ ## x ## _t *)((char *)obj - XtOffsetOf(php_event_ ## x ## _t, zo)) : NULL); \
	}

#define Z_EVENT_X_OBJ_P(x, zv) (EXPECTED(zv) ? php_event_ # x # _fetch_object(Z_OBJ_P(zv)) : NULL)

Z_EVENT_X_FETCH_OBJ_DECL(base)
Z_EVENT_X_FETCH_OBJ_DECL(event)
Z_EVENT_X_FETCH_OBJ_DECL(config)
Z_EVENT_X_FETCH_OBJ_DECL(buffer)

#define Z_EVENT_BASE_OBJ_P(zv)   Z_EVENT_X_OBJ_P(base,   zv)
#define Z_EVENT_EVENT_OBJ_P(zv)  Z_EVENT_X_OBJ_P(event,  zv)
#define Z_EVENT_CONFIG_OBJ_P(zv) Z_EVENT_X_OBJ_P(config, zv)
#define Z_EVENT_BUFFER_OBJ_P(zv) Z_EVENT_X_OBJ_P(buffer, zv)
#define Z_EVENT_BEVENT_OBJ_P(zv) Z_EVENT_X_OBJ_P(bevent, zv)

#ifdef HAVE_EVENT_EXTRA_LIB
Z_EVENT_X_FETCH_OBJ_DECL(dns_base)
Z_EVENT_X_FETCH_OBJ_DECL(listener)
Z_EVENT_X_FETCH_OBJ_DECL(http)
Z_EVENT_X_FETCH_OBJ_DECL(http_conn)
Z_EVENT_X_FETCH_OBJ_DECL(http_req)

#define Z_EVENT_DNS_BASE_OBJ_P(zv)  Z_EVENT_X_OBJ_P(dns_base,  zv)
#define Z_EVENT_LISTENER_OBJ_P(zv)  Z_EVENT_X_OBJ_P(listener,  zv)
#define Z_EVENT_HTTP_OBJ_P(zv)      Z_EVENT_X_OBJ_P(http,      zv)
#define Z_EVENT_HTTP_CONN_OBJ_P(zv) Z_EVENT_X_OBJ_P(http_conn, zv)
#define Z_EVENT_HTTP_REQ_OBJ_P(zv)  Z_EVENT_X_OBJ_P(http_req,  zv)
#endif /* HAVE_EVENT_EXTRA_LIB */

#define Z_EVENT_STD_OBJ_DTOR(o) zend_object_std_dtor(&o->zo)

#ifdef HAVE_EVENT_OPENSSL_LIB
Z_EVENT_X_FETCH_OBJ_DECL(ssl_context)

#define Z_EVENT_SSL_CONTEXT_OBJ_P(zv) Z_EVENT_X_OBJ_P(ssl_context, zv)
#endif /* HAVE_EVENT_OPENSSL_LIB */

/* XXX Replace PHP_EVENT_FETCH_*() occurances with corresponding x = Z_EVENT_x_OBJ_P(); */
#define PHP_EVENT_FETCH_EVENT(x,       zv) x = Z_EVENT_EVENT_OBJ_P(zv)
#define PHP_EVENT_FETCH_BASE(x,        zv) x = Z_EVENT_BASE_OBJ_P(zv)
#define PHP_EVENT_FETCH_CONFIG(x,      zv) x = Z_EVENT_CONFIG_OBJ_P(zv)
#define PHP_EVENT_FETCH_BEVENT(x,      zv) x = Z_EVENT_BEVENT_OBJ_P(zv)
#define PHP_EVENT_FETCH_BUFFER(x,      zv) x = Z_EVENT_BUFFER_OBJ_P(zv)
#define PHP_EVENT_FETCH_DNS_BASE(x,    zv) x = Z_EVENT_DNS_BASE_OBJ_P(zv)
#define PHP_EVENT_FETCH_LISTENER(x,    zv) x = Z_EVENT_LISTENER_OBJ_P(zv)
#define PHP_EVENT_FETCH_HTTP_CONN(x,   zv) x = Z_EVENT_HTTP_OBJ_P(zv)
#define PHP_EVENT_FETCH_HTTP(x,        zv) x = Z_EVENT_HTTP_OBJ_P(zv)
#define PHP_EVENT_FETCH_HTTP_REQ(x,    zv) x = Z_EVENT_HTTP_REQ_OBJ_P(zv)
#define PHP_EVENT_FETCH_BUFFER_POS(x,  zv) x = Z_EVENT_BUFFER_OBJ_P(zv)
#define PHP_EVENT_FETCH_SSL_CONTEXT(x, zv) x = Z_EVENT_SSL_CONTEXT_OBJ_P(zv)

static zend_always_inline void init_properties(zend_object *pzo)/*{{{*/
{
	zend_object_std_init(pzo, ce);
	object_properties_init(pzo, ce);
}/*}}}*/

static zend_always_inline HashTable * find_prop_handler(const zend_class_entry *ce)/*{{{*/
{
	zend_class_entry *ce_parent = ce;

	while (ce_parent->type != ZEND_INTERNAL_CLASS && ce_parent->parent != NULL) {
		ce_parent = ce_parent->parent;
	}

	return zend_hash_find_ptr(&classes, ce_parent->name);
} /*}}}*/

#define PHP_EVENT_OBJ_ALLOC(obj, ce, t)                                \
	do {                                                               \
		obj = ecalloc(1, sizeof(t) + zend_object_properties_size(ce)); \
		PHP_EVENT_ASSERT(obj);                                         \
		obj->prop_handler = find_prop_handler(ce);                     \
		init_properties(&obj->zo);                                     \
	} while (0)

#define PHP_EVENT_TIMEVAL_SET(tv, t)                     \
	do {                                                 \
		tv.tv_sec  = (zend_long)t;                       \
		tv.tv_usec = (zend_long)((t - tv.tv_sec) * 1e6); \
	} while (0)

#define PHP_EVENT_TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)

#define PHP_EVENT_SOCKETS_REQUIRED_NORET \
	php_error_docref(NULL, E_ERROR, "`sockets' extension required. " \
			"If you have `sockets' installed, rebuild `event' extension")

#define PHP_EVENT_SOCKETS_REQUIRED_RET    \
	do {                                  \
		PHP_EVENT_SOCKETS_REQUIRED_NORET; \
		RETURN_FALSE;                     \
	} while (0)

#define PHP_EVENT_REQUIRE_BASE_BY_REF(zbase)                  \
	do {                                                      \
		if (!Z_ISREF_P((zbase)) || Z_REFCOUNT_P(zbase) < 2) { \
			php_error_docref(NULL, E_ERROR,                   \
					"EventBase must be passed by reference"); \
		}                                                     \
	} while (0)

#if defined(PHP_WIN32)
#if defined(ZTS)
#  define PHP_EVENT_TSRMLS_FETCH_FROM_CTX(ctx) tsrm_ls = (void ***)ctx
#  define PHP_EVENT_TSRM_DECL void ***tsrm_ls;
# else
#  define PHP_EVENT_TSRMLS_FETCH_FROM_CTX(ctx)
#  define PHP_EVENT_TSRM_DECL
# endif
#else
# define PHP_EVENT_TSRMLS_FETCH_FROM_CTX(ctx) TSRMLS_FETCH_FROM_CTX(ctx)
# define PHP_EVENT_TSRM_DECL
#endif

#endif /* PHP_EVENT_UTIL_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */

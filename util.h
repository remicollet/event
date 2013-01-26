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
#ifndef PHP_EVENT_UTIL_H
#define PHP_EVENT_UTIL_H

zend_always_inline zend_bool php_event_is_pending(const struct event *e);
php_socket_t php_event_zval_to_fd(zval **ppfd TSRMLS_DC);

#define PHP_EVENT_REGISTER_CLASS(name, create_func, ce, ce_functions) \
{                                                                     \
    zend_class_entry tmp_ce;                                          \
    INIT_CLASS_ENTRY(tmp_ce, name, ce_functions);                     \
    ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);             \
    ce->create_object = create_func;                                  \
}

#define PHP_EVENT_INIT_CLASS_OBJECT(pz, pce) \
        Z_TYPE_P((pz)) = IS_OBJECT;          \
        object_init_ex((pz), (pce));         \
        Z_SET_REFCOUNT_P((pz), 1);           \
        Z_SET_ISREF_P((pz))

#define PHP_EVENT_FETCH_EVENT(e, ze) \
	e = (php_event_t *) zend_object_store_get_object(ze TSRMLS_CC);

#define PHP_EVENT_FETCH_BASE(base, zbase) \
	base = (php_event_base_t *) zend_object_store_get_object(zbase TSRMLS_CC);

#define PHP_EVENT_FETCH_CONFIG(cfg, zcfg) \
	cfg = (php_event_config_t *) zend_object_store_get_object(zcfg TSRMLS_CC);

#define PHP_EVENT_FETCH_BEVENT(b, zb) \
	b = (php_event_bevent_t *) zend_object_store_get_object(zb TSRMLS_CC);

#define PHP_EVENT_FETCH_BUFFER(b, zb) \
	b = (php_event_buffer_t *) zend_object_store_get_object(zb TSRMLS_CC);

#define PHP_EVENT_FETCH_DNS_BASE(b, zb) \
	b = (php_event_dns_base_t *) zend_object_store_get_object(zb TSRMLS_CC);

#define PHP_EVENT_FETCH_LISTENER(b, zb) \
	b = (php_event_listener_t *) zend_object_store_get_object(zb TSRMLS_CC);

#define PHP_EVENT_FETCH_HTTP_CONN(b, zb) \
	b = (php_event_http_conn_t *) zend_object_store_get_object(zb TSRMLS_CC);

#define PHP_EVENT_FETCH_HTTP(b, zb) \
	b = (php_event_http_t *) zend_object_store_get_object(zb TSRMLS_CC);

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


#endif /* PHP_EVENT_UTIL_H */

/* 
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */

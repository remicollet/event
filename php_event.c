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
#include "util.h"
#include "priv.h"

/*
ZEND_DECLARE_MODULE_GLOBALS(event)
*/

zend_class_entry *php_event_ce;
zend_class_entry *php_event_base_ce;
zend_class_entry *php_event_config_ce;
zend_class_entry *php_event_bevent_ce;
zend_class_entry *php_event_buffer_ce;
zend_class_entry *php_event_util_ce;

#if HAVE_EVENT_EXTRA_LIB
zend_class_entry *php_event_dns_base_ce;
zend_class_entry *php_event_listener_ce;
zend_class_entry *php_event_http_conn_ce;
zend_class_entry *php_event_http_ce;
#endif

static HashTable classes;

static HashTable event_properties;
static HashTable event_base_properties;
static HashTable event_config_properties;
static HashTable event_bevent_properties;
static HashTable event_buffer_properties;

#if 0
static HashTable event_util_properties;

#if HAVE_EVENT_EXTRA_LIB
static HashTable event_dns_base_properties;
static HashTable event_listener_properties;
static HashTable event_http_conn_properties;
static HashTable event_http_properties;
#endif
#endif

static zend_object_handlers object_handlers;

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
	NULL, /*event_functions*/
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


/* {{{ Private functions */

/* {{{ event_bevent_object_dtor
 * Required to cleanup bufferevent properly */
static void event_bevent_object_dtor(void *ptr, zend_object_handle handle TSRMLS_DC)
{
	php_event_bevent_t *b = (php_event_bevent_t *) ptr;

	PHP_EVENT_ASSERT(b);

	if (b->bevent) {
		bufferevent_free(b->bevent);
		b->bevent = NULL;
	}

	PHP_EVENT_FREE_FCALL_INFO(b->fci_read,  b->fcc_read);
	PHP_EVENT_FREE_FCALL_INFO(b->fci_write, b->fcc_write);
	PHP_EVENT_FREE_FCALL_INFO(b->fci_event, b->fcc_event);

	zend_objects_destroy_object(ptr, handle TSRMLS_CC);
}
/* }}} */

/* {{{ event_object_dtor
 * Required to cleanup event properly */
static void event_object_dtor(void *ptr, zend_object_handle handle TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) ptr;

	if (e && e->event) {
		event_del(e->event);
		event_free(e->event);
		e->event = NULL;
	}

	zend_objects_destroy_object(ptr, handle TSRMLS_CC);
}
/* }}} */

/* {{{ event_base_object_dtor
 * Required to cleanup event base properly */
static void event_base_object_dtor(void *ptr, zend_object_handle handle TSRMLS_DC)
{
	php_event_base_t *b = (php_event_base_t *) ptr;

	if (!b->internal && b->base) {
		/* TODO: what if events bound to the event_base are not destroyed? */
		event_base_free(b->base);
		b->base = NULL;
	}

	zend_objects_destroy_object(ptr, handle TSRMLS_CC);
}
/* }}} */

/* {{{ event_config_object_dtor
 * Required to cleanup event config properly */
static void event_config_object_dtor(void *ptr, zend_object_handle handle TSRMLS_DC)
{
	php_event_config_t *cfg = (php_event_config_t *) ptr;

	if (cfg && cfg->ptr) {
		event_config_free(cfg->ptr);
		cfg->ptr = NULL;
	}

	zend_objects_destroy_object(ptr, handle TSRMLS_CC);
}
/* }}} */

/* {{{ event_buffer_object_dtor
 * Required to cleanup buffer properly */
static void event_buffer_object_dtor(void *ptr, zend_object_handle handle TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) ptr;

	/* If we got the buffer in, say, a read callback the buffer
	 * is destroyed when the callback is done as any normal variable.
	 * Zend MM calls destructor which eventually calls this function.
	 * We'll definitely crash, if we call evbuffer_free() on an internal
	 * bufferevent buffer. */

	if (!b->internal && b->buf) {
		evbuffer_free(b->buf);
		b->buf = NULL;
	}

	zend_objects_destroy_object(ptr, handle TSRMLS_CC);
}
/* }}} */

/* {{{ event_dns_base_object_dtor
 * Required to cleanup buffer properly */
static void event_dns_base_object_dtor(void *ptr, zend_object_handle handle TSRMLS_DC)
{
	php_event_dns_base_t *dnsb = (php_event_dns_base_t *) ptr;

	if (dnsb && dnsb->dns_base) {
		/* Setting fail_requests to 1 makes all in-flight requests get
	 	 * their callbacks invoked with a canceled error code before it
	 	 * frees the base*/
		evdns_base_free(dnsb->dns_base, 1);
		dnsb->dns_base = NULL;
	}

	zend_objects_destroy_object(ptr, handle TSRMLS_CC);
}
/* }}} */

/* {{{ event_listener_object_dtor
 * Required to cleanup buffer properly */
static void event_listener_object_dtor(void *ptr, zend_object_handle handle TSRMLS_DC)
{
	php_event_listener_t *l = (php_event_listener_t *) ptr;

	if (l && l->listener) {
		evconnlistener_free(l->listener);
		l->listener = NULL;
	}

	zend_objects_destroy_object(ptr, handle TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_conn_object_dtor
 * Required to cleanup http conn obj properly */
static void event_http_conn_object_dtor(void *ptr, zend_object_handle handle TSRMLS_DC)
{
	php_event_http_conn_t *evcon = (php_event_http_conn_t *) ptr;

	if (evcon && evcon->conn) {
		evhttp_connection_free(evcon->conn);
		evcon->conn = NULL;
	}

	zend_objects_destroy_object(ptr, handle TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_object_dtor
 * Required to cleanup http obj properly */
static void event_http_object_dtor(void *ptr, zend_object_handle handle TSRMLS_DC)
{
	php_event_http_t *http = (php_event_http_t *) ptr;

	if (http && http->ptr) {
		evhttp_free(http->ptr);
	}

	zend_objects_destroy_object(ptr, handle TSRMLS_CC);
}
/* }}} */


/* {{{ event_generic_object_free_storage */
static zend_always_inline void event_generic_object_free_storage(void *ptr TSRMLS_DC)
{
	PHP_EVENT_ASSERT(ptr);

	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) ptr;

	zend_object_std_dtor(&obj->zo TSRMLS_CC);

	efree(ptr);
}
/* }}} */

/* {{{ event_object_free_storage */
static void event_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) ptr;

	PHP_EVENT_ASSERT(e);

	if (e->data) {
		zval_ptr_dtor(&e->data);
	}

	PHP_EVENT_FREE_FCALL_INFO(e->fci, e->fcc);

	if (e->stream_id >= 0) { /* stdin fd == 0 */
		zend_list_delete(e->stream_id);
	}

	if (e->event) {
		event_del(e->event);
		event_free(e->event);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_base_object_free_storage */
static void event_base_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_base_t *b = (php_event_base_t *) ptr;

	PHP_EVENT_ASSERT(b);

	if (!b->internal && b->base) {
		/* TODO: what if events bound to the event_base are not destroyed? */
		event_base_free(b->base);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_config_object_free_storage*/
static void event_config_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_config_t *cfg = (php_event_config_t *) ptr;

	PHP_EVENT_ASSERT(cfg);

	if (cfg->ptr) {
		event_config_free(cfg->ptr);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_bevent_object_free_storage */
static void event_bevent_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_bevent_t *b = (php_event_bevent_t *) ptr;

	if (b) {
		if (b->data) {
			zval_ptr_dtor(&b->data);
		}

		PHP_EVENT_FREE_FCALL_INFO(b->fci_read,  b->fcc_read);
		PHP_EVENT_FREE_FCALL_INFO(b->fci_write, b->fcc_write);
		PHP_EVENT_FREE_FCALL_INFO(b->fci_event, b->fcc_event);

		if (b->stream_id >= 0) { /* stdin fd == 0 */
			zend_list_delete(b->stream_id);
		}

		if (b->self) {
			zval_ptr_dtor(&b->self);
		}

		if (b->bevent) {
			bufferevent_free(b->bevent);
		}

		event_generic_object_free_storage(ptr TSRMLS_CC);
	}
}
/* }}} */

/* {{{ event_buffer_object_free_storage */
static void event_buffer_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) ptr;

	PHP_EVENT_ASSERT(b);

	/* If we got the buffer in, say, a read callback the buffer
	 * is destroyed when the callback is done as any normal variable.
	 * Zend MM calls destructor which eventually calls this function.
	 * We'll definitely crash, if we call evbuffer_free() on an internal
	 * bufferevent buffer. */

	if (!b->internal && b->buf) {
		evbuffer_free(b->buf);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_dns_base_object_free_storage */
static void event_dns_base_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_dns_base_t *dnsb = (php_event_dns_base_t *) ptr;

	PHP_EVENT_ASSERT(dnsb);

	if (dnsb->dns_base) {
		/* Setting fail_requests to 1 makes all in-flight requests get
	 	 * their callbacks invoked with a canceled error code before it
	 	 * frees the base*/
		evdns_base_free(dnsb->dns_base, 1);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_listener_object_free_storage */
static void event_listener_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_listener_t *l = (php_event_listener_t *) ptr;

	PHP_EVENT_ASSERT(l);

	if (l->stream_id >= 0) {
		zend_list_delete(l->stream_id);
	}

	if (l->data) {
		zval_ptr_dtor(&l->data);
	}

	if (l->self) {
		zval_ptr_dtor(&l->self);
	}

	PHP_EVENT_FREE_FCALL_INFO(l->fci, l->fcc);
	PHP_EVENT_FREE_FCALL_INFO(l->fci_err, l->fcc_err);

	if (l->listener) {
		evconnlistener_free(l->listener);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_conn_object_free_storage */
static void event_http_conn_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_http_conn_t *evcon = (php_event_http_conn_t *) ptr;

	PHP_EVENT_ASSERT(evcon);

	if (evcon->base) {
		zval_ptr_dtor(&evcon->base);
	}

	if (evcon->dns_base) {
		zval_ptr_dtor(&evcon->dns_base);
	}

	if (evcon->conn) {
		evhttp_connection_free(evcon->conn);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_object_free_storage */
static void event_http_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_http_t *http = (php_event_http_t *) ptr;

	PHP_EVENT_ASSERT(http);

	if (http->base) {
		zval_ptr_dtor(&http->base);
	}

	if (http->ptr) {
		evhttp_free(http->ptr);
	}

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */


/* {{{ register_object */
static zend_always_inline zend_object_value register_object(zend_class_entry *ce, void *obj, zend_objects_store_dtor_t func_dtor, zend_objects_free_object_storage_t func_free_storage TSRMLS_DC)
{
	zend_object_value retval;

	retval.handle   = zend_objects_store_put(obj, func_dtor, func_free_storage, NULL TSRMLS_CC);
	retval.handlers = &object_handlers;

	return retval;
}
/* }}} */

/* {{{ object_new
 * Allocates new object with it's properties.
 * size is a size of struct implementing php_event_abstract_object_t */
static void *object_new(zend_class_entry *ce, size_t size TSRMLS_DC)
{
	php_event_abstract_object_t *obj;
	zend_class_entry *ce_parent = ce;

	obj = emalloc(size);
	memset(obj, 0, size);

	while (ce_parent->type != ZEND_INTERNAL_CLASS && ce_parent->parent != NULL) {
		ce_parent = ce_parent->parent;
	}
	zend_hash_find(&classes, ce_parent->name, ce_parent->name_length + 1,
			(void **) &obj->prop_handler);

	zend_object_std_init(&obj->zo, ce TSRMLS_CC);
	object_properties_init(&obj->zo, ce);

	return (void *) obj;
}
/* }}} */


/* {{{ event_object_create
 * Event object ctor */
static zend_object_value event_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_object_dtor,
			event_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_base_object_create
 * EventBase object ctor */
static zend_object_value event_base_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_base_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_base_object_dtor,
			event_base_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_config_object_create
 * EventConfig object ctor */
static zend_object_value event_config_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_config_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_config_object_dtor,
			event_config_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_bevent_object_create
 * EventBufferEvent object ctor */
static zend_object_value event_bevent_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_bevent_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_bevent_object_dtor,
			event_bevent_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_buffer_object_create
 * EventBuffer object ctor */
static zend_object_value event_buffer_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_buffer_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_buffer_object_dtor,
			event_buffer_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_util_object_create
 * EventUtil object ctor */
static zend_object_value event_util_object_create(zend_class_entry *ce TSRMLS_DC)
{
	/* EventUtil is a singleton. This function must never be called */
	PHP_EVENT_ASSERT(0);

	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_abstract_object_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, (zend_objects_store_dtor_t) zend_objects_destroy_object,
			event_generic_object_free_storage TSRMLS_CC);
}
/* }}} */

#if HAVE_EVENT_EXTRA_LIB

/* {{{ event_dns_base_object_create
 * EventDnsBase object ctor */
static zend_object_value event_dns_base_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_dns_base_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_dns_base_object_dtor,
			event_dns_base_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_listener_object_create
 * EventListener object ctor */
static zend_object_value event_listener_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_listener_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_listener_object_dtor,
			event_listener_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_conn_object_create
 * EventHttpConnection object ctor */
static zend_object_value event_http_conn_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_http_conn_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_http_conn_object_dtor,
			event_http_conn_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_object_create
 * EventHttp object ctor */
static zend_object_value event_http_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_http_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_http_object_dtor,
			event_http_object_free_storage TSRMLS_CC);
}
/* }}} */

#endif /* HAVE_EVENT_EXTRA_LIB */


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


/* {{{ read_property_default */
static int read_property_default(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	*retval = NULL;
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot read property");
	return FAILURE;
}
/* }}} */

/* {{{ write_property_default */
static int write_property_default(php_event_abstract_object_t *obj, zval *newval TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot write property");
	return FAILURE;
}
/* }}} */

/* {{{ add_property */
static void add_property(HashTable *h, const char *name, size_t name_len, php_event_prop_read_t read_func, php_event_prop_write_t write_func, php_event_prop_get_prop_ptr_ptr_t get_ptr_ptr_func TSRMLS_DC) {
	php_event_prop_handler_t p;

	p.name             = (char *) name;
	p.name_len         = name_len;
	p.read_func        = (read_func) ? read_func : read_property_default;
	p.write_func       = (write_func) ? write_func: write_property_default;
	p.get_ptr_ptr_func = get_ptr_ptr_func;
	zend_hash_add(h, name, name_len + 1, &p, sizeof(php_event_prop_handler_t), NULL);
}
/* }}} */

/* {{{ read_property */
static zval *read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC)
{
	zval                         tmp_member;
	zval                        *retval;
	php_event_abstract_object_t *obj;
	php_event_prop_handler_t    *hnd;
	int                          ret;

	ret = FAILURE;
	obj = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);

	if (member->type != IS_STRING) {
	    tmp_member = *member;
	    zval_copy_ctor(&tmp_member);
	    convert_to_string(&tmp_member);
	    member = &tmp_member;
	}

	if (obj->prop_handler != NULL) {
	    ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}

	if (ret == SUCCESS) {
	    ret = hnd->read_func(obj, &retval TSRMLS_CC);
	    if (ret == SUCCESS) {
	        /* ensure we're creating a temporary variable */
	        Z_SET_REFCOUNT_P(retval, 0);
	    } else {
	        retval = EG(uninitialized_zval_ptr);
	    }
	} else {
	    zend_object_handlers * std_hnd = zend_get_std_object_handlers();
	    retval = std_hnd->read_property(object, member, type, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
	    zval_dtor(member);
	}

	return(retval);
}
/* }}} */

/* {{{ write_property */
void write_property(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC)
{
	zval                         tmp_member;
	php_event_abstract_object_t *obj;
	php_event_prop_handler_t    *hnd;
	int                          ret;

	if (member->type != IS_STRING) {
	    tmp_member = *member;
	    zval_copy_ctor(&tmp_member);
	    convert_to_string(&tmp_member);
	    member = &tmp_member;
	}

	ret = FAILURE;
	obj = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
	    ret = zend_hash_find((HashTable *) obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}
	if (ret == SUCCESS) {
	    hnd->write_func(obj, value TSRMLS_CC);
	    if (! PZVAL_IS_REF(value) && Z_REFCOUNT_P(value) == 0) {
	        Z_ADDREF_P(value);
	        zval_ptr_dtor(&value);
	    }
	} else {
	    zend_object_handlers * std_hnd = zend_get_std_object_handlers();
	    std_hnd->write_property(object, member, value, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
	    zval_dtor(member);
	}
}
/* }}} */

/* {{{ object_has_property */
static int object_has_property(zval *object, zval *member, int has_set_exists, const zend_literal *key TSRMLS_DC)
{
	php_event_abstract_object_t *obj;
	int                          ret = 0;
	php_event_prop_handler_t    p;

	obj = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);


	if (obj->prop_handler) {
		if (zend_hash_find(obj->prop_handler, Z_STRVAL_P(member),
					Z_STRLEN_P(member) + 1, (void **) &p) == SUCCESS) {
	    	switch (has_set_exists) {
	        	case 2:
	            	ret = 1;
	            	break;
	        	case 1: {
	                		zval *value = read_property(object, member, BP_VAR_IS, key TSRMLS_CC);
	                		if (value != EG(uninitialized_zval_ptr)) {
	                	    	convert_to_boolean(value);
	                	    	ret = Z_BVAL_P(value)? 1:0;
	                	    	/* refcount is 0 */
	                	    	Z_ADDREF_P(value);
	                	    	zval_ptr_dtor(&value);
	                		}
	                		break;
	                	}
	        	case 0:{
	                   	   zval *value = read_property(object, member, BP_VAR_IS, key TSRMLS_CC);
	                   	   if (value != EG(uninitialized_zval_ptr)) {
	                	   	   ret = Z_TYPE_P(value) != IS_NULL? 1:0;
	                	   	   /* refcount is 0 */
	                	   	   Z_ADDREF_P(value);
	                	   	   zval_ptr_dtor(&value);
	                   	   }
	                   	   break;
	               	   }
	        	default:
	               	   php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid value for has_set_exists");
	    	}
		} else {
	    	zend_object_handlers *std_hnd = zend_get_std_object_handlers();
	    	ret = std_hnd->has_property(object, member, has_set_exists, key TSRMLS_CC);
		}
	}
	return ret;
}
/* }}} */

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
/* {{{ object_get_debug_info */
static HashTable *object_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
	php_event_abstract_object_t *obj;
	HashTable                   *retval;
	HashTable                   *props;
	HashPosition                 pos;
	php_event_prop_handler_t *entry;

	obj   = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);
	props = obj->prop_handler;

	ALLOC_HASHTABLE(retval);

	ZEND_INIT_SYMTABLE_EX(retval, zend_hash_num_elements(props) + 1, 0);

	zend_hash_internal_pointer_reset_ex(props, &pos);
	while (zend_hash_get_current_data_ex(props, (void **) &entry, &pos) == SUCCESS) {
	    zval member;
	    zval *value;

	    INIT_ZVAL(member);
	    ZVAL_STRINGL(&member, entry->name, entry->name_len, 0);

	    value = read_property(object, &member, BP_VAR_IS, 0 TSRMLS_CC);
	    if (value != EG(uninitialized_zval_ptr)) {
	        Z_ADDREF_P(value);
	        zend_hash_add(retval, entry->name, entry->name_len + 1, &value, sizeof(zval *) , NULL);
	    }       

	    zend_hash_move_forward_ex(props, &pos);
	}               

	*is_temp = 1;   

	return retval;
}               
/* }}} */
#endif    

/* {{{ get_property_ptr_ptr */
static zval **get_property_ptr_ptr(zval *object, zval *member, const zend_literal *key TSRMLS_DC)
{
	php_event_abstract_object_t  *obj;
	zval                          tmp_member;
	zval                        **retval     = NULL;
	php_event_prop_handler_t     *hnd;
	int                           ret        = FAILURE;

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	obj = (php_event_abstract_object_t *) zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member) + 1, (void **) &hnd);
	}

	if (ret == FAILURE) {
		retval = zend_get_std_object_handlers()->get_property_ptr_ptr(object, member, key TSRMLS_CC);
	} else if (hnd->get_ptr_ptr_func) {
		retval = hnd->get_ptr_ptr_func(obj TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}

	return retval;
}
/* }}} */


#define PHP_EVENT_ADD_CLASS_PROPERTIES(a, b)                                           \
{                                                                                      \
    int i = 0;                                                                         \
    while (b[i].name != NULL) {                                                        \
        add_property((a), (b)[i].name, (b)[i].name_length,                             \
                (php_event_prop_read_t)(b)[i].read_func,                               \
                (php_event_prop_write_t)(b)[i].write_func,                             \
                (php_event_prop_get_prop_ptr_ptr_t)(b)[i].get_ptr_ptr_func TSRMLS_CC); \
        i++;                                                                           \
    }                                                                                  \
}

#define PHP_EVENT_DECL_CLASS_PROPERTIES(a, b)                            \
{                                                                        \
    int i = 0;                                                           \
    while (b[i].name != NULL) {                                          \
        zend_declare_property_null((a), (b)[i].name, (b)[i].name_length, \
                ZEND_ACC_PUBLIC TSRMLS_CC);                              \
        i++;                                                             \
    }                                                                    \
}

/* {{{ register_classes */
static zend_always_inline void register_classes(TSRMLS_D)
{
	zend_class_entry *ce;

	PHP_EVENT_REGISTER_CLASS("Event", event_object_create, php_event_ce, php_event_ce_functions);
	ce = php_event_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventBase", event_base_object_create, php_event_base_ce,
			php_event_base_ce_functions);
	ce = php_event_base_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	zend_hash_init(&event_base_properties, 0, NULL, NULL, 1);
	PHP_EVENT_ADD_CLASS_PROPERTIES(&event_base_properties, event_base_property_entries);
	PHP_EVENT_DECL_CLASS_PROPERTIES(ce, event_base_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &event_properties, sizeof(event_base_properties), NULL);

	PHP_EVENT_REGISTER_CLASS("EventConfig", event_config_object_create, php_event_config_ce,
			php_event_config_ce_functions);
	ce = php_event_config_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	zend_hash_init(&event_config_properties, 0, NULL, NULL, 1);
	PHP_EVENT_ADD_CLASS_PROPERTIES(&event_config_properties, event_config_property_entries);
	PHP_EVENT_DECL_CLASS_PROPERTIES(ce, event_config_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &event_config_properties, sizeof(event_config_properties), NULL);

	PHP_EVENT_REGISTER_CLASS("EventBufferEvent", event_bevent_object_create, php_event_bevent_ce,
			php_event_bevent_ce_functions);
	ce = php_event_bevent_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	zend_hash_init(&event_bevent_properties, 0, NULL, NULL, 1);
	PHP_EVENT_ADD_CLASS_PROPERTIES(&event_bevent_properties, event_bevent_property_entries);
	PHP_EVENT_DECL_CLASS_PROPERTIES(ce, event_bevent_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &event_bevent_properties, sizeof(event_bevent_properties), NULL);

	PHP_EVENT_REGISTER_CLASS("EventBuffer", event_buffer_object_create, php_event_buffer_ce,
			php_event_buffer_ce_functions);
	ce = php_event_buffer_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	zend_hash_init(&event_buffer_properties, 0, NULL, NULL, 1);
	PHP_EVENT_ADD_CLASS_PROPERTIES(&event_buffer_properties, event_buffer_property_entries);
	PHP_EVENT_DECL_CLASS_PROPERTIES(ce, event_buffer_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &event_buffer_properties, sizeof(event_buffer_properties), NULL);

#if HAVE_EVENT_EXTRA_LIB

	PHP_EVENT_REGISTER_CLASS("EventDnsBase", event_dns_base_object_create, php_event_dns_base_ce,
			php_event_dns_base_ce_functions);
	ce = php_event_dns_base_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventListener", event_listener_object_create, php_event_listener_ce,
			php_event_listener_ce_functions);
	ce = php_event_listener_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventHttpConnection", event_http_conn_object_create, php_event_http_conn_ce,
			php_event_http_conn_ce_functions);
	ce = php_event_http_conn_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventHttp", event_http_object_create, php_event_http_ce,
			php_event_http_ce_functions);
	ce = php_event_http_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

#endif /* HAVE_EVENT_EXTRA_LIB */

	PHP_EVENT_REGISTER_CLASS("EventUtil", event_util_object_create, php_event_util_ce,
			php_event_util_ce_functions);
	ce = php_event_util_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

}
/* }}} */

/* Private functions }}} */


#define REGISTER_EVENT_CLASS_CONST_LONG(pce, const_name, value) \
    zend_declare_class_constant_long((pce), #const_name,        \
            sizeof(#const_name) - 1, (long) value TSRMLS_CC)

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(event)
{
	zend_object_handlers *std_hnd = zend_get_std_object_handlers();

	memcpy(&object_handlers, std_hnd, sizeof(zend_object_handlers));

	object_handlers.clone_obj            = NULL;
	object_handlers.read_property        = read_property;
	object_handlers.write_property       = write_property;
	object_handlers.get_property_ptr_ptr = get_property_ptr_ptr;
	object_handlers.has_property         = object_has_property;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	object_handlers.get_debug_info       = object_get_debug_info;
#endif

	zend_hash_init(&classes, 0, NULL, NULL, 1);
	register_classes(TSRMLS_C);

	/* Loop flags */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, LOOP_ONCE,     EVLOOP_ONCE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, LOOP_NONBLOCK, EVLOOP_NONBLOCK);

	/* Run-time flags of event base usually passed to event_config_set_flag */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, NOLOCK,               EVENT_BASE_FLAG_NOLOCK);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, STARTUP_IOCP,         EVENT_BASE_FLAG_STARTUP_IOCP);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, NO_CACHE_TIME,        EVENT_BASE_FLAG_NO_CACHE_TIME);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, EPOLL_USE_CHANGELIST, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);
#ifdef EVENT_BASE_FLAG_IGNORE_ENV
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, IGNORE_ENV,           IGNORE_ENV);
#endif
#ifdef EVENT_BASE_FLAG_PRECISE_TIMER
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_base_ce, PRECISE_TIMER,        PRECISE_TIMER);
#endif

	/* Event flags */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, ET,      EV_ET);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, PERSIST, EV_PERSIST);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, READ,    EV_READ);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, WRITE,   EV_WRITE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, SIGNAL,  EV_SIGNAL);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_ce, TIMEOUT, EV_TIMEOUT);

	/* Features of event_base usually passed to event_config_require_features */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_config_ce, FEATURE_ET,  EV_FEATURE_ET);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_config_ce, FEATURE_O1,  EV_FEATURE_O1);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_config_ce, FEATURE_FDS, EV_FEATURE_FDS);

	/* Buffer event flags */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, READING,   BEV_EVENT_READING);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, WRITING,   BEV_EVENT_WRITING);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, EOF,       BEV_EVENT_EOF);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, ERROR,     BEV_EVENT_ERROR);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, TIMEOUT,   BEV_EVENT_TIMEOUT);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, CONNECTED, BEV_EVENT_CONNECTED);

	/* Option flags for bufferevents */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, OPT_CLOSE_ON_FREE,    BEV_OPT_CLOSE_ON_FREE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, OPT_THREADSAFE,       BEV_OPT_THREADSAFE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, OPT_DEFER_CALLBACKS,  BEV_OPT_DEFER_CALLBACKS);
#if LIBEVENT_VERSION_NUMBER >= 0x02000500
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_bevent_ce, OPT_UNLOCK_CALLBACKS, BEV_OPT_UNLOCK_CALLBACKS);
#endif

	/* Address families */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, AF_INET,   AF_INET);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, AF_INET6,  AF_INET6);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, AF_UNSPEC, AF_UNSPEC);

	/* DNS options */
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTION_SEARCH,      DNS_OPTION_SEARCH);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTION_NAMESERVERS, DNS_OPTION_NAMESERVERS);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTION_MISC,        DNS_OPTION_MISC);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTION_HOSTSFILE,   DNS_OPTION_HOSTSFILE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_dns_base_ce, OPTIONS_ALL,        DNS_OPTIONS_ALL);

#if HAVE_EVENT_EXTRA_LIB
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_LEAVE_SOCKETS_BLOCKING, LEV_OPT_LEAVE_SOCKETS_BLOCKING);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_CLOSE_ON_FREE,          LEV_OPT_CLOSE_ON_FREE);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_CLOSE_ON_EXEC,          LEV_OPT_CLOSE_ON_EXEC);
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_REUSEABLE,              LEV_OPT_REUSEABLE);
# if LIBEVENT_VERSION_NUMBER >= 0x02010100
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_DISABLED,               LEV_OPT_DISABLED);
# endif
# if LIBEVENT_VERSION_NUMBER >= 0x02000800
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_THREADSAFE,             LEV_OPT_THREADSAFE);
#endif
# if LIBEVENT_VERSION_NUMBER >= 0x02010100
	REGISTER_EVENT_CLASS_CONST_LONG(php_event_listener_ce, OPT_DEFERRED_ACCEPT,        LEV_OPT_DEFERRED_ACCEPT);
# endif
#endif

	REGISTER_EVENT_CLASS_CONST_LONG(php_event_util_ce, LIBEVENT_VERSION_NUMBER, LIBEVENT_VERSION_NUMBER);

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

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */

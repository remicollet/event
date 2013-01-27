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

/* {{{ event_generic_object_free_storage */
static zend_always_inline void event_generic_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) ptr;

	zend_object_std_dtor(&obj->zo TSRMLS_CC);

	efree(obj);
}
/* }}} */

/* {{{ event_object_free_storage */
static void event_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) ptr;

	PHP_EVENT_ASSERT(e && e->event);

	if (e->data) {
		/*zval_ptr_dtor(&e->data);*/
		Z_DELREF_P(e->data);
	}

	PHP_EVENT_FREE_FCALL_INFO(e->fci, e->fcc);

	if (e->stream_id >= 0) { /* stdin fd == 0 */
		zend_list_delete(e->stream_id);
	}

	event_del(e->event);
	event_free(e->event);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_base_object_free_storage */
static void event_base_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_base_t *b = (php_event_base_t *) ptr;

	PHP_EVENT_ASSERT(b && b->base);

	/* TODO: what if events bound to the event_base are not destroyed? */
	event_base_free(b->base);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_config_object_free_storage*/
static void event_config_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_config_t *cfg = (php_event_config_t *) ptr;

	PHP_EVENT_ASSERT(cfg && cfg->ptr);

	event_config_free(cfg->ptr);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_bevent_object_free_storage */
static void event_bevent_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_bevent_t *b = (php_event_bevent_t *) ptr;

	PHP_EVENT_ASSERT(b && b->bevent);

	if (b->data) {
		/*zval_ptr_dtor(&b->data);*/
		Z_DELREF_P(b->data);
	}

	PHP_EVENT_FREE_FCALL_INFO(b->fci_read,  b->fcc_read);
	PHP_EVENT_FREE_FCALL_INFO(b->fci_write, b->fcc_write);
	PHP_EVENT_FREE_FCALL_INFO(b->fci_event, b->fcc_event);

	if (b->stream_id >= 0) { /* stdin fd == 0 */
		zend_list_delete(b->stream_id);
	}

	if (b->self) {
		Z_DELREF_P(b->self);
	}

	bufferevent_free(b->bevent);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_buffer_object_free_storage */
static void event_buffer_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) ptr;

	PHP_EVENT_ASSERT(b && b->buf);

	evbuffer_free(b->buf);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_dns_base_object_free_storage */
static void event_dns_base_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_dns_base_t *dnsb = (php_event_dns_base_t *) ptr;

	PHP_EVENT_ASSERT(dnsb && dnsb->dns_base);

	/* Setting fail_requests to 1 makes all in-flight requests get
	 * their callbacks invoked with a canceled error code before it
	 * frees the base*/
	evdns_base_free(dnsb->dns_base, 1);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_listener_object_free_storage */
static void event_listener_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_listener_t *l = (php_event_listener_t *) ptr;

	PHP_EVENT_ASSERT(l && l->listener);
	if (l->stream_id >= 0) {
		zend_list_delete(l->stream_id);
	}

	if (l->base) {
		Z_DELREF_P(l->base);
	}

	if (l->data) {
		Z_DELREF_P(l->data);
	}

	if (l->self) {
		Z_DELREF_P(l->self);
	}

	PHP_EVENT_FREE_FCALL_INFO(l->fci, l->fcc);
	PHP_EVENT_FREE_FCALL_INFO(l->fci_err, l->fcc_err);

	evconnlistener_free(l->listener);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_conn_object_free_storage */
static void event_http_conn_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_http_conn_t *evcon = (php_event_http_conn_t *) ptr;

	PHP_EVENT_ASSERT(evcon && evcon->conn);

	if (evcon->base) {
		Z_DELREF_P(evcon->base);
	}

	if (evcon->dns_base) {
		Z_DELREF_P(evcon->dns_base);
	}

	evhttp_connection_free(evcon->conn);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_object_free_storage */
static void event_http_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_http_t *http = (php_event_http_t *) ptr;

	PHP_EVENT_ASSERT(http && http->ptr);

	if (http->base) {
		Z_DELREF_P(http->base);
	}

	evhttp_free(http->ptr);

	event_generic_object_free_storage(ptr TSRMLS_CC);
}
/* }}} */


/* {{{ register_object */
static zend_always_inline zend_object_value register_object(zend_class_entry *ce, void *obj, zend_objects_free_object_storage_t func_free_storage TSRMLS_DC)
{
	zend_object_value retval;

	retval.handle = zend_objects_store_put(obj,
			(zend_objects_store_dtor_t) zend_objects_destroy_object,
			func_free_storage, NULL TSRMLS_CC);
	retval.handlers = &object_handlers;

	return retval;
}
/* }}} */

/* {{{ object_new
 * Allocates new object with it's properties.
 * size is a size of struct implementing php_event_abstract_object_t */
static void *object_new(zend_class_entry *ce, size_t size TSRMLS_DC)
{
	void *ptr;
	php_event_abstract_object_t *obj;
	zend_class_entry *ce_parent = ce;

	ptr = ecalloc(1, sizeof(size));
	obj = (php_event_abstract_object_t *) ptr;

	while (ce_parent->type != ZEND_INTERNAL_CLASS && ce_parent->parent != NULL) {
		ce_parent = ce_parent->parent;
	}
	zend_hash_find(&classes, ce_parent->name, ce_parent->name_length + 1,
			(void **) &obj->prop_handler);

	zend_object_std_init(&obj->zo, ce TSRMLS_CC);
	object_properties_init(&obj->zo, ce);

	return ptr;
}
/* }}} */


/* {{{ event_object_create
 * Event object ctor */
static zend_object_value event_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_base_object_create
 * EventBase object ctor */
static zend_object_value event_base_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_base_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_base_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_config_object_create
 * EventConfig object ctor */
static zend_object_value event_config_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_config_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_config_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_bevent_object_create
 * EventBufferEvent object ctor */
static zend_object_value event_bevent_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_bevent_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_bevent_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_buffer_object_create
 * EventBuffer object ctor */
static zend_object_value event_buffer_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_buffer_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_buffer_object_free_storage TSRMLS_CC);
}
/* }}} */

#if HAVE_EVENT_EXTRA_LIB

/* {{{ event_dns_base_object_create
 * EventDnsBase object ctor */
static zend_object_value event_dns_base_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_dns_base_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_dns_base_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_listener_object_create
 * EventListener object ctor */
static zend_object_value event_listener_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_listener_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_listener_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_conn_object_create
 * EventHttpConnection object ctor */
static zend_object_value event_http_conn_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_http_conn_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_http_conn_object_free_storage TSRMLS_CC);
}
/* }}} */

/* {{{ event_http_object_create
 * EventHttp object ctor */
static zend_object_value event_http_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_http_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_http_object_free_storage TSRMLS_CC);
}
/* }}} */

#endif /* HAVE_EVENT_EXTRA_LIB */

/* {{{ event_util_object_create
 * EventUtil object ctor */
static zend_object_value event_util_object_create(zend_class_entry *ce TSRMLS_DC)
{
	/* EventUtil is a singleton. This function must never be called */
	PHP_EVENT_ASSERT(0);

	php_event_abstract_object_t *obj = (php_event_abstract_object_t *) object_new(ce, sizeof(php_event_abstract_object_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_generic_object_free_storage TSRMLS_CC);
}
/* }}} */

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

	PHP_EVENT_REGISTER_CLASS("EventConfig", event_config_object_create, php_event_config_ce,
			php_event_config_ce_functions);
	ce = php_event_config_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventBufferEvent", event_bevent_object_create, php_event_bevent_ce,
			php_event_bevent_ce_functions);
	ce = php_event_bevent_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	PHP_EVENT_REGISTER_CLASS("EventBuffer", event_buffer_object_create, php_event_buffer_ce,
			php_event_buffer_ce_functions);
	ce = php_event_buffer_ce;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

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


/* Private functions }}} */


#define PHP_EVENT_REG_CONST_LONG(name, real_name) \
    REGISTER_LONG_CONSTANT(#name, real_name, CONST_CS | CONST_PERSISTENT);

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

	/* XXX Move constants to corresponding classes */

	/* Loop flags */
	PHP_EVENT_REG_CONST_LONG(EVENT_LOOP_ONCE,     EVLOOP_ONCE);
	PHP_EVENT_REG_CONST_LONG(EVENT_LOOP_NONBLOCK, EVLOOP_NONBLOCK);

	/* Event flags */
	PHP_EVENT_REG_CONST_LONG(EVENT_ET,      EV_ET);
	PHP_EVENT_REG_CONST_LONG(EVENT_PERSIST, EV_PERSIST);
	PHP_EVENT_REG_CONST_LONG(EVENT_READ,    EV_READ);
	PHP_EVENT_REG_CONST_LONG(EVENT_WRITE,   EV_WRITE);
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

	/* Address families */
	PHP_EVENT_REG_CONST_LONG(EVENT_AF_INET,   AF_INET);
	PHP_EVENT_REG_CONST_LONG(EVENT_AF_INET6,  AF_INET6);
	PHP_EVENT_REG_CONST_LONG(EVENT_AF_UNSPEC, AF_UNSPEC);

	/* DNS options */
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTION_SEARCH,      DNS_OPTION_SEARCH);
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTION_NAMESERVERS, DNS_OPTION_NAMESERVERS);
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTION_MISC,        DNS_OPTION_MISC);
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTION_HOSTSFILE,   DNS_OPTION_HOSTSFILE);
	PHP_EVENT_REG_CONST_LONG(EVENT_DNS_OPTIONS_ALL,        DNS_OPTIONS_ALL);

#if HAVE_EVENT_EXTRA_LIB
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_LEAVE_SOCKETS_BLOCKING, LEV_OPT_LEAVE_SOCKETS_BLOCKING);
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_CLOSE_ON_FREE,          LEV_OPT_CLOSE_ON_FREE);
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_CLOSE_ON_EXEC,          LEV_OPT_CLOSE_ON_EXEC);
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_REUSEABLE,              LEV_OPT_REUSEABLE);
# if LIBEVENT_VERSION_NUMBER >= 0x02010100
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_DISABLED,               LEV_OPT_DISABLED);
# endif
# if LIBEVENT_VERSION_NUMBER >= 0x02000800
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_THREADSAFE,             LEV_OPT_THREADSAFE);
#endif
# if LIBEVENT_VERSION_NUMBER >= 0x02010100
	PHP_EVENT_REG_CONST_LONG(EVENT_LEV_OPT_DEFERRED_ACCEPT,        LEV_OPT_DEFERRED_ACCEPT);
# endif
#endif

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

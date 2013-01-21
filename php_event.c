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

#if HAVE_EVENT_EXTRA_LIB
zend_class_entry *php_event_dns_base_ce;
zend_class_entry *php_event_listener_ce;
zend_class_entry *php_event_http_conn_ce;
zend_class_entry *php_event_http_ce;
#endif

static HashTable classes;

static object_handlers;

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
	event_functions,
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

/* {{{ event_object_free_storage */
static void event_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) ptr;

	if (e->data) {
		zval_ptr_dtor(&e->data);
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

/* {{{ event_generic_object_free_storage */
static zend_always_inline void event_generic_object_free_storage(void *ptr TSRMLS_DC)
{
	php_event_object_t *obj = (php_event_object_t *) ptr;
	
	zend_object_std_dtor(&obj->zo TSRMLS_CC);

	efree(obj);
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
static php_event_abstract_object_t *object_new(zend_class_entry *ce, size_t size TSRMLS_DC)
{
	php_event_abstract_object_t *obj;
	zend_class_entry *ce_parent = ce;

	obj = ecalloc(1, sizeof(size));

	while (ce_parent->type != ZEND_INTERNAL_CLASS && ce_parent->parent != NULL) {
		ce_parent = ce_parent->parent;
	}
	zend_hash_add(&classes, ce_parent->name, ce_parent->name_length + 1,
			(void **) &obj->prop_handler);

	zend_object_std_init(&obj->zo, ce TSRMLS_CC);
	object_properties_init(&obj->zo, ce);
}
/* }}} */


/* {{{ event_object_create
 * Event object ctor */
static zend_object_value event_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_event_object_t *obj = (php_event_object_t *)
		object_new(ce, sizeof(php_event_object_t) TSRMLS_CC);

	return register_object(ce, (void *) obj, event_object_free_storage TSRMLS_CC);
}
/* }}} */


/* {{{ register_classes */
static zend_always_inline register_classes(TSRMLS_D)
{
	zend_class_entry *ce;

	PHP_EVENT_REGISTER_CLASS("Event", event_object_create, php_event_ce, php_event_ce_functions);
	ce = php_event_ce;
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

/* {{{ sockaddr_parse
 * Parse in_addr and fill out_arr with IP and port.
 * out_arr must be a pre-allocated empty zend array */
static int sockaddr_parse(const struct sockaddr *in_addr, zval *out_zarr)
{
	char buf[256];
	int  ret      = FAILURE;

	PHP_EVENT_ASSERT(Z_TYPE_P(out_zarr) == IS_ARRAY);

	switch (in_addr->sa_family) {
		case AF_INET:
			if (evutil_inet_ntop(in_addr->sa_family, &((struct sockaddr_in *) in_addr)->sin_addr,
						(void *) &buf, sizeof(buf))) {
				add_next_index_string(out_zarr, (char *)&buf, 1);
				add_next_index_long(out_zarr,
						ntohs(((struct sockaddr_in *) in_addr)->sin_port));

				ret = SUCCESS;
			}
			break;
#if HAVE_IPV6 && HAVE_INET_NTOP
		case AF_INET6:
			if (evutil_inet_ntop(in_addr->sa_family, &((struct sockaddr_in6 *) in_addr)->sin6_addr,
						(void *) &buf, sizeof(buf))) {
				add_next_index_string(out_zarr, (char *)&buf, 1);
				add_next_index_long(out_zarr,
						ntohs(((struct sockaddr_in6 *) in_addr)->sin6_port));

				ret = SUCCESS;
			}
			break;
#endif
#ifdef AF_UNIX
		case AF_UNIX:
			{
				struct sockaddr_un *ua = (struct sockaddr_un *) in_addr;

				if (ua->sun_path[0] == '\0') {
					/* abstract name */
 					zval *tmp;
					int len = strlen(ua->sun_path + 1) + 1;

    				MAKE_STD_ZVAL(tmp);
    				ZVAL_STRINGL(tmp, ua->sun_path, len, 1);
    				Z_STRVAL_P(tmp)[len] = '\0';

    				zend_hash_next_index_insert(Z_ARRVAL_P(out_zarr), &tmp, sizeof(zval *), NULL);
				} else {
					add_next_index_string(out_zarr, ua->sun_path, 1);
				}
			}
			break;
#endif
	}

	return ret;
}
/* }}} */



/* {{{ bevent_rw_cb
 * Is called from the bufferevent read and write callbacks */
static zend_always_inline void bevent_rw_cb(struct bufferevent *bevent, php_event_bevent_t *bev, zend_fcall_info *pfci, zend_fcall_info_cache *pfcc)
{
	PHP_EVENT_ASSERT(bev);
	PHP_EVENT_ASSERT(pfci && pfcc);

	zval  *arg_data   = bev->data;
	zval  *arg_bevent;
	zval **args[2];
	zval  *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(bev->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */
		MAKE_STD_ZVAL(arg_bevent);

		PHP_EVENT_ASSERT(bev->rsrc_id > 0);

		if (bev->rsrc_id > 0) {
			ZVAL_RESOURCE(arg_bevent, bev->rsrc_id);
			zend_list_addref(bev->rsrc_id);
		} else {
			ZVAL_NULL(arg_bevent);
		}
		args[0] = &arg_bevent;

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[1] = &arg_data;

		/* Prepare callback */
		pfci->params		 = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count	 = 2;
		pfci->no_separation  = 1;

        if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS
                && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_bevent);
        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* {{{ bevent_read_cb */
static void bevent_read_cb(struct bufferevent *bevent, void *ptr)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) ptr;

	bevent_rw_cb(bevent, bev, bev->fci_read, bev->fcc_read);
}
/* }}} */

/* {{{ bevent_write_cb */
static void bevent_write_cb(struct bufferevent *bevent, void *ptr)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) ptr;

	bevent_rw_cb(bevent, bev, bev->fci_write, bev->fcc_write);
}
/* }}} */

/* {{{ bevent_event_cb */
static void bevent_event_cb(struct bufferevent *bevent, short events, void *ptr)
{
	php_event_bevent_t    *bev  = (php_event_bevent_t *) ptr;
	zend_fcall_info       *pfci = bev->fci_event;
	zend_fcall_info_cache *pfcc = bev->fcc_event;

	PHP_EVENT_ASSERT(pfci && pfcc);

	zval  *arg_data   = bev->data;
	zval  *arg_bevent;
	zval  *arg_events;
	zval **args[3];
	zval  *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(bev->thread_ctx);

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */
		MAKE_STD_ZVAL(arg_bevent);

		PHP_EVENT_ASSERT(bev->rsrc_id > 0);

		if (bev->rsrc_id >= 0) {
			ZVAL_RESOURCE(arg_bevent, bev->rsrc_id);
			zend_list_addref(bev->rsrc_id);
		} else {
			ZVAL_NULL(arg_bevent);
		}
		args[0] = &arg_bevent;

		MAKE_STD_ZVAL(arg_events);
		ZVAL_LONG(arg_events, events);
		args[1] = &arg_events;

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[2] = &arg_data;

		/* Prepare callback */
		pfci->params		 = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count	 = 3;
		pfci->no_separation  = 1;

        if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS
                && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_bevent);
        zval_ptr_dtor(&arg_events);
        zval_ptr_dtor(&arg_data);
	}

}
/* }}} */

/* {{{ _php_event_listener_cb */
static void _php_event_listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {

	php_event_listener_t  *l = (php_event_listener_t *) ctx;

	PHP_EVENT_ASSERT(l);

	zend_fcall_info       *pfci = l->fci;
	zend_fcall_info_cache *pfcc = l->fcc;

	PHP_EVENT_ASSERT(pfci && pfcc);

	zval  **args[4];
	zval   *arg_listener;
	zval   *arg_fd;
	zval   *arg_address;
	zval   *arg_data     = l->data;
	zval   *retval_ptr;

	php_stream *stream;

	TSRMLS_FETCH_FROM_CTX(l->thread_ctx);

	/* Call user function having proto:
	 * void cb (resource $listener, resource $fd, array $address, mixed $data);
	 * $address = array ("IP-address", *server* port)
	 * Note, address contains the server port(not the one user passed to ex.
	 * evconnlistener_new_bind()!
	 */

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		MAKE_STD_ZVAL(arg_listener);
		ZVAL_RESOURCE(arg_listener, l->rsrc_id);
		zend_list_addref(l->rsrc_id);
		args[0] = &arg_listener;

		/* Convert the socket created by libevent to PHP stream
	 	 * and save it's resource ID in l->stream_id */

		if (l->stream_id > 0) {
			MAKE_STD_ZVAL(arg_fd);
			ZVAL_RESOURCE(arg_fd, l->stream_id);
			zend_list_addref(l->stream_id);
		} else {
			stream = php_stream_fopen_from_fd(fd, "r", NULL);
			if (stream) {
				MAKE_STD_ZVAL(arg_fd);
				php_stream_to_zval(stream, arg_fd);

				l->stream_id = Z_LVAL_P(arg_fd);
				zend_list_addref(l->stream_id);
			} else {
				l->stream_id = -1;
				ALLOC_INIT_ZVAL(arg_fd);
			}
		}
		args[1] = &arg_fd;

		MAKE_STD_ZVAL(arg_address);
		array_init(arg_address);
		sockaddr_parse(address, arg_address);
		args[2] = &arg_address;

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[3] = &arg_data;

 		/* Prepare callback */
        pfci->params         = args;
        pfci->retval_ptr_ptr = &retval_ptr;
        pfci->param_count    = 4;
        pfci->no_separation  = 1;

        if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_listener);
        zval_ptr_dtor(&arg_fd);
        zval_ptr_dtor(&arg_address);
        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

/* {{{ listener_error_cb */
static void listener_error_cb(struct evconnlistener *listener, void *ctx) {
	php_event_listener_t  *l = (php_event_listener_t *) ctx;

	PHP_EVENT_ASSERT(l);

	zend_fcall_info       *pfci = l->fci_err;
	zend_fcall_info_cache *pfcc = l->fcc_err;

	PHP_EVENT_ASSERT(pfci && pfcc);

	zval  **args[2];
	zval   *arg_listener;
	zval   *arg_data     = l->data;
	zval   *retval_ptr;

	TSRMLS_FETCH_FROM_CTX(l->thread_ctx);

	/* Call user function having proto:
	 * void cb (resource $listener, mixed $data); */

	if (ZEND_FCI_INITIALIZED(*pfci)) {
		MAKE_STD_ZVAL(arg_listener);
		ZVAL_RESOURCE(arg_listener, l->rsrc_id);
		zend_list_addref(l->rsrc_id);
		args[0] = &arg_listener;

		if (arg_data) {
			Z_ADDREF_P(arg_data);
		} else {
			ALLOC_INIT_ZVAL(arg_data);
		}
		args[1] = &arg_data;

 		/* Prepare callback */
        pfci->params         = args;
        pfci->retval_ptr_ptr = &retval_ptr;
        pfci->param_count    = 2;
        pfci->no_separation  = 1;

        if (zend_call_function(pfci, pfcc TSRMLS_CC) == SUCCESS && retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "An error occurred while invoking the callback");
        }

        zval_ptr_dtor(&arg_listener);
        zval_ptr_dtor(&arg_data);
	}
}
/* }}} */

#if 0
/* {{{ php_event_dtor */
static void php_event_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) rsrc->ptr;

	if (e) {
		if (e->data) {
			zval_ptr_dtor(&e->data);
		}

		PHP_EVENT_FREE_FCALL_INFO(e->fci, e->fcc);

		if (e->stream_id >= 0) { /* stdin fd == 0 */
			zend_list_delete(e->stream_id);
		}
		event_del(e->event);
		event_free(e->event);
		efree(e);
	}
}
/* }}} */
#endif

/* {{{ php_event_base_dtor */
static void php_event_base_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	/* TODO: what if events bound to the event_base are not destroyed? */
	php_event_base_t *b= (php_event_base_t *) rsrc->ptr;

	if (b) {
		event_base_free(b->base);
		efree(b);
	}
}
/* }}} */

/* {{{ php_event_config_dtor */
static void php_event_config_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	struct event_config *cfg = (struct event_config *) rsrc->ptr;

	event_config_free(cfg);
}
/* }}} */

/* {{{ php_event_bevent_dtor */
static void php_event_bevent_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_bevent_t *b = (php_event_bevent_t *) rsrc->ptr;

	if (b->data) {
		zval_ptr_dtor(&b->data);
	}

	PHP_EVENT_FREE_FCALL_INFO(b->fci_read,  b->fcc_read);
	PHP_EVENT_FREE_FCALL_INFO(b->fci_write, b->fcc_write);
	PHP_EVENT_FREE_FCALL_INFO(b->fci_event, b->fcc_event);

	if (b->stream_id >= 0) { /* stdin fd == 0 */
		zend_list_delete(b->stream_id);
	}

	bufferevent_free(b->bevent);

	efree(b);
}
/* }}} */

/* {{{ php_event_buffer_dtor*/
static void php_event_buffer_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) rsrc->ptr;

	if (b) {
		evbuffer_free(b->buf);
		efree(b);
	}
}
/* }}} */

/* {{{ php_event_dns_base_dtor */
static void php_event_dns_base_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_dns_base_t *dnsb = (php_event_dns_base_t *) rsrc->ptr;

	if (dnsb) {
		/* Setting fail_requests to 1 makes all in-flight requests get
	 	 * their callbacks invoked with a canceled error code before it
	 	 * frees the base*/
		evdns_base_free(dnsb->dns_base, 1);

		efree(dnsb);
	}
}
/* }}} */

/* {{{ php_event_listener_dtor */
static void php_event_listener_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_listener_t *l = (php_event_listener_t *) rsrc->ptr;

	if (l) {
		if (l->stream_id >= 0) {
			zend_list_delete(l->stream_id);
		}

		if (l->base_id) {
			zend_list_delete(l->base_id);
		}

		if (l->data) {
			zval_ptr_dtor(&l->data);
		}

		PHP_EVENT_FREE_FCALL_INFO(l->fci, l->fcc);
		PHP_EVENT_FREE_FCALL_INFO(l->fci_err, l->fcc_err);

		evconnlistener_free(l->listener);

		efree(l);
	}
}
/* }}} */

/* {{{ php_event_http_conn_dtor */
static void php_event_http_conn_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_http_conn_t *evcon = (php_event_http_conn_t *) rsrc->ptr;

	if (evcon) {
		if (evcon->base_id) {
			zend_list_delete(evcon->base_id);
		}

		if (evcon->dns_base_id) {
			zend_list_delete(evcon->dns_base_id);
		}

		evhttp_connection_free(evcon->conn);

		efree(evcon);
	}
}
/* }}} */

/* {{{ php_event_http_dtor */
static void php_event_http_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_event_http_t *http = (php_event_http_t *) rsrc->ptr;

	if (http) {
		if (http->base_id) {
			zend_list_delete(http->base_id);
		}

		evhttp_free(http->ptr);

		efree(http);
	}
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

/* {{{ has_property */
static int has_property(zval *object, zval *member, int has_set_exists, const zend_literal *key TSRMLS_DC)
{
	php_event_abstract_object_t *obj;
	int                          ret = 0;
	php_ev_prop_handler          p;

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
	php_ev_prop_handler         *entry;

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
	php_ev_prop_handler          *hnd;
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
	object_handlers.has_property         = has_property;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	object_handlers.get_debug_info       = get_debug_info;
#endif

	zend_hash_init(&classes, 0, NULL, NULL, 1);
	register_classes(TSRMLS_C);

	le_event_base      = zend_register_list_destructors_ex(php_event_base_dtor,      NULL, PHP_EVENT_BASE_RES_NAME,      module_number);
	le_event_config    = zend_register_list_destructors_ex(php_event_config_dtor,    NULL, PHP_EVENT_CONFIG_RES_NAME,    module_number);
	le_event_bevent    = zend_register_list_destructors_ex(php_event_bevent_dtor,    NULL, PHP_EVENT_BEVENT_RES_NAME,    module_number);
	le_event_buffer    = zend_register_list_destructors_ex(php_event_buffer_dtor,    NULL, PHP_EVENT_BUFFER_RES_NAME,    module_number);
	le_event_dns_base  = zend_register_list_destructors_ex(php_event_dns_base_dtor,  NULL, PHP_EVENT_DNS_BASE_RES_NAME,  module_number);
	le_event_listener  = zend_register_list_destructors_ex(php_event_listener_dtor,  NULL, PHP_EVENT_LISTENER_RES_NAME,  module_number);
	le_event_http_conn = zend_register_list_destructors_ex(php_event_http_conn_dtor, NULL, PHP_EVENT_HTTP_CONN_RES_NAME, module_number);
	le_event_http      = zend_register_list_destructors_ex(php_event_http_dtor, NULL, PHP_EVENT_HTTP_RES_NAME, module_number);

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

/* {{{ API functions */




/* {{{ proto resource event_config_new(void);
 * On success returns a valid resource representing an event configuration
 * which can be passed to <function>event_base_new_with_config</function>. Otherwise returns &false;. */
PHP_FUNCTION(event_config_new)
{
	php_event_config_t *cfg;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	cfg = event_config_new();

	if (cfg) {
		ZEND_REGISTER_RESOURCE(return_value, cfg, le_event_config);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool event_config_avoid_method(resource cfg, string method);
 * Tells libevent to avoid specific event method.
 * See http://www.wangafu.net/~nickm/libevent-book/Ref2_eventbase.html#_creating_an_event_base
 * Returns &true; on success, otherwise &false;.*/
PHP_FUNCTION(event_config_avoid_method)
{
	zval               *zcfg;
	char               *method;
	int                 method_len;
	php_event_config_t *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
				&zcfg, &method, &method_len) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(cfg, php_event_config_t *, &zcfg, -1,
		PHP_EVENT_CONFIG_RES_NAME, le_event_config);

	if (event_config_avoid_method(cfg, method)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool event_config_require_features(resource cfg, int feature);
 * Enters a required event method feature that the application demands. */
PHP_FUNCTION(event_config_require_features)
{
	zval               *zcfg;
	long                feature;
	php_event_config_t *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zcfg, &feature) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(cfg, php_event_config_t *, &zcfg, -1,
		PHP_EVENT_CONFIG_RES_NAME, le_event_config);

	if (event_config_require_features(cfg, feature)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02010000
/* {{{ proto void event_config_set_max_dispatch_interval(resource cfg, int max_interval, int max_callbacks, int min_priority);
 * Prevents priority inversion by limiting how many low-priority event
 * callbacks can be invoked before checking for more high-priority events.
 * Available since libevent 2.1.0-alpha. */
PHP_FUNCTION(event_config_set_max_dispatch_interval)
{
	zval                  *zcfg;
	php_event_timestamp_t  max_interval;
	long                   max_callbacks;
	long                   min_priority;
	php_event_config_t    *cfg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rdll",
				&zcfg, &max_interval, &max_callbacks, min_priority) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(cfg, php_event_config_t *, &zcfg, -1,
		PHP_EVENT_CONFIG_RES_NAME, le_event_config);

	if (max_interval > 0) {
		struct timeval tv;
		PHP_EVENT_TIMEVAL_SET(tv, max_interval);

		event_config_set_max_dispatch_interval(cfg, &tv, max_callbacks, min_priority);
	} else {
		event_config_set_max_dispatch_interval(cfg, NULL, max_callbacks, min_priority);
	}
}
/* }}} */
#endif



/* {{{ proto resource bufferevent_socket_new(resource base[, resource socket = NULL[, int options = 0]]);
 *
 * Create a socket-based bufferevent.
 * options is one of EVENT_BEV_OPT_* constants, or 0.
 * Passing NULL to socket parameter means that the socket stream should be created later,
 * e.g. by means of bufferevent_socket_connect().
 *
 * socket parameter may be created as a stream(not necessarily by means of sockets extension)
 *
 * Returns bufferevent resource optionally associated with socket resource. */
PHP_FUNCTION(bufferevent_socket_new)
{
	zval                *zbase;
	php_event_base_t    *base;
	zval               **ppzfd   = NULL;
	evutil_socket_t      fd;
	long                 options = 0;
	php_event_bevent_t  *bev;
	struct bufferevent  *bevent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|Z!l",
				&zbase, &ppzfd, &options) == FAILURE) {
		return;
	}

	if (ppzfd) {
#ifdef PHP_EVENT_SOCKETS_SUPPORT 
		if (ppzfd) {
			/* php_event_zval_to_fd reports error
	 	 	 * in case if it is not a valid socket resource */
			/*fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);*/
			fd = php_event_zval_to_fd(ppzfd TSRMLS_CC);

		}

		if (fd < 0) {
			RETURN_FALSE;
		}
		/* Make sure that the socket is in non-blocking mode(libevent's tip) */
		evutil_make_socket_nonblocking(fd);
#else
		fd = -1;
#endif
	} else {
 		/* User decided to assign fd later,
 		 * e.g. by means of bufferevent_socket_connect()
 		 * which allocates new socket stream in this case. */
		fd = -1;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	bev = emalloc(sizeof(php_event_bevent_t));
	memset(bev, 0, sizeof(php_event_bevent_t));

	bevent = bufferevent_socket_new(base->base, fd, options);
	if (bevent == NULL) {
		efree(bev);
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate bufferevent for socket");
		RETURN_FALSE;
	}

	bev->bevent = bevent;

	if (ppzfd) {
		/* lval of ppzfd is the resource ID */
		bev->stream_id = Z_LVAL_PP(ppzfd);
		zend_list_addref(Z_LVAL_PP(ppzfd));
	} else {
		/* Should be assigned in bufferevent_socket_connect() later
		 * (by means of bufferevent_getfd()) */
		bev->stream_id = -1;
	}

	/* Make sure base destroyed after the bufferevent */
	zend_list_addref(base->rsrc_id);

	bev->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, bev, le_event_bevent);
	/* XXX Normally I wouldn't do it. But if one creates the resource inside a callback,
	 * particularly called by libevent->event, the resource is destroyed on return automatically.
	 * See examples/listener.php
	 * Maybe add some userspace function like bufferevent_ref() */
	zend_list_addref(bev->rsrc_id);
}
/* }}} */

/* {{{ proto array bufferevent_pair_new(resource base[, int options = 0]);
 *
 * options is one of EVENT_BEV_OPT_* constants, or 0.
 *
 * Returns array of two buffer event resources, each connected to the other.
 * All the usual options are supported, except for EVENT_BEV_OPT_CLOSE_ON_FREE,
 * which has no effect, and EVENT_BEV_OPT_DEFER_CALLBACKS, which is always on.
 */
PHP_FUNCTION(bufferevent_pair_new)
{
	zval               *zbase;
	php_event_base_t   *base;
	long                options        = 0;
	php_event_bevent_t *b[2];
	struct bufferevent *bevent_pair[2];
	int                 i;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l",
				&zbase, &options) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	if (bufferevent_pair_new(base->base, options, bevent_pair)) {
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i = 0; i < 2; i++) {
		b[i] = emalloc(sizeof(php_event_bevent_t));
		memset(b[i], 0, sizeof(php_event_bevent_t));

		b[i]->bevent    = bevent_pair[i];
		b[i]->stream_id = -1;
		b[i]->rsrc_id   = zend_list_insert(b, le_event_bevent TSRMLS_CC);

		add_next_index_resource(return_value, b[i]->rsrc_id);
	}
}
/* }}} */

/* {{{ proto void bufferevent_free(resource bevent);
 * Free a buffer event resource. */
PHP_FUNCTION(bufferevent_free)
{
	php_event_bevent_t *bev;
	zval               *zbevent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	zend_list_delete(bev->rsrc_id);
}

/* }}} */

/* {{{ proto bool bufferevent_socket_connect(resource bevent, string addr[, bool sync_resolve = FALSE]);
 *
 * Connect bufferevent's socket to given address(optionally with port).  The
 * function available since libevent 2.0.2-alpha.
 *
 * This function doesn't require sockets support. If socket is not assigned to
 * the bufferevent, this function allocates a socket stream and makes it
 * non-blocking internally.
 *
 * If sync_resolve parameter is TRUE, the function tries to resolve the
 * hostname within addr *syncronously*(!).  Otherwise addr parameter expected
 * to be an IP address with optional port number. Recognized formats are:
 *
 *    [IPv6Address]:port
 *    [IPv6Address]
 *    IPv6Address
 *    IPv4Address:port
 *    IPv4Address
 * 
 * To resolve DNS names asyncronously, use
 * bufferevent_socket_connect_hostname() function.
 */
PHP_FUNCTION(bufferevent_socket_connect)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	char               *addr;
	int                 addr_len;
	struct sockaddr     sa;
	socklen_t           sa_len = sizeof(struct sockaddr);
	zend_bool           sync_resolve = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|b",
				&zbevent, &addr, &addr_len, &sync_resolve) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (sync_resolve) {
		/* The PHP API *syncronously* resolves hostname, if it doesn't look
		 * like IP(v4/v6) */
		if (php_network_parse_network_address_with_port(addr, addr_len, &sa, &sa_len TSRMLS_CC) != SUCCESS) {
			/* The function reports errors, if necessary */
			RETURN_FALSE;
		}
	} else {
		/* Numeric addresses only. Don't try to resolve hostname. */
		if (evutil_parse_sockaddr_port(addr, &sa, (int *) &sa_len)) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR,
					"Failed parsing address: the address is not well-formed, "
					"or the port is out of range");
			RETURN_FALSE;
		}
	}

	/* bufferevent_socket_connect() allocates a socket stream internally, if we
	 * didn't provide the file descriptor to the bufferevent before, e.g. with
	 * bufferevent_socket_new() */
	if (bufferevent_socket_connect(bev->bevent, &sa, sa_len)) {
		RETURN_FALSE;
	}

	bev->stream_id = bufferevent_getfd(bev->bevent);

	PHP_EVENT_ASSERT(bev->stream_id >= 0);

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool bufferevent_socket_connect_hostname(resource bevent, resource dns_base, string hostname, int port[, int family = EVENT_AF_UNSPEC]);
 *
 * Resolves the DNS name hostname, looking for addresses of type
 * family(EVENT_AF_* constants). If the name resolution fails, it invokes the
 * event callback with an error event. If it succeeds, it launches a connection
 * attempt just as bufferevent_socket_connect would.
 *
 * dns_base is optional. May be NULL, or a resource created with
 * event_dns_base_new()(requires --with-event-extra configure option).
 * For asyncronous hostname resolving pass a valid event dns base resource.
 * Otherwise the hostname resolving will block.
 * Recognized hostname formats are:
 * www.example.com (hostname) 1.2.3.4 (ipv4address) ::1 (ipv6address) [::1] ([ipv6address])
 */
PHP_FUNCTION(bufferevent_socket_connect_hostname)
{
#if LIBEVENT_VERSION_NUMBER < 0x02000300
	PHP_EVENT_LIBEVENT_VERSION_REQUIRED(bufferevent_socket_connect_hostname, 2.0.3-alpha);
	RETVAL_FALSE;
#else
	php_event_bevent_t   *bev;
	zval                 *zbevent;
	zval                 *zdns_base;
	char                 *hostname;
	int                   hostname_len;
	long                  port;
	long                  family;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr!sl|l",
				&zbevent, &zdns_base, &hostname, &hostname_len,
				&port, &family) == FAILURE) {
		return;
	}
	
	if (family & ~(AF_INET | AF_INET6 | AF_UNSPEC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid address family specified");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	/* bufferevent_socket_connect() allocates a socket stream internally, if we
	 * didn't provide the file descriptor to the bufferevent before, e.g. with
	 * bufferevent_socket_new() */

#if HAVE_EVENT_EXTRA_LIB
	php_event_dns_base_t *dnsb;

	if (zdns_base) {
		PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);
		PHP_EVENT_ASSERT(dnsb->rsrc_id);
	}

	if (bufferevent_socket_connect_hostname(bev->bevent,
				(zdns_base ? dnsb->dns_base : NULL),
				family, hostname, port)) {
# ifdef PHP_EVENT_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s",
				evutil_gai_strerror(bufferevent_socket_get_dns_error(bev->bevent)));
# endif
		RETURN_FALSE;
	}
	/*zend_list_addref(dnsb->rsrc_id);*/
#else /* don't HAVE_EVENT_EXTRA_LIB */
	if (bufferevent_socket_connect_hostname(bev->bevent,
				NULL,
				family, hostname, port)) {
# ifdef PHP_EVENT_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s",
				evutil_gai_strerror(bufferevent_socket_get_dns_error(bev->bevent)));
# endif
		RETURN_FALSE;
	}
#endif

	bev->stream_id = bufferevent_getfd(bev->bevent);

	/*
	It may not work with evdns
	if (bev->stream_id < 0) {
		RETURN_FALSE;
	}
	 PHP_EVENT_ASSERT(bev->stream_id >= 0);
	*/

	RETVAL_TRUE;
#endif
}
/* }}} */

/* {{{ proto resource bufferevent_socket_get_dns_error_string(resource bevent);
 * Returns string describing the last failed DNS lookup attempt made by
 * bufferevent_socket_connect_hostname(), or an empty string, if no DNS error
 * detected. */
PHP_FUNCTION(bufferevent_socket_get_dns_error)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	int                 err;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	err = bufferevent_socket_get_dns_error(bev->bevent);

	if (err == 0) {
		RETURN_EMPTY_STRING();
	}
	RETVAL_STRING(evutil_gai_strerror(err), 1);
}
/* }}} */

/* {{{ proto void bufferevent_setcb(resource bevent, callable readcb, callable writecb, callable eventcb[, mixed arg = NULL]);
 * Changes one or more of the callbacks of a bufferevent.
 * A callback may be disabled by passing NULL instead of the callable.
 * arg is an argument passed to the callbacks.
 */
PHP_FUNCTION(bufferevent_setcb)
{
	php_event_bevent_t    *bev;
	zval                  *zbevent;
	zend_fcall_info        fci_read;
	zend_fcall_info_cache  fcc_read;
	zend_fcall_info        fci_write;
	zend_fcall_info_cache  fcc_write;
	zend_fcall_info        fci_event;
	zend_fcall_info_cache  fcc_event;
	zval                  *zarg      = NULL;
	bufferevent_data_cb    read_cb;
	bufferevent_data_cb    write_cb;
	bufferevent_event_cb   event_cb;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rf!f!f!|z!",
				&zbevent,
				&fci_read, &fcc_read,
				&fci_write, &fcc_write,
				&fci_event, &fcc_event,
				&zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (ZEND_FCI_INITIALIZED(fci_read)) {
		read_cb = bevent_read_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_read, bev->fcc_read);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_read, bev->fcc_read, &fci_read, &fcc_read);
	} else {
		read_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_write)) {
		write_cb = bevent_write_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_write, bev->fcc_write);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_write, bev->fcc_write, &fci_write, &fcc_write);
	} else {
		write_cb = NULL;
	}

	if (ZEND_FCI_INITIALIZED(fci_event)) {
		event_cb = bevent_event_cb;
		PHP_EVENT_FREE_FCALL_INFO(bev->fci_event, bev->fcc_event);
		PHP_EVENT_COPY_FCALL_INFO(bev->fci_event, bev->fcc_event, &fci_event, &fcc_event);
	} else {
		event_cb = NULL;
	}

	if (zarg) {
		Z_ADDREF_P(zarg);
		bev->data = zarg;
	}

	TSRMLS_SET_CTX(bev->thread_ctx);

	bufferevent_setcb(bev->bevent, read_cb, write_cb, event_cb, (void *) bev);
}
/* }}} */

/* {{{ proto void bufferevent_enable(resource bevent, int events);
 * Enable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_FUNCTION(bufferevent_enable)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	long                events;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zbevent, &events) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	bufferevent_enable(bev->bevent, events);
}
/* }}} */

/* {{{ proto void bufferevent_disable(resource bevent, int events);
 * Disable events EVENT_READ, EVENT_WRITE, or EVENT_READ | EVENT_WRITE on a buffer event. */
PHP_FUNCTION(bufferevent_disable)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	long                events;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zbevent, &events) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	bufferevent_disable(bev->bevent, events);
}
/* }}} */

/* {{{ proto int bufferevent_get_enabled(resource bevent);
 * Returns bitmask of events currently enabled on the buffer event. */
PHP_FUNCTION(bufferevent_get_enabled)
{
	php_event_bevent_t *bev;
	zval               *zbevent;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	RETVAL_LONG(bufferevent_get_enabled(bev->bevent));
}
/* }}} */

/* {{{ proto resource bufferevent_get_input(resource bevent);
 * Returns the input event buffer resource */
PHP_FUNCTION(bufferevent_get_input)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	php_event_buffer_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	b = emalloc(sizeof(php_event_buffer_t));
	memset(b, 0, sizeof(php_event_buffer_t));

	b->buf       = bufferevent_get_input(bev->bevent);
	b->rsrc_id   = ZEND_REGISTER_RESOURCE(return_value, b, le_event_buffer);

	zend_list_addref(b->rsrc_id);
}
/* }}} */

/* {{{ proto resource bufferevent_get_output(resource bevent);
 * Returns the output event buffer resource */
PHP_FUNCTION(bufferevent_get_output)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	php_event_buffer_t *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbevent) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	b = emalloc(sizeof(php_event_buffer_t));
	memset(b, 0, sizeof(php_event_buffer_t));

	b->buf       = bufferevent_get_output(bev->bevent);
	b->rsrc_id   = ZEND_REGISTER_RESOURCE(return_value, b, le_event_buffer);

	zend_list_addref(b->rsrc_id);
}
/* }}} */

/* {{{ proto void bufferevent_setwatermark(resource bevent, int events, int lowmark, int highmark);
 * Adjusts the read watermarks, the write watermarks, or both, of a single bufferevent. */
PHP_FUNCTION(bufferevent_setwatermark)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	long                events;
	long                lowmark;
	long                highmark;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlll",
				&zbevent, &events, &lowmark, &highmark) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	bufferevent_setwatermark(bev->bevent, events, (size_t) lowmark, (size_t) highmark);
}
/* }}} */

/* {{{ proto bool bufferevent_write(resource bevent, string data);
 * Adds `data' to a bufferevent's output buffer. */
PHP_FUNCTION(bufferevent_write)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	zval               *zdata;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz",
				&zbevent, &zdata) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	convert_to_string(zdata);

	if (bufferevent_write(bev->bevent, Z_STRVAL_P(zdata), Z_STRLEN_P(zdata))) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool bufferevent_write_buffer(resource bevent, resource buf);
 * Adds contents of the entire buffer to a bufferevent's output buffer. */
PHP_FUNCTION(bufferevent_write_buffer)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr",
				&zbevent, &zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (bufferevent_write_buffer(bev->bevent, b->buf)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int bufferevent_read(resource bevent, string &data, int size);
 * Removes up to size bytes from the input buffer, storing them into the memory at data.
 *
 * Returns the number of bytes actually removed.  */
PHP_FUNCTION(bufferevent_read)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	zval               *zdata;
	long                size;
	char               *data;
	long                ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rzl",
				&zbevent, &zdata, &size) == FAILURE) {
		return;
	}

	if (!Z_ISREF_P(zdata)) {
		/* Was not passed by reference */
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	data = safe_emalloc(size, sizeof(char), 1);

	ret = bufferevent_read(bev->bevent, data, size);

	if (ret > 0) {
		convert_to_string(zdata);
		zval_dtor(zdata);
		Z_STRVAL_P(zdata) = estrndup(data, ret);
		Z_STRLEN_P(zdata) = ret;
	}

	efree(data);

	RETVAL_LONG(ret);
}
/* }}} */

/* {{{ proto bool bufferevent_read_buffer(resource bevent, resource buf);
 * Drains the entire contents of the input buffer and places them into buf; it returns 0 on success and -1 on failure. */
PHP_FUNCTION(bufferevent_read_buffer)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr",
				&zbevent, &zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);
	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (bufferevent_read_buffer(bev->bevent, b->buf)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool bufferevent_priority_set(resource bevent, int priority);
 * Assign a priority to a bufferevent.
 * Only supported for socket bufferevents. */
PHP_FUNCTION(bufferevent_priority_set)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	long                priority;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zbevent, &priority) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	if (bufferevent_priority_set(bev->bevent, priority)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool bufferevent_set_timeouts(resource bevent, double timeout_read, double timeout_write);
 * Set the read and write timeout for a bufferevent. */
PHP_FUNCTION(bufferevent_set_timeouts)
{
	php_event_bevent_t *bev;
	zval               *zbevent;
	double              timeout_read;
	double              timeout_write;
	struct timeval      tv_read;
	struct timeval      tv_write;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll",
				&zbevent, &timeout_read, &timeout_write) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BEVENT(bev, zbevent);

	PHP_EVENT_TIMEVAL_SET(tv_read, timeout_read);
	PHP_EVENT_TIMEVAL_SET(tv_write, timeout_write);

	if (bufferevent_set_timeouts(bev->bevent, &tv_read, &tv_write)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */



/* {{{ proto resource evbuffer_new(void);
 * Allocates storage for new event buffer and returns it's resource. */
PHP_FUNCTION(evbuffer_new)
{
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	b = emalloc(sizeof(php_event_buffer_t));
	memset(b, 0, sizeof(php_event_buffer_t));

	b->buf       = evbuffer_new();
	b->rsrc_id   = ZEND_REGISTER_RESOURCE(return_value, b, le_event_buffer);
}
/* }}} */

/* {{{ proto void evbuffer_free(resource buf);
 * Free storage allocated for the event buffer */
PHP_FUNCTION(evbuffer_free)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	zend_list_delete(b->rsrc_id);
}
/* }}} */

/* {{{ proto bool evbuffer_freeze(resource buf, bool at_front);
 * Prevent calls that modify an event buffer from succeeding. */
PHP_FUNCTION(evbuffer_freeze)
{
	php_event_buffer_t *b;
	zval               *zbuf;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rb",
				&zbuf, &at_front) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_freeze(b->buf, at_front)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evbuffer_unfreeze(resource buf, bool at_front);
 * Re-enable calls that modify an evbuffer. */
PHP_FUNCTION(evbuffer_unfreeze)
{
	php_event_buffer_t *b;
	zval               *zbuf;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rb",
				&zbuf, &at_front) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_unfreeze(b->buf, at_front)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto long evbuffer_get_length(resource buf);
 * Returns the total number of bytes stored in the event buffer. */
PHP_FUNCTION(evbuffer_get_length)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	RETVAL_LONG(evbuffer_get_length(b->buf));
}
/* }}} */

/* {{{ proto void evbuffer_lock(resource buf);
 * Acquire the lock on an evbuffer. 
 * Has no effect if locking was not enabled with evbuffer_enable_locking.
 */
PHP_FUNCTION(evbuffer_lock)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_lock(b->buf);
}
/* }}} */

/* {{{ proto void evbuffer_unlock(resource buf);
 * Release the lock on an evbuffer.
 * Has no effect if locking was not enabled with evbuffer_enable_locking.
 */
PHP_FUNCTION(evbuffer_unlock)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_unlock(b->buf);
}
/* }}} */

/* {{{ proto void evbuffer_enable_locking(resource buf);
 *
 * Enable locking on an evbuffer so that it can safely be used by multiple threads at the same time.
 * When locking is enabled, the lock will be held when callbacks are invoked.
 * This could result in deadlock if you aren't careful. Plan accordingly!
 */
PHP_FUNCTION(evbuffer_enable_locking)
{
	php_event_buffer_t *b;
	zval               *zbuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbuf) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_enable_locking(b->buf, NULL);
}
/* }}} */

/* {{{ proto bool evbuffer_add(resource buf, string data); 
 *
 * Append data to the end of an event buffer.
 */
PHP_FUNCTION(evbuffer_add)
{
	php_event_buffer_t   *b;
	zval                 *zbuf;
	zval                **ppzdata;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ",
				&zbuf, &ppzdata) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	convert_to_string_ex(ppzdata);

	if (evbuffer_add(b->buf, (void *) Z_STRVAL_PP(ppzdata), Z_STRLEN_PP(ppzdata))) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evbuffer_add_buffer(resource outbuf, inbuf); 
 * Move all data from one evbuffer into another evbuffer.
 * This is a destructive add. The data from one buffer moves into the other buffer. However, no unnecessary memory copies occur.
 */
PHP_FUNCTION(evbuffer_add_buffer)
{
	php_event_buffer_t *b_out     , *b_in;
	zval               *zbuf_out  , *zbuf_in;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr",
				&zbuf_out, &zbuf_in) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b_out, zbuf_out);
	PHP_EVENT_FETCH_BUFFER(b_in, zbuf_in);

	if (evbuffer_add_buffer(b_out->buf, b_in->buf)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int evbuffer_remove(resource buf, string &data, long max_bytes);
 * Read data from an evbuffer and drain the bytes read.  If more bytes are
 * requested than are available in the evbuffer, we only extract as many bytes
 * as were available.
 *
 * Returns the number of bytes read, or -1 if we can't drain the buffer.
 */
PHP_FUNCTION(evbuffer_remove)
{
	php_event_buffer_t *b;
	zval               *zbuf;
	zval               *zdata;
	long                max_bytes;
	long                ret;
	char               *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rzl",
				&zbuf, &zdata, &max_bytes) == FAILURE) {
		return;
	}

	if (!Z_ISREF_P(zdata)) {
		/* Was not passed by reference */
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	data = emalloc(sizeof(char) * max_bytes + 1);

	ret = evbuffer_remove(b->buf, data, max_bytes);

	if (ret > 0) {
		convert_to_string(zdata);
		zval_dtor(zdata);
		Z_STRVAL_P(zdata) = estrndup(data, ret);
		Z_STRLEN_P(zdata) = ret;
	}

	efree(data);

	RETVAL_LONG(ret);
}
/* }}} */


/* {{{ proto int event_socket_get_last_errno([resource socket = null]);
 * Returns the most recent socket error number(errno). */
PHP_FUNCTION(event_socket_get_last_errno)
{

	zval **ppzfd = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|Z!",
				&ppzfd) == FAILURE) {
		return;
	}

	if (ppzfd) {
		evutil_socket_t fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);

		if (fd < 0) {
			RETURN_FALSE;
		}

		RETVAL_LONG(evutil_socket_geterror(fd));
	} else {
		RETVAL_LONG(EVUTIL_SOCKET_ERROR());
	}
}
/* }}} */

/* {{{ proto string event_socket_get_last_error([resource socket = null]);
 * Returns the most recent socket error */
PHP_FUNCTION(event_socket_get_last_error)
{
	zval **ppzfd = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|Z!",
				&ppzfd) == FAILURE) {
		return;
	}

	if (ppzfd) {
		evutil_socket_t fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);

		if (fd < 0) {
			RETURN_FALSE;
		}

		RETVAL_STRING(evutil_socket_error_to_string(evutil_socket_geterror(fd)), 1);
	} else {
		RETVAL_STRING(evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), 1);
	}
}
/* }}} */


/* API functions END }}} */



#if HAVE_EVENT_EXTRA_LIB
/* {{{ Extra API functions */

/* {{{ proto resource evdns_base_new(resource base, bool initialize);
 *
 * Returns resource representing event dns base.
 *
 * If the initialize argument is true, it tries to configure the DNS base
 * sensibly given your operating system’s default. Otherwise, it leaves the
 * evdns_base empty, with no nameservers or options configured. In the latter
 * case you should configure dns base yourself, e.g. with
 * event_dns_base_resolv_conf_parse() */
PHP_FUNCTION(evdns_base_new)
{
	php_event_base_t     *base;
	zval                 *zbase;
	php_event_dns_base_t *dnsb;
	zend_bool             initialize;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rb",
				&zbase, &initialize) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	dnsb = emalloc(sizeof(php_event_dns_base_t));
	memset(dnsb, 0, sizeof(php_event_dns_base_t));

	dnsb->dns_base = evdns_base_new(base->base, initialize);

	if (dnsb->dns_base) {
		dnsb->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, dnsb, le_event_dns_base);
		PHP_EVENT_ASSERT(dnsb->rsrc_id);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto void evdns_base_free(resource base);
 * Free an evdns base */
PHP_FUNCTION(evdns_base_free)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zdns_base) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	zend_list_delete(dnsb->rsrc_id);
}
/* }}} */

/* {{{ proto bool evdns_base_resolv_conf_parse(resource dns_base, int flags, string filename);
 * Scans the resolv.conf formatted file stored in filename, and read in all the
 * options from it that are listed in flags */
PHP_FUNCTION(evdns_base_resolv_conf_parse)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;
	long                  flags;
	char                 *filename;
	int                   filename_len;
	int                   ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls",
				&zdns_base, &flags, &filename, &filename_len) == FAILURE) {
		return;
	}


	if (flags & ~(DNS_OPTION_NAMESERVERS | DNS_OPTION_SEARCH | DNS_OPTION_MISC
				| DNS_OPTIONS_ALL)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid flags");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	ret = evdns_base_resolv_conf_parse(dnsb->dns_base, flags, filename);

	if (ret) {
		char err[40];

		switch (ret) {
			case 1:
				strcpy(err, "Failed to open file");
				break;
			case 2:
				strcpy(err, "Failed to stat file");
				break;
			case 3:
				strcpy(err, "File too large");
				break;
			case 4:
				strcpy(err, "Out of memory");
				break;
			case 5:
				strcpy(err, "Short read from file");
				break;
			case 6:
				strcpy(err, "No nameservers listed in the file");
				break;
		}

		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", err);
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evdns_base_nameserver_ip_add(resource dns_base, string ip);
 * Adds a nameserver to an existing evdns_base. It takes the nameserver in a
 * text string, either as an IPv4 address, an IPv6 address, an IPv4 address
 * with a port (IPv4:Port), or an IPv6 address with a port ([IPv6]:Port).
 */
PHP_FUNCTION(evdns_base_nameserver_ip_add)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;
	char                 *ip;
	int                   ip_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
				&zdns_base, &ip, &ip_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	if (evdns_base_nameserver_ip_add(dnsb->dns_base, ip)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evdns_base_load_hosts(resource dns_base, string hosts);
 *  Loads a hosts file (in the same format as /etc/hosts) from hosts file
 */
PHP_FUNCTION(evdns_base_load_hosts)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;
	char                 *hosts;
	int                   hosts_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
				&zdns_base, &hosts, &hosts_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	if (evdns_base_load_hosts(dnsb->dns_base, hosts)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void evdns_base_search_clear(resource dns_base);
 * Removes all current search suffixes (as configured by the search option)
 * from the evdns_base; the evdns_base_search_add() function adds a suffix
 */
PHP_FUNCTION(evdns_base_search_clear)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zdns_base) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	evdns_base_search_clear(dnsb->dns_base);
}
/* }}} */

/* {{{ proto void evdns_base_search_add(resource dns_base, string domain);
 */
PHP_FUNCTION(evdns_base_search_add)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;
	char                 *domain;
	int                   domain_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
				&zdns_base, &domain, &domain_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	evdns_base_search_add(dnsb->dns_base, domain);
}
/* }}} */

/* {{{ proto void evdns_base_search_ndots_set(resource dns_base, int ndots);
 */
PHP_FUNCTION(evdns_base_search_ndots_set)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;
	long                  ndots;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
				&zdns_base, &ndots) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	evdns_base_search_ndots_set(dnsb->dns_base, ndots);
}
/* }}} */

/* {{{ proto bool evdns_base_set_option(resource dns_base, string option, string value);
 */
PHP_FUNCTION(evdns_base_set_option)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;
	char                 *option;
	int                   option_len;
	char                 *value;
	int                   value_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss",
				&zdns_base, &option, &option_len, &value, &value_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	if (evdns_base_set_option(dnsb->dns_base, option, value)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int evdns_base_count_nameservers(resource dns_base);
 */
PHP_FUNCTION(evdns_base_count_nameservers)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zdns_base) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	RETURN_LONG(evdns_base_count_nameservers(dnsb->dns_base));
}
/* }}} */


/* {{{ proto resource evconnlistener_new_bind(resource base, callable cb, mixed data, int flags, int backlog, string addr);
 * Creates new connection listener associated with an event base.
 *
 * Returns resource representing the event connection listener.
 */
PHP_FUNCTION(evconnlistener_new_bind)
{
	zval                  *zbase;
	php_event_base_t      *base;
	zend_fcall_info        fci      = empty_fcall_info;
	zend_fcall_info_cache  fcc      = empty_fcall_info_cache;
	php_event_listener_t  *l;
	zval                  *zdata    = NULL;
	zval                  *zaddr;
	long                   flags;
	long                   backlog;
	struct evconnlistener *listener;
	struct sockaddr        sa;
	socklen_t              sa_len   = sizeof(struct sockaddr);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rfz!llz",
				&zbase, &fci, &fcc, &zdata, &flags, &backlog, &zaddr) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	convert_to_string(zaddr);
	if (php_network_parse_network_address_with_port(Z_STRVAL_P(zaddr), Z_STRLEN_P(zaddr),
				&sa, &sa_len TSRMLS_CC) != SUCCESS) {
		RETURN_FALSE;
	}

	l = emalloc(sizeof(php_event_listener_t));
	memset(l, 0, sizeof(php_event_listener_t));

	listener = evconnlistener_new_bind(base->base, _php_event_listener_cb,
			(void *) l, flags, backlog, &sa, sa_len);
	if (!listener) {
		efree(l);
		RETURN_FALSE;
	}
	l->listener = listener;

	if (zdata) {
		l->data = zdata;
		Z_ADDREF_P(zdata);
	}

	PHP_EVENT_COPY_FCALL_INFO(l->fci, l->fcc, &fci, &fcc);

	l->base_id = Z_LVAL_P(zbase);
	zend_list_addref(l->base_id);

	TSRMLS_SET_CTX(l->thread_ctx);

	l->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, l, le_event_listener);

	PHP_EVENT_ASSERT(l->rsrc_id);
}
/* }}} */

/* {{{ proto resource evconnlistener_new(resource base, callable cb, mixed data, int flags, int backlog, resource stream);
 * Creates new connection listener associated with an event base.
 *
 * stream parameter may be a socket resource, or a stream associated with a socket.
 *
 * Returns resource representing the event connection listener.
 */
PHP_FUNCTION(evconnlistener_new)
{
	zval                   *zbase;
	php_event_base_t       *base;
	zend_fcall_info         fci       = empty_fcall_info;
	zend_fcall_info_cache   fcc       = empty_fcall_info_cache;
	php_event_listener_t   *l;
	zval                   *zdata     = NULL;
	long                    flags;
	long                    backlog;
	zval                  **ppzfd;
	evutil_socket_t         fd        = -1;
	struct evconnlistener  *listener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rfz!llZ",
				&zbase, &fci, &fcc, &zdata, &flags, &backlog,
				&ppzfd) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(base, zbase);

	/* php_event_zval_to_fd reports error
	 * in case if it is not a valid socket resource */
	fd = php_event_zval_to_fd(ppzfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	/* Make sure that the socket is in non-blocking mode(libevent's tip) */
	evutil_make_socket_nonblocking(fd);

	l = emalloc(sizeof(php_event_listener_t));
	memset(l, 0, sizeof(php_event_listener_t));

	listener = evconnlistener_new(base->base, _php_event_listener_cb,
			(void *) l, flags, backlog, fd);
	if (!listener) {
		efree(l);
		RETURN_FALSE;
	}
	l->listener = listener;

	if (zdata) {
		l->data = zdata;
		Z_ADDREF_P(zdata);
	}

	PHP_EVENT_COPY_FCALL_INFO(l->fci, l->fcc, &fci, &fcc);

#if 0
	/* Don't do this, since libevent calls accept() afterwards, thus producing
	 * new file descriptor. The new descriptor is available in
	 * _php_event_listener_cb() callback. */

	if (Z_TYPE_PP(ppzfd) == IS_RESOURCE) {
		/* lval of ppzfd is the resource ID */
		l->stream_id = Z_LVAL_PP(ppzfd);
		zend_list_addref(Z_LVAL_PP(ppzfd));
	} else {
		l->stream_id = -1;
	}
#endif
	l->stream_id = -1;

	l->base_id = Z_LVAL_P(zbase);
	zend_list_addref(l->base_id);

	TSRMLS_SET_CTX(l->thread_ctx);

	l->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, l, le_event_listener);

	PHP_EVENT_ASSERT(l->rsrc_id);
}
/* }}} */


/* {{{ proto void evconnlistener_free(resource listener);
 * Free an event connect listener resource */
PHP_FUNCTION(evconnlistener_free)
{
	php_event_listener_t *l;
	zval                 *zlistener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zlistener) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	zend_list_delete(l->rsrc_id);
}
/* }}} */

/* {{{ proto bool evconnlistener_enable(resource listener);
 * Enable an event connect listener resource */
PHP_FUNCTION(evconnlistener_enable)
{
	php_event_listener_t *l;
	zval                 *zlistener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zlistener) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (evconnlistener_enable(l->listener)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evconnlistener_disable(resource listener);
 * Disable an event connect listener resource */
PHP_FUNCTION(evconnlistener_disable)
{
	php_event_listener_t *l;
	zval                 *zlistener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zlistener) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (evconnlistener_disable(l->listener)) {
		RETURN_FALSE;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void evconnlistener_set_cb(resource listener, callable cb[, mixed arg = NULL]);
 * Adjust event connect listener's callback and optionally the callback argument.
 * Both cb and arg may be NULL.
 */
PHP_FUNCTION(evconnlistener_set_cb)
{
	php_event_listener_t  *l;
	zval                  *zlistener;
	zend_fcall_info        fci       = empty_fcall_info;
	zend_fcall_info_cache  fcc       = empty_fcall_info_cache;
	zval                  *zarg      = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rf!|z!",
				&zlistener, &fci, &fcc, &zarg) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (ZEND_FCI_INITIALIZED(fci)) {
		PHP_EVENT_FREE_FCALL_INFO(l->fci, l->fcc);
		PHP_EVENT_COPY_FCALL_INFO(l->fci, l->fcc, &fci, &fcc);
	}

	if (zarg) {
		if (l->data) {
			zval_ptr_dtor(&l->data);
		}

		Z_ADDREF_P(zarg);
		l->data = zarg;
	}

	/*
	 * No sense in the following call, since the callback and the pointer
	 * remain the same
	 * evconnlistener_set_cb(l->listener, _php_event_listener_cb, (void *) l);
	 */
}
/* }}} */

/* {{{ proto void evconnlistener_set_error_cb(resource listener, callable cb);
 * Set event listener's error callback
 */
PHP_FUNCTION(evconnlistener_set_error_cb)
{
	php_event_listener_t  *l;
	zval                  *zlistener;
	zend_fcall_info        fci;
	zend_fcall_info_cache  fcc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rf",
				&zlistener, &fci, &fcc) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	if (ZEND_FCI_INITIALIZED(fci)) {
		PHP_EVENT_FREE_FCALL_INFO(l->fci_err, l->fcc_err);
		PHP_EVENT_COPY_FCALL_INFO(l->fci_err, l->fcc_err, &fci, &fcc);
	}

	/*
	 * No much sense in the following call, since the callback and the pointer
	 * remain the same. However, we have to set it once at least
	 */
	 evconnlistener_set_error_cb(l->listener, listener_error_cb);
}
/* }}} */

#if LIBEVENT_VERSION_NUMBER >= 0x02000300
/* {{{ proto resource evconnlistener_get_base(resource listener);
 * Get event base associated with the connection listener
 */
PHP_FUNCTION(evconnlistener_get_base)
{
	php_event_listener_t *l;
	zval                 *zlistener;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zlistener) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_LISTENER(l, zlistener);

	/*
	   base = evconnlistener_get_base(l->listener);
	*/

	if (l->base_id) {
		/* Make sure base wouldn't be inexpectedly destroyed */
		zend_list_addref(l->base_id);

		RETURN_RESOURCE(l->base_id);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */
#endif



/* {{{ proto resource evhttp_connection_base_new(resource base, resource dns_base, string address, int port);
 * Creates new http connection resource.
 */
PHP_FUNCTION(evhttp_connection_base_new)
{
	zval                     *zbase;
	php_event_base_t         *b;
	zval                     *zdns_base;
	php_event_dns_base_t     *dnsb;
	char                     *address;
	int                       address_len;
	long                      port;
	php_event_http_conn_t    *evcon;
	struct evhttp_connection *conn;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrsl",
				&zbase, &zdns_base, &address, &address_len, &port) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);
	PHP_EVENT_FETCH_DNS_BASE(dnsb, zdns_base);

	evcon = emalloc(sizeof(php_event_http_conn_t));
	memset(evcon, 0, sizeof(php_event_http_conn_t));

	conn = evhttp_connection_base_new(b->base, dnsb->dns_base, address, (unsigned short) port);
	if (!conn) {
		efree(evcon);
		RETURN_FALSE;
	}
	evcon->conn = conn;

	evcon->base_id = Z_LVAL_P(zbase);
	zend_list_addref(evcon->base_id);

	evcon->dns_base_id = Z_LVAL_P(zdns_base);
	zend_list_addref(evcon->dns_base_id);

	evcon->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, evcon, le_event_http_conn);

	PHP_EVENT_ASSERT(evcon->rsrc_id);
}
/* }}} */

/* {{{ proto void evhttp_connection_free(resource evcon);
 * Free an event http connection resource */
PHP_FUNCTION(evhttp_connection_free)
{
	php_event_http_conn_t *evcon;
	zval                  *zevcon;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zevcon) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	zend_list_delete(evcon->rsrc_id);
}
/* }}} */

/* {{{ proto resource evhttp_connection_get_base(resource evcon);
 * Get event base associated with the http connection
 */
PHP_FUNCTION(evhttp_connection_get_base)
{
	php_event_http_conn_t *evcon;
	zval                 *zevcon;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zevcon) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	/*
	 * No sense in this call
	 * base = evhttp_connection_get_base(evcon->con);
	 */

	if (evcon->base_id) {
		/* Make sure base wouldn't be inexpectedly destroyed */
		zend_list_addref(evcon->base_id);

		RETURN_RESOURCE(evcon->base_id);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto void evhttp_connection_get_peer(resource evcon, string &address, int &port);
 * Get the remote address and port associated with this connection. */
PHP_FUNCTION(evhttp_connection_get_peer)
{
	php_event_http_conn_t *evcon;
	zval                  *zevcon;
	zval                  *zaddress;
	zval                  *zport;

	char *address;
	unsigned short port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rzz",
				&zevcon, &zaddress, &zport) == FAILURE) {
		return;
	}

	if (! (Z_ISREF_P(zaddress) && Z_ISREF_P(zport))) {
		/* Was not passed by reference */
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_get_peer(evcon->conn, &address, &port);

	ZVAL_STRING(zaddress, address, 1);
	ZVAL_LONG(zport, port);
}
/* }}} */

/* {{{ proto void evhttp_connection_set_local_address(resource evcon, string address);
 * Sets the ip address from which http connections are made */
PHP_FUNCTION(evhttp_connection_set_local_address)
{
	php_event_http_conn_t *evcon;
	zval                  *zevcon;
	char                  *address;
	int                    address_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
				&zevcon, &address, &address_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_local_address(evcon->conn, address);
}
/* }}} */

/* {{{ proto void evhttp_connection_set_local_port(resource evcon, int port);
 * Sets the port from which http connections are made */
PHP_FUNCTION(evhttp_connection_set_local_port)
{
	php_event_http_conn_t *evcon;
	zval                  *zevcon;
	long                   port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zevcon, &port) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_local_port(evcon->conn, port);
}
/* }}} */

/* {{{ proto void evhttp_connection_set_timeout(resource evcon, int timeout);
 */
PHP_FUNCTION(evhttp_connection_set_timeout)
{
	php_event_http_conn_t *evcon;
	zval                  *zevcon;
	long                   timeout;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zevcon, &timeout) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_timeout(evcon->conn, timeout);
}
/* }}} */

/* {{{ proto void evhttp_connection_set_max_headers_size(resource evcon, int max_size);
 */
PHP_FUNCTION(evhttp_connection_set_max_headers_size)
{
	php_event_http_conn_t *evcon;
	zval                  *zevcon;
	long                   max_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zevcon, &max_size) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_max_headers_size(evcon->conn, (ev_ssize_t) max_size);
}
/* }}} */

/* {{{ proto void evhttp_connection_set_max_body_size(resource evcon, int max_size);
 */
PHP_FUNCTION(evhttp_connection_set_max_body_size)
{
	php_event_http_conn_t *evcon;
	zval                  *zevcon;
	long                   max_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zevcon, &max_size) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_max_body_size(evcon->conn, (ev_ssize_t) max_size);
}
/* }}} */

/* {{{ proto void evhttp_connection_set_retries(resource evcon, int retries);
 */
PHP_FUNCTION(evhttp_connection_set_retries)
{
	php_event_http_conn_t *evcon;
	zval                  *zevcon;
	long                   retries;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
				&zevcon, &retries) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_CONN(evcon, zevcon);

	evhttp_connection_set_retries(evcon->conn, retries);
}
/* }}} */


/* {{{ proto resource evhttp_new(resource base);
 * Creates new http server resource.
 */
PHP_FUNCTION(evhttp_new)
{
	zval             *zbase;
	php_event_base_t *b;
	php_event_http_t *http;
	struct evhttp    *http_ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zbase) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BASE(b, zbase);

	http = emalloc(sizeof(php_event_http_t));
	memset(http, 0, sizeof(php_event_http_t));

	http_ptr = evhttp_new(b->base);
	if (!http_ptr) {
		efree(http);
		RETURN_FALSE;
	}
	http->ptr = http_ptr;

	http->base_id = Z_LVAL_P(zbase);
	zend_list_addref(http->base_id);

	http->stream_id = -1;

	http->rsrc_id = ZEND_REGISTER_RESOURCE(return_value, http, le_event_http);

	PHP_EVENT_ASSERT(http->rsrc_id);
}
/* }}} */

/* {{{ proto void evhttp_free(resource evhttp);
 * Free an event http server resource */
PHP_FUNCTION(evhttp_free)
{
	php_event_http_t *http;
	zval             *zhttp;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zhttp) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	zend_list_delete(http->rsrc_id);
}
/* }}} */

/* {{{ proto bool evhttp_accept_socket(resource evhttp, resource socket);
 * Makes an HTTP server accept connections on the specified socket stream or resource.
 * The socket should be ready to accept connections.
 * Can be called multiple times to accept connections on different sockets. */
PHP_FUNCTION(evhttp_accept_socket)
{
	php_event_http_t  *http;
	zval              *zhttp;
	zval             **ppzfd;
	evutil_socket_t    fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ",
				&zhttp, &ppzfd) == FAILURE) {
		return;
	}

	fd = (evutil_socket_t) php_event_zval_to_fd(ppzfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	if (evhttp_accept_socket(http->ptr, fd)) {
		RETURN_FALSE;
	}

#if 0
	http->stream_id = Z_LVAL_P(ppzfd);
	zend_list_addref(Z_LVAL_P(ppzfd));
#endif

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool evhttp_bind_socket(resource evhttp, string address, int port);
 * Binds an HTTP server on the specified address and port.
 * Can be called multiple times to bind the same http server to multiple different ports. */
PHP_FUNCTION(evhttp_bind_socket)
{
	php_event_http_t  *http;
	zval              *zhttp;
	char *address;
	int address_len;
	long port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl",
				&zhttp, &address, &address_len, &port) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP(http, zhttp);

	if (evhttp_bind_socket(http->ptr, address, port)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */


/* Extra API functions END }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */

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

#ifndef PHP_EVENT_STRUCTS_H
#define PHP_EVENT_STRUCTS_H

/* Thread context. With it we are getting rid of need 
 * to call the heavy TSRMLS_FETCH() */
#ifdef ZTS
# define PHP_EVENT_COMMON_THREAD_CTX void ***thread_ctx
#else
# define PHP_EVENT_COMMON_THREAD_CTX
#endif

#define PHP_EVENT_OBJECT_HEAD \
    zend_object  zo;          /* Extending zend_object */ \
    HashTable   *prop_handler /* no ';' */

/* php_event_abstract_object_t is for type casting only. However, all the
 * class objects must have the same fields at the head of their structs */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;
} php_event_abstract_object_t; 

/* Represents Event object */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;

	struct event          *event;       /* Pointer returned by event_new                        */
	int                    stream_id;   /* Resource ID of the file descriptor. -1 if none */
	zval                  *data;        /* User custom data                                     */
	/* fci and fcc represent userspace callback */
	zend_fcall_info       *fci;
	zend_fcall_info_cache *fcc;

	PHP_EVENT_COMMON_THREAD_CTX;
} php_event_t;

/* Represents EventBase object */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;

	struct event_base *base;
} php_event_base_t;

/* Represents EventConfig object */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;

	struct event_config *ptr;
} php_event_config_t;

/* Represents EventBufferEvent object */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;

	struct bufferevent    *bevent;
	int                    stream_id;   /* Resource ID of the file descriptor. -1 if none */
	zval                  *self;        /* Object itself. For callbacks       */
	zval                  *data;        /* User custom data                   */

    /* fci and fcc members represent userspace callbacks */
	zend_fcall_info       *fci_read;
	zend_fcall_info_cache *fcc_read;
	zend_fcall_info       *fci_write;
	zend_fcall_info_cache *fcc_write;
	zend_fcall_info       *fci_event;
	zend_fcall_info_cache *fcc_event;

	PHP_EVENT_COMMON_THREAD_CTX;
} php_event_bevent_t;

/* Represents EventBuffer object */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;

	struct evbuffer *buf;
} php_event_buffer_t;

#ifdef HAVE_EVENT_EXTRA_LIB/* {{{ */

/* Represents EventDnsBase object */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;

	struct evdns_base *dns_base;
} php_event_dns_base_t;

/* Represents EventListener object */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;

	struct evconnlistener *listener;
	int                    stream_id;   /* Resource ID of the file descriptor. -1 if none */
	zval                  *self;        /* Object itself. For callbacks              */
	zval                  *base;        /* Event base associated with the listener   */
	zval                  *data;        /* User custom data passed to callback       */

	/* Accept callback */
	zend_fcall_info       *fci;
	zend_fcall_info_cache *fcc;

	/* Error callback */
	zend_fcall_info       *fci_err;
	zend_fcall_info_cache *fcc_err;

	PHP_EVENT_COMMON_THREAD_CTX;
} php_event_listener_t;

/* Represents EventHttpConnection object */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;

	struct evhttp_connection *conn;
	zval                     *base;       /* Event base associated with the listener */
	zval                     *dns_base;   /* Associated EventDnsBase                 */
} php_event_http_conn_t;

/* Represents EventHttp object */
typedef struct {
	PHP_EVENT_OBJECT_HEAD;

	struct evhttp *ptr;
	zval          *base;        /* Event base associated with the listener       */
	int            stream_id;   /* Resource ID of socket probably being listened */
} php_event_http_t;

#endif/* HAVE_EVENT_EXTRA_LIB }}} */

typedef double php_event_timestamp_t;

typedef int (*php_event_prop_read_t)(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC);
typedef int (*php_event_prop_write_t)(php_event_abstract_object_t *obj, zval *newval  TSRMLS_DC);
typedef zval **(*php_event_prop_get_prop_ptr_ptr_t)(php_event_abstract_object_t *obj TSRMLS_DC);

typedef struct {
	const char                        *name;
	size_t                             name_length;
	php_event_prop_read_t              read_func;
	php_event_prop_write_t             write_func;
	php_event_prop_get_prop_ptr_ptr_t  get_ptr_ptr_func;
} php_event_property_entry_t;

typedef struct {
	char                              *name;
	size_t                             name_len;
	php_event_prop_read_t              read_func;
	php_event_prop_write_t             write_func;
	php_event_prop_get_prop_ptr_ptr_t  get_ptr_ptr_func;
} php_event_prop_handler_t;


#ifndef LIBEVENT_VERSION_NUMBER
# error "<event2/*.h> must be included before " ## #__FILE__
#endif

#if LIBEVENT_VERSION_NUMBER < 0x02000200
/* These types introduced in libevent 2.0.2-alpha */
typedef void (*bufferevent_data_cb)(struct bufferevent *bev, void *ctx);
typedef void (*bufferevent_event_cb)(struct bufferevent *bev, short events,
		void *ctx);
#endif

#endif	/* PHP_EVENT_STRUCTS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

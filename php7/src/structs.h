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

#ifndef PHP_EVENT_STRUCTS_H
#define PHP_EVENT_STRUCTS_H

#define PHP_EVENT_OBJECT_TAIL \
	HashTable   *prop_handler; \
	zend_object  zo

/* Represents EventBase object */
typedef struct _php_event_base_t {
	struct event_base *base;
	zend_bool          internal;   /* Whether is an internal pointer, e.g. obtained with evconnlistener_get_base() */

	PHP_EVENT_OBJECT_TAIL;
} php_event_base_t;

/* Represents Event object */
typedef struct _php_event_t {
	struct event  *event;       /* Pointer returned by event_new     */
	zend_resource *steam_res;   /* Resource with the file descriptor */
	zval           data;        /* User custom data                  */

	/* fci and fcc represent userspace callback */
	zend_fcall_info       *fci;
	zend_fcall_info_cache *fcc;

	PHP_EVENT_OBJECT_TAIL;
} php_event_t;
/* typedef php_event_t php_event_event_t; */

/* Represents EventConfig object */
typedef struct _php_event_config_t {
	struct event_config *ptr;

	PHP_EVENT_OBJECT_TAIL;
} php_event_config_t;

/* Represents EventBufferEvent object */
typedef struct _php_event_bevent_t {
	struct bufferevent    *bevent;
	int                   _internal;
	zval                  self;        /* Object itself. For callbacks                   */
	zval                  data;        /* User custom data                               */
	zval                  input;       /* Input buffer */
	zval                  output;      /* Output buffer */
	zval                  base;

    /* fci and fcc members represent userspace callbacks */
	zend_fcall_info       *fci_read;
	zend_fcall_info_cache *fcc_read;
	zend_fcall_info       *fci_write;
	zend_fcall_info_cache *fcc_write;
	zend_fcall_info       *fci_event;
	zend_fcall_info_cache *fcc_event;

	PHP_EVENT_OBJECT_TAIL;
} php_event_bevent_t;

/* Represents EventBuffer object */
typedef struct _php_event_buffer_t {
	zend_bool internal; /* Whether is an internal buffer of a bufferevent */
	struct evbuffer *buf;

	PHP_EVENT_OBJECT_TAIL;
} php_event_buffer_t;

#ifdef HAVE_EVENT_EXTRA_LIB/* {{{ */

enum {
	PHP_EVENT_REQ_HEADER_INPUT  = 1,
	PHP_EVENT_REQ_HEADER_OUTPUT = 2,
};

/* Represents EventDnsBase object */
typedef struct _php_event_dns_base_t {
	struct evdns_base *dns_base;

	PHP_EVENT_OBJECT_TAIL;
} php_event_dns_base_t;

/* Represents EventListener object */
typedef struct _php_event_listener_t {
	struct evconnlistener *listener;
	zval                  self;        /* Object itself. For callbacks              */
	zval                  data;        /* User custom data passed to callback       */

	/* Accept callback */
	zend_fcall_info       *fci;
	zend_fcall_info_cache *fcc;

	/* Error callback */
	zend_fcall_info       *fci_err;
	zend_fcall_info_cache *fcc_err;

	PHP_EVENT_OBJECT_TAIL;
} php_event_listener_t;

/* Type for an HTTP server callback */
typedef struct _php_event_http_cb_t {
	php_event_http_cb_t   *next;   /* Linked list                         */
	zval                   data;   /* User custom data passed to callback */
	zend_fcall_info       *fci;
	zend_fcall_info_cache *fcc;
	zval                   base;

} php_event_http_cb_t;

/* Represents EventHttp object */
typedef struct _php_event_http_t {
	struct evhttp         *ptr;
	zval                  base;        /* Event base associated with the listener              */
	zval                  data;        /* User custom data passed to the gen(default) callback */

	/* General(default) callback for evhttp_gencb() */
	zend_fcall_info       *fci;
	zend_fcall_info_cache *fcc;

	/* Linked list of attached callbacks */
	php_event_http_cb_t   *cb_head;

	PHP_EVENT_OBJECT_TAIL;
} php_event_http_t;

/* Represents EventHttpConnection object */
typedef struct _php_event_http_conn_t {
	struct evhttp_connection *conn;
	zval                     base;       /* Event base associated with the listener */
	zval                     dns_base;   /* Associated EventDnsBase                 */
	zval                     self;

	/* User custom data passed to the callback for connection close */
	zval                  data_closecb;
	/* Callback for connection close */
	zend_fcall_info       *fci_closecb;
	zend_fcall_info_cache *fcc_closecb;

	PHP_EVENT_OBJECT_TAIL;
} php_event_http_conn_t;

typedef struct {
	struct evhttp_request *ptr;
	/* Whether is artificially created object that must not free 'ptr' */
	zend_bool              internal;
	zval                  self;
	/* User custom data passed to the gen(default) callback */
	zval                  data;
	/* General(default) callback for evhttp_gencb() */
	zend_fcall_info       *fci;
	zend_fcall_info_cache *fcc;

	PHP_EVENT_OBJECT_TAIL;
} php_event_http_req_t;

#endif/* HAVE_EVENT_EXTRA_LIB }}} */

#ifdef HAVE_EVENT_OPENSSL_LIB/*{{{*/
enum {
	PHP_EVENT_OPT_LOCAL_CERT               = 1,
	PHP_EVENT_OPT_LOCAL_PK                 = 2,
	PHP_EVENT_OPT_PASSPHRASE               = 3,
	PHP_EVENT_OPT_CA_FILE                  = 4,
	PHP_EVENT_OPT_CA_PATH                  = 5,
	PHP_EVENT_OPT_ALLOW_SELF_SIGNED        = 6,
	PHP_EVENT_OPT_VERIFY_PEER              = 7,
	PHP_EVENT_OPT_VERIFY_DEPTH             = 8,
	PHP_EVENT_OPT_CIPHERS                  = 9,
	PHP_EVENT_OPT_NO_SSLv2                 = 10,
	PHP_EVENT_OPT_NO_SSLv3                 = 11,
	PHP_EVENT_OPT_NO_TLSv1                 = 12,
	PHP_EVENT_OPT_NO_TLSv1_1               = 13,
	PHP_EVENT_OPT_NO_TLSv1_2               = 14,
	PHP_EVENT_OPT_CIPHER_SERVER_PREFERENCE = 15
};

enum {
	PHP_EVENT_SSLv2_CLIENT_METHOD  = 1,
	PHP_EVENT_SSLv3_CLIENT_METHOD  = 2,
	PHP_EVENT_SSLv23_CLIENT_METHOD = 3,
	PHP_EVENT_TLS_CLIENT_METHOD    = 4,
	PHP_EVENT_SSLv2_SERVER_METHOD  = 5,
	PHP_EVENT_SSLv3_SERVER_METHOD  = 6,
	PHP_EVENT_SSLv23_SERVER_METHOD = 7,
	PHP_EVENT_TLS_SERVER_METHOD    = 8,
	PHP_EVENT_TLSv11_CLIENT_METHOD = 9,
	PHP_EVENT_TLSv11_SERVER_METHOD = 10,
	PHP_EVENT_TLSv12_CLIENT_METHOD = 11,
	PHP_EVENT_TLSv12_SERVER_METHOD = 12
};

typedef struct _php_event_ssl_context_t {
	SSL_CTX   *ctx;
	HashTable *ht;
	PHP_EVENT_OBJECT_TAIL;
} php_event_ssl_context_t;
#endif /* HAVE_EVENT_OPENSSL_LIB }}} */

typedef double php_event_timestamp_t;

/* Object's internal struct name */
#define Z_EVENT_X_OBJ_T(x) php_event_ ## x ## _object


/* Property handler types */

#define Z_EVENT_X_PROP_READ_HND_T(x)  php_event_ ## x ## _read_t
#define Z_EVENT_X_PROP_WRITE_HND_T(x) php_event_ ## x ## _write_t
#define Z_EVENT_X_PROP_PP_HND_T(x)    php_event_ ## x ## _get_prop_ptr_ptr_t
#define Z_EVENT_X_PROP_HND_TYPEDEF(x) \
	typedef zval *(*Z_EVENT_X_PROP_READ_HND_T(x))(Z_EVENT_X_OBJ_T(x) *obj, zval *retval); \
	typedef int (*Z_EVENT_X_PROP_WRITE_HND_T(x))(Z_EVENT_X_OBJ_T(x) *obj, zval *newval); \
	typedef zval *(*Z_EVENT_X_PROP_PP_HND_T(x))(Z_EVENT_X_OBJ_T(x) *obj)

Z_EVENT_X_PROP_HND_TYPEDEF(event)
Z_EVENT_X_PROP_HND_TYPEDEF(bevent)
Z_EVENT_X_PROP_HND_TYPEDEF(buffer)
#ifdef HAVE_EVENT_EXTRA_LIB
Z_EVENT_X_PROP_HND_TYPEDEF(listener)
#endif
#ifdef HAVE_EVENT_OPENSSL_LIB
Z_EVENT_X_PROP_HND_TYPEDEF(ssl_context)
#endif

#undef Z_EVENT_X_PROP_HND_TYPEDEF


/* Property entry structs */

#define Z_EVENT_X_PROP_ENTRY_T(x) php_event_ ## x ## _property_entry_t
#define Z_EVENT_X_PROP_ENTRY_T_DECL(x)                   \
	typedef struct _ ## Z_EVENT_X_PROP_ENTRY_T(x) {      \
		const char                    *name;             \
		size_t                         name_length;      \
		Z_EVENT_X_PROP_READ_HND_T(x)   read_func;        \
		Z_EVENT_X_PROP_WRITE_HND_T(x)  write_func;       \
		Z_EVENT_X_PROP_PP_HND_T(x)     get_ptr_ptr_func; \
	} Z_EVENT_X_PROP_ENTRY_T(x)

Z_EVENT_X_PROP_ENTRY_T_DECL(event)
Z_EVENT_X_PROP_ENTRY_T_DECL(bevent)
Z_EVENT_X_PROP_ENTRY_T_DECL(buffer)
#ifdef HAVE_EVENT_EXTRA_LIB
Z_EVENT_X_PROP_ENTRY_T_DECL(listener)
#endif
#ifdef HAVE_EVENT_OPENSSL_LIB
Z_EVENT_X_PROP_ENTRY_T_DECL(ssl_context)
#endif

#undef Z_EVENT_X_PROP_ENTRY_T_DECL


/* Property entry handler structs */

#define Z_EVENT_X_PROP_HND_T(x) php_event_ ## x ## _prop_handler_t
#define Z_EVENT_X_PROP_HND_T_DECL(x)                     \
	typedef struct _ ## Z_EVENT_X_PROP_HND_T(x) {        \
		zend_string                       *name;         \
		Z_EVENT_X_PROP_READ_HND_T(x)   read_func;        \
		Z_EVENT_X_PROP_WRITE_HND_T(x)  write_func;       \
		Z_EVENT_X_PROP_PP_HND_T(x)     get_ptr_ptr_func; \
	} Z_EVENT_X_PROP_HND_T(x)

Z_EVENT_X_PROP_HND_T_DECL(event)
Z_EVENT_X_PROP_HND_T_DECL(bevent)
Z_EVENT_X_PROP_HND_T_DECL(buffer)
#ifdef HAVE_EVENT_EXTRA_LIB
Z_EVENT_X_PROP_HND_T_DECL(listener)
#endif
#ifdef HAVE_EVENT_OPENSSL_LIB
Z_EVENT_X_PROP_HND_T_DECL(ssl_context)
#endif

#undef Z_EVENT_X_PROP_HND_T_DECL


#ifndef LIBEVENT_VERSION_NUMBER
# error "<event2/*.h> must be included before " ## #__FILE__
#endif

#if LIBEVENT_VERSION_NUMBER < 0x02000200
/* These types are introduced in libevent 2.0.2-alpha */
typedef void (*bufferevent_data_cb)(struct bufferevent *bev, void *ctx);
typedef void (*bufferevent_event_cb)(struct bufferevent *bev, short events,
		void *ctx);
#endif

#endif /* PHP_EVENT_STRUCTS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

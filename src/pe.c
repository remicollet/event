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
#include "src/common.h"
#include "src/priv.h"
#include "src/util.h"

/* {{{ get_ssl_option */
static zval **get_ssl_option(const HashTable *ht, ulong idx)
{
    zval **val;

	if (zend_hash_index_find(ht, idx, (void **) &val) == SUCCESS) {
		return val;
    }

    return NULL;
}
/* }}} */


/* {{{ event_timer_pending_prop_read */
static int event_timer_pending_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_t *e = (php_event_t *) obj;

	PHP_EVENT_ASSERT(e->event);

	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, (evtimer_pending(e->event, NULL) ? 1 : 0));

	return SUCCESS;
}
/* }}} */



/* {{{ event_buffer_length_prop_read */
static int event_buffer_length_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) obj;

	PHP_EVENT_ASSERT(b->buf);

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, evbuffer_get_length(b->buf));

	return SUCCESS;
}
/* }}} */

/* {{{ event_buffer_contiguous_space_prop_read */
static int event_buffer_contiguous_space_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_buffer_t *b = (php_event_buffer_t *) obj;

	PHP_EVENT_ASSERT(b->buf);

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, evbuffer_get_contiguous_space(b->buf));

	return SUCCESS;
}
/* }}} */

/* {{{ event_buffer_pos_position_prop_read */
static int event_buffer_pos_position_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_buffer_pos_t *pos = (php_event_buffer_pos_t *) obj;

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, pos->p.pos);

	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_priority_prop_write*/
static int event_bevent_priority_prop_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;
	long priority           = Z_LVAL_P(value);

	if (bufferevent_priority_set(bev->bevent, priority)) {
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_priority_prop_read */
static int event_bevent_priority_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	ALLOC_INIT_ZVAL(*retval);
	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_input_prop_read */
static int event_bevent_input_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;

	if (!bev->bevent) {
		return FAILURE;
	}

	if (!bev->input) {
		php_event_buffer_t *b;

		MAKE_STD_ZVAL(bev->input);
		PHP_EVENT_INIT_CLASS_OBJECT(bev->input, php_event_buffer_ce);
		PHP_EVENT_FETCH_BUFFER(b, bev->input);

		b->buf      = bufferevent_get_input(bev->bevent);
		b->internal = 1;
	}

	MAKE_STD_ZVAL(*retval);

	ZVAL_ZVAL(*retval, bev->input, 1, 0);
	Z_SET_ISREF_P(*retval);
	Z_ADDREF_P(*retval);
	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_output_prop_read */
static int event_bevent_output_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;

	if (!bev->bevent) {
		return FAILURE;
	}

	if (!bev->output) {
		php_event_buffer_t *b;

		MAKE_STD_ZVAL(bev->output);
		PHP_EVENT_INIT_CLASS_OBJECT(bev->output, php_event_buffer_ce);
		PHP_EVENT_FETCH_BUFFER(b, bev->output);

		b->buf      = bufferevent_get_output(bev->bevent);
		b->internal = 1;
	}

	MAKE_STD_ZVAL(*retval);

	ZVAL_ZVAL(*retval, bev->output, 1, 0);
	Z_SET_ISREF_P(*retval);
	Z_ADDREF_P(*retval);
	return SUCCESS;
}
/* }}} */


/* {{{ event_bevent_input_prop_ptr_ptr */
static zval **event_bevent_input_prop_ptr_ptr(php_event_abstract_object_t *obj TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;

	return bev->input ? &bev->input : NULL;
}
/* }}} */

/* {{{ event_bevent_output_prop_ptr_ptr */
static zval **event_bevent_output_prop_ptr_ptr(php_event_abstract_object_t *obj TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;

	return bev->output ? &bev->output : NULL;
}
/* }}} */


#if LIBEVENT_VERSION_NUMBER >= 0x02010100
/* {{{ event_bevent_allow_ssl_dirty_shutdown_prop_write*/
static int event_bevent_allow_ssl_dirty_shutdown_prop_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_bevent_t *bev      = (php_event_bevent_t *) obj;
	int allow_ssl_dirty_shutdown = (int) Z_BVAL_P(value);

	bufferevent_openssl_set_allow_dirty_shutdown(bev->bevent, allow_ssl_dirty_shutdown);
	return SUCCESS;
}
/* }}} */

/* {{{ event_bevent_allow_ssl_dirty_shutdown_prop_read */
static int event_bevent_allow_ssl_dirty_shutdown_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *)obj;

	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, (zend_bool) bufferevent_openssl_get_allow_dirty_shutdown(bev->bevent));
	return SUCCESS;
}
/* }}} */
#endif

#ifdef HAVE_EVENT_OPENSSL_LIB
#include "classes/ssl_context.h"

/* {{{ event_ssl_context_local_cert_prop_write*/
static int event_ssl_context_local_cert_prop_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *) obj;
	zval **val                    = get_ssl_option(ectx->ht, PHP_EVENT_OPT_LOCAL_PK);
	char *private_key             = val ? Z_STRVAL_PP(val) : NULL;

	if (_php_event_ssl_ctx_set_local_cert(ectx->ctx, Z_STRVAL_P(value), private_key TSRMLS_CC)) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ event_ssl_context_local_cert_prop_read */
static int event_ssl_context_local_cert_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *) obj;
	zval **val                    = get_ssl_option(ectx->ht, PHP_EVENT_OPT_LOCAL_CERT);

	if (val) {
		MAKE_STD_ZVAL(*retval);
		ZVAL_STRINGL(*retval, Z_STRVAL_PP(val), Z_STRLEN_PP(val), 1);
	} else {
		ALLOC_INIT_ZVAL(*retval);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ event_ssl_context_local_pk_prop_write */
static int event_ssl_context_local_pk_prop_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *) obj;

	if (_php_event_ssl_ctx_set_private_key(ectx->ctx, Z_STRVAL_P(value) TSRMLS_CC)) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ event_ssl_context_local_pk_prop_read */
static int event_ssl_context_local_pk_prop_read(php_event_abstract_object_t *obj, zval **retval TSRMLS_DC)
{
	php_event_ssl_context_t *ectx = (php_event_ssl_context_t *) obj;
	zval **val                    = get_ssl_option(ectx->ht, PHP_EVENT_OPT_LOCAL_PK);

	if (val) {
		MAKE_STD_ZVAL(*retval);
		ZVAL_STRINGL(*retval, Z_STRVAL_PP(val), Z_STRLEN_PP(val), 1);
	} else {
		ALLOC_INIT_ZVAL(*retval);
	}

	return SUCCESS;
}
/* }}} */
#endif


const php_event_property_entry_t event_property_entries[] = {
	{"timer_pending",           sizeof("timer_pending") - 1, event_timer_pending_prop_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_bevent_property_entries[] = {
	{"priority", sizeof("priority") - 1, event_bevent_priority_prop_read, event_bevent_priority_prop_write, NULL },
	{"input",    sizeof("input")    - 1, event_bevent_input_prop_read,    NULL,                             event_bevent_input_prop_ptr_ptr},
	{"output",   sizeof("output")   - 1, event_bevent_output_prop_read,   NULL,                             event_bevent_output_prop_ptr_ptr},

#if LIBEVENT_VERSION_NUMBER >= 0x02010100
	{"allow_ssl_dirty_shutdown", sizeof("allow_ssl_dirty_shutdown") - 1,
		event_bevent_allow_ssl_dirty_shutdown_prop_read,
		event_bevent_allow_ssl_dirty_shutdown_prop_write, NULL },
#endif
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_buffer_property_entries[] = {
	{"length",           sizeof("length")           - 1, event_buffer_length_prop_read,           NULL, NULL},
	{"contiguous_space", sizeof("contiguous_space") - 1, event_buffer_contiguous_space_prop_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_buffer_pos_property_entries[] = {
	{"position", sizeof("position") - 1, event_buffer_pos_position_prop_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
#ifdef HAVE_EVENT_OPENSSL_LIB
const php_event_property_entry_t event_ssl_context_property_entries[] = {
	{"local_cert", sizeof("local_cert") - 1, event_ssl_context_local_cert_prop_read, event_ssl_context_local_cert_prop_write, NULL},
	{"local_pk", sizeof("local_pk") - 1, event_ssl_context_local_pk_prop_read, event_ssl_context_local_pk_prop_write, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
#endif

const zend_property_info event_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "timer_pending", sizeof("timer_pending") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_bevent_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "priority", sizeof("priority") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "input", sizeof("input") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "output", sizeof("output") - 1, -1, 0, NULL, 0, NULL},
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
	{ZEND_ACC_PUBLIC, "allow_ssl_dirty_shutdown", sizeof("allow_ssl_dirty_shutdown") - 1, -1, 0, NULL, 0, NULL},
#endif
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_buffer_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "length",           sizeof("length")           - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "contiguous_space", sizeof("contiguous_space") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_buffer_pos_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "position", sizeof("position") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
#ifdef HAVE_EVENT_OPENSSL_LIB
const zend_property_info event_ssl_context_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "local_cert", sizeof("local_cert") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "local_pk", sizeof("local_pk") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */

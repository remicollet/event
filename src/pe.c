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

/* {{{ event_bevent_priority_write */
static int event_bevent_priority_write(php_event_abstract_object_t *obj, zval *value TSRMLS_DC)
{
	php_event_bevent_t *bev = (php_event_bevent_t *) obj;
	long priority           = Z_LVAL_P(value);

	if (bufferevent_priority_set(bev->bevent, priority)) {
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */


const php_event_property_entry_t event_property_entries[] = {
	{"timer_pending",           sizeof("timer_pending") - 1, event_timer_pending_prop_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_base_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_config_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_bevent_property_entries[] = {
	{"priority", sizeof("priority") - 1, NULL, event_bevent_priority_write, NULL},
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

const zend_property_info event_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "timer_pending", sizeof("timer_pending") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_base_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_config_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_bevent_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "priority", sizeof("priority") - 1, -1, 0, NULL, 0, NULL},
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


#if HAVE_EVENT_EXTRA_LIB

const php_event_property_entry_t event_dns_base_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_listener_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_http_conn_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_http_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};

const zend_property_info event_dns_base_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_listener_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_http_conn_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_http_property_entry_info[] = {
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

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
#include "priv.h"
#include "util.h"

const php_event_property_entry_t event_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_base_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_config_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_bevent_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_buffer_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};
const php_event_property_entry_t event_util_property_entries[] = {
    {NULL, 0, NULL, NULL, NULL}
};

const zend_property_info event_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_base_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_config_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_bevent_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_buffer_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
const zend_property_info event_util_property_entry_info[] = {
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

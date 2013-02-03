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
#ifndef PHP_EVENT_PRIV_H
#define PHP_EVENT_PRIV_H

extern const zend_function_entry php_event_ce_functions[];
extern const zend_function_entry php_event_base_ce_functions[];
extern const zend_function_entry php_event_config_ce_functions[];
extern const zend_function_entry php_event_bevent_ce_functions[];
extern const zend_function_entry php_event_buffer_ce_functions[];
extern const zend_function_entry php_event_util_ce_functions[];

extern zend_class_entry *php_event_ce;
extern zend_class_entry *php_event_base_ce;
extern zend_class_entry *php_event_config_ce;
extern zend_class_entry *php_event_bevent_ce;
extern zend_class_entry *php_event_buffer_ce;
extern zend_class_entry *php_event_util_ce;

extern const php_event_property_entry_t event_property_entries[];
extern const php_event_property_entry_t event_base_property_entries[];
extern const php_event_property_entry_t event_config_property_entries[];
extern const php_event_property_entry_t event_bevent_property_entries[];
extern const php_event_property_entry_t event_buffer_property_entries[];
extern const php_event_property_entry_t event_util_property_entries[];

extern const zend_property_info event_property_entry_info[];
extern const zend_property_info event_base_property_entry_info[];
extern const zend_property_info event_config_property_entry_info[];
extern const zend_property_info event_bevent_property_entry_info[];
extern const zend_property_info event_buffer_property_entry_info[];
extern const zend_property_info event_util_property_entry_info[];

#if HAVE_EVENT_EXTRA_LIB

extern const zend_function_entry php_event_dns_base_ce_functions[];
extern const zend_function_entry php_event_http_conn_ce_functions[];
extern const zend_function_entry php_event_http_ce_functions[];
extern const zend_function_entry php_event_listener_ce_functions[];

extern zend_class_entry *php_event_dns_base_ce;
extern zend_class_entry *php_event_listener_ce;
extern zend_class_entry *php_event_http_conn_ce;
extern zend_class_entry *php_event_http_ce;

extern const php_event_property_entry_t event_dns_base_property_entries[];
extern const php_event_property_entry_t event_listener_property_entries[];
extern const php_event_property_entry_t event_http_conn_property_entries[];
extern const php_event_property_entry_t event_http_property_entries[];

extern const zend_property_info event_dns_base_property_entry_info[];
extern const zend_property_info event_listener_property_entry_info[];
extern const zend_property_info event_http_conn_property_entry_info[];
extern const zend_property_info event_http_property_entry_info[];

#endif

#endif /* PHP_EVENT_PRIV_H */

/* 
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */

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

typedef struct {
	zval                  *data;    /* User custom data                            */
	zend_fcall_info       *fci;     /* fci and fcc represent userspace callback    */
	zend_fcall_info_cache *fcc;
	PHP_EVENT_COMMON_THREAD_CTX;
} php_event_cb_arg_t;

typedef struct {
	struct event       *event;     /* Pointer returned by event_new                         */
	int                 stream_id; /* Resource ID of the file descriptor, or signal number  */
	php_event_cb_arg_t *arg;       /* For calling userspace func associated with this event */
} php_event_t;

typedef struct event_base php_event_base_t;
typedef struct event_config php_event_config_t;
typedef double php_event_timestamp_t;

#endif	/* PHP_EVENT_STRUCTS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

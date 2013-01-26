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

/* {{{ proto resource EventBuffer::__construct(void); */
PHP_METHOD(EventBuffer, __construct)
{
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, getThis());

	b->buf = evbuffer_new();
}
/* }}} */

/* {{{ proto bool EventBuffer::freeze(bool at_front);
 * Prevent calls that modify an event buffer from succeeding. */
PHP_METHOD(EventBuffer, freeze)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b",
				&at_front) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_freeze(b->buf, at_front)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::unfreeze(bool at_front);
 * Re-enable calls that modify an event buffer. */
PHP_METHOD(EventBuffer, unfreeze)
{
	zval               *zbuf     = getThis();
	php_event_buffer_t *b;
	zend_bool           at_front;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b",
				&at_front) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_unfreeze(b->buf, at_front)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto long EventBuffer::getLength(void);
 * XXX Create property?
 * Returns the total number of bytes stored in the event buffer. */
PHP_METHOD(EventBuffer, get_length)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	RETVAL_LONG(evbuffer_get_length(b->buf));
}
/* }}} */

/* {{{ proto void EventBuffer::lock(void);
 * Acquire the lock on an evbuffer. 
 * Has no effect if locking was not enabled with evbuffer_enable_locking.
 */
PHP_METHOD(EventBuffer, lock)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_lock(b->buf);
}
/* }}} */

/* {{{ proto void EventBuffer::unlock(void);
 * Release the lock on an evbuffer.
 * Has no effect if locking was not enabled with evbuffer_enable_locking.
 */
PHP_METHOD(EventBuffer, unlock)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_unlock(b->buf);
}
/* }}} */

/* {{{ proto void EventBuffer::enableLocking(void);
 *
 * Enable locking on an evbuffer so that it can safely be used by multiple threads at the same time.
 * When locking is enabled, the lock will be held when callbacks are invoked.
 * This could result in deadlock if you aren't careful. Plan accordingly!
 */
PHP_METHOD(EventBuffer, enableLocking)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	evbuffer_enable_locking(b->buf, NULL);
}
/* }}} */

/* {{{ proto bool EventBuffer::add(string data); 
 *
 * Append data to the end of an event buffer.
 */
PHP_METHOD(EventBuffer, add)
{
	php_event_buffer_t  *b;
	zval                *zbuf    = getThis();
	zval               **ppzdata;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z",
				&ppzdata) == FAILURE) {
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

/* {{{ proto bool EventBuffer::addBuffer(EventBuffer outbuf, EventBuffer inbuf); 
 * Move all data from one evbuffer into another evbuffer.
 * This is a destructive add. The data from one buffer moves into the other buffer. However, no unnecessary memory copies occur.
 */
PHP_METHOD(EventBuffer, add_buffer)
{
	php_event_buffer_t *b_out     , *b_in;
	zval               *zbuf_out  , *zbuf_in;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO",
				&zbuf_out, php_event_buffer_ce,
				&zbuf_in, php_event_buffer_ce) == FAILURE) {
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

/* {{{ proto int EventBuffer::remove(string &data, long max_bytes);
 *
 * Read data from an evbuffer and drain the bytes read.  If more bytes are
 * requested than are available in the evbuffer, we only extract as many bytes
 * as were available.
 *
 * Returns the number of bytes read, or -1 if we can't drain the buffer.
 */
PHP_METHOD(EventBuffer, remove)
{
	php_event_buffer_t *b;
	zval               *zbuf      = getThis();
	zval               *zdata;
	long                max_bytes;
	long                ret;
	char               *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl",
				&zdata, &max_bytes) == FAILURE) {
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

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */

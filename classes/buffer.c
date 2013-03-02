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
#include "src/util.h"
#include "src/priv.h"

/* {{{ proto EventBuffer EventBuffer::__construct(void); */
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

/* {{{ proto bool EventBuffer::addBuffer(EventBuffer buf); 
 * Move all data from the buffer provided in buf parameter to the current instance of EventBuffer.
 * This is a destructive add. The data from one buffer moves into the other buffer. However, no unnecessary memory copies occur.
 */
PHP_METHOD(EventBuffer, addBuffer)
{
	php_event_buffer_t *b_dst;
	php_event_buffer_t *b_src;
	zval               *zbuf_dst = getThis();
	zval               *zbuf_src;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf_src, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b_dst, zbuf_dst);
	PHP_EVENT_FETCH_BUFFER(b_src, zbuf_src);

	if (evbuffer_add_buffer(b_dst->buf, b_src->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::removeBuffer(EventBuffer buf, int len); 
 * Moves exactly len bytes from buf to the end of current instance of EventBuffer
 */
PHP_METHOD(EventBuffer, removeBuffer)
{
	php_event_buffer_t *b_dst;
	php_event_buffer_t *b_src;
	zval               *zbuf_dst = getThis();
	zval               *zbuf_src;
	long                len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol",
				&zbuf_src, php_event_buffer_ce, &len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b_dst, zbuf_dst);
	PHP_EVENT_FETCH_BUFFER(b_src, zbuf_src);

	if (evbuffer_remove_buffer(b_src->buf, b_dst->buf, (size_t) len)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::expand(int len); 
 * Alters the last chunk of memory in the buffer, or adds a new chunk, such that the buffer is now large enough to contain datlen bytes without any further allocations.
 */
PHP_METHOD(EventBuffer, expand)
{
	php_event_buffer_t *b;
	zval               *zbuf = getThis();
	long                len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_expand(b->buf, (size_t) len)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::prepend(string data); 
 *
 * Prepend data to the front of the event buffer.
 */
PHP_METHOD(EventBuffer, prepend)
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

	if (evbuffer_prepend(b->buf, (void *) Z_STRVAL_PP(ppzdata), Z_STRLEN_PP(ppzdata))) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::prependBuffer(EventBuffer buf); 
 * Behaves as EventBuffer::addBuffer, except that it moves data to the front of the buffer.
 */
PHP_METHOD(EventBuffer, prependBuffer)
{
	php_event_buffer_t *b_dst;
	php_event_buffer_t *b_src;
	zval               *zbuf_dst = getThis();
	zval               *zbuf_src;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf_src, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b_dst, zbuf_dst);
	PHP_EVENT_FETCH_BUFFER(b_src, zbuf_src);

	if (evbuffer_prepend_buffer(b_dst->buf, b_src->buf)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventBuffer::drain(long len);
 *
 * Behaves as EventBuffer::remove(), except that it does not copy the data: it
 * just removes it from the front of the buffer.
 */
PHP_METHOD(EventBuffer, drain)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;
	long                len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	if (evbuffer_drain(b->buf, len)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventBuffer::copyout(string &data, long max_bytes);
 *
 * Behaves just like EventBuffer::remove(), but does not drain any data from the buffer.
 * I.e. it copies the first max_bytes bytes from the front of the buffer into data.
 * If there are fewer than datlen bytes available, the function copies all the bytes there are.
 *
 * Returns the number of bytes copied, or -1 on failure.
 */
PHP_METHOD(EventBuffer, copyout)
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

	ret = evbuffer_copyout(b->buf, data, max_bytes);

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

/* {{{ proto mixed EventBuffer::readLine(int eol_style);
 *
 * Extracts a line from the front of the buffer and returns it in a newly
 * allocated NUL-terminated string. If there is not a whole
 * line to read, the function returns NULL. The line terminator is not included
 * in the copied string.
 *
 * eol_style is one of EventBuffer:EOL_* constants.
 *
 * On success returns the line read from the buffer, otherwise NULL.
 */
PHP_METHOD(EventBuffer, readLine)
{
	zval               *zbuf      = getThis();
	php_event_buffer_t *b;
	long                len;
	long                eol_style;
	char               *res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&eol_style) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	res = evbuffer_readln(b->buf, (size_t *) &len, eol_style);

	if (!res) {
		RETURN_NULL();
	}

	RETVAL_STRINGL(res, len, 1);
}
/* }}} */

/* {{{ proto EventBufferPosition EventBuffer::search(string what[, EventBufferPosition start = NULL[, EventBufferPosition end = NULL]]);
 *
 * Scans the buffer for an occurrence of the len-character string what. It
 * returns object representing the position of the string, or NULL if the
 * string was not found. If the start argument is provided, it's the position
 * at which the search should begin; otherwise, the search is from the start
 * of the string. If end argument provided, the search is performed between
 * start and end buffer positions.
 *
 * Returns EventBufferPosition representing position of the first occurance of the string
 * in the buffer, or NULL if string is not found
 */
PHP_METHOD(EventBuffer, search)
{
	zval                   *zbuf       = getThis();
	zval                   *zstart_pos = NULL;
	zval                   *zend_pos   = NULL;
	char                   *what;
	int                     what_len;
	php_event_buffer_t     *b;
	php_event_buffer_pos_t *start_pos  = NULL;
	php_event_buffer_pos_t *end_pos    = NULL;
	php_event_buffer_pos_t *res_pos;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|O!O!",
				&what, &what_len,
				&zstart_pos, php_event_buffer_pos_ce,
				&zend_pos, php_event_buffer_pos_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_pos_ce);
	PHP_EVENT_FETCH_BUFFER_POS(res_pos, return_value);

	if (zstart_pos) {
		PHP_EVENT_FETCH_BUFFER_POS(start_pos, zstart_pos);
	}

	if (zend_pos) {
		PHP_EVENT_FETCH_BUFFER_POS(end_pos, zend_pos);

		res_pos->p = evbuffer_search_range(b->buf, what, (size_t) what_len,
				&start_pos->p, &end_pos->p);
	} else {
		res_pos->p = evbuffer_search(b->buf, what, (size_t) what_len, (start_pos ? &start_pos->p : NULL));
	}

	if (res_pos->p.pos == -1) {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ proto bool EventBuffer::setPosition(EventBufferPosition &pos, int value, int how);
 *
 * Manipulates the position pos within buffer. If how is EventBuffer::PTR_SET,
 * the pointer is moved to an absolute position position within the buffer. If
 * it is EventBuffer::PTR_ADD, the pointer moves position bytes forward.
 */
PHP_METHOD(EventBuffer, setPosition)
{
	zval                *zbuf  = getThis();
	zval                *zpos;
	long                 value;
	long                 how;
	php_event_buffer_t  *b;
	php_event_buffer_pos_t *pos   = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Oll",
				&zpos, php_event_buffer_pos_ce, &value, &how) == FAILURE) {
		return;
	}

	if (how & ~(EVBUFFER_PTR_SET | EVBUFFER_PTR_ADD)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid mask");
		RETURN_FALSE;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);
	PHP_EVENT_FETCH_BUFFER_POS(pos, zpos);

	if (evbuffer_ptr_set(b->buf, &pos->p, value, how)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto string EventBuffer::pullup(int size);
 *
 * "Linearizes" the first size bytes of the buffer, copying or moving them as needed to
 * ensure that they are all contiguous and occupying the same chunk of memory. If size is
 * negative, the function linearizes the entire buffer. If size is greater than the number
 * of bytes in the buffer, the function returns NULL. Otherwise, EventBuffer::pullup()
 * returns string.
 *
 * Calling EventBuffer::pullup() with a large size can be quite slow, since it potentially
 * needs to copy the entire buffer's contents.
 */
PHP_METHOD(EventBuffer, pullup)
{
	zval               *zbuf = getThis();
	php_event_buffer_t *b;
	long                size;
	unsigned char      *mem;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&size) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_BUFFER(b, zbuf);

	mem = evbuffer_pullup(b->buf, size);

	if (mem == NULL) {
		RETURN_NULL();
	}

	RETVAL_STRING((const char *)mem, 1);
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

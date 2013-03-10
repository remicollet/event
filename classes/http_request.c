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
#include "classes/http.h"

/* {{{ proto int EventHttpRequest::getCommand(void);
 * Returns the request command, one of EventHttpRequest::CMD_* constants. XXX Make property? */
PHP_METHOD(EventHttpRequest, getCommand)
{
	php_event_http_req_t *http_req;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	RETVAL_LONG(evhttp_request_get_command(http_req->ptr));
}
/* }}} */

/* {{{ proto int EventHttpRequest::getUri(void);
 * Returns the request URI. XXX make a property? */
PHP_METHOD(EventHttpRequest, getUri)
{
	php_event_http_req_t *http_req;
	char *uri;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	uri = evhttp_request_get_uri(http_req->ptr);
	RETVAL_STRING(uri, 1);
	free(uri);
}
/* }}} */

/* {{{ proto array EventHttpRequest::getInputHeaders(void);
 * Returns associative array of the input headers. */
PHP_METHOD(EventHttpRequest, getInputHeaders)
{
	php_event_http_req_t *http_req;
	struct evkeyvalq     *headers;
	struct evkeyval      *header;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	array_init(return_value);

	headers = evhttp_request_get_input_headers(http_req->ptr);
	for (header = headers->tqh_first; header;
			header = header->next.tqe_next) {
		add_assoc_string(return_value, header->key, header->value, 1);
	}
}
/* }}} */

/* {{{ proto array EventHttpRequest::getOutputHeaders(void);
 * Returns associative array of the output headers. */
PHP_METHOD(EventHttpRequest, getOutputHeaders)
{
	php_event_http_req_t *http_req;
	struct evkeyvalq     *headers;
	struct evkeyval      *header;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	array_init(return_value);

	headers = evhttp_request_get_output_headers(http_req->ptr);
	for (header = headers->tqh_first; header;
			header = header->next.tqe_next) {
		add_assoc_string(return_value, header->key, header->value, 1);
	}
}
/* }}} */

/* {{{ proto EventBuffer EventHttpRequest::getInputBuffer(void);
 * Returns input buffer. */
PHP_METHOD(EventHttpRequest, getInputBuffer)
{
	php_event_http_req_t *http_req;
	php_event_buffer_t   *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	PHP_EVENT_FETCH_BUFFER(b, return_value);
	b->buf      = evhttp_request_get_input_buffer(http_req->ptr);
	b->internal = 1;
}
/* }}} */

/* {{{ proto EventBuffer EventHttpRequest::getOutputBuffer(void);
 * Returns output buffer. */
PHP_METHOD(EventHttpRequest, getOutputBuffer)
{
	php_event_http_req_t *http_req;
	php_event_buffer_t   *b;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	PHP_EVENT_INIT_CLASS_OBJECT(return_value, php_event_buffer_ce);
	PHP_EVENT_FETCH_BUFFER(b, return_value);
	b->buf      = evhttp_request_get_output_buffer(http_req->ptr);
	b->internal = 1;
}
/* }}} */

/* {{{ proto void EventHttpRequest::sendReply(int code, string reason[, EventBuffer buf=&null;]);
 * Send an HTML reply to client.
 *
 * The body of the reply consists of data in <parameter>buf</parameter>. */
PHP_METHOD(EventHttpRequest, sendReply)
{
	php_event_http_req_t *http_req;
	long                  code;
	char                 *reason;
	int                   reason_len;
	zval                 *zbuf = NULL;
	php_event_buffer_t   *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls|O!",
				&code, &reason, &reason_len,
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	if (zbuf) {
		PHP_EVENT_FETCH_BUFFER(b, zbuf);
		PHP_EVENT_ASSERT(b->buf);
	}

	evhttp_send_reply(http_req->ptr, code, reason,
			(zbuf ? b->buf : NULL));
}
/* }}} */

/* {{{ proto void EventHttpRequest::sendReplyChunk(EventBuffer buf);
 * Send another data chunk as part of an ongoing chunked reply.
 *
 * After calling this method <parameter>buf</parameter> will be	empty. */
PHP_METHOD(EventHttpRequest, sendReplyChunk)
{
	php_event_http_req_t *http_req;
	zval                 *zbuf;
	php_event_buffer_t   *b;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&zbuf, php_event_buffer_ce) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	if (zbuf) {
		PHP_EVENT_FETCH_BUFFER(b, zbuf);
		PHP_EVENT_ASSERT(b->buf);
	}

	evhttp_send_reply_chunk(http_req->ptr, code, reason, b->buf);
}
/* }}} */

/* {{{ proto void EventHttpRequest::sendReplyEnd(void);
 * Complete a chunked reply, freeing the request as appropriate. 
 */
PHP_METHOD(EventHttpRequest, sendReplyEnd)
{
	php_event_http_req_t *http_req;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	evhttp_send_reply_end(http_req->ptr);
}
/* }}} */

/* {{{ proto void EventHttpRequest::sendReplyStart(int code, string reason);
 * Initiate a reply that uses <literal>Transfer-Encoding</literal>
 * <literal>chunked</literal>.
 *
 * This allows the caller to stream the reply back to the client and is useful
 * when either not all of the reply data is immediately available or when
 * sending very large replies.
 *
 * The caller needs to supply data chunks with
 * <method>EventHttpRequest::sendReplyChunk</method> and complete the reply by
 * calling <method>EventHttpRequest::sendReplyEnd</method>.
 */
PHP_METHOD(EventHttpRequest, sendReplyEnd)
{
	php_event_http_req_t *http_req;
	long                  code;
	char                 *reason;
	int                   reason_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls",
				&code, &reason, &reason_len) == FAILURE) {
		return;
	}

	PHP_EVENT_FETCH_HTTP_REQ(http_req, getThis());

	if (!http_req->ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid HTTP request object");
		RETURN_FALSE;
	}

	evhttp_send_reply_start(http_req->ptr, code, reason);
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

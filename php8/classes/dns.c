/*
   +----------------------------------------------------------------------+
   | PHP Version 8                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2023 The PHP Group                                |
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
#include "../src/common.h"
#include "../src/util.h"
#include "../src/priv.h"
#include <limits.h>

/* {{{ proto EventDnsBase EventDnsBase::__construct(EventBase base, mixed initialize);
 *
 * Returns object representing event dns base.
 *
 * If the initialize argument is true, it tries to configure the DNS base
 * sensibly given your operating system’s default. If it is false, it leaves the
 * event dns base empty, with no nameservers or options configured. In the latter
 * case you should configure dns base yourself, e.g. with EventDnsBase::parseResolvConf().
 *
 * If initialize is an integer, it must be one of the following flags:
 * - EventDnsBase::DISABLE_WHEN_INACTIVE - Do not prevent the libevent event loop from exiting when we have no active dns requests.
 * - EventDnsBase::INITIALIZE_NAMESERVERS - Process resolv.conf.
 * - EventDnsBase::NAMESERVERS_NO_DEFAULT - Do not add default nameserver if there are no nameservers in resolv.conf.
 * */
PHP_EVENT_METHOD(EventDnsBase, __construct)
{
	php_event_base_t     *base;
	zval                 *zbase;
	php_event_dns_base_t *dnsb;
	zval                 *zinitialize;
	int                   flags = 0;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_OBJECT_OF_CLASS(zbase, php_event_base_ce)
		Z_PARAM_ZVAL(zinitialize)
	ZEND_PARSE_PARAMETERS_END();

	PHP_EVENT_REQUIRE_BASE_BY_REF(zbase);

	base = Z_EVENT_BASE_OBJ_P(zbase);
	dnsb = Z_EVENT_DNS_BASE_OBJ_P(getThis());

	PHP_EVENT_ASSERT(dnsb);
	PHP_EVENT_ASSERT(base->base != NULL);

	if (Z_TYPE_P(zinitialize) == IS_FALSE) {
		flags = 0;
#if LIBEVENT_VERSION_NUMBER >= 0x02010000
	} else if (Z_TYPE_P(zinitialize) == IS_TRUE) {
		flags = EVDNS_BASE_INITIALIZE_NAMESERVERS;
	} else if (Z_TYPE_P(zinitialize) == IS_LONG) {
		long lflags = Z_LVAL_P(zinitialize);

		if (lflags > INT_MAX || lflags < INT_MIN) {
			zend_throw_exception_ex(php_event_get_exception(), 0, "The value of initialization flags is out of range");
			goto fail;
		}
		flags = lflags;

		if (flags & ~(EVDNS_BASE_DISABLE_WHEN_INACTIVE
#if LIBEVENT_VERSION_NUMBER >= 0x02011000
					| EVDNS_BASE_NAMESERVERS_NO_DEFAULT
#endif
					| EVDNS_BASE_INITIALIZE_NAMESERVERS)) {
			zend_throw_exception_ex(php_event_get_exception(), 0, "Invalid initialization flags");
			goto fail;
		}
#endif /* libevent version >= 2.1 */
	} else {
		zend_throw_exception_ex(php_event_get_exception(), 0, "Invalid type of the initialization flags");
		goto fail;
	}

	if (dnsb != NULL) {
		dnsb->dns_base = evdns_base_new(base->base, flags);
	}
fail:
	;
}
/* }}} */

/* {{{ proto bool EventDnsBase::parseResolvConf(int flags, string filename);
 * Scans the resolv.conf formatted file stored in filename, and read in all the
 * options from it that are listed in flags */
PHP_EVENT_METHOD(EventDnsBase, parseResolvConf)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base    = getThis();
	zend_long             flags;
	char                 *filename;
	size_t                filename_len;
	int                   ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls",
				&flags, &filename, &filename_len) == FAILURE) {
		return;
	}

	if (flags & ~(DNS_OPTION_NAMESERVERS | DNS_OPTION_SEARCH | DNS_OPTION_MISC
				| DNS_OPTIONS_ALL)) {
		php_error_docref(NULL, E_WARNING,
				"Invalid flags");
		RETURN_FALSE;
	}

	dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);

	ret = evdns_base_resolv_conf_parse(dnsb->dns_base, flags, filename);

	if (ret) {
		char err[40];

		switch (ret) {
			case 1:
				strlcpy(err, "Failed to open file", sizeof(err));
				break;
			case 2:
				strlcpy(err, "Failed to stat file", sizeof(err));
				break;
			case 3:
				strlcpy(err, "File too large", sizeof(err));
				break;
			case 4:
				strlcpy(err, "Out of memory", sizeof(err));
				break;
			case 5:
				strlcpy(err, "Short read from file", sizeof(err));
				break;
			case 6:
				strlcpy(err, "No nameservers listed in the file", sizeof(err));
				break;
		}

		php_error_docref(NULL, E_WARNING, "%s", err);
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventDnsBase::addNameserverIp(string ip);
 * Adds a nameserver to an existing evdns_base. It takes the nameserver in a
 * text string, either as an IPv4 address, an IPv6 address, an IPv4 address
 * with a port (IPv4:Port), or an IPv6 address with a port ([IPv6]:Port).
 */
PHP_EVENT_METHOD(EventDnsBase, addNameserverIp)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();
	char                 *ip;
	size_t                ip_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
				&ip, &ip_len) == FAILURE) {
		return;
	}

	dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);

	if (evdns_base_nameserver_ip_add(dnsb->dns_base, ip)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto bool EventDnsBase::loadHosts(string hosts);
 *  Loads a hosts file (in the same format as /etc/hosts) from hosts file
 */
PHP_EVENT_METHOD(EventDnsBase, loadHosts)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();
	char                 *hosts;
	size_t                hosts_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
				&hosts, &hosts_len) == FAILURE) {
		return;
	}

	dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);

	if (evdns_base_load_hosts(dnsb->dns_base, hosts)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto void EventDnsBase::clearSearch(void);
 * Removes all current search suffixes (as configured by the search option)
 * from the evdns_base; the evdns_base_search_add() function adds a suffix
 */
PHP_EVENT_METHOD(EventDnsBase, clearSearch)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);

	evdns_base_search_clear(dnsb->dns_base);
}
/* }}} */

/* {{{ proto void EventDnsBase::addSearch(string domain);
 */
PHP_EVENT_METHOD(EventDnsBase, addSearch)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base  = getThis();
	char                 *domain;
	size_t                domain_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
				&domain, &domain_len) == FAILURE) {
		return;
	}

	dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);

	evdns_base_search_add(dnsb->dns_base, domain);
}
/* }}} */

/* {{{ proto void EventDnsBase::setSearchNdots(int ndots);
 */
PHP_EVENT_METHOD(EventDnsBase, setSearchNdots)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();
	zend_long             ndots;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&ndots) == FAILURE) {
		return;
	}

	dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);

	evdns_base_search_ndots_set(dnsb->dns_base, ndots);
}
/* }}} */

/* {{{ proto bool EventDnsBase::setOption(string option, string value);
 */
PHP_EVENT_METHOD(EventDnsBase, setOption)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base  = getThis();
	char                 *option;
	size_t                option_len;
	char                 *value;
	size_t                value_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss",
				&option, &option_len, &value, &value_len) == FAILURE) {
		return;
	}

	dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);

	if (evdns_base_set_option(dnsb->dns_base, option, value)) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int EventDnsBase::countNameservers(void);
 */
PHP_EVENT_METHOD(EventDnsBase, countNameservers)
{
	php_event_dns_base_t *dnsb;
	zval                 *zdns_base = getThis();

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	dnsb = Z_EVENT_DNS_BASE_OBJ_P(zdns_base);

	RETURN_LONG(evdns_base_count_nameservers(dnsb->dns_base));
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

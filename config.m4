dnl +----------------------------------------------------------------------+
dnl | PHP Version 5                                                        |
dnl +----------------------------------------------------------------------+
dnl | Copyrght (C) 1997-2013 The PHP Group                                 |
dnl +----------------------------------------------------------------------+
dnl | This source file is subject to version 3.01 of the PHP license,      |
dnl | that is bundled with this package in the file LICENSE, and is        |
dnl | available through the world-wide-web at the following url:           |
dnl | http://www.php.net/license/3_01.txt                                  |
dnl | If you did not receive a copy of the PHP license and are unable to   |
dnl | obtain it through the world-wide-web, please send a note to          |
dnl | license@php.net so we can mail you a copy immediately.               |
dnl +----------------------------------------------------------------------+
dnl | Author: Ruslan Osmanov <osmanov@php.net>                             |
dnl +----------------------------------------------------------------------+

PHP_ARG_WITH(event-core, for event core support,
[  --with-event-core        Include core libevent support])

PHP_ARG_WITH(event-pthreads, for event thread safety support,
[  --with-event-pthreads    Include libevent's pthreads library and enable thread safety support in event], yes, no)

PHP_ARG_WITH(event-extra, for event extra functionality support,
[  --with-event-extra       Include libevent protocol-specific functionality support including HTTP, DNS, and RPC], yes, no)

PHP_ARG_WITH(event-openssl, for OpenSSL support in event,
[  --with-event-openssl Include libevent OpenSSL support], yes, no)

PHP_ARG_WITH(openssl-dir, OpenSSL installation prefix,
[  --with-openssl-dir[=DIR]  Event: openssl installation prefix], no, no)

PHP_ARG_WITH([event-libevent-dir], [],
[  --with-event-libevent-dir[=DIR] Event: libevent installation prefix], no, no)

PHP_ARG_ENABLE(event-debug, Event: debug support,
[  --enable-event-debug     Enable debug support in event], no, no)

if test "$PHP_EVENT_CORE" != "no"; then
  dnl {{{ Check for PHP version
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_EV"
  AC_MSG_CHECKING(PHP version)
  AC_TRY_COMPILE([#include <php_version.h>], [
  #if PHP_VERSION_ID < 50400
  # error  this extension requires at least PHP version 5.4.0
  #endif
  ],
  [AC_MSG_RESULT(ok)],
  [AC_MSG_ERROR([need at least PHP 5.4.0])])
  export CPPFLAGS="$OLD_CPPFLAGS"
  dnl }}}
  
  dnl {{{ --enable-event-debug
  if test "$PHP_EVENT_DEBUG" != "no"; then
    CFLAGS="$CFLAGS -Wall -g -ggdb -O0"
    AC_DEFINE(PHP_EVENT_DEBUG, 1, [Enable event debug support])
  else
    AC_DEFINE(NDEBUG, 1, [With NDEBUG defined assert generates no code])
  fi
  dnl }}}

  dnl {{{ Include libevent headers
  AC_MSG_CHECKING([for include/event2/event.h])
  EVENT_DIR=
  for i in "$PHP_EVENT_CORE" "$PHP_EVENT_LIBEVENT_DIR" /usr/local /usr /opt; do
	  if test -f "$i/include/event2/event.h"; then
		  EVENT_DIR=$i
		  break
	  fi
  done

  if test "x$EVENT_DIR" = "x"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the event library, or provide the installation prefix via --with-event-libevent-dir option])
  fi
	AC_MSG_RESULT([found in $EVENT_DIR])

	PHP_ADD_INCLUDE($EVENT_DIR/include)
  dnl }}}

	dnl {{{ Check if it's at least libevent 2.0.2-alpha
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_EV"
  AC_MSG_CHECKING(for libevent version)
  AC_TRY_COMPILE([#include <event2/event.h>], [
  #if LIBEVENT_VERSION_NUMBER < 0x02000200
  # error this extension requires at least libevent version 2.0.2-alpha
  #endif
  ],
  [AC_MSG_RESULT(ok)],
  [AC_MSG_ERROR([need at least libevent 2.0.2-alpha])])
  export CPPFLAGS="$OLD_CPPFLAGS"
  dnl }}}
	
  if test -d $EVENT_DIR/$PHP_LIBDIR/event2; then
    dnl FreeBSD
    EVENT_LIBS="-L$EVENT_DIR/$PHP_LIBDIR -L$EVENT_DIR/$PHP_LIBDIR/event2"
    EVENT_LIBDIR=$EVENT_DIR/$PHP_LIBDIR/event2
  else
    EVENT_LIBS="-L$EVENT_DIR/$PHP_LIBDIR"
    EVENT_LIBDIR=$EVENT_DIR/$PHP_LIBDIR
  fi
  LDFLAGS="$EVENT_LIBS -levent_core -levent_pthreads $LDFLAGS"

  dnl {{{ event_core
	AC_CHECK_LIB(event_core, event_free, [
	  PHP_ADD_LIBRARY_WITH_PATH(event_core, $EVENT_LIBDIR, EVENT_SHARED_LIBADD)
	], [
    AC_MSG_ERROR([event_free not found in event_core library, or the library is not installed])
	])

  event_src="php_event.c \
    src/util.c \
    src/fe.c \
    src/pe.c \
    classes/event.c \
    classes/base.c \
    classes/event_config.c \
    classes/buffer_event.c \
    classes/buffer.c \
    classes/buffer_pos.c \
    classes/event_util.c"
  dnl }}}

  dnl {{{ --with-event-pthreads
  if test "$PHP_EVENT_PTHREADS" != "no"; then
	  AC_CHECK_LIB(event_pthreads, evthread_use_pthreads, [
	    PHP_ADD_LIBRARY_WITH_PATH(event_pthreads, $EVENT_LIBDIR, EVENT_SHARED_LIBADD)
      LDFLAGS="-levent_pthreads $LDFLAGS"
      AC_DEFINE(HAVE_EVENT_PTHREADS_LIB, 1, [ ])
	  ], [
      AC_MSG_ERROR([evthread_use_pthreads not found in event_pthreads library, or the library is not installed])
	  ])
  fi
  dnl }}}

  dnl {{{ --with-event-extra
  if test "$PHP_EVENT_EXTRA" != "no"; then
    AC_CHECK_LIB(event_extra, evdns_base_free, [
	    PHP_ADD_LIBRARY_WITH_PATH(event_extra, $EVENT_LIBDIR, EVENT_SHARED_LIBADD)
      LDFLAGS="-levent_extra $LDFLAGS"
      AC_DEFINE(HAVE_EVENT_EXTRA_LIB, 1, [ ])
    ], [
      AC_MSG_ERROR([evdns_base_free not found in event_extra library, or the library is not installed])
    ])

    event_src="$event_src \
      classes/dns.c \
      classes/listener.c \
      classes/http.c \
      classes/http_connection.c"
  fi
  dnl }}}
  
  dnl {{{ --with-event-openssl
  if test "$PHP_EVENT_OPENSSL" != "no"; then
    test -z "$PHP_OPENSSL" && PHP_OPENSSL=no

    if test -z "$PHP_OPENSSL_DIR" || test $PHP_OPENSSL_DIR == "no"; then
      PHP_OPENSSL_DIR=yes
    fi

    PHP_SETUP_OPENSSL(EVENT_SHARED_LIBADD)

    AC_CHECK_LIB(event_openssl, bufferevent_openssl_get_ssl, [
      PHP_ADD_LIBRARY_WITH_PATH(event_openssl, $EVENT_LIBDIR, EVENT_SHARED_LIBADD)
      LDFLAGS="-levent_openssl $LDFLAGS"
      AC_DEFINE(HAVE_EVENT_OPENSSL_LIB, 1, [ ])
    ], [
      AC_MSG_ERROR([bufferevent_openssl_get_ssl not found in event_openssl library, or the library is not installed])
    ])

    event_src="$event_src classes/ssl_context.c"
  fi
  dnl }}}
 
  PHP_NEW_EXTENSION(event, $event_src, $ext_shared,,$CFLAGS)
  PHP_ADD_BUILD_DIR($ext_builddir/src)
  PHP_ADD_BUILD_DIR($ext_builddir/classes)
  PHP_ADD_INCLUDE($ext_builddir/src)
  PHP_ADD_INCLUDE($ext_builddir/classes)
  PHP_ADD_EXTENSION_DEP(event, sockets, true)
  PHP_SUBST(EVENT_SHARED_LIBADD)
  PHP_SUBST(CFLAGS)
  PHP_SUBST(LDLAGS)
fi

dnl vim: ft=m4.sh fdm=marker cms=dnl\ %s
dnl vim: et ts=2 sts=2 sw=2

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

PHP_ARG_WITH(event-extra, for event extra functionality support,
[  --with-event-extra       Include libevent protocol-specific functionality support including HTTP, DNS, and RPC], yes, yes)

PHP_ARG_WITH(event-openssl, for OpenSSL support in event,
[  --with-event-extra       Include libevent OpenSSL support], yes, yes)

PHP_ARG_WITH(openssl-dir, OpenSSL installation prefix,
[  --with-openssl-dir[=DIR]  Event: openssl installation prefix], $PHP_EVENT_CORE, $PHP_EVENT_CORE)

PHP_ARG_WITH([event-libevent-dir], [],
[  --with-event-libevent-dir[=DIR] Event: libevent installation prefix], $PHP_EVENT_CORE, $PHP_EVENT_CORE)

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

  dnl {{{ Include libevent
  AC_MSG_CHECKING([for event2/event.h])
  EVENT_DIR=
  for i in "$PHP_EVENT_CORE" "$PHP_EVENT_LIBEVENT_DIR" /usr/local /usr /opt; do
	  if test -f "$i/include/event.h"; then
		  EVENT_DIR=$i
		  break
	  fi
  done

  if test "x$EVENT_DIR" = "x"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the event library, or provide the installation prefix via --with-event-libevent-dir option])
  fi
	AC_MSG_RESULT([found in $EVENT_DIR])

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
	
	PHP_ADD_INCLUDE($EVENT_DIR/include)
	PHP_ADD_LIBRARY_WITH_PATH(event_core, $EVENT_DIR/$PHP_LIBDIR, EVENT_SHARED_LIBADD)
  LDFLAGS="-L$EVENT_DIR -levent_core $LDFLAGS"
  dnl }}}

  dnl {{{ --enable-event-debug
  if test "$PHP_EVENT_DEBUG" != "no"; then
    CFLAGS="$CFLAGS -Wall -g -ggdb -O0"
    AC_DEFINE(PHP_EVENT_DEBUG, 1, [Enable event debug support])
  else
    AC_DEFINE(NDEBUG, 1, [With NDEBUG defined assert generates no code])
  fi
  dnl }}}
  
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

  dnl {{{ --with-event-extra
  if test "$PHP_EVENT_EXTRA" != "no"; then
	  PHP_ADD_LIBRARY_WITH_PATH(event_extra, $EVENT_DIR/$PHP_LIBDIR, EVENT_SHARED_LIBADD)
    LDFLAGS="-levent_extra $LDFLAGS"
    AC_DEFINE(HAVE_EVENT_EXTRA_LIB, 1, [ ])

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
    PHP_ADD_LIBRARY_WITH_PATH(event_openssl, $EVENT_LIB_DIR, EVENT_SHARED_LIBADD)
    LDFLAGS="-levent_openssl $LDFLAGS"
    AC_DEFINE(HAVE_EVENT_OPENSSL_LIB, 1, [ ])

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
fi

dnl vim: ft=m4.sh fdm=marker cms=dnl\ %s
dnl vim: et ts=2 sts=2 sw=2

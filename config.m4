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
[  --with-event-extra       Include libevent protocol-specific functionality support including HTTP, DNS, and RPC], yes, no)

PHP_ARG_ENABLE(event-debug, for ev debug support,
[  --enable-event-debug     Enable event debug support], no, no)

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

  dnl {{{ include path
  SEARCH_PATH="/usr/local /usr /opt"
  SEARCH_FOR="/include/event2/event.h"

  if test -r $PHP_EVENT_CORE/$SEARCH_FOR; then # path given as parameter
    EVENT_DIR=$PHP_EVENT_CORE
  else # search default path list
    AC_MSG_CHECKING([for event files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        EVENT_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$EVENT_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the event library])
  fi

  PHP_ADD_INCLUDE($EVENT_DIR/include)
  dnl }}}

  dnl {{{ --enable-event-debug
  if test "$PHP_EVENT_DEBUG" != "no"; then
    CFLAGS="$CFLAGS -Wall -g -ggdb -O0"
    AC_DEFINE(PHP_EVENT_DEBUG, 1, [Enable event debug support])
  else
    AC_DEFINE(NDEBUG, 1, [With NDEBUG defined assert generates no code])
  fi
  dnl }}}

  AC_MSG_CHECKING([for directory storing libevent binaries])
  if test -r $EVENT_DIR/$PHP_LIBDIR/libevent_core.$SHLIB_SUFFIX_NAME; then 
    EVENT_LIB_DIR=$EVENT_DIR/$PHP_LIBDIR
    AC_MSG_RESULT(found in $EVENT_LIB_DIR)
  else # Usually FreeBSD and other non-standard setups
    for i in /usr/$PHP_LIBDIR /usr/local/$PHP_LIBDIR /usr/local/$PHP_LIBDIR/event2 /opt/$PHP_LIBDIR; do
      if test -r $i/libevent_core.$SHLIB_SUFFIX_NAME; then
        EVENT_LIB_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  if test -z "$EVENT_LIB_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the event library using a common path prefix])
  fi

  dnl {{{ --with-event-core
  dnl bufferevent_getfd first appeared in 2.0.2-alpha
  PHP_CHECK_LIBRARY(event_core, bufferevent_getfd,
  [
    PHP_ADD_LIBRARY_WITH_PATH(event_core, $EVENT_LIB_DIR, EVENT_SHARED_LIBADD)
    AC_DEFINE(HAVE_EVENT_CORE_LIB,1,[ ])
  ],[
    AC_MSG_ERROR([libevent_core >= 2.0.2-alpha not found])
  ],[
    -L$EVENT_LIB_DIR
  ])
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
    PHP_CHECK_LIBRARY(event_extra, evhttp_new,
    [
      PHP_ADD_LIBRARY_WITH_PATH(event_extra, $EVENT_LIB_DIR, EVENT_SHARED_LIBADD)
      AC_DEFINE(HAVE_EVENT_EXTRA_LIB,1,[ ])
    ],[
      AC_MSG_ERROR([libevent_extra >= 2.0 not found])
    ],[
      -L$EVENT_LIB_DIR -levent_core
    ])

    event_src="$event_src \
      classes/dns.c \
      classes/listener.c \
      classes/http.c \
      classes/http_connection.c"
  fi
  dnl }}}
 
  PHP_NEW_EXTENSION(event, $event_src, $ext_shared,,$CFLAGS)
  PHP_ADD_BUILD_DIR($ext_builddir/src)
  PHP_ADD_BUILD_DIR($ext_builddir/classes)
  PHP_ADD_INCLUDE($ext_builddir/src)
  PHP_ADD_EXTENSION_DEP(event, sockets, true)
  PHP_SUBST(EVENT_SHARED_LIBADD)
  PHP_SUBST(CFLAGS)
fi

dnl vim: ft=m4.sh fdm=marker cms=dnl\ %s
dnl vim: et ts=2 sts=2 sw=2

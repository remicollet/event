Event - PECL extension
======================

Event is a PECL extension providing interface to `libevent` C library.

ABOUT LIBEVENT
--------------

The `libevent` API provides a mechanism to execute a callback function when a
specific event occurs on a file descriptor or after a timeout has been reached.
Furthermore, libevent also support callbacks due to *signals* or regular
*timeouts*.

`libevent` is meant to replace the event loop found in event driven network
servers. An application just needs to call `event_dispatch()` and then add or
remove events dynamically without having to change the event loop.

For details refer to `libevent`'s homepage: <http://libevent.org/>.

For installation instructions see file named <INSTALL.md>.


AUTHOR
======

Ruslan Osmanov <osmanov@php.net>


COPYRIGHT
=========

	Copyright (c) 2013,2014 Ruslan Osmanov <osmanov@php.net>

	This project is subject to version 3.01 of the PHP license, that is bundled
	with this package in the file LICENSE, and is available through the
	world-wide-web at the following url: http://www.php.net/license/3_01.txt If you
	did not receive a copy of the PHP license and are unable to obtain it through
	the world-wide-web, please send a note to license@php.net so we can mail you a
	copy immediately.

vim: ft=markdown ts=4 sts=4 sw=4

#!/bin/bash -
phpize
aclocal && libtoolize --force && autoreconf
./configure --with-event-core --without-event-extra --without-event-openssl --enable-event-debug
make clean
make -j3

#!/bin/bash -
phpize
aclocal && libtoolize --force && autoreconf
./configure --with-event-core --enable-event-debug
make clean
make -j3

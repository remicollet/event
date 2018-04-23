#!/bin/bash -
set -x
phpize
aclocal && libtoolize --force && autoreconf
./configure --with-event-core --with-event-extra --with-event-openssl --enable-event-debug "$@"
make clean
make -j3

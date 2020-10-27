#!/bin/bash -
set -x
phpize --clean
phpize
#The following may be useful on Gentoo
#aclocal && libtoolize --force && autoreconf
#./configure --with-event-core --with-event-extra --with-event-openssl --with-event-ns='SomeNs' --enable-event-debug "$@"
./configure --with-event-core --with-event-extra --with-event-openssl --enable-event-debug "$@"
make clean
make -j3

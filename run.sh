#!/bin/bash -
sockets_so_path="$(php-config --extension-dir)/sockets.so"
sockets_option=
if [ -e "$sockets_so_path" ]; then
  cp "$sockets_so_path" ./.libs/
  sockets_option='-dextension=sockets.so'
fi

posix_so_path="$(php-config --extension-dir)/posix.so"
posix_option=
if [ -e "$posix_so_path" ]; then
  cp "$posix_so_path" ./.libs/
  posix_option='-dextension=posix.so'
fi

MALLOC_PERTURB_=$(($RANDOM % 255 + 1)) \
	MALLOC_CHECK_=3 \
	USE_ZEND_ALLOC=0 \
	ZEND_DONT_UNLOAD_MODULES=1 \
	php -n $posix_option $sockets_option -dextension=event.so  -dextension_dir=./.libs "$@"

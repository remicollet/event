#!/bin/bash -

sockets_so_path="$(php-config --extension-dir)/sockets.so"
if [ -e "$sockets_so_path" ]; then
  cp "$sockets_so_path" ./.libs/
fi

MALLOC_PERTURB_=$(($RANDOM % 255 + 1)) \
	MALLOC_CHECK_=3 \
	USE_ZEND_ALLOC=0 \
	ZEND_DONT_UNLOAD_MODULES=1 \
	php -n -dextension=sockets.so -dextension=event.so  -dextension_dir=./.libs "$@"

#!/bin/bash -
USE_ZEND_ALLOC=0 valgrind --leak-check=full php -n -d extension=event.so  -dextension_dir=./.libs "$@"

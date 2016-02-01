#!/bin/bash -
php -n -d extension=event.so  -dextension_dir=./.libs "$@"

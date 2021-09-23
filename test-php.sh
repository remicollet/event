#!/bin/sh -
# Can be used to run tests when `make test` fails because of the extension loading order issues.
# E.g.: TEST_PHP_EXECUTABLE="$(pwd)/run-tests.sh" php run-tests.php

dir=$(cd $(dirname "$0"); pwd)
local_libs_dir="${dir}/.libs"

if ! test -e "${local_libs_dir}/sockets.so"; then
    sockets_so_path="$(php-config --extension-dir)/sockets.so"
    cp "$sockets_so_path" "${local_libs_dir}"
fi
sockets_option='-dextension=sockets.so'

php -n $sockets_option -dextension=event.so  -dextension_dir="${local_libs_dir}" "$@"

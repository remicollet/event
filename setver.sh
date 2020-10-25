#!/bin/bash -
# Set PECL package version

if [[ $# < 1 ]]; then
	echo >&2 'Version string expected'
	exit 1
fi

php_event_version="$1"

for subdir in php5 php7 php8
do
	sed -i -r 's/(# *define *PHP_EVENT_VERSION\ +").*"/\1'$php_event_version'"/g' $subdir/php_event.h
done

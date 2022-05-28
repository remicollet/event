#!/bin/bash -
# Set PECL package version

set -e

if [[ $# < 1 ]]; then
	echo >&2 'Version string expected'
	exit 1
fi

php_event_version="$1"

perl -pi -e 's/(# *define *PHP_EVENT_VERSION\ +).*/$1"'$php_event_version'"/g' php{5,7,8}/php_event.h

echo "Done"
echo "(!) Don't forget to update package.xml"

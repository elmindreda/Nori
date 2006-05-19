#!/bin/sh
#
# This script transforms the autotools-generated config.h into a well-behaved
# configuration header, which may be safely installed on a system.
#

if [ ! -f "$1" -o -z "$2" ]; then
  echo "Invalid user"
  exit 1
fi

sed -e 's/PACKAGE_/WENDY_PACKAGE_/;s/HAVE_/WENDY_HAVE_/' "$1" > "$2"


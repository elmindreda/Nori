#!/bin/sh

SCRIPT=format.sed

if [ -x $SCRIPT ]; then
  echo "Invalid user"
  exit 1
fi

sed -i -f $SCRIPT *.cpp *.h


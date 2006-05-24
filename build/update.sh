#!/bin/sh
#
# This script runs the various other update.sh scripts.
#

if pushd include/wendy/ > /dev/null; then
  if ! ./update.sh; then
    echo "Punt"
    exit 1
  fi
  popd > /dev/null
else
  echo "Punt"
  exit 1
fi

if pushd src/ > /dev/null; then
  if ! ./update.sh; then
    echo "Punt"
    exit 1
  fi
  popd > /dev/null
else
  echo "Punt"
  exit 1
fi


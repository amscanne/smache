#!/bin/bash

#
# This a simple script to aim in testing and debugging.
# It basically zips up the build contents and prints the
# hashes of the original files and the extracted ones.
#
set -e

export LD_LIBRARY_PATH=/home/amscanne/bio/smache/lib/:$LD_LIBRARY_PATH
export PYTHONPATH=/home/amscanne/bio/smache/utils/build/lib.linux-x86_64-2.5/:$PYTHONPATH
#DEBUG="gdb --args python2.5"
#DEBUG="valgrind --leak-check=full"

#
# Remake things.
#
(cd utils && make)
(cd lib && make)

#
# Build the test config.
#
cat >/tmp/sanity.conf <<EOF
[smache]
index = /tmp/sanity.index
compression = lzo
block = rabin
debug = 1

[berkeleydb]
filename = /tmp/sanity.db
debug    = 1
EOF

#
# Remove the old files.
#
rm -f /tmp/sanity.db
rm -f /tmp/sanity.index

#
# Build the reference set.
#
CUR=`pwd`
cd utils/build/lib.linux-x86_64-2.5
DIR=`pwd`

if "x$DEBUG" == "x"; then
    ($DIR/../scripts-2.5/smachezip a /tmp/sanity.conf . 2> $CUR/compress.stderr) &
    PID=$!
    (sleep 5; kill $PID 2>/dev/null) &
    PID2=$!
    wait $PID
    kill $PID2 2>/dev/null
else
    $DEBUG $DIR/../scripts-2.5/smachezip a /tmp/sanity.conf .
fi

echo "Reference:"
for file in `find . | sort`; do 
    [ -f $file ] && md5sum $file
done

#
# Extract it to the test set.
#
mkdir -p /tmp/sanity-check
cd /tmp/sanity-check

if "x$DEBUG" == "x"; then
    ($DIR/../scripts-2.5/smachezip x /tmp/sanity.conf 2> $CUR/uncompress.stderr) &
    PID=$!
    (sleep 5; kill $PID 2>/dev/null) &
    PID2=$!
    wait $PID
    kill $PID2 2>/dev/null
else
    $DEBUG $DIR/../scripts-2.5/smachezip x /tmp/sanity.conf
fi

echo "Test:"
for file in `find . | sort`; do
    [ -f $file ] && md5sum $file
done

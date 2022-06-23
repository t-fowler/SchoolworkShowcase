#!/bin/bash

set -x

FILE=hello_out.txt
FRAMESIZE=12
NUMFRAMES=100
BIN=./virtmem

if [ $# -eq 0 ]; then
    echo "usage: ./test5.sh [fifo|lru|secondchance|optimal]"
    exit 1
fi

ALGORITHM=$1

$BIN --file=$FILE --framesize=$FRAMESIZE \
    --numframes=$NUMFRAMES --replace=$ALGORITHM --progress

#!/bin/bash

FILE=3.txt
FRAMESIZE=9
NUMFRAMES=6
BIN=./virtmem
ALGORITHM=lru

echo "LRU: Should be a single swapout, 7 pagefaults, 8 memory refs"
echo

ALGORITHM=fifo
$BIN --file=$FILE --framesize=$FRAMESIZE \
    --numframes=$NUMFRAMES --replace=$ALGORITHM --progress

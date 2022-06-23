#!/bin/bash

FILE=2.txt
FRAMESIZE=9
NUMFRAMES=6
BIN=./virtmem
ALGORITHM=fifo

echo "FIFO: Should be a single swapout, 8 pagefaults, 9 memory refs"
echo

ALGORITHM=fifo
$BIN --file=$FILE --framesize=$FRAMESIZE \
    --numframes=$NUMFRAMES --replace=$ALGORITHM --progress

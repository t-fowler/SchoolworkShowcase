#!/bin/bash

FILE=4.txt
FRAMESIZE=9
NUMFRAMES=6
BIN=./virtmem
ALGORITHM=secondchance

echo "SECONDCHANCE: Should be a two swapouts, 8 pagefaults, 9 memory refs"
echo

ALGORITHM=fifo
$BIN --file=$FILE --framesize=$FRAMESIZE \
    --numframes=$NUMFRAMES --replace=$ALGORITHM --progress

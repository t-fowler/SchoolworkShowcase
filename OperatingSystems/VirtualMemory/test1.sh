#!/bin/bash

FILE=1.txt
FRAMESIZE=9
NUMFRAMES=6
BIN=./virtmem
ALGORITHM=fifo

echo "Sanity check for FIFO..."
ALGORITHM=fifo
$BIN --file=$FILE --framesize=$FRAMESIZE \
    --numframes=$NUMFRAMES --replace=$ALGORITHM --progress

echo
echo "Sanity check for LRU..."
ALGORITHM=lru
$BIN --file=$FILE --framesize=$FRAMESIZE \
    --numframes=$NUMFRAMES --replace=$ALGORITHM --progress

echo
echo "Sanity check for SECONDCHANCE..."
ALGORITHM=secondchance
$BIN --file=$FILE --framesize=$FRAMESIZE \
    --numframes=$NUMFRAMES --replace=$ALGORITHM --progress

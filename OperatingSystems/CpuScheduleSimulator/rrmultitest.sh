#!/bin/bash

mkdir -p sim

SEED=(52565 25565 55265 52655 25655 65255 56255 26555 62555 65525 56525 55625 55256 25556 52556 55526 56552 65552 55652 55562)

for i in $(seq 20)
do
./rrsim.sh ${SEED[${i} - 1]} ${i} &
done

wait

output=$(python3 rrsimstats.py)
echo "${output}"

./reset.sh

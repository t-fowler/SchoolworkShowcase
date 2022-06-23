#!/bin/bash

./simgen 1000 "$1" | ./rrsim --quantum 50 --dispatch 0 > "./sim/q50d0-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 50 --dispatch 5 > "./sim/q50d5-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 50 --dispatch 10 > "./sim/q50d10-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 50 --dispatch 15 > "./sim/q50d15-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 50 --dispatch 20 > "./sim/q50d20-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 50 --dispatch 25 > "./sim/q50d25-${2}.txt"

./simgen 1000 "$1" | ./rrsim --quantum 100 --dispatch 0 > "./sim/q100d0-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 100 --dispatch 5 > "./sim/q100d5-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 100 --dispatch 10 > "./sim/q100d10-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 100 --dispatch 15 > "./sim/q100d15-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 100 --dispatch 20 > "./sim/q100d20-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 100 --dispatch 25 > "./sim/q100d25-${2}.txt"

./simgen 1000 "$1" | ./rrsim --quantum 250 --dispatch 0 > "./sim/q250d0-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 250 --dispatch 5 > "./sim/q250d5-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 250 --dispatch 10 > "./sim/q250d10-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 250 --dispatch 15 > "./sim/q250d15-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 250 --dispatch 20 > "./sim/q250d20-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 250 --dispatch 25 > "./sim/q250d25-${2}.txt"

./simgen 1000 "$1" | ./rrsim --quantum 500 --dispatch 0 > "./sim/q500d0-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 500 --dispatch 5 > "./sim/q500d5-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 500 --dispatch 10 > "./sim/q500d10-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 500 --dispatch 15 > "./sim/q500d15-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 500 --dispatch 20 > "./sim/q500d20-${2}.txt"
./simgen 1000 "$1" | ./rrsim --quantum 500 --dispatch 25 > "./sim/q500d25-${2}.txt"
# CSC 360 Assignment 3
Author: Tyler Fowler
ID: V00752565

## Task 1
For task 1, I have simulated a round robin scheduler. It keeps track of cpu time
via the time_tick function which adds arriving tasks to the ready_q the moment time
is ticked forward. It will accept and simulate provided dispatch costs and quantum
lengths. 

## Task 2
For task 2, I used 2 bash scripts to run the tests, a python program to parse the data
and an r script for plotting the graphs. There is also a 3rd bash script that I have
included called reset.sh. This just deletes the files created by rrsim.sh since they
add up to a whopping 11gb of storage space.

The first bash script (rrsim.sh) simulates a run for a given seed with 1000 tasks and
all 24 combinations of the required quantum lengths and dispatch costs. The second
bash script (rrmultitest.sh) runs rrsim.sh 20 times with different seeds. As such,
480 files are created in a directory called sim. rrmultitest.sh also calls the python
program (rrsimstats.py) which checks each of the 480 files for exit information for
each task, calculating the average waiting times and turnaround times for each
quantum/dispatch combination.

I then hard coded the results into rrplot.r which plotted the data points onto the
appropriate graphs which I used rstudio to export as pdf.
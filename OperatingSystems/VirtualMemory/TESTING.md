# Test cases 


## Quick check on which required page-replacement algorithms have been
   implemented.

## Note on memory traces

If you look at the contents of the trace files, and compute the offset and page
numbers given the framesize used by the test script, then you'll notice that
the pages follow a specific order.

* Sanity trace: six reads in different frames, six swapins (test 1)
    * TRACE = "1R 2R 3R 4R 5R 6R" 

* FIFO trace: Should be a single swapout (test 2)
    * That is, victim frame at 7R should have had 1W in it. 
    * TRACE = "1W 2R 3R 4R 5R 6R 1R 7R 1R" 

* LRU trace: Should be a single swapout (test 3)
    * That is, victim frame at 7R shuld have had 2W in it. 
    * TRACE = "1R 2W 3W 4W 5W 6W 1R 7R" 

* SECONDCHANCE trace: Should be two swapouts (test 4)
    * That is, victim frame at 7R should have 1W in it, and victim
      frame at 8R should have 3W in it. 
    * TRACE = "1W 2R 3W 4R 5R 6R 7R 2R 8R" 


### Test 1
    * Purpose: Sanity check -- no page replacement, check that FIFO, LRU,
      and SECONDCHANCE are implemented.
    * Memory-trace data: `1.txt`
    * Script: `test1.sh`


## Check each of FIFO, LRU, and SECONDCHANCE

### Test 2
    * Purpose: Simple check for FIFO behavior.
    * Memory-trace data: `2.txt`
    * Script: `test2.sh`

### Test 3
    * Purpose: Simple check for LRU behavior.
    * Memory-trace data: `3.txt`
    * Script: `test3.sh`

### Test 4
    * Purpose: Simple check for SECONDCHANCE behavior.
    * Memory-trace data: `4.txt`
    * Script: `test4.sh`


## Use given traces with submitted code.

### Test 5
    * Purpose: Run submitted code with `hello_out.txt`
    * Script expects the page-replacement algorithm to be specified
      (as `fifo` or `lru` or `secondchance`).
    * Memory-trace data: `hello_out.txt`
    * Script: `test5.sh`

### Test 6
    * Purpose: Run submitted code with `ls_out.txt`
    * Script expects the page-replacement algorithm to be specified
      (as `fifo` or `lru` or `secondchance`).
    * Memory-trace data: `ls_out.txt`
    * Script: `test6.sh`

### Test 7
    * Purpose: Run submitted code with `matrixmult_out.txt`
    * Script expects the page-replacement algorithm to be specified
      (as `fifo` or `lru` or `secondchance`).
    * Memory-trace data: `matrixmult_out.txt`
    * Script: `test7.sh`

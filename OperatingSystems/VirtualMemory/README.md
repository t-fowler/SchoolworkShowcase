# CSC 360 Assignment 4
Author: Tyler Fowler
ID: V00752565

## Replacemnt Policies
Here is a list of the replacement policies that I have implemented
in this program. I used sys/queues.h to implement the queues for the
page table entries. The queues are then used to determine which frame
will be replaced when required by a page fault.

### FIFO
FIFO replacement is implemented with a singly linked tail queue. Page table
entries are added to the tail of the queue when they arrive, while the head
of the queue is determined to be the victim frame when required by a page
fault.

### LRU
LRU replacement is implemented with a doubly linked tail queue. I chose to
use the doubly linked queue for this replacement scheme for the O(1) remove
macro when removing elements that are not at the head of the queue. This is
because, for my LRU implementation, table entries are added to the tail of
the queue. Then, whenever a frame is accesed, its entry is removed from the
queue and replaced on the tail. This ensures that the head of the queue is
always the least recently used. Therefore, when a frame needs to be replaced,
the head is removed and the new entry is swapped in.

### Secondchance
Secondchance replacement is implemented with the same singly linked tail
queue used for the FIFO scheme. Reference bits are then maintained for the
entries allowing the secondchance replacement policy to check the head of
the queue to see if it has been referenced, then continually move the head
to the back until an unreferenced entry has been found. The reference bit
is changed to zero after the replacement policy checks it so as to ensure
that the policy will always find a victim frame.
# Virtual-Memory-Management

## 1. Problem
You will need to write a simulator that reads a memory trace and simulates the action of a virtual memory system with a single level page table. The system needs to simulate swapping to disk and use a specified algorithm for page replacement.

* Your simulator should keep track of the status of the pages, including which page frames would be loaded in physical memory and which pages reside on disk.\
The size of a page (in bytes) will be passed as the 2nd argument on the command line (See Running your code below for details).\
The number of page frames that can be held in main memory will be passed as the 3rd argument on the command line (See Running your code below for details).

* As the simulator processes each memory event from the trace:
1. It should check to see if the corresponding page is in physical memory (hit), or whether it needs to be swapped in from disk (pagefault).
2. If a pagefault has occurred, your simulator may need to choose a page to remove from physical memory. When choosing a page to replace, your simulator should use the algorithm specified by the 4th argument passed on the command line (See Running your code below for details).
3. If the page to be replaced is dirty, it would need to be saved back to the swap disk. Your simulator
should track when this occurs.
4. Finally, the new page is to be loaded into memory from disk, and the page table is updated.



### Part 1 - Second Chance Algorithm
The least recently used (LRU) page replacement algorithm consistently provides near-optimal replacement, however implementing true LRU can be expensive. A simple way of approxuimating LRU is the Second Chance (SC) algorithm, which gives recently referenced pages a second chance before replacement.\
Your task is to set up your simulator to use the Second Chance algorithm.
Your simulator should use the Second Chance page replacement algorithm if the 4th argument passed on the command line is set to SC (See Running your code below for details).

### Part 2 - Enhanced Second Chance Algorithm
The Second Chance algorithm is simple, but many systems will try to improve on this using an algorithm that takes into account the added delay of writing a modified page to disk during replacement. The Enhanced Second Chance (ESC) algorithm tracks whether a page has been modified or not and uses that information as well as whether the page was recently referenced to choose which page to replace.\
Update your simulator to use the Enhanced Second Chance algorithm.\
Your simulator should use the Enhanced Second Chance page replacement algorithm if the 4th argument passed on the command line is set to ESC (See Running your code below for details).

### Part 3 - Additional Reference Bits Algorithm
A closer approximation of LRU than the second chance algortihm is the Additional Reference Bits (ARB) algorithm, which uses multiple bits to keep track of page access history. These bits are stored in a shift register that regularly shifts right, removing the oldest bit.\
Your task is to update your simulator to use the Additional Reference Bits algorithm.\
Your simulator should use the Additional Reference Bits page replacement algorithm if the 4th argument passed on the command line is set to ARB (See Running your code below for details).\
Your implementation should use an 8-bit shift register, that shifts to the right.\
Bit shifting (right) should occur at a regular interval specified by the 5th argument passed on the command line (See Running your code below for details).\
For example, if the interval provided is 5, then bit shifting should occur for all pages every 5th memory access.

### Part 4 - Enhanced Additional Reference Bits Algorithm
Let’s combine the best aspects of ESC and ARB to create a new algorithm, Enhanced ARB (EARB). This algorithm will combine the improved access history of ARB with the awareness of modified pages from ESC.\
Your task is to update your simulator to use the Enhanced ARB algorithm described below.\
Your simulator should use the Enhanced Additional Reference Bits page replacement algorithm if the 4th argument passed on the command line is set to EARB (See Running your code below for details).

This algorithm works as follows:\
If no pages are modified, or if only modified pages are resident, the algorithm should perform the same as ARB.\
If both modified and unmodified pages are resident:\
We want to avoid replacing the modified page unless it is several intervals older than the non modified page, so A modified page should only be replaced if there does not exist a non-modified page that has been referenced within 3 intervals of the modified page


## 2. Running 

for compiling it, need to tpye in the terminal, and run "memsim.exe". 

```
gcc -std=c11 *.c -o memsim
```

you can run like this;

```
./memsim input_file.trace PAGESIZE NUMPAGES ALGORITHM INTERVAL
```

For example, your code might be run like this:

```
./memsim test.trace 4096 32 EARB 4
```
where:
* The program being run is ./memsim
* the name of the input file is test.trace
* a page is 4096 bytes,
* there are 32 frames in physical memory,
* the Enhanced ARB algortihm ( EARB ) is used for page replacement,
* and the (E)ARB shift register shifts every 4 memory accesses.

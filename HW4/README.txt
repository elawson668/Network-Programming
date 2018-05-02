README.txt for HOMEWORK 4
AUTHORS: MATTHEW MOHR AND ERIC LAWSON
Included files:
mpi_primes.c
SPEEDUP GRAPH AND ANALYSSI FOR HW4.pdf
README.txt
Makefile


Code Description: This code is designed to compute the number of unqiue primes in parallel up to the maximum 32bit integer in C or until it receives the SIGUSR1 signal from an external process.
The code is designed to split prime computation between 1, 2, and 4 MPI ranks. Prime computations are performed using the following algorithm (also included in the PDF for convenience:

-Have the n value for a particular rank start from its rank number + 2 (e.g. Rank 0 would start at 2, rank 1 would start at 3, Rank 2 would start at 4, and rank 3 would start at 5).
-For the n value, perform the prime computation loop (i.e divide n one at a time by the numbers ranging from 2 to sqrt(n)  until you either determine that n is prime or composite).
-Perform 2 MPI_Gather, one after the other, calls to make 2 arrays whose size are equal to the total number of ranks. The first MPI_Gather call collects an array of the values of n for each rank in order (such as [2,3,4,5]), and the second array contains a series of 1s and 0s indicating whether or not the numbers in the previous array are prime or not. Traverse both arrays to compute the number of primes found this round and what number we are currently up to for n. If you hit the limit, which is a power of 10, while going through the arrays, print the aggregate results obtained up to that limit. 
-Perform an AllReduce() call after computing the necessary information from gather to check if any process got the signal. If any process got the signal, exit the prime loop and return the final results.
-If we keep going, increase the value of n for the current rank by the total number of ranks (so an n value of 2 with 4 ranks means that the next number we check would be 6, then 10, then 14, etc.). Since all ranks start at a different number and the n value is a constant offset, we can guarantee that no two ranks will ever work on the same number (i.e. starting from n, n+1, n+2, and n+3 for four ranks, increasing each by the same constant will still result in a different number).



ISSUES ENCOUNTERED: Our group did not see speedup using multiple ranks, but this was determined to be the result of using 3 synchronous calls per prime computation iteratio (see analysis). We tried to improve performance by altering the number of synchronouscalls or altering how often they were tirggered, but this often ended up in a race condition. After consulting with the instructor, we were informed that submitting a working version with no deadlocks and a sufficient explanation for slowdown would be sufficient. 

Appoximate time spent: 3-5 hours a day for the course of a weke, mainly just to get the algorithm working and not result in deadlock. Also needed time to come up with our accuracy optimized alogortihm, and spent 2 hours during last instructor's offie hours discussing results of what we had.

Work Breaksdown:

MAtt: Came up with intial algorithm and implemented it, wrote analysis for speedup graph, documented the README and .pdf
Eric: Resolved Matt's deadlock conditions after initial implemenation, was instrumental in discussing with instructor during office hours, did runs for the speedup data, commented code 

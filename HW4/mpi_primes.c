#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>
 
int end_now = 0;

// Handle timeout signal, set end_now to 1 to notify processes to stop
void sig_handler(int signo)
{
    if (signo == SIGUSR1) {
        end_now = 1;
    }
}

int main(int argc, char **argv)
{
    int count, id;
    int total_primes = 0;
    int total_end = 0;
   
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &count);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    signal(SIGUSR1, sig_handler);

    int n = id + 2;
    int limit = 10;
    int narray[count];
    int flags[count];	

    if (id == 0) {printf("           N	       Primes\n");}

    while (n < 2147483647) { //max 32 bit int

		int i;
		int flag = 0;

		// Determine if n is a prime number, set flag to 1 if it is
		int x = (int) sqrt(n);
		for (i = 2; i <= x; i++)
		{	
			if (n % i != 0) {continue;}
			else {flag = 1; break;}
		}
	
	
		int sendn = n;
		int sendflag = flag;	

		// Gather the n and flag values from each rank, insert into narray and flags
		MPI_Gather(&sendn, 1, MPI_INT, narray, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Gather(&sendflag, 1, MPI_INT, flags, 1, MPI_INT, 0, MPI_COMM_WORLD);

		// Reduce the endnow values from each rank, find the max value. Max value would be 1, meaning all processes should stop
		MPI_Allreduce(&end_now, &total_end, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	
		// If the allreduce finds an end_now value of 1, stop the process
		if (total_end == 1)
		{
			break;
		}

		int j;

		// If on base rank, 
		if (id == 0)
		{
			// Iterate through narray and flags
			for (j = 0; j < count; j++)
			{
				// If prime number, increment total primes
				if(flags[j] == 0) {total_primes++;}

				// If current n is at the current limit, output info, then multiply limit by 10
				if (narray[j] == limit) { printf("%12d\t%12d\n", limit, total_primes); limit = limit * 10;}

			}

		}

		// Each rank increments n by count(the number of ranks) each iteration. With 4 ranks, rank 0 handles 1,5,9..., rank 1 handles 2,6,10...., etc.
		n = n + count;
	
    }
    
    // Output final info
    if (id == 0) {printf("<Signal received>\n"); printf("%12d\t%12d\n", n, total_primes);}

    MPI_Finalize();
	
    
    return 0;
}

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>

int end_now = 0;

void sig_handler(int signo)
{
    if (signo == SIGUSR1) {
        end_now = 1;
    }
}

int main(int argc, char **argv)
{
    int count, id;
    int local_primes=0;
    int total_primes=1;
    int total_end = 0;
    int total_n;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &count);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    signal(SIGUSR1, sig_handler);
    int n = id + 2;
    //while (n < 2147483647) {
    while (n < 100){


	//MPI_Allreduce(&end_now, &total_end, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	
	if (end_now == 1)
	{
		break;
	}

	int i;
	int flag=0;
	int x =  (int) sqrt(n);
	for (i=2; i <= x; i++)
	{	
		if (end_now ==1) {flag=1; break;}
		if (n%i != 0) {continue;}
		else {flag=1; break;}
	}
	
	

	if (flag == 0) {local_primes++;}	
	n= n + count;
	
    }
    
    MPI_Allreduce(&n, &total_n, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&local_primes, &total_primes, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (id ==0) {printf("total_primes = %d\n", total_primes); printf("total_n = %d\n", total_n); }
    //printf("RANK %d got to %d\n", id, n);
    MPI_Finalize();
	
    
    return 0;
}

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
    int total_primes=0;
    int total_end = 0;
   

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &count);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    signal(SIGUSR1, sig_handler);
    int n = id + 2;
    int limit=10;
    int narray[count];
    int flags[count];	
    if (id ==0) {printf("           N	       Primes\n");}
    while (n < 2147483647) {
    //while (n < 10){


	MPI_Allreduce(&end_now, &total_end, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	
	if (total_end == 1)
	{
		break;
	}

	int i;
	int flag=0;
	int x =  (int) sqrt(n);
	for (i=2; i <= x; i++)
	{	
		//if (end_now ==1) {flag=1; break;}
		if (n%i != 0) {continue;}
		else {flag=1; break;}
	}
	
	
	int sendn=n;
	int sendflag=flag;	

	MPI_Gather(&sendn, 1, MPI_INT, narray, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(&sendflag, 1, MPI_INT, flags, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int j;
	if (id==0)
	{
	for (j=0; j < count; j++)
	{
		if(flags[j] == 0) {total_primes++;}
		if (narray[j] == limit) { printf("%12d\t%12d\n", limit, total_primes); limit = limit*10;}

	}
	
	

	}

	n= n + count;
	
    }
    
    //MPI_Allreduce(&n, &total_n, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    //MPI_Allreduce(&local_primes, &total_primes, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (id ==0) {printf("<Signal received>\n"); printf("%12d\t%12d\n", n, total_primes);}
    //printf("RANK %d got to %d\n", id, n);
    MPI_Finalize();
	
    
    return 0;
}

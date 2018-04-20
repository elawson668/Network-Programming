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
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &count);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    signal(SIGUSR1, sig_handler);
    int n = id+1;
    while (n < 10) {
        if (end_now == 1) {
            break;
        }
	
	int i;
	int flag=0;
	int prime=0;
	int x =  sqrt(n);
	for (i=2; i < x; i++)
	{
	if (n%i == 0)
		{flag = 1;
		break;}
		 
	}

	if (flag == 0) {prime = 1;}
	if (prime == 1) {printf("RANK%d %d\n", id, n);}
	n= n + count;
	
    }
    
    MPI_Finalize();
    
    return 0;
}

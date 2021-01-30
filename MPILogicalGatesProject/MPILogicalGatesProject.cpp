#include <iostream>
#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

using namespace std;

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        char helloMsg[] = "Hello World";
        MPI_Send(helloMsg, _countof(helloMsg), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    }
    else if (rank == 1)
    {
        char helloMsg[12];
        MPI_Recv(helloMsg, _countof(helloMsg), MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "Rank 1 received this: " << helloMsg;
    }

    MPI_Finalize();
    return 0;
}
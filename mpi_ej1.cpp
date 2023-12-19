// Created by gitpod on 12/19/23.

#include <mpi.h>
#include <stdio.h>  // Incluimos la biblioteca para printf

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // Utilizamos printf para imprimir el mensaje
    printf("Rank %d of %d processes\n", rank, nprocs);

    MPI_Finalize();

    return 0;
}
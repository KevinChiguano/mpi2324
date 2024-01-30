#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>

#define  MAX_ITEM 14


//tmp = tmp + A[i*cols+j]*b[j];

std::vector<int> multiplicarMatriz(int* A, int* B, int rows, int cols) {
    std::vector<int> res(rows * cols, 0);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int tmp = 0;
            for (int k = 0; k < cols; ++k) {
                tmp += A[i * cols + k] * B[k * cols + j];  // Corregir índices
            }
            res[i * cols + j] = tmp;  // Corregir índices
        }
    }

    std::printf("res tam: %ld \n", res.size());
    return res;
}


int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> A;
    std::vector<int> B;
    std::vector<int> C;

    std::vector<int> A_local;

    int row_per_rank, padding = 0, row_size = MAX_ITEM;

    if(row_size % nprocs != 0){
        row_size = std::ceil((double) MAX_ITEM/nprocs) * nprocs;
        padding = row_size - MAX_ITEM;
    }

    row_per_rank = row_size / nprocs;

    A_local.resize(row_per_rank*MAX_ITEM);
    B.resize(MAX_ITEM*MAX_ITEM);



    if(rank == 0){
        A.resize(MAX_ITEM*MAX_ITEM);
        B.resize(MAX_ITEM*MAX_ITEM);
        C.resize(row_size*MAX_ITEM);

        for (int i = 0; i < MAX_ITEM; i++) {
            for (int j = 0; j < MAX_ITEM; j++) {
                int index = i * MAX_ITEM + j;
                A[index] = 1;
                B[index] = 2;
            }
        }
    }

    MPI_Bcast(B.data(), MAX_ITEM*MAX_ITEM, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Scatter(A.data(), row_per_rank*MAX_ITEM, MPI_INT,
                A_local.data(), row_per_rank*MAX_ITEM, MPI_INT,
                0, MPI_COMM_WORLD);


    int row_per_rank_tmp = row_per_rank;

    if(rank == nprocs -1){
        row_per_rank_tmp = MAX_ITEM - rank * row_per_rank;
    }

    std::vector<int> res_local(row_per_rank_tmp * MAX_ITEM);

    res_local = multiplicarMatriz(A_local.data(), B.data(), row_per_rank_tmp, MAX_ITEM);

    std::printf("res tam segundo: %ld \n", res_local.size());


    MPI_Gather(res_local.data(), row_per_rank_tmp * MAX_ITEM, MPI_INT,
               C.data(), row_per_rank_tmp * MAX_ITEM, MPI_INT,
               0, MPI_COMM_WORLD);

    if(rank == 0){

        for (int i = 0; i < MAX_ITEM; i++) {
            for (int j = 0; j < MAX_ITEM; j++) {
                int index = i * MAX_ITEM + j;
                std::printf(" %d ", C[index]);
            }
            std::printf("\n");
        }
    }

    MPI_Finalize();

    return 0;
}
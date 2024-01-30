#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>

#define MAX_ITEM 100

void axpy(int alpha, int size,int* x, int* y, int* res){

    for (int i = 0; i < size; ++i) {
        res[i] = alpha*x[i] + y[i];
    }

}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size , size = MAX_ITEM, padding = 0;

    if(MAX_ITEM % nprocs != 0){
        size = std::ceil((double) MAX_ITEM/nprocs) * nprocs;
        padding = size - MAX_ITEM;
    }

    block_size = size / nprocs;

    std::printf("size: %d, block_size: %d, padding: %d \n", size, block_size, padding);

    std::vector<int> x;
    std::vector<int> y;
    std::vector<int> res;
    int alpha;

    std::vector<int> x_local;
    std::vector<int> y_local;
    std::vector<int> res_local;

    if(rank == 0){
        x.resize(MAX_ITEM);
        y.resize(MAX_ITEM);
        res.resize(MAX_ITEM);

        for (int i = 0; i < MAX_ITEM; ++i) {
            x[i] = i;
            y[i] = i+i;
        }

        alpha = 5;
    }

    MPI_Bcast(&alpha, 1, MPI_INT, 0, MPI_COMM_WORLD);



    x_local.resize(block_size);
    y_local.resize(block_size);
    res_local.resize(block_size);

    MPI_Scatter(x.data(), block_size, MPI_INT,
            x_local.data(), block_size, MPI_INT,
            0, MPI_COMM_WORLD);

    MPI_Scatter(y.data(), block_size, MPI_INT,
                y_local.data(), block_size, MPI_INT,
                0, MPI_COMM_WORLD);

    if(rank == nprocs-1){
        block_size = block_size - padding;
    }

    axpy(alpha, block_size,x_local.data(), y_local.data(), res_local.data());


    MPI_Gather(res_local.data(), block_size, MPI_INT,
               res.data(), block_size, MPI_INT,
               0, MPI_COMM_WORLD);

    if(rank == 0){
        std::printf("OPERACION axpy \n");
        for (int i = 0; i < res.size(); ++i) {
            std::printf("%d, ", res[i]);
        }
    }

    MPI_Finalize();

    return 0;
}
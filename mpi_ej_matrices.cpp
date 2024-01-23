#include <iostream>
#include <vector>
#include <mpi.h>
#include <stdio.h>
#include <cmath>

#define MATRIX_DIMENSION 25

void matrix_mult(double* A, double* b, double* c, int rows, int cols){
    for(int i = 0; i < rows; i++){
        double tmp = 0;
        for(int j = 0; j < cols; j++){
            tmp = tmp + A[i*cols+j]*b[j];
        }
        c[i] = tmp;
    }
}


int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int rows_per_rank;
    int rows_alloc = MATRIX_DIMENSION;
    int padding = 0;

    if(MATRIX_DIMENSION%nprocs != 0){
        rows_alloc = std::ceil((double) MATRIX_DIMENSION/nprocs) * nprocs;
        padding = rows_alloc - MATRIX_DIMENSION;
    }

    //la division siempre es exacta
    rows_per_rank = rows_alloc / nprocs;

    if(rank == 0){
        std::printf("Dimension: %d, rows_alloc: %d, rows_per_rank: %d, padding: %d\n",
                    MATRIX_DIMENSION, rows_alloc, rows_per_rank, padding);

        std::vector<double> A(MATRIX_DIMENSION*rows_alloc);
        std::vector<double> b(MATRIX_DIMENSION);
        std::vector<double> c(rows_alloc);

        for(int i = 0; i < MATRIX_DIMENSION; i++){
            for(int j = 0; j < MATRIX_DIMENSION; j++){
                int index = i * MATRIX_DIMENSION + j;
                A[index] = i;
            }

        }
        for(int i = 0; i < MATRIX_DIMENSION; i++) b[i] = 1;

        //enviar la matriz A
        MPI_Scatter(A.data(), MATRIX_DIMENSION*rows_per_rank, MPI_DOUBLE,
                    MPI_IN_PLACE, 0, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);

        //enviar el vector b
        MPI_Bcast(b.data(), MATRIX_DIMENSION, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        //realizar el calculo: c = A x b
        matrix_mult(A.data(), b.data(), c.data(), rows_per_rank ,MATRIX_DIMENSION);

        //recibir los resultados parciales
        MPI_Gather(MPI_IN_PLACE, 0, MPI_DOUBLE,
                   c.data(), rows_per_rank, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

        c.resize(MATRIX_DIMENSION);


        std::printf("resultado \n");
        for(int i = 0; i < MATRIX_DIMENSION; i++){
            std::printf("%.0f, ", c[i]);
        }
        std::printf("\n");


    }else{

        std::vector<double> A_local(MATRIX_DIMENSION*rows_per_rank);
        std::vector<double> b_local(MATRIX_DIMENSION);
        std::vector<double> c_local(rows_per_rank);

        MPI_Scatter(nullptr, 0, MPI_DOUBLE,
                    A_local.data(), MATRIX_DIMENSION*rows_per_rank, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);

        std::printf("RANK_%d: [%.0f..%.0f]\n", rank, A_local[0], A_local.back());

        //recivir el vector b
        MPI_Bcast(b_local.data(), MATRIX_DIMENSION, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        //realizar el calculo: c = A x b
        int rows_per_rank_tmp = rows_per_rank;

        if(rank == nprocs-1){
            rows_per_rank_tmp = MATRIX_DIMENSION - rank * rows_per_rank;
        }

        matrix_mult(A_local.data(), b_local.data(), c_local.data(), rows_per_rank_tmp ,MATRIX_DIMENSION);

        //enviar el resultado parcial
        MPI_Gather(c_local.data(), rows_per_rank, MPI_DOUBLE,
                   nullptr, 0, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);


    }

    MPI_Finalize();

    return 0;
}

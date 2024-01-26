#include <mpi.h>
#include <stdio.h>  // Incluimos la biblioteca para printf
#include <string>
#include <vector>
#include <cmath>

#define MAX_ITEMS 25

int sumar(int* tmp, int n){
    int suma = 0;
    for(int i = 0; i < n; i++){
        suma = suma +tmp[i];
    }
    return suma;
}


int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size;
    int real_size;
    int padding = 0;

    if(MAX_ITEMS%nprocs != 0){
        real_size = std::ceil((double) MAX_ITEMS/nprocs) * nprocs;
        block_size = real_size/nprocs;
        padding = real_size - MAX_ITEMS;
    }

    std::vector<int> data;
    std::vector<int> data_local(block_size);

    if(rank == 0){
        //inicializar
        data.resize(real_size);
        std::vector<int> sumas_rank(nprocs);

        std::printf("Dimension: %d, rows_alloc: %d, rows_per_rank: %d, padding: %d\n",
                    MAX_ITEMS, real_size, block_size, padding);

        for(int i = 0; i < MAX_ITEMS; i++){
            data[i] = i;
        }

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&padding, 1, MPI_INT, 0, MPI_COMM_WORLD);

        //Enviar los datos
        MPI_Scatter(data.data(), block_size, MPI_INT,
                    MPI_IN_PLACE, 0, MPI_INT,
                    0, MPI_COMM_WORLD);

        MPI_Gather(MPI_IN_PLACE, 0, MPI_INT,
                   sumas_rank.data(), block_size, MPI_INT,
                   0, MPI_COMM_WORLD);

    }else{

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&padding, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Calculo la suma parcial
        if (rank == nprocs - 1) {
            block_size = block_size - padding;
        }

        MPI_Scatter(nullptr, 0, MPI_INT,
                    data_local.data(), block_size, MPI_INT,
                    0, MPI_COMM_WORLD);

        int suma_parcial = sumar(data_local.data(), block_size);

        std::printf("RANK_%d: suma parcial = %d\n", rank, suma_parcial);

        // enviar la suma parcial al Rank_0
        int suma_total = 0;

        MPI_Reduce(&suma_parcial, &suma_total, 1, MPI_INT,
                   MPI_SUM, 0, MPI_COMM_WORLD);

    }


    MPI_Finalize();

    return 0;
}
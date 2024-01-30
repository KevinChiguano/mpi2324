#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>

std::vector<double> read_file() {
    //colocar la ruta del archivo con los datos
    std::fstream fs("./datos.txt", std::ios::in );
    std::string line;
    std::vector<double> ret;
    while( std::getline(fs, line) ){
        ret.push_back( std::stoi(line) );
    }
    fs.close();
    return ret;
}


int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size, real_size, padding = 0;
    int suma_total, suma_local;

    std::vector<double> datos;

    if(rank == 0){

        datos = read_file();

        real_size = datos.size();


        if(real_size % nprocs != 0){
            real_size = std::ceil((double) real_size/nprocs) *nprocs;
            padding = real_size-datos.size();
        }

        block_size = real_size/nprocs;

    }

    MPI_Bcast(&real_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<double> datos_local(block_size);

    MPI_Scatter(datos.data(), block_size, MPI_DOUBLE,
                datos_local.data(), block_size, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    double local_sum = 0.0;
    for (int i = 0; i < block_size; ++i) {
        local_sum += (datos_local[i]*datos_local[i]);
    }

    double global_sum = 0.0;

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if(rank == 0){
        printf("Modulo del vector: %f\n", sqrt(global_sum));
    }

    MPI_Finalize();

    return 0;
}
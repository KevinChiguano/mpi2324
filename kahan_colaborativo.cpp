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

double kahan_suma(double values[], int n) {
    double sum = 0.0;
    double compensator = 0.0;

    for (int i = 0; i < n; i++) {
        double y = values[i] - compensator;
        double temp_sum = sum + y;
        compensator = (temp_sum - sum) - y;
        sum = temp_sum;
    }

    return sum;
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size, real_size, size, padding = 0;


    if(rank == 0){
        std::vector<double> datos = read_file();
        size = datos.size();

        if(size % nprocs != 0){
            real_size = std::ceil((double) size/nprocs) * nprocs;
            padding = real_size - size;
            size = real_size;
            datos.resize(size);
        }

        block_size = size / nprocs;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&padding, 1, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Scatter(datos.data(), block_size, MPI_DOUBLE,
                    MPI_IN_PLACE, 0, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);

        std::vector<double> suma_rank(nprocs);

        MPI_Gather(MPI_IN_PLACE, 0, MPI_DOUBLE,
                   suma_rank.data(), 1, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

        suma_rank[0] = kahan_suma(datos.data(), block_size);



        double  suma_total_kahan = kahan_suma(suma_rank.data(), nprocs);

        std::printf("Suma total kahan colaborativo: %.0f\n", suma_total_kahan);

    }else{

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&padding, 1, MPI_INT, 0, MPI_COMM_WORLD);

        std::vector<double> datos_local(block_size);

        MPI_Scatter(nullptr, 0, MPI_DOUBLE,
                    datos_local.data(), block_size, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);

        if (rank == nprocs-1){
            block_size = block_size - padding;
            datos_local.resize(block_size);
        }

        double suma_local_kahan = kahan_suma(datos_local.data(), block_size);

        MPI_Gather(&suma_local_kahan, 1, MPI_DOUBLE,
                   nullptr, 0, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

    }

    MPI_Finalize();

    return 0;
}
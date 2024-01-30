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

    std::vector<double> datos;
    int block_size;

    if(rank == 0){

        datos = read_file();
        int size = datos.size();
        block_size = size / nprocs;
        int padding = size % nprocs;

        for (int id_rank = 1; id_rank < nprocs ; ++id_rank) {
            int start = id_rank * block_size + padding;
            MPI_Send(&start, 1, MPI_INT, id_rank, 0, MPI_COMM_WORLD);
            MPI_Send(&datos[start], block_size, MPI_DOUBLE, id_rank, 0, MPI_COMM_WORLD);
        }

        std::vector<double> suma_rank(nprocs);
        suma_rank[0] = kahan_suma(datos.data(), block_size+padding);

        for (int id_rank = 1; id_rank < nprocs; ++id_rank) {
            MPI_Recv(&suma_rank[id_rank], 1, MPI_DOUBLE, id_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        double suma_total_kahan = kahan_suma(suma_rank.data(), suma_rank.size());

        std::printf("Suma total de kahan: %.0f\n", suma_total_kahan);

    }else{

        MPI_Recv(&block_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        datos.resize(block_size);
        MPI_Recv(datos.data(), block_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        double suma_local = kahan_suma(datos.data(), datos.size());

        MPI_Send(&suma_local, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

    }

    MPI_Finalize();

    return 0;
}
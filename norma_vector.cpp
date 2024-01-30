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

double sumaPotencia(double* data, int size){
    double suma = 0;
    for (int i = 0; i < size; i++){
        suma = suma + (data[i]*data[i]);
    }
    return suma;
}

double sumar(double* data, int size){
    double suma = 0;
    for (int i = 0; i < size; i++){
        suma+= data[i];
    }
    return suma;
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<double> data;
    int tam;

    if(rank == 0){

        int block_size, padding;

        data = read_file();
        tam = data.size();
        block_size = tam/nprocs;
        padding = tam%nprocs;

        std::printf("Total ranks: %d\n", nprocs);
        std::printf("tam: %d, paquete: %d, residuo: %d\n", tam, block_size, padding);

        for(int id_rank = 1; id_rank < nprocs; id_rank++){
            int start = id_rank * block_size + padding;
            MPI_Send(&block_size, 1, MPI_INT, id_rank, 0, MPI_COMM_WORLD);
            MPI_Send(&data[start], block_size, MPI_DOUBLE, id_rank, 0, MPI_COMM_WORLD);
        }

        std::vector<double> suma_rank(nprocs);
        suma_rank[0] = sumaPotencia(data.data(), block_size+padding);

        for (int id_rank = 1; id_rank < nprocs; id_rank++){
            MPI_Recv(&suma_rank[id_rank], 1, MPI_DOUBLE, id_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for (int i = 0; i < suma_rank.size(); ++i) {
            printf("suma: %f\n", suma_rank[i]);
        }

        double raiz = sumar(suma_rank.data(), suma_rank.size());


        std::printf("Modulo del vector: %.0f\n", sqrt(raiz));

    }else{

        MPI_Recv(&tam, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        data.resize(tam);
        MPI_Recv(data.data(), tam, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


        double suma_pot_loc = sumaPotencia(data.data(), tam);

        std::printf("suma Potencia local: %f\n", suma_pot_loc);

        MPI_Send(&suma_pot_loc, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);


    }

    MPI_Finalize();

    return 0;
}

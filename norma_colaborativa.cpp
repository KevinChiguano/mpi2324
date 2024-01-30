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
} // Incluimos la biblioteca para printf

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


    if(rank == 0){

        std::vector<double> datos;
        int tam;

        datos = read_file();
        tam = datos.size();

        int block_size = tam/nprocs, real_size = tam, padding = 0;

        printf("real tam: %d\n", real_size);

        if(tam%nprocs != 0){
            real_size = std::ceil((double) tam/nprocs) * nprocs;
            block_size = real_size/nprocs;
            padding = real_size - tam;
        }

        for (int i = 0; i < padding; ++i) {
            datos.push_back(0);
        }

        printf("Real size: %d, Real size: %ld, block size: %d, padding: %d\n",real_size , datos.size(), block_size, padding);

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&padding, 1, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Scatter(datos.data(), block_size, MPI_DOUBLE,
                    MPI_IN_PLACE, 0, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);


        std::vector<double> suma_rank(nprocs);

        MPI_Gather(MPI_IN_PLACE, 0, MPI_DOUBLE,
                   suma_rank.data(), 1, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

        suma_rank[0] = sumaPotencia(datos.data(), block_size);

        for (int i = 0; i < suma_rank.size(); ++i) {
            printf("suma: %f\n", suma_rank[i]);
        }

        double raiz = sumar(suma_rank.data(), suma_rank.size());

        printf("Modulo del vector: %f\n", sqrt(raiz));

    }else{

        int block_size, padding;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&padding, 1, MPI_INT, 0, MPI_COMM_WORLD);

        std::vector<double> datos_local(block_size);

        MPI_Scatter(nullptr, 0, MPI_DOUBLE,
                    datos_local.data(), block_size, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);

        if(rank == nprocs-1){
            block_size = block_size-padding;
            datos_local.resize(block_size);
        }


        double sumaLocal = sumaPotencia(datos_local.data(), datos_local.size());
        std::printf("suma Potencia local: %f\n", sumaLocal);

        MPI_Gather(&sumaLocal, 1, MPI_DOUBLE,
                   nullptr, 0, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

    }

    MPI_Finalize();

    return 0;
}

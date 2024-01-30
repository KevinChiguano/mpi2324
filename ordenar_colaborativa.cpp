#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>

std::vector<int> read_file() {
    //colocar la ruta del archivo con los datos
    std::fstream fs("./datos.txt", std::ios::in );
    std::string line;
    std::vector<int> ret;
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

    std::vector<int> datos;
    std::vector<int> datos_local;
    std::vector<std::vector<int>> rank_orden(nprocs);

    std::vector<int> tmp;

    int block_size, size, real_size, padding = 0;


    if(rank == 0){
        datos = read_file();
        size = datos.size();

        if(size % nprocs != 0){
            real_size = std::ceil((double) size/nprocs)*nprocs;
            padding = real_size-size;
            datos.resize(real_size);
            tmp.resize(real_size);
        }

        block_size = real_size/nprocs;

    }

    MPI_Bcast(&padding, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    datos_local.resize(block_size);

    //enviar datos
    MPI_Scatter(datos.data(), block_size, MPI_INT,
                datos_local.data(), block_size, MPI_INT,
                0, MPI_COMM_WORLD);

    if(rank == nprocs-1){
        block_size = block_size-padding;
        datos_local.resize(block_size);
    }

    std::sort(datos_local.begin(), datos_local.end());

    std::printf("Rank: %d\n", rank);


    MPI_Gather(datos_local.data(), block_size, MPI_INT,
               tmp.data(), block_size, MPI_INT,
               0, MPI_COMM_WORLD);



    if(rank == 0){
        for (int i = 0; i < nprocs; i++) {
            std::vector<int> guardar(tmp.begin()+(i*block_size), tmp.begin()+(i*block_size+block_size));
            rank_orden[i] = guardar;
            std::printf("tam: %ld\n", rank_orden[i].size());
        }



        std::vector<int> vector_ordenado;

        // Combinar los vectores ordenados en rank_orden
        for (int i = 0; i < nprocs; i++) {
            // Combinar el vector actual con el vector_ordenado
            std::vector<int> resultado_combinado(vector_ordenado.size() + rank_orden[i].size());
            std::merge(vector_ordenado.begin(), vector_ordenado.end(),
                       rank_orden[i].begin(), rank_orden[i].end(),
                       resultado_combinado.begin());

            // Actualizar el vector_ordenado con el resultado combinado
            vector_ordenado = std::move(resultado_combinado);
        }

        for (int i = 0; i < 6000; ++i) {
            std::printf("%d , ", vector_ordenado[i]);
        }

    }


    MPI_Finalize();

    return 0;
}

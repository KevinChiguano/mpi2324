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
    int block_size;


    if(rank == 0){

        datos = read_file();
        int size = datos.size();
        int padding = size % nprocs;
        block_size = size/nprocs;

        std::printf("Size: %d, block_size: %d, padding: %d\n", size, block_size, padding);

        for (int id_rank = 1; id_rank < nprocs; id_rank++) {
            int start = id_rank * block_size + padding;
            MPI_Send(&block_size, 1, MPI_INT, id_rank, 0, MPI_COMM_WORLD);
            MPI_Send(&datos[start], block_size, MPI_INT, id_rank, 0, MPI_COMM_WORLD);
        }



        std::vector<std::vector<int>> rank_orden(nprocs);
        std::vector<int> tmp_guardar(block_size);

        std::vector<int> tmp(datos.begin(), datos.begin()+(block_size+padding));
        std::sort(tmp.begin(), tmp.end());

        rank_orden[0] = tmp;

        for (int id_rank = 1; id_rank < nprocs; id_rank++) {
            MPI_Recv(tmp_guardar.data(), block_size, MPI_INT, id_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            rank_orden[id_rank].assign(tmp_guardar.begin(), tmp_guardar.end());
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

        for (int i = 700000; i < 800000; ++i) {
            std::printf("%d , ", vector_ordenado[i]);
        }


    }else{

        MPI_Recv(&block_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        datos.resize(block_size);
        MPI_Recv(datos.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::sort(datos.begin(), datos.end());

        MPI_Send(datos.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD);


    }


    MPI_Finalize();

    return 0;
}
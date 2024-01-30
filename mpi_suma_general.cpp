#include <mpi.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>

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

int sumar(int* tmp, int n){
    int suma = 0;
    for(int i = 0; i < n; i++){
        suma = suma +tmp[i];
    }
    return suma;
}


int main(int argc, char** argv){

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);


    std::vector<int> data;
    int size, tam;


    if(rank == 0){

        data = read_file();
        tam = data.size();

        std::printf("Total ranks: %d\n", nprocs);

        int residuo = tam % nprocs;
        size = tam/nprocs;

        std::printf("tamanio paquete: %d\n", size);
        std::printf("residuo: %d\n", residuo);



        for(int rank_id = 1; rank_id < nprocs; rank_id++){
            int start = rank_id*size+residuo;
            MPI_Send(&tam, 1, MPI_INT, rank_id, 0, MPI_COMM_WORLD);
            MPI_Send(&data[start], size, MPI_INT, rank_id, 0, MPI_COMM_WORLD);
        }

        int suma_ranks[nprocs];
        suma_ranks[0] = sumar(data.data(), size+residuo);

        for(int rank_id = 1; rank_id < nprocs; rank_id++){
            MPI_Recv(&suma_ranks[rank_id], 1, MPI_INT, rank_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        int sumaTotal = sumar(suma_ranks,nprocs);

        std::printf("sumas Total: %d\n", sumaTotal);

    }else{

        MPI_Recv(&tam, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        data.resize(tam);
        MPI_Recv(data.data(), tam, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int suma_local = sumar(data.data(), tam);
        std::printf("Proceso %d: Suma local = %d\n", rank, suma_local);

        MPI_Send(&suma_local, 1, MPI_INT, 0,0,MPI_COMM_WORLD);

    }


    MPI_Finalize();

}
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <map>


std::vector<int> read_file() {
    std::fstream fs("./datos.txt", std::ios::in );
    std::string line;
    std::vector<int> ret;
    while( std::getline(fs, line) ){
        ret.push_back( std::stoi(line) );
    }
    fs.close();
    return ret;
}

void contar(int* datos, int* conteo, int size){
    for (int i = 0; i < size; ++i) {
        conteo[datos[i]]+=1;
    }
}

int sumar(int* datos, int size){
    int suma = 0;
    for (int i = 0; i < size; ++i) {
        suma += datos[i];
    }
    return suma;
}

int maximo(int* datos, int size){
    int max = datos[1];
    for (int i = 0; i < size; ++i) {
        if(datos[i] > max){
            max = datos[i];
        }
    }
    return max;
}

int minimo(int* datos, int size){
    int min = datos[1];
    for (int i = 0; i < size; ++i) {
        if(datos[i] < min){
            min = datos[i];
        }
    }
    return min;
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int size , block_size, padding = 0;

    std::vector<int> datos;


    int tam_conteo = 101;
    std::vector<int> conteo(tam_conteo, 0);
    std::vector<int> res(tam_conteo);

    int suma_parcial = 0;
    int suma_total = 0;

    int max;
    int max_local;

    int min = 0;
    int min_local;

    if(rank == 0){

        datos = read_file();

        size = datos.size();

        std::printf("tam: %ld \n", datos.size());
        datos.resize(size);

        if(size % nprocs){
            size = ceil((double) size / nprocs) *nprocs;
            padding = size - datos.size();
        }

        block_size = size / nprocs;

        std::printf("size: %d, block_size: %d, padding: %d \n", size, block_size, padding);



        std::map<int, int> contador;

        for (int elemento : datos) {
            contador[elemento]++;
        }

        std::cout << "Elemento\tFrecuencia\n";
        for (const auto& par : contador) {
            std::cout << par.first << "\t\t" << par.second << "\n";
        }

        int suma = 0;
        for (int elemento : datos) {
            suma += elemento;
        }

        double promedio = static_cast<double>(suma) / datos.size();
        std::cout << "\nSuma: " << suma << "\n";
        std::cout << "Promedio: " << promedio << "\n";
    }

    MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> datos_local(block_size);

    MPI_Scatter(datos.data(), block_size, MPI_INT,
                datos_local.data(), block_size, MPI_INT,
                0, MPI_COMM_WORLD);

    if(rank == nprocs-1){
        block_size = block_size-padding;
    }

    contar(datos_local.data(), conteo.data(), block_size);


    MPI_Reduce(conteo.data(), res.data(), tam_conteo, MPI_INT,
               MPI_SUM, 0, MPI_COMM_WORLD);

    suma_parcial = sumar(datos_local.data(), block_size);

    MPI_Reduce(&suma_parcial, &suma_total, 1, MPI_INT,
               MPI_SUM, 0, MPI_COMM_WORLD);

    max_local = maximo(datos_local.data(), block_size);

    MPI_Reduce(&max_local, &max, 1, MPI_INT,
               MPI_MAX, 0,MPI_COMM_WORLD);

    min_local = minimo(datos_local.data(), block_size);

    MPI_Reduce(&min_local, &min, 1, MPI_INT,
               MPI_MIN, 0, MPI_COMM_WORLD);


    if(rank == 0){

        std::printf("Conteo: \n");
        for(int i = 0; i < res.size(); i++){
            std::printf("[%d] = %d \n", i, res[i]);
        }

        double promedio = (((double)suma_total)/((double)datos.size()));
        std::printf("Promedio = %f \n", promedio);
        std::printf("Suma = %d \n", suma_total);
        std::printf("Maximo = %d \n", max);
        std::printf("Minimo = %d \n", min);
    }

    MPI_Finalize();

    return 0;
}
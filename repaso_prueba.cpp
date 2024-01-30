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

std::vector<double> contar(double* datos, int size){
    std::vector<double> tmp(101, 0);
    for (int i = 0; i < size; ++i) {
        tmp[datos[i]] += 1;
    }

    return tmp;
}

double sumar(double* datos, int size){
    double suma = 0;
    for (int i = 0; i < size; ++i) {
        suma += datos[i];
    }

    return suma;
}

std::vector<double> max_min(double* datos, int size){

    double max = datos[0], min= datos[0];

    for (int i = 0; i < size; ++i) {
        if(max < datos[i]){
            max = datos[i];
        }

        if(min > datos[i]){
            min = datos[i];
        }
    }

    std::vector<double> res = {min, max};
    return res;

}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int size, real_size, block_size, padding = 0;

    if(rank == 0){

        std::vector<double> datos = read_file();
        size = datos.size();

        if(size % nprocs != 0){
            real_size = std::ceil((double) size/nprocs) * nprocs;
            padding = real_size - size;
            size = real_size;
        }

        block_size = size / nprocs;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&padding, 1, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Scatter(datos.data(), block_size, MPI_DOUBLE,
                    MPI_IN_PLACE, 0, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);

        std::vector<std::vector<double>> rank_res(nprocs);
        std::vector<double> tmp(101*nprocs);
        rank_res[0] = contar(datos.data(), block_size);

        MPI_Gather(MPI_IN_PLACE, 0, MPI_DOUBLE,
                   tmp.data(), 101, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

        for (int i = 1; i < nprocs; ++i) {
            std::vector<double> guardar(tmp.begin()+(i*101), tmp.begin()+(i*101+101));
            rank_res[i] = guardar;
        }

        std::vector<double> tabla(101, 0);

        for (int i = 0; i < nprocs; ++i) {
            for (int j = 0; j < 101; ++j) {
                tabla[j] += rank_res[i][j];
            }
        }

        std::printf("|valor | cantidad |\n");
        for (int i = 100; i >= 0 ; i--) {
            std::printf("| %d | %f |\n", i, tabla[100-i]);
        }

        std::vector<double> suma_rank(nprocs);

        MPI_Gather(MPI_IN_PLACE, 0, MPI_DOUBLE,
                   suma_rank.data(), 1, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

        suma_rank[0] = sumar(datos.data(), block_size);
        double promedio = sumar(suma_rank.data(), nprocs);

        std::printf("Promedio: %f \n", promedio/(size-padding));

        std::vector<std::vector<double>> min_max_res(nprocs);
        std::vector<double> tmp_min_max(nprocs*2);
        min_max_res[0] = max_min(datos.data(), block_size);

        MPI_Gather(MPI_IN_PLACE, 0, MPI_DOUBLE,
                   tmp_min_max.data(), 2, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

        for (int i = 1; i < nprocs; ++i) {
            std::vector<double> guardar(tmp_min_max.begin()+ i *2, tmp_min_max.begin()+ i*2+2);
            min_max_res[i] = guardar;
        }

        double min = min_max_res[0][0], max=min_max_res[0][0];

        for (int i = 0; i < nprocs; ++i) {
            for (int j = 0; j < 2; ++j) {
                if(max < min_max_res[i][j]){
                    max = min_max_res[i][j];
                }

                if(min > min_max_res[i][j]){
                    min = min_max_res[i][j];
                }
            }
        }

        std::printf("MIN: %f  MAX: %f\n", min, max);



    }else{

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

        std::vector<double> tmp = contar(datos_local.data(), block_size);

        MPI_Gather(tmp.data(), 101, MPI_DOUBLE,
                   nullptr, 0, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);


        double suma_local = sumar(datos_local.data(), block_size);

        MPI_Gather(&suma_local, 1, MPI_DOUBLE,
                   nullptr, 0, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

        std::vector<double> res = max_min(datos_local.data(), block_size);

        MPI_Gather(res.data(), 2, MPI_DOUBLE,
                   nullptr, 0, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);


    }

    MPI_Finalize();

    return 0;
}
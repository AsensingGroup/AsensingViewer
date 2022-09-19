#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int ret = MPI_Init(&argc, &argv);
    if (ret != MPI_SUCCESS) {
        printf("failed to initialize MPI: %d\n", ret);
        return EXIT_FAILURE;
    }

    int nprocs;
    ret = MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    if (ret != MPI_SUCCESS) {
        printf("failed to get comm size: %d\n", ret);
        return EXIT_FAILURE;
    }
    int rank;
    ret = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (ret != MPI_SUCCESS) {
        printf("failed to get comm rank: %d\n", ret);
        return EXIT_FAILURE;
    }

    printf("Process %d of %d\n", rank, nprocs);

    ret = MPI_Finalize();
    if (ret != MPI_SUCCESS) {
        printf("failed to finalize MPI: %d\n", ret);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

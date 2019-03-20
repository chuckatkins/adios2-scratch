#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <mpi.h>

#include <adios2_c.h>

int main(int argc, char **argv)
{
    int nLocal;
    int err;
    int mpiSize, mpiRank;

    err = MPI_Init(&argc, &argv);
    if(err != MPI_SUCCESS)
    {
        printf("Writer: ERROR: Unable to initialize MPI\n");
        return 1;
    }
    err = MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    if(err != MPI_SUCCESS)
    {
        printf("Writer: ERROR: Unable to get MPI rank\n");
        return 1;
    }
    err = MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
    if(err != MPI_SUCCESS)
    {
        printf("Writer: ERROR: Unable to get MPI size\n");
        return 1;
    }

    if(argc != 3)
    {
        if(mpiRank == 0)
        {
            printf("Writer: Usage: %s output engine\n", argv[0]);
        }
        MPI_Finalize();
        return 2;
    }

    const char *outFileName = argv[1];
    const char *engine = argv[2];

    // Use a random size 10000-11000 for each rank
    srand(time(0));
    nLocal = 10000 + rand()%1000+mpiRank;

    printf("Writer: MPIRank:%03d MPISize:%d DatSize:%d\n", mpiRank, mpiSize, nLocal);

    // Allocate:
    double *dataLocal = malloc(sizeof(double)*nLocal);

    // Init the ADIOS subsystem
    adios2_error adiosErr;
    adios2_adios* adios = adios2_init(MPI_COMM_WORLD, adios2_debug_mode_on);

    adios2_io* ioWrite = adios2_declare_io(adios, "A");
    adiosErr = adios2_set_engine(ioWrite, engine);
    if(adiosErr != adios2_error_none)
    {
        printf("Writer: ERROR: set_engine failed: %d\n", adiosErr);
        adios2_finalize(adios);
        MPI_Finalize();
        return 3;
    }

    // Declare the variable
    size_t count[1];
    count[0] = (size_t)nLocal;
    adios2_variable* varDataLocal = adios2_define_variable(
        ioWrite, "a", adios2_type_double,
        1, NULL, NULL, count,
        adios2_constant_dims_true);

    size_t gShape[1];
    size_t gStart[1];
    size_t gCount[1];
    gShape[0] = mpiSize;
    gStart[0] = mpiRank;
    gCount[0] = 1;
    adios2_variable* varDataLocalSizes = adios2_define_variable(
        ioWrite, "aSizes", adios2_type_int32_t,
        1, gShape, gStart, gCount,
        adios2_constant_dims_true);

    // Open the file for writing
    adios2_engine* engineWrite = adios2_open(ioWrite, outFileName,
        adios2_mode_write);

    size_t t;
    for(t = 0; t < 100; ++t)
    {
        if(mpiRank == 0)
        {
            printf("Writer: CurrentStep %d\n", t);
        }

        // fill the local data
        size_t i;
        for(i = 0; i < nLocal; ++i)
        {
            dataLocal[i] = (double)(t);
        }

        // Write the step
        adios2_step_status adiosStatus;
        adiosErr = adios2_begin_step(engineWrite, adios2_step_mode_append, 0.0f,
            &adiosStatus);
        if(adiosErr != adios2_error_none)
        {
            printf("Writer: ERROR: MPIRank: %03d begin_step failed:%d\n",
                mpiRank, adiosErr);
            break;
        }

        if(t == 0)
        {
            adiosErr = adios2_put(engineWrite, varDataLocalSizes, &nLocal,
                adios2_mode_deferred);
            if(adiosErr != adios2_error_none)
           {
                printf("Writer: ERROR: MPIRank: %03d put failed:%d\n",
                    mpiRank, adiosErr);
                break;
            }
        }

        adiosErr = adios2_put(engineWrite, varDataLocal, dataLocal,
            adios2_mode_deferred);
        if(adiosErr != adios2_error_none)
        {
            printf("Writer: ERROR: MPIRank: %03d put failed:%d\n",
                mpiRank, adiosErr);
            break;
        }

        adiosErr = adios2_end_step(engineWrite);
        if(adiosErr != adios2_error_none)
        {
            printf("Writer: ERROR: MPIRank: %03d end_step failed:%d\n",
                mpiRank, adiosErr);
            break;
        }

    }

    // Close
    adios2_close(engineWrite);
    adios2_finalize(adios);

    // Cleanup
    if(dataLocal) { free(dataLocal); }

    MPI_Finalize();

    return 0;
}

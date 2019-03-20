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
        printf("Reader: ERROR: Unable to initialize MPI\n");
        return 1;
    }
    err = MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    if(err != MPI_SUCCESS)
    {
        printf("Reader: ERROR: Unable to get MPI rank\n");
        return 1;
    }
    err = MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
    if(err != MPI_SUCCESS)
    {
        printf("Reader: ERROR: Unable to get MPI size\n");
        return 1;
    }

    if(argc != 3)
    {
        if(mpiRank == 0)
        {
            printf("Reader: Usage: %s input engine\n", argv[0]);
        }
        MPI_Finalize();
        return 2;
    }

    const char *inFileName = argv[1];
    const char *engine = argv[2];

    // Allocate:
    double *dataRemote = NULL;

    // Init the ADIOS subsystem
    adios2_error adiosErr;
    adios2_adios* adios = adios2_init(MPI_COMM_WORLD, adios2_debug_mode_on);

    adios2_io* ioRead = adios2_declare_io(adios, "A");
    adiosErr = adios2_set_engine(ioRead, engine);
    if(adiosErr != adios2_error_none)
    {
        printf("Reader: ERROR: set_engine failed: %d\n", adiosErr);
        adios2_finalize(adios);
        MPI_Finalize();
        return 3;
    }
 
    // Open the file for writing
    adios2_engine* engineRead = adios2_open(ioRead, inFileName,
        adios2_mode_read);

    int numProcsRead;
    int sizePerBlock;

    while(1)
    {
        size_t currentStep;

        adios2_step_status adiosStatus;
        adiosErr = adios2_begin_step(engineRead,
            adios2_step_mode_next_available, 100.0f, &adiosStatus);
        if(adiosErr != adios2_error_none)
        {
            printf("Reader: ERROR: MPIRank: %03d begin_step failed:%d\n",
                mpiRank, adiosErr);
            break;
        }
        else if(adiosStatus != adios2_step_status_ok)
        {
            printf("Reader: Exiting.  Status: %d\n", adiosStatus);
            break;
        }

        adiosErr = adios2_current_step(&currentStep, engineRead);
        if(adiosErr != adios2_error_none)
        {
            printf("Reader: ERROR: MPIRank: %03d current_step failed:%d\n",
                mpiRank, adiosErr);
            break;
        }
        if(mpiRank == 0)
        {
            printf("Reader: CurrentStep: %d\n", currentStep);
        }

        adios2_variable* varDataRemoteSizes = adios2_inquire_variable(ioRead,
            "aSizes");
        if(varDataRemoteSizes)
        {
            size_t gShape[1];
            adios2_variable_shape(gShape, varDataRemoteSizes);
            numProcsRead = (int)gShape[0];
            
            size_t gStart[1];
            size_t gCount[1];
            gStart[0] = mpiRank;
            gCount[0] = 1;
            adios2_set_selection(varDataRemoteSizes, 1, gStart, gCount);
            adios2_get(engineRead, varDataRemoteSizes, &sizePerBlock,
                adios2_mode_sync);
            if(!dataRemote)
            {
                free(dataRemote);
            }
            dataRemote = malloc(sizeof(double)*sizePerBlock);
        }
        
        adios2_variable* varDataRemote = adios2_inquire_variable(ioRead,
            "a");
        if(!varDataRemote)
        {
            printf("Reader: ERROR: inquire_variable 'a' returned NULL\n");
            break;
        }

        adiosErr = adios2_set_block_selection(varDataRemote, mpiRank);
        if(adiosErr != adios2_error_none)
        {
            printf("Reader: ERROR: MPIRank: %03d set_block_selection failed:%d\n",
                mpiRank, adiosErr);
            break;
        }

        adiosErr = adios2_get(engineRead, varDataRemote, dataRemote,
            adios2_mode_deferred);
        if(adiosErr != adios2_error_none)
        {
            printf("Reader: ERROR: MPIRank: %03d get failed:%d\n",
                mpiRank, adiosErr);
            break;
        }

        adiosErr = adios2_end_step(engineRead);
        if(adiosErr != adios2_error_none)
        {
            printf("Reader: ERROR: MPIRank: %03d end_step failed:%d\n",
                mpiRank, adiosErr);
            break;
        }

        printf("Reader: Step: %d, MPIRank: %03d, BlockSize: %d\n", currentStep, mpiRank,
          sizePerBlock);
    }
    if(mpiRank == 0)
    {
        printf("Reader: Steps complete\n");
    }

    // Close
    adios2_close(engineRead);
    adios2_finalize(adios);

    // Cleanup
    if(dataRemote) { free(dataRemote); }

    MPI_Finalize();

    return 0;
}

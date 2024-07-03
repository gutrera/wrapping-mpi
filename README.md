# Set the Makefile:

Set the corresponding paths to the MPI library and PAPI library (in case it will be used) in use inside the Makefile

# Build the library:

make vm.so

# Usage:

Do the preload by setting the environment variable:

export LD_PRELOAD=$PATH_TO_THE_LIBRARY/vm.so

Execute the MPI program as usual:

mpirun -np 4 program

# PAPI counters on a specific code:

Instrument the code by adding MPI_Test at the beginning and at the end of the code you want to track HW counters specifically.
The API for MPI_Test is:

     int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status)

In order to differentiate from an ordinary call the parameters must be set in this way:

- request = NULL
- flag is a pointer to an integer variable with value 1 to indicate the beginning of the region to be tracked or with value 2 to indicate  the ending of the region to be tracked.
- status is unused

Example of use:

    MPI_Request *request;
    flag = 1;
    request = NULL;
    MPI_Test (request, &flag, MPI_STATUS_IGNORE);
    // region of code to be tracked
    flag = 2;
    MPI_Test (request, &flag, MPI_STATUS_IGNORE);
    

     

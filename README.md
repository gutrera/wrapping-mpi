# Set the Makefile:

Set the corresponding paths to the MPI library and PAPI library (in case it will be used) in use inside the Makefile

# Build the library:

make vm.so

# Usage

Do the preload by setting the environment variable:

export LD_PRELOAD=$PATH_TO_THE_LIBRARY/vm.so

Execute the MPI program as usual:

mpirun -np 4 program

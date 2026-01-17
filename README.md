# Config the Makefile:
# Set the corresponding paths to the MPI library and PAPI library 

# Build the library:

make vm.so

# Usage:

#Set PAPI_LIST environment variable:
#List of papi counters to trace, comma-separated and without spaces. 
#(Ensure first that combination is permitted)

#If none are specified, PAPI_TOT_INS and PAPI_L3_TCM are set

export PAPI_LIST="PAPI_TOT_INS,PAPI_L3_TCM"

# Shared library preload
export LD_PRELOAD=$PATH_TO_THE_LIBRARY/vm.so

# Execute the MPI program as usual:

mpirun -np 4 program

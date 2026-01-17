FLAGS	= -c 
LDFLAGS	= -fPIC -shared
MPICC   = mpiicx
CC	= icx
AR	= ar rc
RANLIB	= ranlib

#Intel MPI 
MPI_LIB = /gpfs/apps/MN5/GPP/ONEAPI/2023.2.0/mpi/2021.10.0/lib 
MPI_INC = /gpfs/apps/MN5/GPP/ONEAPI/2023.2.0/mpi/2021.10.0/include

#PAPI 
PAPI_INC = /apps/GPP/PAPI/7.1.0/GCC/include
PAPI_LIB = /apps/GPP/PAPI/7.1.0/GCC/lib

all:  vm.so

vm.so: vm.o vm.c 
	$(MPICC) -m64 -fPIC vm.o -L$(MPI_LIB) -lmpi -L$(PAPI_LIB) -lpapi -shared -o vm.so 

vm.o: vm.h vm.c
	$(MPICC) -fPIC $(FLAGS)  vm.c -I$(MPI_INC) -I$(PAPI_INC)

clean: 
	rm *.o *.a *.so


#include <mpi.h>
#include "vm.h"

#define MPI_INTERFAZ
#define PAPI_INTERFAZ  

#ifdef PAPI_INTERFAZ
#include "papi.h"
#endif

int mpi_finalize_wrapper ();
int mpi_init_wrapper ();

pid_t mypid=-1;
int mycpu;
int nprocesses;

int myrank=-1; 

double start_time, init_time, end_time;

double alltoall_time=0, wait_time=0, irecv_time=0, isend_time=0, send_time=0, recv_time=0, barrier_time=0, waitall_time=0, reduce_time=0, allreduce_time=0, split_time, bcast_time=0,  waitany_time=0, sendrecv_time=0, gather_time=0, allgather_time=0, scatter_time=0;


#ifdef PAPI_INTERFAZ
    #define NUM_EVENTS 3
    int EventSet = PAPI_NULL;
    long long values[NUM_EVENTS];
    //int events[NUM_EVENTS] = {PAPI_VEC_DP, PAPI_DP_OPS}; //{PAPI_FP_OPS}; //{PAPI_TOT_INS};
    int events[NUM_EVENTS] = {PAPI_VEC_DP, PAPI_VEC_SP, PAPI_L2_DCM}; //{PAPI_FP_OPS}; //{PAPI_TOT_INS};
#endif

/* Called from Fortran programs */


#ifdef MPI_INTERFAZ

void mpi_reduce_(const void *sendbuf, void *recvbuf, int *count, MPI_Datatype *datatype, MPI_Op *op, int *root, MPI_Comm *comm, int *ierr)
{
int ret;
double ini, end;

        ini=MPI_Wtime();
        ret = PMPI_Reduce(sendbuf, recvbuf, *count, *datatype, *op, *root, *comm);
        end=MPI_Wtime();
        reduce_time += end-ini;

        ierr = &ret;
}

void mpi_allreduce_(const void *sendbuf, void *recvbuf, int *count,
                         MPI_Datatype *datatype, MPI_Op *op, MPI_Comm *comm, int *ierr)
{
int ret;
double ini, end;

	ini=MPI_Wtime();
	ret = PMPI_Allreduce(sendbuf, recvbuf, *count, *datatype, *op, *comm);
        end=MPI_Wtime();
        allreduce_time += end-ini;

        ierr=&ret; 
}

void mpi_alltoall_ (void *sendbuf, int *sendcount, MPI_Datatype *sendtype,
                    void *recvbuf, int *recvcount, MPI_Datatype *recvtype, MPI_Comm *comm, int *ierr)
{
int ret;
double ini, end;

        ini=MPI_Wtime();
	ret=PMPI_Alltoall(sendbuf, *sendcount, *sendtype, recvbuf, *recvcount, *recvtype,  *comm);
        end=MPI_Wtime();
        alltoall_time += end-ini;

        ierr=&ret;
}

void mpi_bcast_( void *buffer, int *count, MPI_Datatype *datatype, int *root, MPI_Comm *comm, int *ierr)
{
int ret;
double ini, end;

        ini=MPI_Wtime();
	ret=PMPI_Bcast(buffer, *count,  *datatype, *root,  *comm );
	end=MPI_Wtime();
        bcast_time += end-ini;

        ierr = &ret;

}


void mpi_recv_(void *buf, int *count, MPI_Datatype *datatype, int *source, int *tag,
                    MPI_Comm *comm, MPI_Status *status, int *ierr)
{
int ret;
double ini, end;

        ini=MPI_Wtime();
	ret=PMPI_Recv(buf, *count, *datatype, *source, *tag, *comm, status);
        end=MPI_Wtime();
        recv_time += end-ini;

        ierr = &ret;

}

void mpi_wait_(MPI_Request *request, MPI_Status *status, int *ierr)
{
int ret;
double ini, end;

        ini=MPI_Wtime();
	ret=PMPI_Wait(request, status);
        end=MPI_Wtime();
        wait_time += end-ini;

        ierr = &ret;

}

void mpi_waitall_(int *count, MPI_Request array_of_requests[],
                      MPI_Status array_of_statuses[], int *ierr)
{
int ret;
double ini, end;

        ini=MPI_Wtime();
        ret=PMPI_Waitall(*count, array_of_requests, array_of_statuses);
        end=MPI_Wtime();
        waitall_time += end-ini;

        ierr = &ret;
}

void mpi_irecv_(void *buf, int *count, MPI_Datatype *datatype, int *source,
                     int *tag, MPI_Comm *comm, MPI_Request *request, int *ierr)
{
double ini, end;
int ret;

        ini=MPI_Wtime();
        ret=PMPI_Irecv(buf, *count, *datatype, *source, *tag, *comm, request);
        end=MPI_Wtime();
        irecv_time += end-ini;

        ierr = &ret;
}


void mpi_isend_(const void *buf, int *count, MPI_Datatype *datatype,
                     int *dest, int *tag, MPI_Comm *comm, MPI_Request *request, int *ierr)
{
double ini, end;
int ret;

        ini=MPI_Wtime();
        ret=PMPI_Isend(buf, *count, *datatype, *dest, *tag, *comm, request);
        end=MPI_Wtime();
        isend_time += end-ini;

        ierr = &ret;
}

void mpi_send_(const void *buf, int *count, MPI_Datatype *datatype, int *dest,
                    int *tag, MPI_Comm *comm, int *ierr)
{
double ini, end;
int ret;

        ini=MPI_Wtime();
	ret=PMPI_Send(buf, *count, *datatype, *dest, *tag, *comm);
        end=MPI_Wtime();
        send_time += end-ini;

        ierr = &ret;
}

void mpi_barrier_ (MPI_Comm *comm, int *ierr)
{
double ini, end;
        ini = MPI_Wtime();
	*ierr=PMPI_Barrier(*comm);
        end = MPI_Wtime();
	barrier_time += (end-ini);
}

void mpi_gather_ (const void *sendbuf, int *sendcount, MPI_Datatype *sendtype,
    void *recvbuf, int *recvcount, MPI_Datatype *recvtype, int *root,
    MPI_Comm *comm, int *ierr)
{
	double ini, end;
        ini = MPI_Wtime();
        *ierr=PMPI_Gather (sendbuf, *sendcount, *sendtype, recvbuf, *recvcount, 
			   *recvtype, *root, *comm);
        end = MPI_Wtime();
        gather_time += (end-ini);
}

void mpi_allgather_ (const void *sendbuf, int  *sendcount,
     MPI_Datatype *sendtype, void *recvbuf, int *recvcount,
     MPI_Datatype *recvtype, MPI_Comm *comm, int *ierr)
{
double ini, end;
        ini = MPI_Wtime();
        *ierr=PMPI_Allgather(sendbuf, *sendcount, *sendtype, recvbuf, *recvcount,
			*recvtype, *comm);
        end = MPI_Wtime();
        allgather_time += (end-ini);
}

void mpi_scatter_ (const void *sendbuf, int *sendcount, MPI_Datatype *sendtype,
    void *recvbuf, int *recvcount, MPI_Datatype *recvtype, int *root,
    MPI_Comm *comm, int *ierr)
{
double ini, end;
        ini = MPI_Wtime();
        *ierr=PMPI_Scatter (sendbuf, *sendcount, *sendtype, recvbuf, *recvcount, 
			*recvtype, *root, *comm);
        end = MPI_Wtime();
        scatter_time += (end-ini);
}

void mpi_waitany_ (int *count, MPI_Request array_of_requests[],
    int *index, MPI_Status *status, int *ierr)
{
	double ini, end;
        ini = MPI_Wtime();
        *ierr=PMPI_Waitany (*count, array_of_requests, index, status);
        end = MPI_Wtime();
        waitany_time += (end-ini);
}
#endif

int MPI_Finalize ()
{
int ret;
        ret=mpi_finalize_wrapper();
        return ret;
}

int MPI_Init (int *argc, char ***argv)
{
int ret;

     ret= PMPI_Init (argc, argv);

     ret= mpi_init_wrapper ();
     return ret;
}


/**** Wrappers code *****/

int mpi_init_wrapper ()
{
int ret;
int color, root;
double ini, end;

     ret=PMPI_Comm_rank (MPI_COMM_WORLD, &myrank);
     ret=PMPI_Comm_size (MPI_COMM_WORLD, &nprocesses);

     init_time = PMPI_Wtime();

#ifdef PAPI_INTERFAZ
    // Inicializar PAPI
    int retval;
    retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI error: Initialization\n");
        exit(1);
    }
    retval = PAPI_create_eventset(&EventSet);
    if (retval != PAPI_OK) {
        fprintf(stderr, "PAPI error: creating event set \n");
        exit(1);
    }

    retval = PAPI_add_events(EventSet, events, NUM_EVENTS);
    if (retval != PAPI_OK) {
        fprintf(stderr, "PAPI error: adding events\n");
        exit(1);
    }

    // Leer el contador de instrucciones
    retval = PAPI_start(EventSet);
    if (retval != PAPI_OK) {
        fprintf(stderr, "PAPI error: starting counters \n");
        exit(1);
    }
#endif


     start_time = PMPI_Wtime();
     return ret;
}


int mpi_finalize_wrapper ()
{
int ret;
double end_time, tot_mpi=0.0, mpi_time;
int mycpu, who;
struct rusage usage;
int n;
time_t t;


	end_time = PMPI_Wtime();

#ifdef PAPI_INTERFAZ
    int retval = PAPI_stop(EventSet, values);
    if (retval != PAPI_OK) {
        fprintf(stderr, "PAPI error: stop counters\n");
        exit(1);
    }

    printf("PAPI_counters: %lld  %lld %lld \n", values[0], values[1], values[2]);

    long long tot[NUM_EVENTS];

    MPI_Reduce(values, tot, NUM_EVENTS, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (myrank==0)
    printf("TOT: %lld %lld %lld  %.3f \n", tot[0], tot[1], tot[2], end_time-start_time);

    // Free PAPI resources
    PAPI_shutdown();

#endif

#ifdef MPI_INTERFAZ 

	printf("\n %d: RES=%.2f  allreduce= %.3f reduce=%.3f isend=%.3f send=%.3f irecv=%.3f recv=%.3f sendrecv=%.3f wait=%.3f waitall=%.3f waitany=%.3f barrier=%.3f alltoall=%.3f scatter=%.3f gather=%.3f allgather=%.3f \n", 
		myrank, end_time - start_time,  allreduce_time, reduce_time, isend_time, send_time, irecv_time, recv_time, sendrecv_time, wait_time, waitall_time, waitany_time, barrier_time, alltoall_time, scatter_time, gather_time, allgather_time); 


	mpi_time = allreduce_time+reduce_time+isend_time+send_time+irecv_time+recv_time+sendrecv_time+wait_time+waitall_time+waitany_time+barrier_time+alltoall_time+scatter_time+gather_time+allgather_time+bcast_time;
        
        MPI_Reduce(&mpi_time, &tot_mpi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if (myrank==0)
        printf("\n EXECTIME: %.3f %.3f\n",  end_time-start_time, tot_mpi);

#else
	if (myrank==0)
        printf("\n EXECTIME: %.3f \n",  end_time-start_time);
#endif

	MPI_Barrier (MPI_COMM_WORLD);

return ret;
}


#include <mpi.h>
#include <string.h>
#include "vm.h"

#define PAPI_INTERFAZ  
#ifdef PAPI_INTERFAZ
#include "papi.h"
#endif

int mpi_finalize_wrapper ();
int mpi_init_wrapper ();

pid_t mypid=-1;
int mycpu;
int nprocesses;
int myrank;

double start_time, init_time, end_time;


#ifdef PAPI_INTERFAZ
    #define DEFAULT_PAPI_LIST "PAPI_L3_TCM,PAPI_DP_OPS,PAPI_TOT_CYC"
    int NUM_EVENTS=0;
    int EventSet = PAPI_NULL;
    char *papi_list;
#endif


int MPI_Init (int *argc, char ***argv)
{
int ret;

     ret= PMPI_Init (argc, argv);
     ret= mpi_init_wrapper ();
     return ret;
}

int MPI_Init_thread( int *argc, char ***argv, int required, int *provided )
{
int ret;
     ret = PMPI_Init_thread(argc, argv, required, provided);
     ret= mpi_init_wrapper ();
     return ret;
}

int MPI_Finalize ()
{
int ret;
        ret=mpi_finalize_wrapper();
        return ret;
}


/**** Wrappers code *****/

int mpi_init_wrapper ()
{
int ret;
char *count1, *count2;

     ret=PMPI_Comm_rank (MPI_COMM_WORLD, &myrank);
     ret=PMPI_Comm_size (MPI_COMM_WORLD, &nprocesses);

     init_time = PMPI_Wtime();

#ifdef PAPI_INTERFAZ
    // Initialize PAPI
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

    // Initialize the set of events
    const char *env = getenv("PAPI_EVENTS");
    if (env == NULL) env = getenv("PAPI_LIST");  // Fallback for backward compatibility
    char *env_papi_list;

    if (env==NULL) {
	if (myrank==0)
        fprintf(stderr, "Environment variable PAPI_EVENTS not defined. Assuming %s\n",
			DEFAULT_PAPI_LIST);
	env_papi_list = (char *)malloc(strlen(DEFAULT_PAPI_LIST)+1);
	strcpy(env_papi_list, DEFAULT_PAPI_LIST);
    }
    else {
        env_papi_list = (char *)malloc(strlen(env)+1);
        strcpy(env_papi_list, env);
    }
    papi_list = (char *)malloc(strlen(env_papi_list)+1);
    strcpy (papi_list, env_papi_list);

    NUM_EVENTS=0;
    char *token = strtok(env_papi_list, ",");
    while (token != NULL) {
       retval = PAPI_add_named_event(EventSet, token);
       if (retval != PAPI_OK) {
          fprintf(stderr, "PAPI error %d: adding events %s\n", retval, token);
       }
       NUM_EVENTS++;
       token = strtok(NULL, ",");
    }

    //fprintf(stderr, "PAPI: %d events added\n", NUM_EVENTS);
    

    // Start counting events
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
double end_time;

	end_time = PMPI_Wtime();

#ifdef PAPI_INTERFAZ
    long long tot[NUM_EVENTS];
    long long values[NUM_EVENTS];

    int retval = PAPI_stop(EventSet, values);
    if (retval != PAPI_OK) {
        fprintf(stderr, "PAPI error %d: stop counters\n", retval);
        exit(1);
    }

    ret = PMPI_Reduce(values, tot, NUM_EVENTS, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Get output format from environment variable (default: csv)
    const char *output_format = getenv("PAPI_OUTPUT_FORMAT");
    if (output_format == NULL) output_format = "csv";
    
    // Parse event names into array
    char *event_names[NUM_EVENTS];
    char *papi_list_copy = strdup(papi_list);
    char *token = strtok(papi_list_copy, ",");
    int ev_idx = 0;
    while (token != NULL && ev_idx < NUM_EVENTS) {
        event_names[ev_idx++] = token;
        token = strtok(NULL, ",");
    }
    
    if (strcmp(output_format, "yaml") == 0) {
        // YAML format
        if (myrank == 0) {
            printf("\npapi_results:\n");
            printf("  total:\n");
            for (int i=0; i<NUM_EVENTS; i++) {
                printf("    %s: %lld\n", event_names[i], tot[i]);
            }
            printf("  execution_time: %.3f\n", end_time-start_time);
        }
        printf("  rank_%d:\n", myrank);
        for (int i=0; i<NUM_EVENTS; i++) {
            printf("    %s: %lld\n", event_names[i], values[i]);
        }
    } else if (strcmp(output_format, "csv") == 0) {
        // CSV format
        if (myrank == 0) {
            // Header
            printf("\ntype,rank");
            for (int i=0; i<NUM_EVENTS; i++) {
                printf(",%s", event_names[i]);
            }
            printf("\n");
            
            // Total row
            printf("TOTAL,0");
            for (int i=0; i<NUM_EVENTS; i++) {
                printf(",%lld", tot[i]);
            }
            printf("\n");
        }
        
        // Wait for rank 0 to print header
        PMPI_Barrier(MPI_COMM_WORLD);
        
        // Per-rank rows (in order)
        for (int r = 0; r < nprocesses; r++) {
            if (r == myrank) {
                printf("PARTIAL,%d", myrank);
                for (int i=0; i<NUM_EVENTS; i++) {
                    printf(",%lld", values[i]);
                }
                printf("\n");
                fflush(stdout);
            }
            PMPI_Barrier(MPI_COMM_WORLD);
        }
        
        if (myrank == 0) {
            printf("EXECTIME,0,%.3f\n", end_time-start_time);
        }
    } else {
        // Original format (default fallback)
        if (myrank == 0) {
            printf("\n %d: TOTAL PAPI_counters %s: ", myrank, papi_list);
            for (int i=0; i<NUM_EVENTS; i++) {
                printf(" %lld  ", tot[i]);
            }
            printf("\n");
        }
        printf("\n %d: PARTIAL PAPI_counters %s: ", myrank, papi_list);
        for (int i=0; i<NUM_EVENTS; i++) {
            printf(" %lld  ", values[i]);
        }
        printf("\n");
    }
    
    free(papi_list_copy);

    // Free PAPI resources
    PAPI_shutdown();
#else
    // Print execution time even without PAPI
    if (myrank==0)
        printf("\n EXECTIME: %.3f  \n",  end_time-start_time);
#endif

	ret=PMPI_Finalize ();
return ret;
}





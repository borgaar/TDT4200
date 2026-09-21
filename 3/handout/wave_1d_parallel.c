#define _XOPEN_SOURCE 600
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// TASK: T1a
// Include the MPI headerfile
// BEGIN: T1a
#include <mpi.h>
// END: T1a

// Option to change numerical precision.
typedef int64_t int_t;
typedef double real_t;

// TASK: T1b
// Declare variables each MPI process will need
// BEGIN: T1b
static int world_size;
static int world_rank;
// END: T1b

static int_t elem_count;
static int_t offset;

// Simulation parameters: size, step count, and how often to save the state.
const int_t N = 65536, max_iteration = 100000, snapshot_freq = 500;

// Wave equation parameters, time step is derived from the space step.
const real_t c = 1.0, dx = 1.0;
real_t dt;

// Buffers for three time steps, indexed with 2 ghost points for the boundary.
real_t *buffers[3] = {NULL, NULL, NULL};

real_t *root_buff;

#define U_prv(i) buffers[0][(i) + 1]
#define U(i) buffers[1][(i) + 1]
#define U_nxt(i) buffers[2][(i) + 1]

// TASK: T8
// Save the present time step in a numbered file under 'data/'.
void domain_save(int_t step) {
    // BEGIN: T8
    if (world_rank == 0) {
        char filename[256];
        sprintf(filename, "data/%.5ld.dat", step);
        FILE *out = fopen(filename, "wb");
        fwrite(root_buff, sizeof(real_t), N, out);
        fclose(out);
    }
    // END: T8
}

// TASK: T3
// Allocate space for each process' sub-grids
// Set up our three buffers, fill two with an initial cosine wave,
// and set the time step.
void domain_initialize(void) {
    // BEGIN: T3

    // evenly distribute work
    int_t base = N / world_size;
    int_t remainder = N % world_size;
    if (world_rank < remainder) {
        elem_count = base + 1;
    } else {
        elem_count = base;
    }

    // get the sub-processes offset
    offset =
        world_rank * base + (world_rank < remainder ? world_rank : remainder);

    // malloc root_buffer for rank 0
    if (world_rank == 0) {
        root_buff = malloc(N * sizeof(real_t));
    }

    buffers[0] = malloc((elem_count + 2) * sizeof(real_t));
    buffers[1] = malloc((elem_count + 2) * sizeof(real_t));
    buffers[2] = malloc((elem_count + 2) * sizeof(real_t));

    // init buffers
    for (int_t i = 0; i < elem_count; i++) {
        U_prv(i) = U(i) = cos(2 * M_PI * (offset + i) / (real_t)N);
    }
    // END: T3

    // Set the time step for 1D case.
    dt = dx / c;
}

// Return the memory to the OS.
void domain_finalize(void) {
    free(buffers[0]);
    free(buffers[1]);
    free(buffers[2]);
    if (world_rank == 0) {
        free(root_buff);
    }
}

// Rotate the time step buffers.
void move_buffer_window(void) {
    real_t *temp = buffers[0];
    buffers[0] = buffers[1];
    buffers[1] = buffers[2];
    buffers[2] = temp;
}

// TASK: T4
// Derive step t+1 from steps t and t-1.
void time_step(void) {
    // BEGIN: T4
    for (int_t i = 0; i < elem_count; i++) {
        U_nxt(i) =
            -U_prv(i) + 2.0 * U(i) +
            (dt * dt * c * c) / (dx * dx) * (U(i - 1) + U(i + 1) - 2.0 * U(i));
    }
    // END: T4
}

// TASK: T6
// Neumann (reflective) boundary condition.
void boundary_condition(void) {
    // BEGIN: T6
    if (world_rank == 0) {
        U(-1) = U(1);
    }
    if (world_rank == world_size - 1) {
        U(elem_count) = U(elem_count - 2);
    }
    // END: T6
}

// TASK: T5
// Communicate the border between processes.
void border_exchange(void) {
    // BEGIN: T5
    // send backwards
    real_t buff[1] = {U(0)};
    MPI_Send(buff, 1, MPI_DOUBLE,
             world_rank == 0 ? MPI_PROC_NULL : world_rank - 1, 0,
             MPI_COMM_WORLD);

    // send forwards
    buff[0] = U(elem_count - 1);
    MPI_Send(buff, 1, MPI_DOUBLE,
             world_rank == world_size - 1 ? MPI_PROC_NULL : world_rank + 1, 0,
             MPI_COMM_WORLD);

    // receive behind
    if (world_rank != 0) {
        MPI_Recv(buff, 1, MPI_DOUBLE, world_rank - 1, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        U(-1) = buff[0];
    }

    // receive front
    if (world_rank != world_size - 1) {
        MPI_Recv(buff, 1, MPI_DOUBLE, world_rank + 1, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        U(elem_count) = buff[0];
    }
    // END: T5
}

// TASK: T7
// Every process needs to communicate its results
// to root and assemble it in the root buffer
void send_data_to_root() {
    // BEGIN: T7
    // send to rank 0 (or copy for rank 0)
    if (world_rank != 0) {
        MPI_Send(&U(0), elem_count, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        memcpy(root_buff, &U(0), elem_count * sizeof(real_t));
    }

    if (world_rank == 0) {
        // stitch data together
        for (int rank = 1; rank < world_size; rank++) {
            int_t base = N / world_size;
            int_t remainder = N % world_size;
            int_t recv_count;
            if (rank < remainder) {
                recv_count = base + 1;
            } else {
                recv_count = base;
            }

            int recv_offset =
                rank * base + (rank < remainder ? rank : remainder);

            MPI_Recv(&root_buff[recv_offset], recv_count, MPI_DOUBLE, rank, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    // END: T7
}

// Main time integration.
void simulate(void) {
    // Go through each time step.
    for (int_t iteration = 0; iteration <= max_iteration; iteration++) {
        if ((iteration % snapshot_freq) == 0) {
            send_data_to_root();
            domain_save(iteration / snapshot_freq);
        }

        // Derive step t+1 from steps t and t-1.
        border_exchange();
        boundary_condition();
        time_step();

        move_buffer_window();
    }
}

int main(int argc, char **argv) {
    // TASK: T1c
    // Initialise MPI
    // BEGIN: T1c
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    // END: T1c

    domain_initialize();

    // TASK: T2
    // Time your code
    // BEGIN: T2
    struct timespec start;
    if (world_rank == 0) {
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    simulate();

    if (world_rank == 0) {
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        long int elapsed = (end.tv_sec * 1e9) + end.tv_nsec -
                           ((start.tv_sec * 1e9) + start.tv_nsec);
        printf("Elapsed time (rank 0): %f s\n", elapsed / 1e9);
    }
    // END: T2

    domain_finalize();

    // TASK: T1d
    // Finalise MPI
    // BEGIN: T1d
    MPI_Finalize();
    // END: T1d

    exit(EXIT_SUCCESS);
}

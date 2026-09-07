#define _XOPEN_SOURCE 600
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Option to change numerical precision.
typedef int64_t int_t;
typedef double real_t;

// Simulation parameters: size, step count, and how often to save the state.
const int_t N = 1024, max_iteration = 4000, snapshot_freq = 10;

// Wave equation parameters, time step is derived from the space step.
const real_t c = 1.0, dx = 1.0;
real_t dt;

// Buffers for three time steps, indexed with 2 ghost points for the boundary.
real_t *buffers[3] = {NULL, NULL, NULL};

#define U_prv(i) buffers[0][(i) + 1]
#define U(i) buffers[1][(i) + 1]
#define U_nxt(i) buffers[2][(i) + 1]

// Save the present time step in a numbered file under 'data/'.
void domain_save(int_t step) {
  char filename[256];
  sprintf(filename, "data/%.5ld.dat", step);
  FILE *out = fopen(filename, "wb");
  fwrite(&U(0), sizeof(real_t), N, out);
  fclose(out);
}

// TASK: T1
// Set up our three buffers, fill two with an initial cosine wave,
// and set the time step.
void domain_initialize(void) {
  // BEGIN: T1
  size_t BUFF_SIZE = N + 2;

  buffers[0] = calloc(BUFF_SIZE, sizeof(real_t));
  buffers[1] = calloc(BUFF_SIZE, sizeof(real_t));
  buffers[2] = calloc(BUFF_SIZE, sizeof(real_t));

  for (int_t i = 0; i < N; i++) {
    real_t x = (real_t)i / N;
    U_prv(i) = U(i) = cos(2 * M_PI * x);
  }

  dt = dx / c;
  // END: T1
}

// TASK T2:
// Return the memory to the OS.
// BEGIN: T2
void domain_finalize(void) {
  free(buffers[0]);
  free(buffers[1]);
  free(buffers[2]);
}
// END: T2

// TASK: T3
// Rotate the time step buffers.
// BEGIN: T3
void shift_forward(void) {
  real_t *temp = buffers[0];
  buffers[0] = buffers[1];
  buffers[1] = buffers[2];
  buffers[2] = temp;
};
// END: T3

// TASK: T4
// BEGIN: T4
void next_iter(void) {
  const real_t coeff = (dt * dt * c * c) / (dx * dx);

  for (int_t i = 0; i < N; i++) {
    U_nxt(i) = -U_prv(i) + 2 * U(i) + coeff * (U(i - 1) + U(i + 1) - 2 * U(i));
  }
}
// END: T4

// TASK: T5
// Neumann (reflective) boundary condition.
// BEGIN: T5
void apply_boundary(void) {
  U(-1) = U(1);
  U(N) = U(N - 2);
}
// END: T5

// TASK: T6
// Main time integration.
void simulate(void) {
  // BEGIN: T6
  int_t iteration = 0;
  while (iteration < max_iteration) {
    apply_boundary();
    next_iter();
    shift_forward();
    if (iteration % snapshot_freq == 0) {
      domain_save(iteration / snapshot_freq);
    }
    iteration++;
  }
  // END: T6
}

int main(void) {
  domain_initialize();

  simulate();

  domain_finalize();
  exit(EXIT_SUCCESS);
}

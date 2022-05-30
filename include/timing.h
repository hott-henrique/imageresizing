#ifndef TP2_TIMING_HEADER
#define TP2_TIMING_HEADER

#include <stdio.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

typedef struct timing_t {
	struct timeval start;
	struct timeval end;

	struct rusage rstart;
	struct rusage rend;
} timing;

void t_Start(timing * t);
void t_Finalize(timing * t);

void t_PrintHeader(FILE * f);
void t_Print(timing * t, FILE * f, const char * functionName, int inputSize);

#endif

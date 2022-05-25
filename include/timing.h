#ifndef TP2_TIMING_HEADER
#define TP2_TIMING_HEADER

#include <stdio.h>

typedef struct timing_t timing;

void em_Start(timing * t);
void em_Finalize(timing * t);

void em_PrintHeader(FILE * f);
void em_Print(timing * t, FILE * f, const char * functionName);

#endif

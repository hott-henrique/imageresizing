#include "timing.h"

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

void t_Start(timing * t) {
	gettimeofday(&t->start, 0);
	getrusage(RUSAGE_SELF, &t->rstart);
}

void t_Finalize(timing * t) {
	gettimeofday(&t->end, 0);
	getrusage(RUSAGE_SELF, &t->rend);
}

void t_PrintHeader(FILE * f) {
		fprintf(f, "FunctionName\t\tgettimeofday(microseconds)\t\tgetrusage(seconds)\n");
}

void t_Print(timing * t, FILE * f, const char * functionName) {
	long long elapsed = (t->end.tv_sec - t->start.tv_sec) * 1000000LL + t->end.tv_usec - t->start.tv_usec;
	float user_time = (t->rend.ru_utime.tv_sec - t->rstart.ru_utime.tv_sec) + 1e-6 * (t->rend.ru_utime.tv_usec - t->rstart.ru_utime.tv_usec);
	fprintf(f, "%s\t\t%lld\t\t\t\t\t\t%0.6f\n", functionName, elapsed, user_time);
}



#include "image.h"
#include "pixel.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(TIMING)
#include "timing.h"
#endif

#define HORIZONTAL 'h'
#define VERTICAL 'w'

#if defined(TIMING)
FILE * timingstdout = NULL;
#endif

typedef struct arguments_t {
	char * inputFilePath;
	char * outputFilePath;
	char imageMode;
	long columnsToRemove;
	long linesToRemove;
	int operator;
} arguments;

short args_Parse(arguments * args, int argc, char ** argv);
void args_ShowUsage(const char * programName);

int main(int argc, char ** argv) {
	arguments args = {
		.inputFilePath = NULL,
		.outputFilePath = NULL,
		.imageMode = 'N',
		.columnsToRemove = 0,
		.linesToRemove = 0,
		.operator = Sobel
	};

	args_Parse(&args, argc, argv);

	printf("inputFilePath: %s\n"
		   "outputFilePath: %s\n"
		   "imageMode: %c\n"
		   "columnsToRemove: %ld\n"
		   "linesToRemove: %ld\n"
		   "energyOperator: %c\n",
		   args.inputFilePath,
		   args.outputFilePath,
		   args.imageMode,
		   args.columnsToRemove,
		   args.linesToRemove,
		   args.operator);

#if defined(TIMING)
	timingstdout = fopen("timing.out", "w");
	t_PrintHeader(timingstdout);

	timing t = { 0 };
	t_Start(&t);
#endif

	Image i = img_Load(args.inputFilePath, args.imageMode);

	if (args.linesToRemove != 0 && args.columnsToRemove != 0) {
		img_RemoveLinesAndColumns(i, args.linesToRemove, args.columnsToRemove, args.operator);
	} else if (args.linesToRemove != 0) {
		img_RemoveLines(i, args.linesToRemove, args.operator);
	} else {
		img_RemoveColumns(i, args.columnsToRemove, args.operator);
	}

	printf("Dale.\n");


	FILE * f = NULL == args.outputFilePath ? stdout : fopen(args.outputFilePath, "w");

	if (NULL == f) {
		fprintf(stderr, "Couldn't open output file: %s\n", args.outputFilePath);
	} else {
		img_Print(i, f);
		if (NULL != f && stdout != f) fclose(f);
	}

	img_Free(i);

#if defined(TIMING)
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, 0);

	fclose(timingstdout);
#endif

	return EXIT_SUCCESS;
}

short args_Parse(arguments * args, int argc, char ** argv) {
	if (argc < 5) args_ShowUsage(argv[0]);

	int option = 0;

	while ((option = getopt(argc, argv, ":i:f:o:w:h:e:m:")) != -1) {
		switch (option) {
			case 'i':
			case 'f':
				args->inputFilePath = optarg;
			break;

			case 'o':
				args->outputFilePath = optarg;
			break;

			case 'w':
				args->columnsToRemove = strtol(optarg, NULL, 10);
			break;

			case 'h':
				args->linesToRemove = strtol(optarg, NULL, 10);
			break;

			case 'e':
				args->operator = *optarg;
			break;

			case 'm':
				args->imageMode = *optarg;
			break;
		}
	}

	for (; optind < argc; optind++) {
		size_t argSize = strlen(argv[optind]);
		if (argSize == 1) {
			switch (*argv[optind]) {
				case 'M':
				case 'G':
					args->imageMode = *argv[optind];
				break;

				case 'S':
				case 'F':
					args->operator = *argv[optind];
				break;
			}
		} else {
			args->inputFilePath = argv[optind];
		}
	}

	return 0;
}

void args_ShowUsage(const char * programName) {
	fprintf(stderr, "Usage: %s <path/to/image.ppm> <image_mode> <energy_operator> -[options]\n", programName);
	fprintf(stderr, "\toptions:\n");
	fprintf(stderr, "\t-m(ode) M for matrix representation | G for graph representation.\n");
	fprintf(stderr, "\t-(i(nput)/f(ile)) <path/to/image.ppm>\n");
	fprintf(stderr, "\t-h(eight) <remotions> for height | -w(width) <remotions> for width | or both.\n");
	fprintf(stderr, "\t-o(output) <path/to/output.ppm> or none for stdout.\n");
	fprintf(stderr, "\t-e(nergy) S for Sobel operator | F for Sobel Feldman operator.\n");
	exit(EXIT_FAILURE);
}

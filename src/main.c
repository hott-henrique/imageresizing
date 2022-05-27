#include "image.h"

#include <stdlib.h>
#include <stdio.h>

#if defined(TIMING)
#include "timing.h"
#endif

#define HORIZONTAL 'h'
#define VERTICAL 'w'

short args_Parse(int argc, const char ** argv);
void args_ShowUsage(const char * programName);

static const char * filePath = NULL;
static char imageMode = 'N';
static long columnsToRemove = 0;
static long linesToRemove = 0;

#if defined(TIMING)
FILE * timingstdout = NULL;
#endif

int main(int argc, const char ** argv) {
	while (args_Parse(argc, argv));

#if defined(TIMING)
	timingstdout = fopen("timing.out", "w");
	t_PrintHeader(timingstdout);
#endif

	Image i = img_Load(filePath, imageMode);

	if (linesToRemove != 0 && columnsToRemove != 0) {
		img_RemoveLinesAndColumns(i, linesToRemove, columnsToRemove);
	} else if (linesToRemove != 0) {
		img_RemoveLines(i, linesToRemove);
	} else {
		img_RemoveColumns(i, columnsToRemove);
	}

	img_Save(i, "Result.ppm");

	img_Free(i);

#if defined(TIMING)
	fclose(timingstdout);
#endif

	return EXIT_SUCCESS;
}

short args_Parse(int argc, const char ** argv) {
	static int i = 0;

	if (i >= argc) return 0;

	switch (i) {
		case 0:
			if (argc < 5) args_ShowUsage(argv[0]);
			i++;
			return 1;
		break;

		case 1:
			filePath = argv[1];
			i++;
			return 1;
		break;

		case 2:
			imageMode = argv[2][0];
			i++;
			return 1;
		break;

		case 3:
		case 5:
			switch (argv[i][1]) {
				case 'w':
					columnsToRemove = strtol(argv[i + 1], NULL, 10);
					i += 2;
					return 1;
				break;

				case 'h':
					linesToRemove = strtol(argv[i + 1], NULL, 10);
					i += 2;
					return 1;
				break;
			}
		break;
	}

	return 0;
}

void args_ShowUsage(const char * programName) {
	fprintf(stderr, "Usage: %s <image.ppm> <mode> -[option] <lines>\n", programName);
	fprintf(stderr, "\nOptions:\n\th for height\n\tw for width.\n");
	exit(EXIT_FAILURE);
}

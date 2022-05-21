#include "image.h"

#include <stdlib.h>
#include <stdio.h>

#define HORIZONTAL 'h'
#define VERTICAL 'w'

short args_Parse(int argc, const char ** argv);
void args_ShowUsage(const char * programName);

static const char * filePath = NULL;
static char imageMode = 'N';
static long columnsToRemove = 0;
static long linesToRemove = 0;

int main(int argc, const char ** argv) {
	// TODO: Se for fazer o extra tem que dar suporte para remover linhas e colunas ao mesmo tempo.

	while (args_Parse(argc, argv));

	fprintf(stdout, "%s %c %ld %ld\n", filePath, imageMode, linesToRemove, columnsToRemove);

	Image i = img_Load(filePath, imageMode);

	if (linesToRemove != 0 && columnsToRemove != 0) {
	// Pediu para remover linhas e colunas.	
		img_RemoveLinesAndColumns(i, linesToRemove, columnsToRemove);
	} else if (linesToRemove != 0) {
	// Pediu para remover apenas linhas.
		img_RemoveLines(i, linesToRemove);
	} else {
	// Pediu para remover apenas colunas.
		img_RemoveColumns(i, columnsToRemove);
	}

	img_Save(i, "Result.ppm");

	img_Free(i);

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

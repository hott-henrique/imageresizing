#include "image.h"

#include <stdlib.h>
#include <stdio.h>

#define HORIZONTAL 'h'
#define VERTICAL 'w'

int main(int argc, const char ** argv) {
	if (argc < 5) {
		fprintf(stderr, "Usages:\n");
		fprintf(stderr, "\t%s <image.ppm> <mode> -h <lines>\n", argv[0]);
		fprintf(stderr, "\t%s <image.ppm> <mode> -w <columns>\n", argv[0]);
		return EXIT_FAILURE;
	}
	// TODO: Se for fazer o extra tem que dar suporte para remover linhas e colunas ao mesmo tempo.

	const char * filePath = argv[1];
	char imageMode = argv[2][0];

	for (int i = 3; i < argc; i++) {
		//parse arguments
	}
	char direction = argv[3][1];

	long amountRemotions = strtol(argv[4], NULL, 10); // Converte uma string que representa um número para um número(long).
	fprintf(stdout, "%s %c %c %ld\n", filePath, imageMode, direction, amountRemotions);

	Image i = img_Load(filePath, imageMode);

	switch (direction) {
		case HORIZONTAL:
			img_RemoveLines(i, amountRemotions);
		break;

		case VERTICAL:
			img_RemoveColumns(i, amountRemotions);
		break;
	}

	img_Save(i, "Result.ppm");

	img_Free(i);

	return EXIT_SUCCESS;
}

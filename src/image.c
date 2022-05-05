#include "image.h"

#include "img_matrix.h"
#include "img_graph.h"

#include <stdlib.h>
#include <stdio.h>

struct image_t {
	char mode;
	union T {
		GImage G;
		MImage M;
	} representations;
};

Image img_Load(const char * filePath, char mode) {
	Image i = (Image) malloc(sizeof(struct image_t));
	i->mode = mode;

	switch (i->mode) {
		case 'M':
			i->representations.M = mimg_Load(filePath);
			break;

		case 'G':
			i->representations.G = gimg_Load(filePath);
			break;

		default:
			fprintf(stderr, "Unknown image mode: %c", i->mode);
			exit(EXIT_FAILURE);
	}

	return i;
}

void img_RemoveLines(Image i, int amount) {
	switch (i->mode) {
		case 'M':
			mimg_RemoveLines(i->representations.M, amount);
			break;

		case 'G':
			gimg_RemoveLines(i->representations.G, amount);
			break;

		default:
			fprintf(stderr, "Unknown image mode: %c", i->mode);
			exit(EXIT_FAILURE);
	}
}

void img_RemoveColumns(Image i, int amount) {
	switch (i->mode) {
		case 'M':
			mimg_RemoveColumns(i->representations.M, amount);
			break;

		case 'G':
			gimg_RemoveColumns(i->representations.G, amount);
			break;

		default:
			fprintf(stderr, "Unknown image mode: %c", i->mode);
			exit(EXIT_FAILURE);
	}
}

void img_Save(Image i, const char * fileName) {
	switch (i->mode) {
		case 'M':
			mimg_Save(i->representations.M, fileName);
			break;

		case 'G':
			gimg_Save(i->representations.G, fileName);
			break;

		default:
			fprintf(stderr, "Unknown image mode: %c", i->mode);
			exit(EXIT_FAILURE);
	}
}

void img_Free(Image i) {
	switch (i->mode) {
		case 'M':
			mimg_Free(i->representations.M);
			break;

		case 'G':
			gimg_Free(i->representations.G);
			break;

		default:
			fprintf(stderr, "Unknown image mode: %c", i->mode);
			exit(EXIT_FAILURE);
	}
	free(i);
}

// FILE * f = fopen(filePath, "r");
// if (NULL == f) {
// 	fprintf(stderr, "Could not open file: %s\n", filePath);
// 	exit(EXIT_FAILURE);
// }

// fseek(f, sizeof(char) * 3, SEEK_SET); // Ignorar a assinatura do arquivo.

// // A especificação diz que todas as linhas tem no maximo 70 caracteres.
// char line[70];

// fgets(line, sizeof(line), f);

// // Se a primeira linha após a assinatura é um comentario, ignore.
// // E repita enquanto houver mais comentarios.
// while (line[0] == '#') fgets(line, sizeof(line), f);

// // A próxima linha que tem um real significado vai ser a que indica altura e largura da imagem.
// int width = 0;
// int height = 0;
// sscanf(line, "%d %d", &width, &height);

// fprintf(stdout, "%d %d\n", width, height);

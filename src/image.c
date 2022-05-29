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

Image img_Load(const char * filePath, char mode) { // O(n^2)
	Image i = (Image) malloc(sizeof(struct image_t));
	i->mode = mode;

	switch (i->mode) {
		case 'M':
			i->representations.M = mimg_Load(filePath); // O(n^2)
		break;

		case 'G':
			i->representations.G = gimg_Load(filePath); // O(n^2)
		break;

		default:
			fprintf(stderr, "Unknown image mode: %c\n", i->mode);
			exit(EXIT_FAILURE);
	}

	return i;
}

void img_RemoveLines(Image i, int amount, char operator) {
	switch (i->mode) {
		case 'M':
			mimg_RemoveLines(i->representations.M, amount, operator);
		break;

		case 'G':
			gimg_RemoveLines(i->representations.G, amount, operator);
		break;

		default:
			fprintf(stderr, "Unknown image mode: %c\n", i->mode);
			exit(EXIT_FAILURE);
	}
}

void img_RemoveColumns(Image i, int amount, char operator) {
	switch (i->mode) {
		case 'M':
			mimg_RemoveColumns(i->representations.M, amount, operator);
		break;

		case 'G':
			gimg_RemoveColumns(i->representations.G, amount, operator);
		break;

		default:
			fprintf(stderr, "Unknown image mode: %c\n", i->mode);
			exit(EXIT_FAILURE);
	}
}

void img_RemoveLinesAndColumns(Image i, int amountLines, int amountColumns, char operator) {
	switch (i->mode) {
		case 'M':
			mimg_RemoveLinesAndColumns(i->representations.M, amountLines, amountColumns, operator);
		break;

		case 'G':
			gimg_RemoveLinesAndColumns(i->representations.G, amountLines, amountColumns, operator);
		break;

		default:
			fprintf(stderr, "Unknown image mode: %c\n", i->mode);
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
			fprintf(stderr, "Unknown image mode: %c\n", i->mode);
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
			fprintf(stderr, "Unknown image mode: %c\n", i->mode);
			exit(EXIT_FAILURE);
	}
	free(i);
}


#include "img_matrix.h"
#include "pixel.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

struct img_matrix_t {
	int width, height;	
	int maxComponentValue;
	pixel * matrix;
};

void mimg_SetPixel(int x, int y, pixel p, void * miPtr) {
	MImage mi = (MImage)(miPtr);

	int index = (x * mi->width) + y;

	mi->matrix[index].r = p.r;
	mi->matrix[index].g = p.g;
	mi->matrix[index].b = p.b;

	mi->matrix[index].li = LI(&p);
}

MImage mimg_Load(const char * filePath) {
	MImage mi = (MImage) malloc(sizeof(struct img_matrix_t));

	ppm_GetProperties(filePath, &mi->width, &mi->height, &mi->maxComponentValue);

	mi->matrix = (pixel *) malloc((mi->width * mi->height) * sizeof(pixel));

	ppm_ForEachPixel(filePath, mimg_SetPixel, mi);

	return mi;
}

void mimg_RemoveLines(MImage mi, int amount) {
	printf("Message from remove lines matrix version.\n");
}

void mimg_RemoveColumns(MImage mi, int amount) {
	printf("Message from remove columns matrix version.\n");
}

void mimg_Save(MImage mi, const char * fileName) { 
	FILE * f = fopen(fileName, "w");

	fprintf(f, "P3\n");
	fprintf(f, "%d %d\n", mi->width, mi->height);
	fprintf(f, "%d\n", mi->maxComponentValue);
	
	for (int i = 0; i < mi->width * mi->height; i++) {
		pixel p = mi->matrix[i];
		fprintf(f, "%d %d %d\n", p.r, p.g, p.b);	
	}

	fclose(f);
}

void mimg_Free(MImage mi) {
	free(mi->matrix);
	free(mi);
}


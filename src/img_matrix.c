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

#define INDEX(x, y, lineSz) ((x * lineSz) + y)
//int INDEX(int x, int y, int lineSz) {
//	int index = (x * lineSz) + y;
//	return index;
//};


void mimg_SetPixel(int x, int y, pixel p, void * miPtr) {
	MImage mi = (MImage)(miPtr);

	int index = INDEX(x, y, mi->width);
	//printf("(%d, %d)[%d]\n", x, y, index);

	mi->matrix[index].r = p.r;
	mi->matrix[index].g = p.g;
	mi->matrix[index].b = p.b;

	px_CalculateLI(&mi->matrix[index]);
}

MImage mimg_Load(const char * filePath) {
	MImage mi = (MImage) malloc(sizeof(struct img_matrix_t));

	ppm_GetProperties(filePath, &mi->width, &mi->height, &mi->maxComponentValue);

	mi->matrix = (pixel *) malloc((mi->width * mi->height) * sizeof(pixel));

	ppm_ForEachPixel(filePath, mimg_SetPixel, mi);

	return mi;
}

void mimg_RemoveLines(MImage mi, int amount) {
	mimg_CalculateEnergies(mi);

	printf("Message from remove lines matrix version.\n");
}

void mimg_CalculateEnergies(MImage mi) {
	//for (int x = 0; x < mi->width; x++) {
	//	for (int y = 0; y < mi->height; y++) {
	//		mimg_CalculateEnergy(mi, x, y);
	//	}
	//}
	mimg_CalculateEnergy(mi, 0, 0);
}

int mimg_GetNextX(MImage mi, int current) {
	if (current == mi->height - 1) return current;
	else return current + 1;
}

int mimg_GetPreviousX(MImage mi, int current) {
	if (current == 0) return 0;
	else return current - 1;
}

int mimg_GetNextY(MImage mi, int current) {
	if (current == mi->width - 1) return current;
	else return current + 1;
}

int mimg_GetPreviousY(MImage mi, int current) {
	if (current == 0) return current;
	else return current - 1;
}

void mimg_CalculateEnergy(MImage mi, int x, int y) {
	int xPrevious = mimg_GetPreviousX(mi, x);
	int xNext = mimg_GetNextX(mi, x);

	int yPrevious = mimg_GetPreviousY(mi, y);
	int yNext = mimg_GetNextY(mi, y);

	printf("Pairs:\n");
	printf("(%d, %d) (%d, %d) (%d, %d)\n", xPrevious, yPrevious, xPrevious, y, xPrevious, yNext);
	printf("(%d, %d) (%d, %d) (%d, %d)\n", x,  		  yPrevious, x,  		y, x,  		  yNext);
	printf("(%d, %d) (%d, %d) (%d, %d)\n", xNext, 	  yPrevious, xNext, 	y, xNext, 	  yNext);

	int index = INDEX(x, y, mi->width);
	int indexT = INDEX(xPrevious, y, mi->width);
	int indexB = INDEX(xNext, y, mi->width);
	int indexR = INDEX(x, yNext, mi->width);
	int indexL = INDEX(x, yPrevious, mi->width);
	int indexTL = INDEX(xPrevious, yPrevious, mi->width);
	int indexBL = INDEX(xNext, yPrevious, mi->width);
	int indexTR = INDEX(xPrevious, yNext, mi->width);
	int indexBR = INDEX(xNext, yNext, mi->width);

	float region[3][3] = {
		{ mi->matrix[indexTL].li, mi->matrix[indexT].li, mi->matrix[indexTR].li },
		{ mi->matrix[indexL].li,  mi->matrix[index].li,  mi->matrix[indexR].li  },
		{ mi->matrix[indexBL].li, mi->matrix[indexB].li, mi->matrix[indexBR].li },
	};

	printf("LI's (region):\n");
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			printf("%f ", region[i][j]);
		}
		printf("\n");
	}

	mi->matrix[INDEX(x, y, mi->width)].energy = px_Sobel(region);
	printf("energy(%d, %d) = %f\n", x, y, mi->matrix[INDEX(x, y, mi->width)].energy);
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


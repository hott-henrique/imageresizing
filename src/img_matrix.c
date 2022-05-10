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

int INDEX(MImage mi, int x, int y) {
	int index = (x * mi->width) + y;
	return index;
};


void mimg_SetPixel(int x, int y, pixel p, void * miPtr) {
	MImage mi = (MImage)(miPtr);

	int index = INDEX(mi, x, y);
	//printf("(%d, %d)[%d]\n", x, y, index);

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
	mimg_CalculateEnergies(mi);

	printf("Message from remove lines matrix version.\n");
}

void mimg_CalculateEnergies(MImage mi) {
	for (int x = 0; x < mi->width; x++) {
		for (int y = 0; y < mi->height; y++) {
			int index = (mi->width * x) + y;
			mi->matrix[index].energy = mimg_CalculateEnergy(mi, x, y);
		}
	}
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

float mimg_CalculateEnergy(MImage mi, int x, int y) {
	// Right adjacent
	int xR = x; int yR = mimg_GetNextY(mi, y);

	// Left adjacent
	int xL = x; int yL = mimg_GetPreviousY(mi, y);

	// Top adjacent
	int xT = mimg_GetPreviousX(mi, x); int yT = y;

	// Bottom adjacent
	int xB = mimg_GetNextX(mi, x); int yB = y;

	// Top left adjacent
	int xTL = mimg_GetPreviousX(mi, x); int yTL = mimg_GetPreviousY(mi, y);

	// Top right adjacent
	int xTR = mimg_GetPreviousX(mi, x); int yTR = mimg_GetNextY(mi, y);

	// Bottom left adjacent
	int xBL = mimg_GetNextX(mi, x); int yBL = mimg_GetPreviousY(mi,y);

	// Bottom right adjacent
	int xBR = mimg_GetNextX(mi, x); int yBR = mimg_GetNextY(mi, y);

	printf("Pairs:\n");
	printf("(%d, %d) (%d, %d) (%d, %d)\n", xTL, yTL, xT, yT, xTR, yTR);
	printf("(%d, %d) (%d, %d) (%d, %d)\n", xL,  yL,  x,  y,  xR,  yR);
	printf("(%d, %d) (%d, %d) (%d, %d)\n", xBL, yBL, xB, yB, xBR, yBR);

	int index = INDEX(mi, x, y);
	int indexT = INDEX(mi, xT, yT);
	int indexB = INDEX(mi, xB, yB);
	int indexR = INDEX(mi, xR, yR);
	int indexL = INDEX(mi, xL, yL);
	int indexTL = INDEX(mi, xTL, yTL);
	int indexBL = INDEX(mi, xBL, yBL);
	int indexTR = INDEX(mi, xTR, yTR);
	int indexBR = INDEX(mi, xBR, yBR);

	//printf("MAX INDEX: %d\n", (mi->width * mi->height) - 1);
	//printf("%d %d %d\n", indexTL, indexT,indexTR);
	//printf("%d %d %d\n", indexL, index, indexR);
	//printf("%d %d %d\n", indexBL, indexB, indexBR);

	pixel region[3][3] = {
		{ mi->matrix[indexTL], mi->matrix[indexT], mi->matrix[indexTR] },
		{ mi->matrix[indexL],  mi->matrix[index],  mi->matrix[indexR] },
		{ mi->matrix[indexBL], mi->matrix[indexB], mi->matrix[indexBR] },
	};

	printf("Pixels:\n");
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			printf("(%d, %d, %d) ", region[i][j].r, region[i][j].g, region[i][j].b);
		}
		printf("\n");
	}

	return 0.0f;
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


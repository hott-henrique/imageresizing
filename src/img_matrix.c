#include "img_matrix.h"
#include "pixel.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/**
 * matrix:
 * 		x = lines (height)
 *		y = columns (width)
 * */

static void mimg_SetPixel(int x, int y, pixel p, void * miPtr);

static void mimg_Transpose(MImage mi);

static int mimg_GetBestPath(MImage mi);
static void mimg_CalculatePaths(MImage mi);
static float mimg_CalculatePathOfPixel(MImage mi, int x, int y);
static void mimg_RemovePath(MImage mi, int index, int * start, int * end);
static void mimg_RemovePixel(MImage mi, int x, int y);

static void mimg_CalculateEnergies(MImage mi, int start, int end);
static void mimg_CalculateEnergy(MImage mi, int x, int y);

static int mimg_GetPreviousX(MImage mi, int current);
static int mimg_GetNextX(MImage mi, int current);

static int mimg_GetPreviousY(MImage mi, int current);
static int mimg_GetNextY(MImage mi, int current);

struct img_matrix_t {
	int width, height;
	int maxComponentValue;
	pixel * matrix;
	int countRemovedColumns;
};

#define INDEX(x, y, lineSz) ((x * lineSz) + y)
MImage mimg_Load(const char * filePath) {
	MImage mi = (MImage) malloc(sizeof(struct img_matrix_t));
	mi->countRemovedColumns = 0;

	ppm_GetProperties(filePath, &mi->width, &mi->height, &mi->maxComponentValue);

	mi->matrix = (pixel *) malloc((mi->width * mi->height) * sizeof(pixel));

	ppm_ForEachPixel(filePath, mimg_SetPixel, mi);

	return mi;
}

void mimg_SetPixel(int x, int y, pixel p, void * miPtr) {
	MImage mi = (MImage)(miPtr);

	int index = INDEX(x, y, mi->width);

	mi->matrix[index].r = p.r;
	mi->matrix[index].g = p.g;
	mi->matrix[index].b = p.b;

	px_CalculateLI(&mi->matrix[index]);

	mi->matrix[index].next = NOT_CHECKED_YET;
}


void mimg_RemoveLines(MImage mi, int amount) {
	printf("Function call: %s\n", __FUNCTION__);
	mimg_Transpose(mi); 

	// mi->countRemovedColumns -= amount; 
	// mi->countRemovedLines += amount; 

	mimg_RemoveColumns(mi, amount);

	mimg_Transpose(mi);
}

static void mimg_Transpose(MImage mi) {
	printf("Function call: %s\n", __FUNCTION__);
	int indexN, indexT, aux;

	pixel * matrix = (pixel *) malloc((mi->height * mi->width) * sizeof(pixel));

	for(int x = 0; x < mi->height; x++){
		for(int y = 0; y < mi->width; y++){
			indexN = INDEX(x, y, mi->width);
			indexT = INDEX(y, x, mi->height);
			matrix[indexT] = mi->matrix[indexN];
		}
	}

	aux = mi->width;
	mi->width = mi->height;
	mi->height = aux;

	free(mi->matrix);
	mi->matrix = matrix;
}

void mimg_RemoveColumns(MImage mi, int amount) {
	printf("Function call: %s\n", __FUNCTION__);

	mi->countRemovedColumns += amount;

	int start = 0;
	int end = mi->width;
	for (int i = 0; i < amount; i++) { // for each column
		mimg_CalculateEnergies(mi, start, end);

		//for (int x = 0; x < mi->height; x++) {
		//	for (int y = 0; y < mi->width; y++) {
		//		printf("%f ", mi->matrix[INDEX(x, y, mi->width)].energy);
		//	}
		//	printf("\n");
		//}

		mimg_CalculatePaths(mi);

		int index = mimg_GetBestPath(mi);

		mimg_RemovePath(mi, index, &start, &end);

		start = mimg_GetPreviousX(mi, start);
		end = mimg_GetNextX(mi, end);	
	}	
}

static void mimg_CalculateEnergies(MImage mi, int start, int end) {
	printf("Function call: %s\n", __FUNCTION__);

	for (int x = 0; x < mi->height; x++) {
		for (int y = start; y < end; y++) {
			mimg_CalculateEnergy(mi, x, y);
		}
	}
}

static void mimg_CalculateEnergy(MImage mi, int x, int y) {
	int xPrevious = mimg_GetPreviousX(mi, x);
	int xNext = mimg_GetNextX(mi, x);

	int yPrevious = mimg_GetPreviousY(mi, y);
	int yNext = mimg_GetNextY(mi, y);

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

	mi->matrix[INDEX(x, y, mi->width)].energy = px_Sobel(region);
}

static int mimg_GetBestPath(MImage mi) {
	printf("Function call: %s\n", __FUNCTION__);

	int minIndex = 0;
	for (int y = 1; y < mi->width; y++) {
		float minEnergy = mi->matrix[INDEX(0, minIndex, mi->width)].energyInThatPath;
		float currentEnergy = mi->matrix[INDEX(0, y, mi->width)].energyInThatPath;

		if (currentEnergy < minEnergy) {
			minIndex = y;
		}
	}

	return minIndex;
}

static void mimg_CalculatePaths(MImage mi) {
	printf("Function call: %s\n", __FUNCTION__);
	for (int y = 0; y < mi->width; y++) {
		float result = mimg_CalculatePathOfPixel(mi, 0, y);

		//printf("Path value from column %d = %f\n", y, result);
	}

	//for (int x = 0; x < mi->height; x++) {
	//	for (int y = 0; y < mi->width; y++) {
	//		printf("%i\t", mi->matrix[INDEX(x, y, mi->width)].next);
	//	}
	//	printf("\n");
	//}

}

static float mimg_CalculatePathOfPixel(MImage mi, int x, int y) {
	//printf("Function call: %s\n", __FUNCTION__);

	int index = INDEX(x, y, mi->width);
	Pixel p = &mi->matrix[index];

	if (p->next != NOT_CHECKED_YET) return p->energyInThatPath;

	if (x == mi->height - 1) return p->energy;

	int yPrevious = mimg_GetPreviousY(mi, y);
	int yNext = mimg_GetNextY(mi, y);

	int xNext = mimg_GetNextX(mi, x);

	int minIndex = 0;
	float cheapestPath = 0.0f;
	for (int j = 0, i = yPrevious; i <= yNext; i++, j++) {
		float pathCost = mimg_CalculatePathOfPixel(mi, xNext, i);

		if (i == yPrevious || pathCost < cheapestPath) {
			minIndex = j;
			cheapestPath = pathCost;
		}
	}

	short paths[3] = { LEFT, CENTER, RIGHT };

	p->next = paths[minIndex];

	p->energyInThatPath = p->energy + cheapestPath;

	return p->energyInThatPath;
}

// start represents the most left column that had a pixel removed.
// end represents the most right column that had a pixel removed.
// y represents the column that have the best path.
static void mimg_RemovePath(MImage mi, int y, int * start, int * end) {
	printf("Function call: %s\n", __FUNCTION__);
	*(start) = y;
	*(end) = y;

	for (int x = 0; x < mi->height; x++) {
		short pathToFollow = mi->matrix[INDEX(x, y, mi->width)].next;	
		
		mimg_RemovePixel(mi, x, y);

		switch (pathToFollow) {
			case LEFT:
				y = mimg_GetPreviousY(mi, y);
				*(start) = y;
			break;

			case CENTER: continue;

			case RIGHT:
				y = mimg_GetNextY(mi, y);
				*(end) = y; 
			break;
		}
	}
}

static void mimg_RemovePixel(MImage mi, int x, int y) {
	//printf("Removing(%d, %d)\n", x, y);

	int index = INDEX(x, y, mi->width);

	if (index == mi->width * mi->height - 1) return;

	Pixel dest = &mi->matrix[index];
	Pixel src = &mi->matrix[index + 1];
	size_t n = mi->width - y - 1;

	memcpy(dest, src, sizeof(pixel) * n);
}

static int mimg_GetNextX(MImage mi, int current) {
	if (current == mi->height - 1) return current;
	else return current + 1;
}

static int mimg_GetPreviousX(MImage mi, int current) {
	if (current == 0) return 0;
	else return current - 1;
}

static int mimg_GetNextY(MImage mi, int current) {
	if (current == mi->width - 1) return current;
	else return current + 1;
}

static int mimg_GetPreviousY(MImage mi, int current) {
	if (current == 0) return current;
	else return current - 1;
}

void mimg_Save(MImage mi, const char * fileName) { 
	printf("Function call: %s\n", __FUNCTION__);

	FILE * f = fopen(fileName, "w");

	fprintf(f, "P3\n");
	fprintf(f, "%d %d\n", mi->width - mi->countRemovedColumns, mi->height);
	fprintf(f, "%d\n", mi->maxComponentValue);

	for (int x = 0; x < mi->height; x++) {
		for (int y = 0; y < mi->width - mi->countRemovedColumns; y++) {
			// printf("matrix(%d, %d)[%d]\n", x, y, INDEX(x, y, mi->width));
			pixel p = mi->matrix[INDEX(x, y, mi->width)];
			fprintf(f, "%d %d %d\n", p.r, p.g, p.b);	
		}
	}

	fclose(f);
}

void mimg_Free(MImage mi) {
	printf("Function call: %s\n", __FUNCTION__);

	free(mi->matrix);
	free(mi);
}


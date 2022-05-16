#include "img_matrix.h"
#include "pixel.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* *
 * Matrix:
 * 		x = rows (height)
 *		y = columns (width)
 * */

struct img_matrix_t {
	int currentWidth, currentHeight;
	int allocatedWidth, allocatedHeight;
	int maxComponentValue;
	pixel * matrix;
};

#define INDEX(x, y, rowSize) ((x * rowSize) + y)

static void mimg_SetPixel(int x, int y, pixel p, void * miPtr);

static void mimg_Transpose(MImage mi);

static int mimg_GetBestPath(MImage mi);

static void mimg_SetAllPathsNotChecked(MImage mi);

static void mimg_CalculatePaths(MImage mi);
static float mimg_CalculatePathOfPixel(MImage mi, int x, int y);

static void mimg_RemovePath(MImage mi, int index, int * outStart, int * outEnd);
static void mimg_RemovePixel(MImage mi, int x, int y);

static void mimg_SetPathToPink(MImage mi, int index);

static void mimg_CalculateEnergies(MImage mi, int start, int end);
static void mimg_CalculateEnergy(MImage mi, int x, int y);

static int mimg_GetPreviousX(MImage mi, int current);
static int mimg_GetNextX(MImage mi, int current);

static int mimg_GetPreviousY(MImage mi, int current);
static int mimg_GetNextY(MImage mi, int current);

MImage mimg_Load(const char * filePath) {
	MImage mi = (MImage) malloc(sizeof(struct img_matrix_t));

	ppm_GetProperties(filePath, &mi->allocatedWidth, &mi->allocatedHeight, &mi->maxComponentValue);

	mi->matrix = (Pixel) malloc((mi->allocatedWidth * mi->allocatedHeight) * sizeof(pixel));

	mi->currentWidth = mi->allocatedWidth;
	mi->currentHeight = mi->allocatedHeight;

	ppm_ForEachPixel(filePath, mimg_SetPixel, mi);

	return mi;
}

void mimg_SetPixel(int x, int y, pixel p, void * miPtr) {
	MImage mi = (MImage)(miPtr);

	int index = INDEX(x, y, mi->allocatedWidth);

	mi->matrix[index].r = p.r;
	mi->matrix[index].g = p.g;
	mi->matrix[index].b = p.b;

	px_CalculateLI(&mi->matrix[index]);

	mi->matrix[index].next = NOT_CHECKED_YET;
}

void mimg_RemoveLines(MImage mi, int amount) {
	mimg_Transpose(mi); 

	mimg_RemoveColumns(mi, amount);

	mimg_Transpose(mi);
}

static void mimg_Transpose(MImage mi) {
	Pixel newMatrix = (Pixel) malloc((mi->allocatedHeight * mi->allocatedWidth) * sizeof(pixel));

	for (int x = 0; x < mi->allocatedHeight; x++){
		for (int y = 0; y < mi->allocatedWidth; y++){
			int indexN = INDEX(x, y, mi->allocatedWidth);
			int indexT = INDEX(y, x, mi->allocatedHeight);
			newMatrix[indexT] = mi->matrix[indexN];
		}
	}

	int temp = 0;

	temp = mi->allocatedWidth;
	mi->allocatedWidth = mi->allocatedHeight;
	mi->allocatedHeight = temp;

	temp = mi->currentWidth;
	mi->currentWidth = mi->currentHeight;
	mi->currentHeight = temp;

	free(mi->matrix);

	mi->matrix = newMatrix;
}

void mimg_RemoveColumns(MImage mi, int amount) {
	int start = 0;
	int end = mi->allocatedWidth;

	for (int i = 0; i < amount; i++) {
		mimg_CalculateEnergies(mi, start, end);

		mimg_CalculatePaths(mi);

		int index = mimg_GetBestPath(mi);

		#if defined(SAVE_TEMPS) && defined(SAVE_FREQUENCY)
		if (i % SAVE_FREQUENCY  == 0) {
			char filePath[200];
			sprintf(filePath, "Remotion%d.ppm", i);
			mimg_SetPathToPink(mi, index);
			mimg_Save(mi, filePath);
		}
		#endif

		mimg_RemovePath(mi, index, &start, &end);

		start = mimg_GetPreviousX(mi, start);
		end = mimg_GetNextX(mi, end);	

		mi->currentWidth--;

		mimg_SetAllPathsNotChecked(mi);
	}	
}

static void mimg_CalculateEnergies(MImage mi, int start, int end) {
	for (int x = 0; x < mi->currentHeight; x++) {
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

	int index = INDEX(x, y, mi->allocatedWidth);
	int indexT = INDEX(xPrevious, y, mi->allocatedWidth);
	int indexB = INDEX(xNext, y, mi->allocatedWidth);
	int indexR = INDEX(x, yNext, mi->allocatedWidth);
	int indexL = INDEX(x, yPrevious, mi->allocatedWidth);
	int indexTL = INDEX(xPrevious, yPrevious, mi->allocatedWidth);
	int indexBL = INDEX(xNext, yPrevious, mi->allocatedWidth);
	int indexTR = INDEX(xPrevious, yNext, mi->allocatedWidth);
	int indexBR = INDEX(xNext, yNext, mi->allocatedWidth);

	float region[3][3] = {
		{ mi->matrix[indexTL].li, mi->matrix[indexT].li, mi->matrix[indexTR].li },
		{ mi->matrix[indexL].li,  mi->matrix[index].li,  mi->matrix[indexR].li  },
		{ mi->matrix[indexBL].li, mi->matrix[indexB].li, mi->matrix[indexBR].li },
	};

	mi->matrix[INDEX(x, y, mi->allocatedWidth)].energy = px_Sobel(region);
}

static int mimg_GetBestPath(MImage mi) {
	int minIndex = 0;
	for (int y = 1; y < mi->currentWidth; y++) {
		float minEnergy = mi->matrix[INDEX(0, minIndex, mi->allocatedWidth)].energyInThatPath;
		float currentEnergy = mi->matrix[INDEX(0, y, mi->allocatedWidth)].energyInThatPath;

		if (currentEnergy < minEnergy) {
			minIndex = y;
		}
	}

	return minIndex;
}

static void mimg_CalculatePaths(MImage mi) {
	for (int y = 0; y < mi->currentWidth; y++) {
		mimg_CalculatePathOfPixel(mi, 0, y);
	}
}

static float mimg_CalculatePathOfPixel(MImage mi, int x, int y) {
	int index = INDEX(x, y, mi->allocatedWidth);
	Pixel p = &mi->matrix[index];

	if (p->next != NOT_CHECKED_YET) return p->energyInThatPath;

	if (x == mi->currentHeight - 1) return p->energy;

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

	static short paths[3] = { LEFT, CENTER, RIGHT };

	p->next = paths[minIndex];

	p->energyInThatPath = p->energy + cheapestPath;

	return p->energyInThatPath;
}

static void mimg_SetPathToPink(MImage mi, int y) {
	for (int x = 0; x < mi->currentHeight; x++) {
		short pathToFollow = mi->matrix[INDEX(x, y, mi->allocatedWidth)].next;	
		
		Pixel p = &mi->matrix[INDEX(x, y, mi->allocatedWidth)];
		p->r = 255;
		p->g = 0;
		p->b = 255;

		switch (pathToFollow) {
			case LEFT:
				y = mimg_GetPreviousY(mi, y);
			break;

			case CENTER: continue;

			case RIGHT:
				y = mimg_GetNextY(mi, y);
			break;
		}
	}
}

static void mimg_RemovePath(MImage mi, int y, int * outStart, int * outEnd) {
	*(outStart) = y;
	*(outEnd) = y;

	for (int x = 0; x < mi->currentHeight; x++) {
		short pathToFollow = mi->matrix[INDEX(x, y, mi->allocatedWidth)].next;	
		
		mimg_RemovePixel(mi, x, y);

		switch (pathToFollow) {
			case LEFT:
				y = mimg_GetPreviousY(mi, y);
				*(outStart) = y;
			break;

			case CENTER: continue;

			case RIGHT:
				y = mimg_GetNextY(mi, y);
				*(outEnd) = y;
			break;
		}
	}
}

static void mimg_RemovePixel(MImage mi, int x, int y) {
	int index = INDEX(x, y, mi->allocatedWidth);

	if (index == mi->currentWidth * mi->currentHeight - 1) return;

	Pixel dest = &mi->matrix[index];
	Pixel src = &mi->matrix[index + 1];
	size_t n = mi->currentWidth - y - 1;

	memcpy(dest, src, sizeof(pixel) * n);
}

static int mimg_GetNextX(MImage mi, int current) {
	if (current == mi->currentHeight - 1) return current;
	else return current + 1;
}

static int mimg_GetPreviousX(MImage mi, int current) {
	if (current == 0) return 0;
	else return current - 1;
}

static int mimg_GetNextY(MImage mi, int current) {
	if (current == mi->currentWidth - 1) return current;
	else return current + 1;
}

static int mimg_GetPreviousY(MImage mi, int current) {
	if (current == 0) return current;
	else return current - 1;
}

static void mimg_SetAllPathsNotChecked(MImage mi) {
	for (int y = 0; y < mi->currentWidth; y++) {
		for (int x = 0; x < mi->currentHeight; x++) {
			int index = INDEX(x, y, mi->allocatedWidth);
			Pixel p = &mi->matrix[index];
			p->next = NOT_CHECKED_YET;
		}
	}
}

void mimg_Save(MImage mi, const char * fileName) { 
	FILE * f = fopen(fileName, "w");

	fprintf(f, "P3\n");
	fprintf(f, "%d %d\n", mi->currentWidth, mi->currentHeight);
	fprintf(f, "%d\n", mi->maxComponentValue);

	for (int x = 0; x < mi->currentHeight; x++) {
		for (int y = 0; y < mi->currentWidth; y++) {
			pixel p = mi->matrix[INDEX(x, y, mi->allocatedWidth)];
			fprintf(f, "%d %d %d\n", p.r, p.g, p.b);	
		}
	}

	fclose(f);
}

void mimg_Free(MImage mi) {
	free(mi->matrix);
	free(mi);
}


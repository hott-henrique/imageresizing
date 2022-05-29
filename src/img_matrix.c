#include "img_matrix.h"
#include "mlimits.h"
#include "pixel.h"
#include "ppm.h"
#include "timing.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* *
 * Matrix representation:
 * 		x = rows (height)
 *		y = columns (width)
 * */

extern FILE * timingstdout;

struct img_matrix_t {
	int currentWidth, currentHeight;
	int allocatedWidth, allocatedHeight;
	int maxComponentValue;
	Pixel matrix;
};

static void mimg_SetPixel(int x, int y, pixel p, void * miPtr);

static void mimg_Transpose(MImage mi);

static int mimg_GetBestPath(MImage mi);

static void mimg_SetAllPathsNotChecked(MImage mi);

static void mimg_CalculatePaths(MImage mi);
static float mimg_CalculatePathOfPixel(MImage mi, int x, int y);

static void mimg_RemovePath(MImage mi, int index, int * outStart, int * outEnd);
static void mimg_RemovePixel(MImage mi, int x, int y);

static void mimg_CalculateEnergies(MImage mi, int start, int end, short operator);
static void mimg_CalculateEnergy(MImage mi, int x, int y, short operator);
 
static float mimg_GetBestPossiblePathValue(MImage mi, short operator);

MImage mimg_Load(const char * filePath) { // O(n^2)
	MImage mi = (MImage) malloc(sizeof(struct img_matrix_t));

	ppm_GetProperties(filePath, &mi->allocatedWidth, &mi->allocatedHeight, &mi->maxComponentValue); // O(n)

	mi->matrix = (Pixel) malloc((mi->allocatedWidth * mi->allocatedHeight) * sizeof(pixel));

	mi->currentWidth = mi->allocatedWidth;
	mi->currentHeight = mi->allocatedHeight;

	ppm_ForEachPixel(filePath, mimg_SetPixel, mi); // O(n^2)

	return mi;
}

void mimg_SetPixel(int x, int y, pixel p, void * miPtr) { // O(1)
	MImage mi = (MImage)(miPtr);

	int index = INDEX(x, y, mi->allocatedWidth);

	Pixel px = &mi->matrix[index];

	px->r = p.r;
	px->g = p.g;
	px->b = p.b;

	px_CalculateLI(px); // O(1)

	px->next = NOT_CHECKED_YET;
}

void mimg_RemoveLinesAndColumns(MImage mi, int amountLines, int amountColumns, short operator) {
	while (amountLines != 0 && amountColumns != 0) {
		float columnBestPathValue = mimg_GetBestPossiblePathValue(mi, operator);

		mimg_Transpose(mi);

		float lineBestPathValue = mimg_GetBestPossiblePathValue(mi, operator);

		if (lineBestPathValue < columnBestPathValue) {
			mimg_RemoveColumns(mi, 1, operator);
			mimg_Transpose(mi);
			amountLines--;
		} else {
			mimg_Transpose(mi);
			mimg_RemoveColumns(mi, 1, operator);
			amountColumns--;
		}
	}

	// Any of these will be 0 so it will just return.
	mimg_RemoveLines(mi, amountLines, operator);
	mimg_RemoveColumns(mi, amountColumns, operator);
}

static float mimg_GetBestPossiblePathValue(MImage mi, short operator) {
	mimg_CalculateEnergies(mi, 0, mi->currentWidth - 1, operator);

	mimg_CalculatePaths(mi);

	int pathIndex = mimg_GetBestPath(mi);
	float pathValue = mi->matrix[INDEX(0, pathIndex, mi->currentWidth)].energyInThatPath;

	mimg_SetAllPathsNotChecked(mi);

	return pathValue;
}

void mimg_RemoveLines(MImage mi, int amount, short operator) { // max(O(n^2)O(n^4)) -> O(n^4)
	if (amount == 0) return;

	mimg_Transpose(mi);  // O(n^2)

	mimg_RemoveColumns(mi, amount, operator); // O(n^4)

	mimg_Transpose(mi);
}

static void mimg_Transpose(MImage mi) { // O(n^2)
#if defined(TIMING)
	timing t;
	t_Start(&t);
#endif
	Pixel newMatrix = (Pixel) malloc((mi->allocatedHeight * mi->allocatedWidth) * sizeof(pixel));

	for (int y = 0; y < mi->currentWidth; y++){
		for (int x = 0; x < mi->currentHeight; x++){
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
#if defined(TIMING)
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth * mi->currentHeight);
#endif
}

void mimg_RemoveColumns(MImage mi, int amount, short operator) { // O(n^4)
	int start = 0;
	int end = mi->currentWidth - 1;

	for (int i = 0; i < amount; i++) {
		mimg_CalculateEnergies(mi, start, end, operator); // O(n^2)

		mimg_CalculatePaths(mi); // O(n^3)

		int index = mimg_GetBestPath(mi); // O(n)

		mimg_RemovePath(mi, index, &start, &end); // O(n^2)

		start = ml_LimitedUMinus(start, 0); // O(1)
		end = ml_LimitedUPlus(end, mi->currentHeight - 1);

		mi->currentWidth--;

		mimg_SetAllPathsNotChecked(mi); // O(n^2)
	}	
}

static void mimg_CalculateEnergies(MImage mi, int start, int end, short operator) { // O(n^2)
	for (int y = start; y <= end; y++) {
		for (int x = 0; x < mi->currentHeight; x++) {
			mimg_CalculateEnergy(mi, x, y, operator);
		}
	}
}

static void mimg_CalculateEnergy(MImage mi, int x, int y, short operator) { // O(1)
	int xPrevious = ml_LimitedUMinus(x, 0);
	int xNext = ml_LimitedUPlus(x, mi->currentHeight - 1);

	int yPrevious = ml_LimitedUMinus(y, 0);
	int yNext = ml_LimitedUPlus(y, mi->currentWidth - 1);

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

	mi->matrix[INDEX(x, y, mi->allocatedWidth)].energy = px_CalculateEnergy(region, operator); // O(1)
}

static int mimg_GetBestPath(MImage mi) { // O(n)
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

static void mimg_CalculatePaths(MImage mi) { // O(n^3)
	for (int y = 0; y < mi->currentWidth; y++) {
		mimg_CalculatePathOfPixel(mi, 0, y); 
	}
}

static float mimg_CalculatePathOfPixel(MImage mi, int x, int y) { // {caso base: T(1) = 1; T(n-1) + 8} -> f(n) = 4n^2 + 33 -> O(n^2)
	int index = INDEX(x, y, mi->allocatedWidth);
	Pixel p = &mi->matrix[index];

	if (p->next != NOT_CHECKED_YET) return p->energyInThatPath; // 1

	if (x == mi->currentHeight - 1) { // 1
		p->energyInThatPath = p->energy;
		p->next = LAST_PIXEL;
	} else {
		int yPrevious = ml_LimitedUMinus(y, 0);
		int yNext = ml_LimitedUPlus(y, mi->currentWidth - 1);

		int xNext = ml_LimitedUPlus(x, mi->currentHeight - 1);

		int minIndex = 0;
		float cheapestPath = 0.0f;

		// If it is the first column, it starts analyzing from center.
		int nextIndex = yPrevious == 0 ? 1 : 0;

		// For each of the below adjacent.
		for (int i = yPrevious; i <= yNext; i++, nextIndex++) { // 6
			float pathCost = mimg_CalculatePathOfPixel(mi, xNext, i);

			if (i == yPrevious || pathCost < cheapestPath) {
				minIndex = nextIndex;
				cheapestPath = pathCost;
			}
		}

		static short paths[3] = { LEFT, CENTER, RIGHT };

		p->next = paths[minIndex];

		p->energyInThatPath = p->energy + cheapestPath;
	}

	return p->energyInThatPath;
}

static void mimg_RemovePath(MImage mi, int y, int * outStart, int * outEnd) {
	if (outStart != NULL) *(outStart) = y;
	if (outEnd != NULL) *(outEnd) = y;

	for (int x = 0; x < mi->currentHeight; x++) {
		short pathToFollow = mi->matrix[INDEX(x, y, mi->allocatedWidth)].next;	
		
		mimg_RemovePixel(mi, x, y); // O(n)

		switch (pathToFollow) {
			case LEFT:
				y = ml_LimitedUMinus(y, 0); // O(1)
				if (outStart != NULL) *(outStart) = y;
			break;

			case CENTER: continue;

			case RIGHT:
				y = ml_LimitedUPlus(y, mi->currentWidth - 1);
				if (outEnd != NULL) *(outEnd) = y;
			break;

			case LAST_PIXEL: break;
		}
	}
}

static void mimg_RemovePixel(MImage mi, int x, int y) { // O(n)
	int index = INDEX(x, y, mi->allocatedWidth);

	if (index == mi->currentWidth * mi->currentHeight - 1) return;

	Pixel dest = &mi->matrix[index];
	Pixel src = &mi->matrix[index + 1];
	size_t n = mi->currentWidth - y - 1;

	memcpy(dest, src, sizeof(pixel) * n); // O(n)
}

static void mimg_SetAllPathsNotChecked(MImage mi) { // O(n^2)
	for (int y = 0; y < mi->currentWidth; y++) {
		for (int x = 0; x < mi->currentHeight; x++) {
			int index = INDEX(x, y, mi->allocatedWidth);
			Pixel p = &mi->matrix[index];
			p->next = NOT_CHECKED_YET;
		}
	}
}

void mimg_Save(MImage mi, const char * fileName) { // O(n^2)
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

void mimg_Free(MImage mi) { // O(1)
	free(mi->matrix);
	free(mi);
}


#include "img_matrix.h"
#include "mlimits.h"
#include "pixel.h"
#include "ppm.h"

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

#if defined(SAVE_TEMPS) && defined(SAVE_FREQUENCY)
	static void mimg_SetPathToPink(MImage mi, int index);
#endif

static void mimg_CalculateEnergies(MImage mi, int start, int end);
static void mimg_CalculateEnergy(MImage mi, int x, int y);

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

	Pixel px = &mi->matrix[index];

	px->r = p.r;
	px->g = p.g;
	px->b = p.b;

	px_CalculateLI(px);

	px->next = NOT_CHECKED_YET;
}

void mimg_RemoveLinesAndColumns(MImage mi, int amountLines, int amountColumns) {
	while (amountLines != 0 && amountColumns != 0) {
		// Get best path column value.
		// Get best path line value.
		// if column value < line value: remove column && amountColumns--
		// else: remove line && amountLines--
	}

	// Any of these will be 0 so it will just return.
	mimg_RemoveLines(mi, amountLines);
	mimg_RemoveColumns(mi, amountColumns);
}

void mimg_RemoveLines(MImage mi, int amount) {
	if (amount == 0) return;

	mimg_Transpose(mi); 

	mimg_RemoveColumns(mi, amount);

	mimg_Transpose(mi);
}

static void mimg_Transpose(MImage mi) {
	Pixel newMatrix = (Pixel) malloc((mi->allocatedHeight * mi->allocatedWidth) * sizeof(pixel));

	for (int y = 0; y < mi->allocatedWidth; y++){
		for (int x = 0; x < mi->allocatedHeight; x++){
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
	int end = mi->allocatedWidth - 1;

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

		start = ml_LimitedUMinus(start, 0);
		end = ml_LimitedUPlus(end, mi->currentHeight - 1);

		mi->currentWidth--;

		mimg_SetAllPathsNotChecked(mi);
	}	
}

static void mimg_CalculateEnergies(MImage mi, int start, int end) {
	for (int y = start; y <= end; y++) {
		for (int x = 0; x < mi->currentHeight; x++) {
			mimg_CalculateEnergy(mi, x, y);
		}
	}
}

static void mimg_CalculateEnergy(MImage mi, int x, int y) {
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

	mi->matrix[INDEX(x, y, mi->allocatedWidth)].energy = px_CalculateEnergy(region);
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

	int yPrevious = ml_LimitedUMinus(y, 0);
	int yNext = ml_LimitedUPlus(y, mi->currentWidth - 1);

	int xNext = ml_LimitedUPlus(x, mi->currentHeight - 1);

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

#if defined(SAVE_TEMPS) && defined(SAVE_FREQUENCY)
static void mimg_SetPathToPink(MImage mi, int y) {
	for (int x = 0; x < mi->currentHeight; x++) {
		short pathToFollow = mi->matrix[INDEX(x, y, mi->allocatedWidth)].next;	
		
		Pixel p = &mi->matrix[INDEX(x, y, mi->allocatedWidth)];
		p->r = 255;
		p->g = 0;
		p->b = 255;

		switch (pathToFollow) {
			case LEFT:
				y = li_LimitedUMinus(y, 0);
			break;

			case CENTER: continue;

			case RIGHT:
				y = li_LimitedUPlus(y, mi->width - 1);
			break;
		}
	}
}
#endif

static void mimg_RemovePath(MImage mi, int y, int * outStart, int * outEnd) {
	*(outStart) = y;
	*(outEnd) = y;

	for (int x = 0; x < mi->currentHeight; x++) {
		short pathToFollow = mi->matrix[INDEX(x, y, mi->allocatedWidth)].next;	
		
		mimg_RemovePixel(mi, x, y);

		switch (pathToFollow) {
			case LEFT:
				y = ml_LimitedUMinus(y, 0);
				*(outStart) = y;
			break;

			case CENTER: continue;

			case RIGHT:
				y = ml_LimitedUPlus(y, mi->currentWidth - 1);
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

	for (int y = 0; y < mi->currentWidth; y++) {
		for (int x = 0; x < mi->currentHeight; x++) {
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


#include "img_matrix.h"

#include "pixel.h"
#include "ppm.h"
#include "mlimits.h"
#include "timing.h"

extern FILE * timingstdout;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* *
 * Matrix representation:
 * 		x = rows (height)
 *		y = columns (width)
 * */

enum PathOptions {
	NOT_CHECKED_YET = -2,
	LEFT = -1,
	CENTER = 0,
	RIGHT = 1,
	LAST_PIXEL = 2,
};

struct mpixel_t {
	pixel px;
	short next;
	float pathCost;
};

typedef struct mpixel_t mpixel;
typedef struct mpixel_t * MPixel;

struct img_matrix_t {
	int currentWidth, currentHeight;
	int allocatedWidth, allocatedHeight;
	int maxComponentValue;
	MPixel matrix;
};

static void mimg_SetPixel(int x, int y, pixel p, void * miPtr);

static void mimg_Transpose(MImage mi);

static int mimg_GetBestPath(MImage mi);

static void mimg_SetAllPathsNotChecked(MImage mi);

static void mimg_CalculatePaths(MImage mi);
static float mimg_CalculatePathOfPixel(MImage mi, int x, int y);

static void mimg_RemovePath(MImage mi, int index);
static void mimg_RemovePixel(MImage mi, int x, int y);

static void mimg_CalculateEnergies(MImage mi, char energyOp);
static void mimg_CalculateEnergy(MImage mi, int x, int y, char energyOp);
 
static float mimg_GetBestPossiblePathValue(MImage mi, char energyOp);

MImage mimg_Load(const char * filePath) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif

	MImage mi = (MImage) malloc(sizeof(struct img_matrix_t));

	ppm_GetProperties(filePath, &mi->allocatedWidth, &mi->allocatedHeight, &mi->maxComponentValue);

	mi->matrix = (MPixel) malloc((mi->allocatedWidth * mi->allocatedHeight) * sizeof(mpixel));

	mi->currentWidth = mi->allocatedWidth;
	mi->currentHeight = mi->allocatedHeight;

	ppm_ForEachPixel(filePath, mimg_SetPixel, mi);
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
	return mi;
}

void mimg_SetPixel(int x, int y, pixel p, void * miPtr) {
	MImage mi = (MImage)(miPtr);

	int index = INDEX(x, y, mi->allocatedWidth);

	MPixel mpx = &mi->matrix[index];

	mpx->px.r = p.r;
	mpx->px.g = p.g;
	mpx->px.b = p.b;

	px_CalculateLI(&mpx->px);

	mpx->next = NOT_CHECKED_YET;
}

void mimg_RemoveLinesAndColumns(MImage mi, int amountLines, int amountColumns, char energyOp) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	while (amountLines != 0 && amountColumns != 0) {
		float columnBestPathValue = mimg_GetBestPossiblePathValue(mi, energyOp);

		mimg_Transpose(mi);

		float lineBestPathValue = mimg_GetBestPossiblePathValue(mi, energyOp);

		if (lineBestPathValue < columnBestPathValue) {
			mimg_RemoveColumns(mi, 1, energyOp);
			mimg_Transpose(mi);
			amountLines--;
		} else {
			mimg_Transpose(mi);
			mimg_RemoveColumns(mi, 1, energyOp);
			amountColumns--;
		}
	}

	// Any of these will be 0 so it will just return.
	mimg_RemoveLines(mi, amountLines, energyOp);
	mimg_RemoveColumns(mi, amountColumns, energyOp);
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
}

static float mimg_GetBestPossiblePathValue(MImage mi, char energyOp) {
#ifdef TIMING
	timing t = { 0 };
	t_Start(&t);
#endif
	mimg_CalculateEnergies(mi, energyOp);

	mimg_CalculatePaths(mi);

	int pathIndex = mimg_GetBestPath(mi);
	float pathValue = mi->matrix[INDEX(0, pathIndex, mi->currentWidth)].pathCost;

	mimg_SetAllPathsNotChecked(mi);

#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
	return pathValue;
}

void mimg_RemoveLines(MImage mi, int amount, char energyOp) {
#ifdef TIMING
	timing t = { 0 };
	t_Start(&t);
#endif
	if (amount == 0) return;

	mimg_Transpose(mi);

	mimg_RemoveColumns(mi, amount, energyOp);

	mimg_Transpose(mi);
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
}

static void mimg_Transpose(MImage mi) {
#if defined(TIMING)
	timing t;
	t_Start(&t);
#endif
	MPixel newMatrix = (MPixel) malloc((mi->allocatedHeight * mi->allocatedWidth) * sizeof(mpixel));

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
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
}

void mimg_RemoveColumns(MImage mi, int amount, char energyOp) {
#if defined(TIMING)
	timing t = { 0 };
	t_Start(&t);
#endif
	for (int i = 0; i < amount; i++) {
		mimg_CalculateEnergies(mi, energyOp);

		mimg_CalculatePaths(mi);

		int index = mimg_GetBestPath(mi);

		mimg_RemovePath(mi, index);

		mi->currentWidth--;

		mimg_SetAllPathsNotChecked(mi);
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
}

static void mimg_CalculateEnergies(MImage mi, char energyOp) {
#ifdef TIMING
	timing t = { 0 };
	t_Start(&t);
#endif
	for (int y = 0; y < mi->currentWidth; y++) {
		for (int x = 0; x < mi->currentHeight; x++) {
			mimg_CalculateEnergy(mi, x, y, energyOp);
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
}

static void mimg_CalculateEnergy(MImage mi, int x, int y, char energyOp) {
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
		{ mi->matrix[indexTL].px.li, mi->matrix[indexT].px.li, mi->matrix[indexTR].px.li },
		{ mi->matrix[indexL].px.li,  mi->matrix[index].px.li,  mi->matrix[indexR].px.li  },
		{ mi->matrix[indexBL].px.li, mi->matrix[indexB].px.li, mi->matrix[indexBR].px.li },
	};

	mi->matrix[INDEX(x, y, mi->allocatedWidth)].px.energy = px_CalculateEnergy(region, energyOp);
}

static int mimg_GetBestPath(MImage mi) {
#ifdef TIMING
	timing t = { 0 };
	t_Start(&t);
#endif
	int minIndex = 0;
	for (int y = 1; y < mi->currentWidth; y++) {
		float minEnergy = mi->matrix[INDEX(0, minIndex, mi->allocatedWidth)].pathCost;
		float currentEnergy = mi->matrix[INDEX(0, y, mi->allocatedWidth)].pathCost;

		if (currentEnergy < minEnergy) {
			minIndex = y;
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
	return minIndex;
}

static void mimg_CalculatePaths(MImage mi) {
#ifdef TIMING
	timing t = { 0 };
	t_Start(&t);
#endif
	for (int y = 0; y < mi->currentWidth; y++) {
		mimg_CalculatePathOfPixel(mi, 0, y); 
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
}

static float mimg_CalculatePathOfPixel(MImage mi, int x, int y) {
	int index = INDEX(x, y, mi->allocatedWidth);
	MPixel mpx = &mi->matrix[index];

	if (mpx->next != NOT_CHECKED_YET) return mpx->pathCost;

	if (x == mi->currentHeight - 1) {
		mpx->pathCost = mpx->px.energy;
		mpx->next = LAST_PIXEL;
	} else {
		int yPrevious = ml_LimitedUMinus(y, 0);
		int yNext = ml_LimitedUPlus(y, mi->currentWidth - 1);

		int xNext = ml_LimitedUPlus(x, mi->currentHeight - 1);

		int minIndex = 0;
		float cheapestPath = 0.0f;

		// If it is the first column, it starts analyzing from center.
		int nextIndex = yPrevious == 0 ? 1 : 0;

		// For each of the below adjacent.
		for (int i = yPrevious; i <= yNext; i++, nextIndex++) {
			float pathCost = mimg_CalculatePathOfPixel(mi, xNext, i);

			if (i == yPrevious || pathCost < cheapestPath) {
				minIndex = nextIndex;
				cheapestPath = pathCost;
			}
		}

		static short paths[3] = { LEFT, CENTER, RIGHT };

		mpx->next = paths[minIndex];

		mpx->pathCost = mpx->px.energy + cheapestPath;
	}
	return mpx->pathCost;
}

static void mimg_RemovePath(MImage mi, int y) {
#ifdef TIMING
	timing t = { 0 };
	t_Start(&t);
#endif
	for (int x = 0; x < mi->currentHeight; x++) {
		short pathToFollow = mi->matrix[INDEX(x, y, mi->allocatedWidth)].next;	
		
		mimg_RemovePixel(mi, x, y);

		switch (pathToFollow) {
			case LEFT:
				y = ml_LimitedUMinus(y, 0);
			break;

			case CENTER: continue;

			case RIGHT:
				y = ml_LimitedUPlus(y, mi->currentWidth - 1);
			break;

			case LAST_PIXEL: break;
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
}

static void mimg_RemovePixel(MImage mi, int x, int y) {
	int index = INDEX(x, y, mi->allocatedWidth);

	if (index == mi->currentWidth * mi->currentHeight - 1) return;

	MPixel dest = &mi->matrix[index];
	MPixel src = &mi->matrix[index + 1];
	size_t n = mi->currentWidth - y - 1;

	memcpy(dest, src, sizeof(mpixel) * n);
}

static void mimg_SetAllPathsNotChecked(MImage mi) {
#ifdef TIMING
	timing t = { 0 };
	t_Start(&t);
#endif
	for (int y = 0; y < mi->currentWidth; y++) {
		for (int x = 0; x < mi->currentHeight; x++) {
			int index = INDEX(x, y, mi->allocatedWidth);
			MPixel p = &mi->matrix[index];
			p->next = NOT_CHECKED_YET;
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
}

void mimg_Print(MImage mi, FILE * f) {
#ifdef TIMING
	timing t = { 0 };
	t_Start(&t);
#endif
	fprintf(f, "P3\n");
	fprintf(f, "%d %d\n", mi->currentWidth, mi->currentHeight);
	fprintf(f, "%d\n", mi->maxComponentValue);

	for (int x = 0; x < mi->currentHeight; x++) {
		for (int y = 0; y < mi->currentWidth; y++) {
			pixel p = mi->matrix[INDEX(x, y, mi->allocatedWidth)].px;
			fprintf(f, "%d %d %d\n", p.r, p.g, p.b);	
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, mi->currentWidth);
#endif
}

void mimg_Free(MImage mi) {
	free(mi->matrix);
	free(mi);
}


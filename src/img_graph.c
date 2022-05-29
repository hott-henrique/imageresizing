#include "img_graph.h"

#include "pixel.h"
#include "ppm.h"
#include "mlimits.h"
#include "timing.h"
extern FILE * timingstdout;

#include <stdlib.h>
#include <stdio.h>

struct vpixel_t {
	pixel px;
	struct vpixel_t ** adjacent;
};

typedef struct vpixel_t vpixel;
typedef vpixel * VPixel;

struct img_graph_t {
	int currentWidth, currentHeight; 
	int allocatedWidth, allocatedHeight; 
	int maxComponentValue;
	VPixel vpixels;
};

enum AdjacentIndices {
	BL = 0,
	B = 1,
	BR = 2,
	R = 3,
	TR = 4,
	T = 5,
	TL = 6,
	L = 7,
};

static void gimg_SetPixel(int x, int y, pixel p, void * giPtr);

static void gimg_SetAllReferences(GImage gi);
static void gimg_SetVPixelReferences(GImage gi, int x, int y);

static void gimg_Transpose(GImage gi);

static void gimg_CalculateEnergies(GImage gi, char operator);
static void gimg_CalculateEnergy(GImage gi, int x, int y, char operator);

static void gimg_CalculatePaths(GImage gi);
static void gimg_CalculateAllPathsInLine(GImage gi, int x);
static void gimg_CalculatePathOfPixel(GImage, int x, int y);

static int gimg_GetBestPath(GImage gi);

static void gimg_RemovePath(GImage gi, int index);
static void gimg_RemovePixel(GImage gi, int x, int y);

static float gimg_GetBestPossiblePathValue(GImage gi, char operator);

GImage gimg_Load(const char * filePath) { // max(O(n)O(n^2)) -> O(n^2)
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif

	GImage gi = (GImage) malloc(sizeof(struct img_graph_t)); 

	ppm_GetProperties(filePath, &gi->allocatedWidth, &gi->allocatedHeight, &gi->maxComponentValue); // O(n)

	gi->vpixels = (VPixel) malloc((gi->allocatedWidth * gi->allocatedHeight) * sizeof(vpixel));

	gi->currentWidth = gi->allocatedWidth;
	gi->currentHeight = gi->allocatedHeight;

	for (int x = 0; x < gi->allocatedHeight; x++) { // O(n^2)
		for (int y = 0; y < gi->allocatedWidth; y++) {
			int index = INDEX(x, y, gi->allocatedWidth);

			VPixel vp = &gi->vpixels[index];
			vp->adjacent = (VPixel *) malloc(sizeof(VPixel) * 8);
		}
	}

	ppm_ForEachPixel(filePath, gimg_SetPixel, gi); // O(n^2)

	gimg_SetAllReferences(gi); // O(n^2)
											   //
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
	return gi;
}

static void gimg_SetPixel(int x, int y, pixel p, void * giPtr) { // O(1)
	GImage gi = (GImage)(giPtr);

	int index = INDEX(x, y, gi->allocatedWidth);	

	Pixel px = &gi->vpixels[index].px;

	px->r = p.r;
	px->g = p.g;
	px->b = p.b;

	px_CalculateLI(px); // O(1)
	px->next = NOT_CHECKED_YET;
}

static void gimg_SetAllReferences(GImage gi) { // O(n^2)
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	for (int x = 0; x < gi->allocatedHeight; x++) {
		for (int y = 0; y < gi->allocatedWidth; y++) {
			gimg_SetVPixelReferences(gi, x, y);
		}
	}
											   //
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

static void gimg_SetVPixelReferences(GImage gi, int x, int y) { // O(1)
	int xPrevious = ml_LimitedUMinus(x, 0);
	int xNext = ml_LimitedUPlus(x, gi->currentHeight - 1);

	int yPrevious = ml_LimitedUMinus(y, 0);
	int yNext = ml_LimitedUPlus(y, gi->currentWidth - 1);

	int index = INDEX(x, y, gi->allocatedWidth);
	int indexT = INDEX(xPrevious, y, gi->allocatedWidth);
	int indexB = INDEX(xNext, y, gi->allocatedWidth);
	int indexR = INDEX(x, yNext, gi->allocatedWidth);
	int indexL = INDEX(x, yPrevious, gi->allocatedWidth);
	int indexTL = INDEX(xPrevious, yPrevious, gi->allocatedWidth);
	int indexBL = INDEX(xNext, yPrevious, gi->allocatedWidth);
	int indexTR = INDEX(xPrevious, yNext, gi->allocatedWidth);
	int indexBR = INDEX(xNext, yNext, gi->allocatedWidth);

	VPixel vpx = &gi->vpixels[index];

	vpx->adjacent[T] = &gi->vpixels[indexT];
	vpx->adjacent[B] = &gi->vpixels[indexB];
	vpx->adjacent[R] = &gi->vpixels[indexR];
	vpx->adjacent[L] = &gi->vpixels[indexL];
	vpx->adjacent[TL] = &gi->vpixels[indexTL];
	vpx->adjacent[BL] = &gi->vpixels[indexBL];
	vpx->adjacent[TR] = &gi->vpixels[indexTR];
	vpx->adjacent[BR] = &gi->vpixels[indexBR];
}

void gimg_RemoveLinesAndColumns(GImage gi, int amountLines, int amountColumns, char operator) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	while (amountLines != 0 && amountColumns != 0) {
		float columnBestPathValue = gimg_GetBestPossiblePathValue(gi, operator);

		gimg_Transpose(gi);

		float lineBestPathValue = gimg_GetBestPossiblePathValue(gi, operator);

		if (lineBestPathValue < columnBestPathValue) {
			gimg_RemoveColumns(gi, 1, operator);
			gimg_Transpose(gi);
			amountLines--;
		} else {
			gimg_Transpose(gi);
			gimg_RemoveColumns(gi, 1, operator);
			amountColumns--;
		}
	}

	gimg_RemoveLines(gi, amountLines, operator);
	gimg_RemoveColumns(gi, amountColumns, operator);
											   //
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

static float gimg_GetBestPossiblePathValue(GImage gi, char operator) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	gimg_CalculateEnergies(gi, operator);

	gimg_CalculatePaths(gi);

	int pathIndex = gimg_GetBestPath(gi);
	float pathValue = gi->vpixels[INDEX(0, pathIndex, gi->currentWidth)].px.energyInThatPath;
											   //
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
	return pathValue;
}

void gimg_RemoveLines(GImage gi, int amount, char operator) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	gimg_Transpose(gi);

	gimg_RemoveColumns(gi, amount, operator); // O(n^3)

	gimg_Transpose(gi);
											   //
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

static void gimg_Transpose(GImage gi) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	VPixel newVertices = (VPixel) malloc((gi->allocatedHeight * gi->allocatedWidth) * sizeof(vpixel));

	for (int y = 0; y < gi->allocatedWidth; y++) {
		for (int x = 0; x < gi->allocatedHeight; x++) {
			int indexN = INDEX(x, y, gi->allocatedWidth);
			int indexT = INDEX(y, x, gi->allocatedHeight);
			newVertices[indexT] = gi->vpixels[indexN];
		}
	}

	int temp = 0;

	temp = gi->allocatedWidth;
	gi->allocatedWidth = gi->allocatedHeight;
	gi->allocatedHeight = temp;

	temp = gi->currentWidth;
	gi->currentWidth = gi->currentHeight;
	gi->currentHeight = temp;

	for (int x = 0; x < gi->allocatedHeight; x++) {
		for (int y = 0; y < gi->allocatedWidth; y++) {
			int index = INDEX(x, y, gi->allocatedWidth);
			VPixel vp = &gi->vpixels[index];
			free(vp->adjacent);
		}
	}
	free(gi->vpixels);

	gi->vpixels = newVertices;

	for (int x = 0; x < gi->allocatedHeight; x++) {
		for (int y = 0; y < gi->allocatedWidth; y++) {
			int index = INDEX(x, y, gi->allocatedWidth);
			VPixel vp = &gi->vpixels[index];
			vp->adjacent = (VPixel *) malloc(sizeof(VPixel) * 8);
		}
	}

	gimg_SetAllReferences(gi);
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

void gimg_RemoveColumns(GImage gi, int amount, char operator) { // o(n^3) 
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	for (int i = 0; i < amount; i++) { // No pior caso, eu removo todas as colunas - 1 -> O(n) * max(O(n)O(n^2))
		gimg_CalculateEnergies(gi, operator);

		gimg_CalculatePaths(gi); // O(n^2)

		int index = gimg_GetBestPath(gi); // O(n)

		gimg_RemovePath(gi, index);

		gi->currentWidth--;
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif

}

static void gimg_CalculateEnergies(GImage gi, char operator) { // O(n^2)
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	for (int x = 0; x < gi->currentHeight; x++) {
		for (int y = 0; y < gi->currentWidth; y++) {
			gimg_CalculateEnergy(gi, x, y, operator); // O(1)
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif

}

static void gimg_CalculateEnergy(GImage gi, int x, int y, char operator) { // O(1)
	int index = INDEX(x, y, gi->allocatedWidth);
	VPixel center = &gi->vpixels[index];
	VPixel * adjacent = center->adjacent;

	float region[3][3] = {
		{ adjacent[TL]->px.li, adjacent[T]->px.li,  adjacent[TR]->px.li },
		{ adjacent[L]->px.li,  center->px.li, 		adjacent[R]->px.li  },
		{ adjacent[BL]->px.li, adjacent[B]->px.li, adjacent[BR]->px.li }
	};

	center->px.energy = px_CalculateEnergy(region, operator); // O(1)
}

static void gimg_CalculatePaths(GImage gi) { // O(n^2)
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	int currentHeight = gi->currentHeight - 1;
	for (int x = currentHeight; x >= 0; x--){ 
		gimg_CalculateAllPathsInLine(gi, x); // O(n)
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif

}

static void gimg_CalculateAllPathsInLine(GImage gi, int x) { // O(n)
//#ifdef TIMING
//	timing t;
//	t_Start(&t);
//#endif
	for (int y = 0; y < gi->currentWidth; y++) {
		gimg_CalculatePathOfPixel(gi, x, y); // O(1)
	}	
//#ifdef TIMING
//	t_Finalize(&t);
//	t_Print(&t, timingstdout, __func__, gi->currentWidth);
//#endif

}

static void gimg_CalculatePathOfPixel(GImage gi, int x, int y) { // O(1)
	int index = INDEX(x, y, gi->allocatedWidth);
	VPixel p = &gi->vpixels[index];

	int currentHeight = gi->currentHeight - 1;
	if (currentHeight == x) {
		p->px.energyInThatPath = p->px.energy;
		p->px.next = LAST_PIXEL;
		return;
	}

	int ginIndex;

	float cheapestPath = 0.0f;

	for (int adjacentIndex = BL; adjacentIndex <= BR; adjacentIndex++) {
		float pathCost = p->adjacent[adjacentIndex]->px.energyInThatPath;

		if (adjacentIndex == BL || pathCost < cheapestPath) {
			cheapestPath = pathCost;
			ginIndex = adjacentIndex;
		} 
	}

	static short paths[3] = { LEFT, CENTER, RIGHT };
	
	p->px.next = paths[ginIndex];

	p->px.energyInThatPath = cheapestPath + p->px.energy;
}

static int gimg_GetBestPath(GImage gi) { // O(n)
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	int ginIndex = 0;

	for (int y = 1; y < gi->currentWidth; y++) { // O(n)
		int indexMinEnergyAdjacent = INDEX(0, ginIndex, gi->allocatedWidth); 
		int indexCurrentEnergyAdjacent = INDEX(0, y, gi->allocatedWidth);

		float ginEnergy = gi->vpixels[indexMinEnergyAdjacent].px.energyInThatPath;
		float currentEnergy = gi->vpixels[indexCurrentEnergyAdjacent].px.energyInThatPath;

		if (currentEnergy < ginEnergy) {
			ginIndex = y;
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
	return ginIndex;
}

static void gimg_RemovePath(GImage gi, int y) { // O(n^2)
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	for (int x = 0; x < gi->currentHeight; x++) { // O(n*n) -> O(n^2)
		int index = INDEX(x, y, gi->allocatedWidth);
		VPixel p = &gi->vpixels[index];

		int pathToFollow = p->px.next;

		gimg_RemovePixel(gi, x, y); // O(n)

		switch (pathToFollow) {
			case LEFT:
				y = ml_LimitedUMinus(y, 0); // (1)
				break;

			case CENTER: continue;

			case RIGHT:
				y = ml_LimitedUPlus(y, gi->currentWidth - 1); // O(1)
				break;

			case LAST_PIXEL: break;
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

static void gimg_RemovePixel(GImage gi, int x, int y) { // O(n)
	for (int column = y; column < gi->currentWidth; column++) { 
		int index = INDEX(x, y, gi->allocatedWidth);
		VPixel p = &gi->vpixels[index];

		p->px.r = p->adjacent[3]->px.r;
		p->px.g = p->adjacent[3]->px.g;
		p->px.b = p->adjacent[3]->px.b;

		p->px.li = p->adjacent[3]->px.li;

		y++;
	}
}

void gimg_Print(GImage gi, FILE * f) { // O(n^2)
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	fprintf(f, "P3\n");
	fprintf(f, "%d %d\n", gi->currentWidth, gi->currentHeight);
	fprintf(f, "%d\n", gi->maxComponentValue);

	for (int x = 0; x < gi->currentHeight; x++) {
		for (int y = 0; y < gi->currentWidth; y++) {
			pixel p = gi->vpixels[INDEX(x, y, gi->allocatedWidth)].px;
			fprintf(f, "%d %d %d\n", p.r, p.g, p.b);	
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

void gimg_Free(GImage gi) { // O(n^2)
	for (int x = 0; x < gi->allocatedHeight; x++) {
		for (int y = 0; y < gi->allocatedWidth; y++) {
			int index = INDEX(x, y, gi->allocatedWidth);
			VPixel vp = &gi->vpixels[index];
			free(vp->adjacent);
		}
	}

	free(gi->vpixels);
	free(gi);
}


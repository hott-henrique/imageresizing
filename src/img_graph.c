#include "img_graph.h"
#include "pixel.h"
#include "ppm.h"
#include "mlimits.h"

#include <stdlib.h>
#include <stdio.h>

#define INDEX_ADJ(x) (x + 2)

// Path to remove a column:
// Load image in grath
// Calculate intensity of pixels
// Calculate energy of pixels
// For each width pixel, calculate the path to follow
// Remove pixel of line and go to next
// Remove all pixels of column

struct vpixel_t {
	pixel px;
	struct vpixel_t ** adjacent; // array of references (pointers).
};

typedef struct vpixel_t vpixel;
typedef vpixel * VPixel;

struct img_graph_t {
	int currentWidth, currentHeight;// The width and height arent fixeds 
	int allocatedWidth, allocatedHeight; 
	int maxComponentValue;
	VPixel vpixels; // array of pixels 
};

enum AdjacentIndices {
	TL = 6,
	T = 5,
	TR = 4,
	R = 3,
	BR = 2,
	B = 1,
	BL = 0,
	L = 7,
};

static void gimg_SetPixel(int x, int y, pixel p, void * giPtr);

static void gimg_SetAllReferences(GImage gi);
static void gimg_SetVPixelReferences(GImage gi, int x, int y);

static void gimg_Transpose(GImage gi);

static void gimg_CalculateEnergies(GImage gi, int start, int end);
static void gimg_CalculateEnergy(GImage gi, int x, int y);

//static void gimg_PaintPathToPink(GImage gi, int x, int y);

static void gimg_CalculatePaths(GImage gi);
static void gimg_CalculateAllPathsInLine(GImage gi, int x);
static void gmig_CalculatePathOfPixel(GImage, int x, int y);
static int gimg_GetBestPath(GImage gi);

//static void gicpy(GImage gi, int y, VPixel p, size_t n);

static void gimg_RemovePath(GImage gi, int index, int * start, int * end);
static void gimg_RemovePixel(GImage gi, int x, int y);

static void gimg_SetAllPathsNotChecked(GImage gi);

GImage gimg_Load(const char * filePath) {
	printf("Call: %s\n", __func__);

	GImage gi = (GImage) malloc(sizeof(struct img_graph_t));

	ppm_GetProperties(filePath, &gi->allocatedWidth, &gi->allocatedHeight, &gi->maxComponentValue);

	gi->vpixels = (VPixel) malloc((gi->allocatedWidth * gi->allocatedHeight) * sizeof(vpixel));

	gi->currentWidth = gi->allocatedWidth;
	gi->currentHeight = gi->allocatedHeight;

	for (int x = 0; x < gi->allocatedHeight; x++) {
		for (int y = 0; y < gi->allocatedWidth; y++) {
			int index = INDEX(x, y, gi->allocatedWidth);

			VPixel vp = &gi->vpixels[index];
			vp->adjacent = (VPixel *) malloc(sizeof(VPixel) * 8); // alocatted array size 8 of pixels adjacents
		}
	}

	ppm_ForEachPixel(filePath, gimg_SetPixel, gi);

	gimg_SetAllReferences(gi);

	//for (int x = 0; x < gi->currentHeight; x++) {
	//	for (int y = 0; y < gi->currentWidth; y++) {
	//		VPixel center = &gi->vpixels[INDEX(x, y, gi->allocatedWidth)];
	//		VPixel * adjacent = center->adjacent;
	//		printf("adjcent[TL] = (%d, %d, %d) ", adjacent[TL]->px.r, adjacent[TL]->px.g, adjacent[TL]->px.b);
	//		printf("adjcent[T] = (%d, %d, %d) ", adjacent[T]->px.r, adjacent[T]->px.g, adjacent[T]->px.b);
	//		printf("adjcent[TR] = (%d, %d, %d)\n", adjacent[TR]->px.r, adjacent[TR]->px.g, adjacent[TR]->px.b);
	//		printf("adjcent[L] = (%d, %d, %d) ", adjacent[L]->px.r, adjacent[L]->px.g, adjacent[L]->px.b);
	//		printf("center = (%d, %d, %d) ", center->px.r, center->px.g, center->px.b);
	//		printf("adjcent[R] = (%d, %d, %d)\n", adjacent[R]->px.r, adjacent[R]->px.g, adjacent[R]->px.b);
	//		printf("adjcent[BL] = (%d, %d, %d) ", adjacent[BL]->px.r, adjacent[BL]->px.g, adjacent[BL]->px.b);
	//		printf("adjcent[B] = (%d, %d, %d) ", adjacent[B]->px.r, adjacent[B]->px.g, adjacent[B]->px.b);
	//		printf("adjcent[BR] = (%d, %d, %d)\n", adjacent[BR]->px.r, adjacent[BR]->px.g, adjacent[BR]->px.b);
	//		printf("---------------------------------------------------------\n");
	//	}
	//}

	return gi;
}

static void gimg_SetPixel(int x, int y, pixel p, void * giPtr) {
	GImage gi = (GImage)(giPtr);

	int index = INDEX(x, y, gi->allocatedWidth);	

	Pixel px = &gi->vpixels[index].px;

	px->r = p.r;
	px->g = p.g;
	px->b = p.b;

	px_CalculateLI(px);
	px->next = NOT_CHECKED_YET;
}

static void gimg_SetAllReferences(GImage gi) {
	for (int x = 0; x < gi->allocatedHeight; x++) {
		for (int y = 0; y < gi->allocatedWidth; y++) {
			gimg_SetVPixelReferences(gi, x, y);
		}
	}
}

static void gimg_SetVPixelReferences(GImage gi, int x, int y) {
	int xPrevious = ml_LimitedUMinus(x, 0); // if x = 0 -> return itself; else -> return x - 1 = previous pixel
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

void gimg_RemoveLinesAndColumns(GImage gi, int amountLines, int amountColumns) {
	//printf("Call: %s\n", __func__);

}

void gimg_RemoveLines(GImage gi, int amount) {
	//printf("Call: %s\n", __func__);

	gimg_Transpose(gi);

	gimg_RemoveColumns(gi, amount);

	gimg_Transpose(gi);
}

static void gimg_Transpose(GImage gi) {
	//printf("Call: %s\n", __func__);

	
}

void gimg_RemoveColumns(GImage gi, int amount) {
	//printf("Call: %s\n", __func__);

	int start = 0;
	int end = gi->allocatedWidth - 1;

	for (int i = 0; i < amount; i++) {
		gimg_CalculateEnergies(gi, start, end);

		gimg_CalculatePaths(gi);

		int index = gimg_GetBestPath(gi);

		gimg_RemovePath(gi, index, &start, &end);

		//start = ml_LimitedUMinus(start, 0);
		//end = ml_LimitedUPlus(end, gi->currentWidth - 1);

		gi->currentWidth--;

		gimg_SetAllPathsNotChecked(gi);
	}
}

static void gimg_CalculateEnergies(GImage gi, int start, int end) {
	//printf("Call: %s\n", __func__);

	for (int y = start; y <= end; y++) {
		for (int x = 0; x < gi->currentHeight; x++) {
			gimg_CalculateEnergy(gi, x, y);
		}
	}
}

static void gimg_CalculateEnergy(GImage gi, int x, int y) {

	int index = INDEX(x, y, gi->allocatedWidth);
	VPixel center = &gi->vpixels[index];
	VPixel * adjacent = center->adjacent;

	float region[3][3] = {
		{ adjacent[TL]->px.li, adjacent[T]->px.li,  adjacent[TR]->px.li },
		{ adjacent[L]->px.li,  center->px.li, 		adjacent[R]->px.li  },
		{ adjacent[BL]->px.li, adjacent[B]->px.li, adjacent[BR]->px.li }
	};

	center->px.energy = px_CalculateEnergy(region);
}

static void gimg_CalculatePaths(GImage gi) {
	//printf("Call: %s\n", __func__);
	int currentHeight = gi->currentHeight - 1;
	for (int x = currentHeight; x >= 0; x--){ 
		gimg_CalculateAllPathsInLine(gi, x);
	}
}

static void gimg_CalculateAllPathsInLine(GImage gi, int x) {
	//printf("Call: %s\n", __func__);
	// Funct to calc path of each pixel
	for (int y = 0; y < gi->currentWidth; y++) {
		gmig_CalculatePathOfPixel(gi, x, y);
	}	

}

static void gmig_CalculatePathOfPixel(GImage gi, int x, int y) {
	//printf("Call: %s\n", __func__);

	int index = INDEX(x, y, gi->allocatedWidth);
	VPixel p = &gi->vpixels[index];

	int currentHeight = gi->currentHeight - 1;
	if (currentHeight == x) {
		p->px.energyInThatPath = p->px.energy;
		p->px.next = LAST_PIXEL;
		return;
	}

	int minIndex;

	float cheapestPath = 0.0f;

	for (int adjacentIndex = BL; adjacentIndex <= BR; adjacentIndex++) {
		float pathCost = p->adjacent[adjacentIndex]->px.energyInThatPath;

		if (adjacentIndex == BL || pathCost < cheapestPath) {
			cheapestPath = pathCost;
			minIndex = adjacentIndex;
		} 
	}

	static short paths[3] = { LEFT, CENTER, RIGHT };
	
	p->px.next = paths[minIndex];

	p->px.energyInThatPath = cheapestPath + p->px.energy;
}

static int gimg_GetBestPath(GImage gi) {
	//printf("Call: %s\n", __func__);

	int minIndex = 0;

	for (int y = 1; y < gi->currentWidth; y++) {
		int indexMinEnergyAdjacent = INDEX(0, minIndex, gi->allocatedWidth); 
		int indexCurrentEnergyAdjacent = INDEX(0, y, gi->allocatedWidth);

		float minEnergy = gi->vpixels[indexMinEnergyAdjacent].px.energyInThatPath;
		float currentEnergy = gi->vpixels[indexCurrentEnergyAdjacent].px.energyInThatPath;

		if (currentEnergy < minEnergy) {
			minIndex = y;
		}
	}

	return minIndex;

}

static void gimg_RemovePath(GImage gi, int y, int * outStart, int * outEnd) {
	//printf("Call: %s\n", __func__);
	
	*(outStart) = y;
	*(outEnd) = y;

	for (int x = 0; x < gi->currentHeight; x++) {

		int index = INDEX(x, y, gi->allocatedWidth);
		VPixel p = &gi->vpixels[index];

		int pathToFollow = p->px.next;

		gimg_RemovePixel(gi, x, y);
		//gimg_PaintPathToPink(gi, x, y);

		switch (pathToFollow) {
			case LEFT:
				y = ml_LimitedUMinus(y, 0);
				if (y < *(outStart)) {
					*(outStart) = y;
				}
				break;

			case LAST_PIXEL:
				break;
			case CENTER:	
				continue;

			case RIGHT:
				y = ml_LimitedUPlus(y, gi->currentWidth - 1);
				if (y > *(outEnd)) {
					*(outEnd) = y;
				}
				break;
		}

	}

}

// static void gimg_PaintPathToPink(GImage gi, int x, int y) {

// 	int index = INDEX(x, y, gi->allocatedWidth);
//  	VPixel p = &gi->vpixels[index];

// 	p->px.r = 255;
// 	p->px.r = 0;
// 	p->px.r = 255;
	
// }

static void gimg_RemovePixel(GImage gi, int x, int y) {
	//printf("Call: %s\n", __func__);

	size_t n = gi->currentWidth;

	//gicpy(GImage gi, int x, int y, size_t n); - Funct to copy grath
	for (size_t column = y; column < n; column++) {
		int index = INDEX(x, y, gi->allocatedWidth);
		VPixel p = &gi->vpixels[index];

		p->px.r = p->adjacent[3]->px.r;
		p->px.g = p->adjacent[3]->px.g;
		p->px.b = p->adjacent[3]->px.b;

		p->px.li = p->adjacent[3]->px.li;

		y++;
	}

}

static void gimg_SetAllPathsNotChecked(GImage gi) {
	//printf("Call: %s\n", __func__);

	for (int y = 0; y < gi->currentWidth; y++) {
		for (int x = 0; x < gi->currentHeight; x++) {
			int index = INDEX(x, y, gi->allocatedWidth);
			VPixel p = &gi->vpixels[index];
			p->px.next = NOT_CHECKED_YET;
		}
	}
}

void gimg_Save(GImage gi, const char * fileName) {
	printf("Call: %s\n", __func__);

	FILE * f = fopen(fileName, "w");

	fprintf(f, "P3\n");
	fprintf(f, "%d %d\n", gi->currentWidth, gi->currentHeight);
	fprintf(f, "%d\n", gi->maxComponentValue);

	for (int x = 0; x < gi->currentHeight; x++) {
		for (int y = 0; y < gi->currentWidth; y++) {
			pixel p = gi->vpixels[INDEX(x, y, gi->allocatedWidth)].px;
			fprintf(f, "%d %d %d\n", p.r, p.g, p.b);	
		}
	}

	fclose(f);
}

void gimg_Free(GImage gi) {
	printf("Call: %s\n", __func__);

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


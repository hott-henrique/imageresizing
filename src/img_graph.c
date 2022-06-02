#include "img_graph.h"

#include "pixel.h"
#include "ppm.h"
#include "mlimits.h"
#include "timing.h"
extern FILE * timingstdout;

#include <stdlib.h>
#include <stdio.h>
#include <float.h>

typedef struct vpixel_t vpixel;
typedef struct vpixel_t * VPixel;

struct vpixel_t {
	pixel px;

	VPixel * adjacent;
	int szAdjacent;

	float pathCost;
	VPixel previous;

	struct point_2d_t{
		int x, y;
	} position;
};

struct img_graph_t {
	int currentWidth, currentHeight; 
	int allocatedWidth, allocatedHeight; 
	int maxComponentValue;
	VPixel vpixels;
	vpixel startingPoint;
};

enum AdjacentIndices {
	TL = 6,	T = 5, TR = 4,
	L = 7, 			R = 3,
	BL = 0, B = 1, BR = 2,
};

static void gimg_AllocateStartingPoint(GImage gi);

static void gimg_SetPixel(int x, int y, pixel p, void * giPtr);

static void gimg_SetAllReferences(GImage gi);
static void gimg_SetVPixelReferences(GImage gi, int x, int y);

static void gimg_Transpose(GImage gi);

static void gimg_CalculateEnergies(GImage gi, char energyOp);
static void gimg_CalculateEnergy(GImage gi, int x, int y, char energyOp);

static void gimg_Djikstra(GImage gi, VPixel startingPoint);

static void gimg_RemovePath(GImage gi);
static void gimg_RemovePixel(GImage gi, int x, int y);

static float gimg_GetBestPossiblePathCost(GImage gi, char energyOp);

typedef struct heap_vertex_t {
	GImage img;
	int * positions;
	VPixel vertices;
	int size;
	int capacity;
} * HeapV;

HeapV hp_New(GImage gi);
void hp_Free(HeapV h);
void hp_Populate(HeapV h);
void hp_Insert(HeapV h, vpixel vpx);
vpixel hp_Pop(HeapV h);
void hp_Sort(HeapV h);
void hp_Swap(HeapV h, int indexA, int indexB);
void hp_UpdateVPixel(HeapV h, VPixel v, VPixel u);

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
			vp->szAdjacent = 3;
		}
	}

	ppm_ForEachPixel(filePath, gimg_SetPixel, gi); // O(n^2)

	gimg_SetAllReferences(gi); // O(n^2)

	gimg_AllocateStartingPoint(gi);
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
	return gi;
}

static void gimg_AllocateStartingPoint(GImage gi) {
	VPixel startingPoint = &gi->startingPoint;
	startingPoint->adjacent = (VPixel *) malloc(sizeof(VPixel) * gi->currentWidth);
	startingPoint->szAdjacent = gi->currentWidth;	

	startingPoint->position.x = -1;
	startingPoint->position.y = -1;

	startingPoint->px.energy = 0.0f;

	for (int y = 0; y < gi->currentWidth; y++) {
		int index =  INDEX(0, y, gi->currentWidth);
		startingPoint->adjacent[y] = &gi->vpixels[index];
	}
}

static void gimg_SetPixel(int x, int y, pixel p, void * giPtr) { // O(1)
	GImage gi = (GImage)(giPtr);

	int index = INDEX(x, y, gi->allocatedWidth);	

	VPixel vpx = &gi->vpixels[index];
	vpx->previous = NULL;
	vpx->position.x = x;
	vpx->position.y = y;

	Pixel px = &vpx->px;
	px->r = p.r;
	px->g = p.g;
	px->b = p.b;

	px_CalculateLI(px); // O(1)
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

void gimg_RemoveLinesAndColumns(GImage gi, int amountLines, int amountColumns, char energyOp) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	while (amountLines != 0 && amountColumns != 0) {
		float columnBestPathValue = gimg_GetBestPossiblePathCost(gi, energyOp);

		gimg_Transpose(gi);

		float lineBestPathValue = gimg_GetBestPossiblePathCost(gi, energyOp);

		if (lineBestPathValue < columnBestPathValue) {
			gimg_RemoveColumns(gi, 1, energyOp);
			gimg_Transpose(gi);
			amountLines--;
		} else {
			gimg_Transpose(gi);
			gimg_RemoveColumns(gi, 1, energyOp);
			amountColumns--;
		}
	}

	gimg_RemoveLines(gi, amountLines, energyOp);
	gimg_RemoveColumns(gi, amountColumns, energyOp);
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

static float gimg_GetBestPossiblePathCost(GImage gi, char energyOp) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	gimg_CalculateEnergies(gi, energyOp);

	gimg_Djikstra(gi, &gi->startingPoint);

	printf("Not implemented yet: %s\n", __func__);
	exit(0);
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
	return 0;
}

void gimg_RemoveLines(GImage gi, int amount, char energyOp) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	gimg_Transpose(gi);

	gimg_RemoveColumns(gi, amount, energyOp); // O(n^3)

	gimg_Transpose(gi);
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
			newVertices[indexT].position.x = x;
			newVertices[indexT].position.y = y;
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

	free(gi->startingPoint.adjacent);

	gi->vpixels = newVertices;

	for (int x = 0; x < gi->allocatedHeight; x++) {
		for (int y = 0; y < gi->allocatedWidth; y++) {
			int index = INDEX(x, y, gi->allocatedWidth);
			VPixel vp = &gi->vpixels[index];
			vp->adjacent = (VPixel *) malloc(sizeof(VPixel) * 8);
		}
	}

	gimg_SetAllReferences(gi);

	gimg_AllocateStartingPoint(gi);
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

void gimg_RemoveColumns(GImage gi, int amount, char energyOp) { // o(n^3) 
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	for (int i = 0; i < amount; i++) {
		gimg_CalculateEnergies(gi, energyOp);

		gimg_Djikstra(gi, &gi->startingPoint);

		gimg_RemovePath(gi);

		//gi->currentWidth--;
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

static void gimg_CalculateEnergies(GImage gi, char energyOp) { // O(n^2)
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	for (int x = 0; x < gi->currentHeight; x++) {
		for (int y = 0; y < gi->currentWidth; y++) {
			gimg_CalculateEnergy(gi, x, y, energyOp); // O(1)
		}
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

static void gimg_CalculateEnergy(GImage gi, int x, int y, char energyOp) { // O(1)
	int index = INDEX(x, y, gi->allocatedWidth);
	VPixel center = &gi->vpixels[index];
	VPixel * adjacent = center->adjacent;

	float region[3][3] = {
		{ adjacent[TL]->px.li, adjacent[T]->px.li,  adjacent[TR]->px.li },
		{ adjacent[L]->px.li,  center->px.li, 		adjacent[R]->px.li  },
		{ adjacent[BL]->px.li, adjacent[B]->px.li, adjacent[BR]->px.li }
	};

	center->px.energy = px_CalculateEnergy(region, energyOp); // O(1)
}

static void gimg_Djikstra(GImage gi, VPixel startingPoint) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	for (int x = 0; x < gi->currentHeight; x++) {
		for (int y = 0; y < gi->currentWidth; y++) {
			int index = INDEX(x, y, gi->allocatedWidth);
			VPixel v = &gi->vpixels[index];	
			v->pathCost = FLT_MAX;
			v->previous = NULL;
		}
	}

	startingPoint->pathCost = 0.0f;	
	startingPoint->previous = NULL;

	HeapV h = hp_New(gi);

	while (h->size > 0) {
		vpixel u = hp_Pop(h);

		if (u.position.x == gi->currentHeight - 1) continue;

		for (int i = 0; i < u.szAdjacent; i++) {
			VPixel v = u.adjacent[i];

			float newPathCost = u.pathCost + v->px.energy;

			if (v->pathCost > newPathCost) {
				hp_UpdateVPixel(h, v, &u);
			}
		}

		hp_Sort(h);
	}

	hp_Free(h);
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, gi->currentWidth);
#endif
}

static void gimg_RemovePath(GImage gi) { // O(n^2)
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif

	float minPathCost = 0;
	int minIndex = 0;
	int x = gi->currentHeight - 1;
	for (int y = 0; y < gi->currentWidth; y++) {
		int index = INDEX(x, y, gi->allocatedWidth);
		VPixel v = &gi->vpixels[index];	
		printf("%.1f ", v->pathCost);
		if (y == 0 || v->pathCost < minPathCost) {
			minPathCost = v->pathCost;
			minIndex = y;
		}
	}
	printf("\n");

	VPixel current = &gi->vpixels[INDEX(x, minIndex, gi->allocatedWidth)];
	while (current != NULL) {
		int xpx = current->position.x;
		int ypx = current->position.y;

		if (xpx == -1 || ypx == -1) break;

		//gimg_RemovePixel(gi, xpx, ypx);
		
		current->px.r = 0;
		current->px.g = 255;
		current->px.b = 0;

		current = current->previous;
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

		p->px.r = p->adjacent[R]->px.r;
		p->px.g = p->adjacent[R]->px.g;
		p->px.b = p->adjacent[R]->px.b;

		p->px.li = p->adjacent[R]->px.li;

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
	free(gi->startingPoint.adjacent);

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

HeapV hp_New(GImage gi) {
	HeapV h = (HeapV) malloc(sizeof(struct heap_vertex_t));

	h->img = gi;

	/* + 2: 1 because it's a heap, and the other is for the startingPoint. */
	int size = (gi->allocatedWidth * gi->allocatedHeight) + 2;
	h->capacity = size;

	h->vertices = (VPixel) malloc(sizeof(vpixel) * size);
	h->positions = (int *) malloc(sizeof(int) * size);
	for ( int i = 0; i < size; i++) {
		h->vertices[i].pathCost = FLT_MAX;
		h->positions[i] = 0;
	}
	h->size = 0;

	hp_Populate(h);
	return h;
}

void hp_Populate(HeapV h) {
	GImage gi = h->img;
	hp_Insert(h, gi->startingPoint);
	for (int x = 0; x < gi->currentHeight; x++) {
		for (int y = 0; y < gi->currentWidth; y++) {
			int index = INDEX(x, y, gi->currentWidth);
			hp_Insert(h, gi->vpixels[index]);
		}
	}
}

void hp_Insert(HeapV h, vpixel vpx) {
	h->size++;
	h->vertices[h->size] = vpx;

	if (vpx.position.x >= 0 && vpx.position.y >= 0) {
		int indexPos = INDEX(vpx.position.x, vpx.position.y, h->img->allocatedWidth);
		h->positions[indexPos] = h->size;
	}

	if (h->size == 1) return;

	hp_Sort(h);
}

vpixel hp_Pop(HeapV h) {
	vpixel v = h->vertices[1];
	
	vpixel vnew = h->vertices[h->size];
	h->vertices[1] = vnew;
	//h->positions[INDEX(vnew.position.x, vnew.position.y, h->img->allocatedWidth)] = 1;
	h->size--;

	hp_Sort(h);

	return v;
}

void hp_Sort(HeapV h) {
#ifdef TIMING
	timing t;
	t_Start(&t);
#endif
	int current = h->size / 2;
	for (int i = current; i >= 1; i--) {
		VPixel vpxCurrent = &h->vertices[i];

		int minIndex = 0;

		int firstChild = i * 2;
		VPixel vpxFC = &h->vertices[firstChild];

		if (firstChild < h->capacity &&
			vpxCurrent->pathCost > vpxFC->pathCost) {
			minIndex = firstChild;
		}

		int secondChild = firstChild + 1;
		VPixel vpxSC = &h->vertices[secondChild];

		if (secondChild < h->capacity &&
			vpxCurrent->pathCost > vpxSC->pathCost &&
			vpxSC->pathCost < vpxFC->pathCost) {
			minIndex = secondChild;
		}

		if (minIndex) hp_Swap(h, i, minIndex);
	}
#ifdef TIMING
	t_Finalize(&t);
	t_Print(&t, timingstdout, __func__, 0);
#endif
}

void hp_Swap(HeapV h, int indexA, int indexB) {
	int allocatedWidth = h->img->allocatedWidth;

	VPixel vA = &h->vertices[indexA];
	VPixel vB = &h->vertices[indexB];

	int indexA_Pos = INDEX(vA->position.x, vA->position.y, allocatedWidth);
	int indexB_Pos = INDEX(vB->position.x, vB->position.y, allocatedWidth);

	if (vA->position.x > 0 && vA->position.y > 0)
		h->positions[indexA_Pos] = indexB;

	if (vB->position.x > 0 && vB->position.y > 0)
		h->positions[indexB_Pos] = indexA;

	vpixel temp = *vA;
	*vA = *vB;
	*vB = temp;
}

void hp_UpdateVPixel(HeapV h, VPixel selected, VPixel previous) {
	float newPathCost = previous->pathCost + selected->px.energy;

	selected->pathCost = newPathCost;

	int indexPos = INDEX(selected->position.x, selected->position.y, h->img->allocatedWidth);
	int indexHeap = h->positions[indexPos];

	h->vertices[indexHeap].pathCost = newPathCost;

	if (previous->position.x == -1 && previous->position.y == -1) {
		h->img->vpixels[indexPos].previous = &h->img->startingPoint;
	} else {
		int indexPrevious = INDEX(previous->position.x, previous->position.y, h->img->allocatedWidth);
		h->img->vpixels[indexPos].previous = &h->img->vpixels[indexPrevious];
	}

	if (h->img->vpixels[indexPos].previous == NULL) {
		printf("OPA\n");
	}
}

void hp_Free(HeapV h) {
	free(h->positions);
	free(h->vertices);
	free(h);
	h = NULL;
}

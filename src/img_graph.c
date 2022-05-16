#include "img_graph.h"
#include "pixel.h"
#include "ppm.h"

#include <stdlib.h>
#include <stdio.h>

struct vpixel_t {
	pixel px;
	struct vpixel_t * adjacent;
};

typedef struct vpixel_t vpixel;
typedef vpixel * VPixel;

struct img_graph_t {
	int currentWidth, currentHeight;
	int allocatedWidth, allocatedHeight;
	int maxComponentValue;
	VPixel pixels;
};

GImage gimg_Load(const char * filePath) {
	printf("Message from load graph version.\n");
	return NULL;
}

void gimg_RemoveLines(GImage gi, int amount) {
	printf("Message from remove lines graph version.\n");
}

void gimg_RemoveColumns(GImage gi, int amount) {
	printf("Message from remove columns graph version.\n");
}

void gimg_Save(GImage gi, const char * fileName) {
	printf("Message from save graph version.\n");
}

void gimg_Free(GImage gi) {
	printf("Message from free graph version.\n");
}


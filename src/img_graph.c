#include "img_graph.h"

#include <stdlib.h>
#include <stdio.h>

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


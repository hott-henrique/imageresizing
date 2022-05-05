#include "img_matrix.h"

#include <stdlib.h>
#include <stdio.h>

MImage mimg_Load(const char * filePath) {
	printf("Message from load matrix version.\n");
	return NULL;
}

void mimg_RemoveLines(MImage mi, int amount) {
	printf("Message from remove lines matrix version.\n");
}

void mimg_RemoveColumns(MImage mi, int amount) {
	printf("Message from remove columns matrix version.\n");

}

void mimg_Save(MImage mi, const char * fileName) { 
	printf("Message from save matrix version.\n");
}

void mimg_Free(MImage mi) {
	printf("Message from free matrix version.\n");
}



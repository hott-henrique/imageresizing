#include "img_matrix.h"
#include "pixel.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

struct img_matrix_t {
	int width, height;	
	int maxComponentValue;
	pixel * matrix;
};

void mimg_SetPixel(int x, int y, pixel p, void * miPtr) {
	MImage mi = (MImage)(miPtr);

	int offset = (x * mi->width) + y;

	mi->matrix[offset].r = p.r;
	mi->matrix[offset].g = p.g;
	mi->matrix[offset].b = p.b;

	mi->matrix[offset].li = LI(&p);
}

MImage mimg_Load(const char * filePath) {
	MImage mi = (MImage) malloc(sizeof(struct img_matrix_t));

	ppm_GetProperties(filePath, &mi->width, &mi->height, &mi->maxComponentValue);

	mi->matrix = (pixel *) malloc((mi->width * mi->height) * sizeof(pixel));

	ppm_ForEachPixel(filePath, mimg_SetPixel, mi);

	return mi;
}

void mimg_RemoveLines(MImage mi, int amount) {
	printf("Message from remove lines matrix version.\n");
}

void mimg_RemoveColumns(MImage mi, int amount) {
	printf("Message from remove columns matrix version.\n");
}

void mimg_Save(MImage mi, const char * fileName) { 
	FILE * f = fopen(fileName, "w");

	fprintf(f, "P3\n");
	fprintf(f, "%d %d\n", mi->width, mi->height);
	fprintf(f, "%d\n", mi->maxComponentValue);
	
	for (int i = 0; i < mi->width * mi->height; i++) {
		pixel p = mi->matrix[i];
		fprintf(f, "%d %d %d\n", p.r, p.g, p.b);	
	}

	fclose(f);
}

void mimg_Free(MImage mi) {
	free(mi->matrix);
	free(mi);
}

//MImage mimg_Load(const char * filePath) {
//	FILE * file = fopen(filePath, "r");
//	if (NULL == file) {
//		fprintf(stderr, "Could not open file: %s\n", filePath);
//		exit(EXIT_FAILURE);
//	}
//
//	char line[200];
//
//	fgets(line, sizeof(line), file); // Signature, first line.
//
//	MImage mi = (MImage) malloc(sizeof(struct img_matrix_t));
//
//	// Ignore comments between signature and properties of file.
//	fgets(line, sizeof(line), file);
//	while (line[0] == '#') fgets(line, sizeof(line), file);
//
//	sscanf(line, "%d %d", &mi->width, &mi->height);
//
//	// Ignore comments between properties and max component value.
//	fgets(line, sizeof(line), file);
//	while (line[0] == '#') fgets(line, sizeof(line), file);
//
//	sscanf(line, "%d", &mi->maxComponentValue);
//
//	int maxValueLen = mi->maxComponentValue > 0 ? floor(log10(abs(mi->maxComponentValue))) + 1 : 1;
//
//	// Ignore comments between max value and first line with any pixel.
//	fgets(line, sizeof(line), file);
//	while (line[0] == '#') fgets(line, sizeof(line), file);
//
//	// Allocate pixel matrix.
//	mi->matrix = (pixel *) malloc((mi->width * mi->height) * sizeof(pixel));
//
//	short component = 0;
//
//	int x = 0; int y = 0;
//
//	char * lineRef = &line[0];
//
//	char componentStr[maxValueLen + 1];
//	componentStr[maxValueLen] = '\0';
//
//	int red = -1; int green = -1; int blue = -1;
//
//	while (1) {
//		int lineLength = strlen(lineRef);
//
//		short bHasFoundComponent = 0;
//		for (int i = 0; i < lineLength && !bHasFoundComponent; i++) {
//			char c = lineRef[i];
//			int cCode = (int)(c);
//
//			if (c == '#') {
//				bHasFoundComponent = 0;
//				break;
//			}
//
//			if (cCode >= 48 && cCode <= 57) {
//				bHasFoundComponent = 1;
//
//				int j = 0;
//				for (; j < maxValueLen; j++) {
//					char d = lineRef[i + j];
//					int dCode = (int)(d);
//
//					if (!(dCode >= 48 && dCode <= 57)) {
//						componentStr[j] = '\0';
//						break;
//					}
//
//					componentStr[j] = d;
//				}
//				lineRef = &lineRef[i+j];
//			}
//		}
//
//		if (!bHasFoundComponent) {
//			void * OK = fgets(line, sizeof(line), file);
//
//			if (OK == NULL) break;
//
//			while (line[0] == '#') fgets(line, sizeof(line), file);
//			lineRef = &line[0];	
//
//			continue;
//		}
//
//		int componentValue = atoi(componentStr);
//
//		switch (component) {
//			case 0:
//				component++;
//				red = componentValue;
//			continue;
//
//			case 1:
//				component++;
//				green = componentValue;
//			continue;
//
//			case 2:
//				component = 0;
//				blue = componentValue;
//
//				int offset = (x * mi->width) + y;
//
//				mi->matrix[offset].r = red;
//				mi->matrix[offset].g = green;
//				mi->matrix[offset].b = blue;
//
//				y++;
//			break;
//		}
//
//		if (y == mi->width) {
//			y = 0;
//			x++;
//		}
//
//		if ((x == mi->height)) break;
//	}
//
//	fclose(file);
//
//	return mi;
//}

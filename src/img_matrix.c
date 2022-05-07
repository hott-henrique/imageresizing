#include "img_matrix.h"
#include "pixel.h"

#include <stdlib.h>
#include <stdio.h>

void SetPixel(int x, int y, pixel p, void * structPointer) {
	// esse funçao e chamada para cada pixel da img.
	// Acessar a estrutura a partir de structData.
	// Preencher toda ela.
	// Ja pode calcular a intensidade luminosa.

	// MImage mi = (MImage) structPointer;

	// mi->matrix[x][y].r = p.r;
	// mi->matrix[x][y].g = p.g;
	// mi->matrix[x][y].b = p.b;

	// mi->matrix[x][y].il = IL(&p);

	printf("pixel(%d, %d) = (%d, %d, %d)\n", x, y, p.r, p.g, p.b);
}

MImage mimg_Load(const char * filePath) {
	// Alocar base da estrutura

	int width, height;
	ppm_GetProperties(filePath, &width, &height);

	// Alocar matriz de pixels

	ppm_ForEachPixel(filePath, SetPixel, NULL); // Passar estrutura como argumento.

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



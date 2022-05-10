#include "pixel.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

float Gx[3][3] = { { -1, 0, +1 }, { -2, 0, +2 }, { -1, 0, +1 } };

float Gy[3][3] = { { -1, -2, -1 }, { 0, 0, 0 }, { +1, +2, +1 } };

void px_CalculateLI(Pixel p) {
	p->li = 0.30 * p->r + 0.59 * p->g + 0.11 * p->b;
}

float px_Sobel(float liRegion[3][3]) {
	float gx = px_ApplyWeigths(liRegion, Gx);
	float gy = px_ApplyWeigths(liRegion, Gy);
	return sqrtf((gx * gx) + (gy * gy));	
}

float px_ApplyWeigths(float liRegion[3][3], float weigths[3][3]) {
	float matResult[3][3];
	memset(matResult, 0, sizeof(matResult));
	// memset 0

	// Multiply matrix
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				matResult[i][j] += liRegion[i][k] * weigths[k][j];
			}
		}
	}

	float result = 0.0f;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result += matResult[i][j];
		}
	}

	return result;
}


#include "pixel.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

float Gx_Sobel[3][3] = { { -1,  0, +1 }, { -2, 0, +2 }, { -1,  0, +1 } };
float Gy_Sobel[3][3] = { { -1, -2, -1 }, {  0, 0,  0 }, { +1, +2, +1 } };

float Gx_Scharr[3][3] = { { +3,   0, -3 }, { +10, 0, -10 }, { +3,   0, -3 } };
float Gy_Scharr[3][3] = { { +3, -10, +3 }, {   0, 0,   0 }, { -3, -10, -3 } };

static float px_ApplyWeigths(float liRegion[3][3], float weigths[3][3]);

void px_CalculateLI(Pixel p) { // O(1)
	p->li = 0.30 * p->r + 0.59 * p->g + 0.11 * p->b;
}

float px_CalculateEnergy(float liRegion[3][3], char operator) { // O(1)
	float gx, gy;

	switch (operator) {
		case Sobel:
			gx = px_ApplyWeigths(liRegion, Gx_Sobel);
			gy = px_ApplyWeigths(liRegion, Gy_Sobel);
		break;

		case Scharr:
			gx =px_ApplyWeigths(liRegion, Gx_Scharr);
			gy =px_ApplyWeigths(liRegion, Gy_Scharr);
		break;

		default:
			fprintf(stderr, "Unknow operator: %c.\n", operator);
			exit(EXIT_FAILURE);
	}

	return sqrtf((gx * gx) + (gy * gy));	
}

float px_ApplyWeigths(float liRegion[3][3], float weigths[3][3]) { // O(1)
	float result = 0.0f;
	for (int i = 0; i < 3; i++) { // 3
		for (int j = 0; j < 3; j++) { // 3
			result += liRegion[i][j] * weigths[i][j];
		}
	}

	return result;
}


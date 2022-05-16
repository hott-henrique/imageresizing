#include "pixel.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

float S_Gx[3][3] = { { -1,  0, +1 }, { -2, 0, +2 }, { -1,  0, +1 } };
float S_Gy[3][3] = { { -1, -2, -1 }, {  0, 0,  0 }, { +1, +2, +1 } };

float SF_Gx[3][3] = { { +3,   0, -3 }, { +10, 0, -10 }, { +3,   0, -3 } };
float SF_Gy[3][3] = { { +3, -10, +3 }, {   0, 0,   0 }, { -3, -10, -3 } };

void px_CalculateLI(Pixel p) {
	p->li = 0.30 * p->r + 0.59 * p->g + 0.11 * p->b;
}

float px_Sobel(float liRegion[3][3]) {
	float gx = px_ApplyWeigths(liRegion, S_Gx);
	float gy = px_ApplyWeigths(liRegion, S_Gy);
	return sqrtf((gx * gx) + (gy * gy));	
}

float px_SobelFeldman(float liRegion[3][3]) {
	float gx = px_ApplyWeigths(liRegion, SF_Gx);
	float gy = px_ApplyWeigths(liRegion, SF_Gy);
	return sqrtf((gx * gx) + (gy * gy));	
}

float px_ApplyWeigths(float liRegion[3][3], float weigths[3][3]) {
	float result = 0.0f;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result += liRegion[i][j] * weigths[i][j];
		}
	}

	return result;
}


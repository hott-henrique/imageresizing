#include "pixel.h"

int Gx[3][3] = { { -1, 0, +1 }, { -2, 0, +2 }, { -1, 0, +1 } };

int Gy[3][3] = { { -1, -2, -1 }, { 0, 0, 0 }, { +1, +2, +1 } };

float LI(Pixel p) {
	return 0.30 * p->r + 0.59 * p->g + 0.11 * p->b;
}


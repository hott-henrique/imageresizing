#include "pixel.h"

float LI(Pixel p) {
	return 0.30 * p->r + 0.59 * p->g + 0.11 * p->b;
}


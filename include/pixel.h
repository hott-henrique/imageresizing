#ifndef TP2_PIXEL_HEADER
#define TP2_PIXEL_HEADER

typedef struct pixel_t {
	int r, g, b;
	float li;
	float energy;
} * Pixel;

float LI(int R, int G, int B);

#endif

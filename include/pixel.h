#ifndef TP2_PIXEL_HEADER
#define TP2_PIXEL_HEADER

struct pixel_t {
	int r, g, b;
	float li;
	float energy;
};

typedef struct pixel_t pixel;
typedef struct pixel_t * Pixel;

float LI(Pixel p);

#endif

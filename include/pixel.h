#ifndef TP2_PIXEL_HEADER
#define TP2_PIXEL_HEADER

extern int Gx[3][3];
extern int Gy[3][3];

struct pixel_t {
	int r, g, b;
	float li;
	float energy;
};

typedef struct pixel_t pixel;
typedef struct pixel_t * Pixel;

float LI(Pixel p);

#endif

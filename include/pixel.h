#ifndef TP2_PIXEL_HEADER
#define TP2_PIXEL_HEADER

extern float Gx[3][3];
extern float Gy[3][3];

struct pixel_t {
	int r, g, b;
	float li;
	float energy;
};

typedef struct pixel_t pixel;
typedef struct pixel_t * Pixel;

void px_CalculateLI(Pixel p);
float px_Sobel(float liRegion[3][3]);
float px_ApplyWeigths(float liRegion[3][3], float weigths[3][3]);

#endif

#ifndef TP2_PIXEL_HEADER
#define TP2_PIXEL_HEADER

enum Operators {
	Sobel = 'S',
	Scharr = 'C',
};

struct pixel_t {
	int r, g, b;
	float li;
	float energy;
};

typedef struct pixel_t pixel;
typedef struct pixel_t * Pixel;

void px_CalculateLI(Pixel p);
float px_CalculateEnergy(float liRegion[3][3], char operator);

#endif

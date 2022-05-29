#ifndef TP2_PIXEL_HEADER
#define TP2_PIXEL_HEADER

enum PathOptions {
	NOT_CHECKED_YET = -2,
	LEFT = -1,
	CENTER = 0,
	RIGHT = 1,
	LAST_PIXEL = 2,
};

struct pixel_t {
	int r, g, b;
	float li;
	float energy;
	short next; //Represent the next pixel in path - It can var: -1, 0, 1
	short previous;
	float energyInThatPath;	// Energy that it were acumulated when get the best path
};

typedef struct pixel_t pixel;
typedef struct pixel_t * Pixel;

struct vpixel_t {
	pixel px;
	struct vpixel_t ** adjacent;
};

typedef struct vpixel_t vpixel;
typedef vpixel * VPixel;

void px_CalculateLI(Pixel p);
float px_CalculateEnergy(float liRegion[3][3]);

#endif

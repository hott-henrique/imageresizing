#ifndef TP2_PIXEL_HEADER
#define TP2_PIXEL_HEADER

enum Operators {
	Sobel = 'S',
	SobelFeldman = 'F',
};

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
float px_CalculateEnergy(float liRegion[3][3], char operator);

#endif

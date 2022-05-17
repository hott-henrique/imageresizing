#ifndef TP2_PIXEL_HEADER
#define TP2_PIXEL_HEADER

enum PathOptions {
	NOT_CHECKED_YET = -2,
	LEFT = -1,
	CENTER = 0,
	RIGHT = 1,
};

struct pixel_t {
	int r, g, b;
	float li;
	float energy;
	short next; //Represent the next pixel in path
	float energyInThatPath;	
};

typedef struct pixel_t pixel;
typedef struct pixel_t * Pixel;

void px_CalculateLI(Pixel p);
float px_CalculateEnergy(float liRegion[3][3]);

#endif

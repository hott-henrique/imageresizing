#ifndef TP2_PPM_HEADER
#define TP2_PPM_HEADER

#include "pixel.h"


void ppm_GetProperties(const char * filePath, int * outWidth, int * outHeight);

#define pixelCallback void(*f)(int x, int y, pixel p, void * data)

void ppm_ForEachPixel(const char * filePath, pixelCallback, void * data);

#endif

#ifndef TP2_PPM_HEADER
#define TP2_PPM_HEADER

#include "pixel.h"

void ppm_GetProperties(const char * filePath, int * outWidth, int * outHeight, int * outMaxComponentValue);

#define pixelCallback void(*callback)(int x, int y, pixel p, void * structData)

void ppm_ForEachPixel(const char * filePath, pixelCallback, void * structData);

#endif

#ifndef TP2_IMAGE_GRAPH_HEADER
#define TP2_IMAGE_GRAPH_HEADER

#include <stdio.h>

typedef struct img_graph_t * GImage;

GImage gimg_Load(const char * filePath);

void gimg_RemoveLines(GImage gi, int amount, char operator);
void gimg_RemoveColumns(GImage gi, int amount, char operator);
void gimg_RemoveLinesAndColumns(GImage gi, int amountLines, int amountColumns, char operator);

void gimg_Print(GImage gi, FILE * f);

void gimg_Free(GImage gi);

#endif

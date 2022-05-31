#ifndef TP2_IMAGE_MATRIX_HEADER
#define TP2_IMAGE_MATRIX_HEADER

#include <stdio.h>

typedef struct img_matrix_t * MImage;

MImage mimg_Load(const char * filePath);

void mimg_RemoveLines(MImage mi, int amount, char energyOp);
void mimg_RemoveColumns(MImage mi, int amount, char energyOp);
void mimg_RemoveLinesAndColumns(MImage mi, int amountLines, int amountColumns, char energyOp);

void mimg_Print(MImage mi, FILE * f);

void mimg_Free(MImage mi);

#endif

#ifndef TP2_IMAGE_MATRIX_HEADER
#define TP2_IMAGE_MATRIX_HEADER

typedef struct img_matrix_t * MImage;

MImage mimg_Load(const char * filePath);

void mimg_RemoveLines(MImage mi, int amount);
void mimg_RemoveColumns(MImage mi, int amount);

void mimg_Save(MImage mi, const char * fileName);

void mimg_Free(MImage mi);

#endif

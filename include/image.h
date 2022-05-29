#ifndef TP2_IMAGE_HEADER
#define TP2_IMAGE_HEADER

enum ImageMode {
	MATRIX = 'M',
	GRAPH = 'G'
};

typedef struct image_t * Image;

Image img_Load(const char * filePath, char mode);

void img_RemoveLines(Image i, int amount, short operator);
void img_RemoveColumns(Image i, int amount, short operator);
void img_RemoveLinesAndColumns(Image i, int amountLines, int amountColumns, short operator);

void img_Save(Image i, const char * fileName);

void img_Free(Image i);

#endif

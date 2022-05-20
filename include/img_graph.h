#ifndef TP2_IMAGE_GRAPH_HEADER
#define TP2_IMAGE_GRAPH_HEADER

typedef struct img_graph_t * GImage;

GImage gimg_Load(const char * filePath);

void gimg_RemoveLines(GImage gi, int amount);
void gimg_RemoveColumns(GImage gi, int amount);
void gimg_RemoveLinesAndColumns(GImage gi, int amountLines, int amountColumns);

void gimg_Save(GImage gi, const char * fileName);

void gimg_Free(GImage gi);

#endif

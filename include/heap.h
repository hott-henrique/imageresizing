#ifndef TP2_GRATH_HEAP_HEADER
#define TP2_GRATH_HEAP_HEADER

#include "pixel.h"



typedef struct heap_t heap;
typedef struct heap * Heap;

Heap heap_BuildHeap(int size);
void heap_insertPixel(Heap h);
VPixel heap_DeletePixel(Heap h); 
void heap_ToMinHeap(Heap h);
void heap_DestroyHeap(Heap h);

#endif
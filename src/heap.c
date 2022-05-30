#include "heap.h"

#include <stdio.h>
#include <stdlib.h>

#define HEAP_MAX_PARENTS 3

struct heap_t {
    struct vpixel_t ** pixelsInHeap;
    int current_size;
    int capacity;
};

void swap(VPixel x, VPixel y) {
    VPixel temp = x;

    *x = *y;
    *y = *temp;
}

Heap heap_BuildHeap(int size) {

    Heap h = (Heap) malloc(sizeof(heap_t));

    h->pixelsInHeap = (VPixel) malloc(sizeof(struct vpixel_t) * size);
    h->capacity = size;
    h->current_size = 0;

    return h;
}

void heap_insertPixel(Heap h, VPixel pixel) {

    if (h->current_size == h->capacity) {
        printf("Cannot insert more pixels in heap.\n");
        return;
    }

    h->current_size++;
    g->pixelsInHeap[h->current_size] = y;

    heap_ToMinHeap(h);

    return;
} 


VPixel heap_DeletePixel(Heap h) {

    VPixel p = h->pixelsInHeap[0];
    size_t size = h->current_size - 1;
    for (int i = size; i > 0; i--) {
        int previousIndex = i - 1;
        h->pixelsInHeap[previousIndex] = h->pixelsInHeap[i]; 
    }

    return p;
}

void heap_ToMinHeap(Heap h) { // ???

    VPixel currentPixelInserted = h->pixelsInHeap[h->current_size];
    VPixel elementsInHeap = h->pixelsInHeap;

    int parents = (h->current_size - 1) HEAP_MAX_PARENTS;
    // if (size % 2 == 1) size++;
    // [1 2 3 4 5 6 7 8] 

    while (size > 1 && elementsInHeap[parents].px.energy > currentPixelInserted.px.energy) {
        swap(elementsInHeap[parents], currentPixelInserted);

        parents = (parents - 1) / HEAP_MAX_PARENTS;
    }
}

void heap_DestroyHeap(Heap h) {

    free(pixelsInHeap);
    free(h);
}
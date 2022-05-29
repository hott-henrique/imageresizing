#include "heap.h"

#include <stdio.h>
#include <stdlib.h>

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

void heap_insertPixel(Heap h, VPixel pixel) {

    vpixel_t * Pixels = h->pixelsInHeap;

    if (h->current_size == h->capacity) {
        printf("Overflow: Cannot insert more pixels in heap.\n");
        return;
    }

    h->current_size++;
    int indexOfPixel = h->current_size - 1;

    // Aplly min heap and orden pixels by min path
    while (indexOfPixel != 0 && Pixels[indexOfPixel].px.energyInThatPath > ) {
        // Get the parents of pixel
        // Swap case min ocorr
    }
} 


VPixel heap_DeletePixel(Heap h) {

}

void heap_DestroyHeap(Heap h) {

}
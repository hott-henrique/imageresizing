#include "stack.h"

// Stack struct: stack_t stack;
// Stack pointer struct: stack * Stack;

// Stack Item struct: stack_Pixel_Item_t stack_Pixel_Item;
// Stack Item pointer struct: stack_Pixel_Item * Stack_Pixel_Item;

struct stack_t {
    Stack_Pixel_Item last;

    size_t height;
};

struct stack_Pixel_Item_t {
    VPixel p;
    
    Stack_Pixel_Item previous;
};

Stack stk_BuildStack() {

    Stack s = (Stack) malloc(sizeof(stack_t));
    s->height = 0;
    return s;
}

void stk_InsertInStack(Stack s, VPixel v) {

    Stack_Pixel_Item pixelInStack = (Stack_Pixel_Item) malloc(sizeof(stack_Pixel_t));
    pixelInStack->p = v;

    pixelInStack->previous = s->last;
    s->last = pixelInStack;

    s->height++;
}


VPixel stk_Unstack(Stack s) {

    VPixel p = s->last->p;
    Stack_Pixel_Item itemToFree = s->last;

    s->last = itemToFree->previous;

    free(s);
}

void stk_DestroyStack(Stack s);
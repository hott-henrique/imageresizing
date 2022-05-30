#ifndef GRATH_STACK_HEADER
#define GRATH_STACK_HEADER

#include "pixel.h"

#include <stdio.h>

typedef struct stack_t stack;
typedef struct stack * Stack;

typedef struct stack_Pixel_Item_t stack_Pixel_Item;
typedef struct stack_Pixel_Item * Stack_Pixel_Item;

Stack stk_BuildStack(); 
void stk_InsertInStack(Stack s, VPixel v);
VPixel stk_Unstack(Stack s);
void stk_DestroyStack(Stack s);

#endif
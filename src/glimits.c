#include "glimits.h"

int gl_LimitedPlus(int value, int limit) { // O(1)
    if (value > limit) return value;
    else return limit;
    
}
int gl_LimitedMinus(int value, int limit) { // O(1)
    if (value < limit) return value;
    else return limit;

}
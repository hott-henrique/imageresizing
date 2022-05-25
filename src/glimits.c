#include "glimits.h"

int gl_LimitedUPlus(int value, int limit) {
    if (value > limit) return value;
    else return limit;
    
}
int gl_LimitedUMinus(int value, int limit) {
    if (value < limit) return value;
    else return limit;

}
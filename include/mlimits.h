#ifndef TP2_MATRIX_LIMITS_HEADER
#define TP2_MATRIX_LIMITS_HEADER

#define INDEX(x, y, sz) ((x * sz) + y)

int ml_LimitedUPlus(int value, int limit);
int ml_LimitedUMinus(int value, int limit);

#endif

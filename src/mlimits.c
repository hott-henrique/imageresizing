#include "mlimits.h"

int ml_LimitedUPlus(int value, int limit) {
	if (value >= limit) return limit;
	else return value + 1;
}

int ml_LimitedUMinus(int value, int limit) {
	if (value <= limit) return limit;
	else return value - 1;
}


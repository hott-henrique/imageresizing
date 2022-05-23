#include "mlimits.h"

// make 2 functs to indentify which operation is doing it
int ml_LimitedUPlus(int value, int limit) { // return next
	if (value >= limit) return limit;
	else return value + 1;
} // Funct to compare coord values

int ml_LimitedUMinus(int value, int limit) { // return previous
	if (value <= limit) return limit;
	else return value - 1;
}


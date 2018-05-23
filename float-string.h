
// By defitinition, Contiki OS don't support floating point unit.
// This implements functions to convert floats into strings

#include <stdio.h>
#include <math.h>

#define FLOAT_PRECISION 3

// Private function
//void reverse(unsigned char *str, int len);
//int intToStr(int x, unsigned char str[], int d);

// Public functions
void ftoa(double n, unsigned char *res, int afterpoint);
void ftos(double n, unsigned char *res);

//    char res[20];
//    float n = 233.007;
//  ftoa(n, res, 4);
// printf("\n\"%s\"\n", res);
//    return 0;

#include "float-string.h"

 
// reverses a string 'str' of length 'len'
void reverse(unsigned char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}
 
 // Converts a given integer x to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
int intToStr(int x, unsigned char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }
 
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}
 
// Converts a floating point number to string.
void ftoa(double n, unsigned char *res, int afterpoint)
{
    float fpart;
    // Extract integer part
    int ipart = (int)n;

    if(ipart < 0) { // if the number is negative
        fpart = (-1) * (n - (float)ipart);    // Extract floating part
        ipart = -1 * ipart;
        res[0] = '-';   // put the minus signal
        res++;  // increment position
    } else {
        fpart = n - (float)ipart;    // Extract floating part
    }

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

void
ftos(double n, unsigned char *res) {
  ftoa(n, res, FLOAT_PRECISION);
}
 

//    char res[20];
//    float n = 233.007;
//  ftoa(n, (void*) &res, 4);
// printf("\n\"%s\"\n", res);
//    return 0;

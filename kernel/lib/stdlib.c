/*
stdlib.c
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================

Implementation of stdlib for crunchy
*/

#include <types.h>
#include <ctype.h>
#include <stdlib.h>

/*double atof(char* s) {
	double integer = 0;
	double decimal = 0;
	double divisor = 1.0;
	double sign = 1.0;

	bool fraction = false;
	double tot = 0;

	while (*s != '\0') {
		if (isdigit(*s)) {
			if(fraction) {
				decimal = decimal*10 + (*s - '0');
				divisor *= 10;
				vga_puts(ftoa(decimal/divisor));
				vga_putc('\n');
			} else {
				integer = (*s - '0');
				//integer *= 10;
				//integer += (*s - '0');
				tot = tot*10 + integer;
				vga_puts(ftoa(tot));
				vga_putc('\n');
			}

		} else if (*s == '.') {
			fraction = true;
			vga_puts("Decimal found\n");
		} else if (*s == '-') {
			sign = -1.0;
		} else {
			//return sign * (integer + decimal/divisor);;
		}
		s++;
	}
	return sign * (tot + decimal/divisor);

}*/


# define PRECISION 5
// adapted from http://stackoverflow.com/questions/2302969/how-to-implement-char-ftoafloat-num-without-sprintf-library-function-i/2303011#2303011
char*  ftoa(double num, char* str)
{
   int whole_part = num;
   int digit = 0, reminder =0;
   int log_value = dlog10(num), index = log_value;
   long wt =0;

   //Initilise stirng to zero
   memset(str, 0 ,20);

   //Extract the whole part from float num
   for(int i = 1 ; i < log_value + 2 ; i++)
   {
       wt  =  pow(10,i);
       reminder = whole_part  %  wt;
       digit = (reminder - digit) / (wt/10);

       //Store digit in string
       str[index--] = digit + 48;              // ASCII value of digit  = digit + 48
       if (index == -1)
          break;    
   }

    index = log_value + 1;
    str[index] = '.';

   double fraction_part  = num - whole_part;
   double tmp1 = fraction_part,  tmp =0;

   //Extract the fraction part from  num
   for( int i= 1; i < PRECISION; i++)
   {
      wt = 10; 
      tmp  = tmp1 * wt;
      digit = tmp;

      //Store digit in string
      str[++index] = digit + 48;           // ASCII value of digit  = digit + 48
      tmp1 = tmp - digit;
   }    

   return str;
}

// log10 for doubles
int dlog10(double v) {
    return (v >= 1000000000u) ? 9 : (v >= 100000000u) ? 8 : 
        (v >= 10000000u) ? 7 : (v >= 1000000u) ? 6 : 
        (v >= 100000u) ? 5 : (v >= 10000u) ? 4 :
        (v >= 1000u) ? 3 : (v >= 100u) ? 2 : (v >= 10u) ? 1u : 0u; 
}

// log10 for integers
int log10(int v) {
    return (v >= 1000000000u) ? 9 : (v >= 100000000u) ? 8 : 
        (v >= 10000000u) ? 7 : (v >= 1000000u) ? 6 : 
        (v >= 100000u) ? 5 : (v >= 10000u) ? 4 :
        (v >= 1000u) ? 3 : (v >= 100u) ? 2 : (v >= 10u) ? 1u : 0u; 
}

uint32_t abs(int x) {
	if (x < 0)
		return x * -1;
	return x;
}

// raise n^x
int pow(int n, int x) {
	if (x = 0) return 1;
	while(x-- > 1)
	{
		n = n * n;
	}
	return n;
}

// string to integer
int atoi(char* s) {
	int num = 0;
	int sign = 1;

	if (s[0] == '-')
		sign = -1;

	for (int i = 0; i < strlen(s) && s[i] != '\0'; i++) {
		if (isdigit(s[i]))
			num = num*10 + (s[i] - '0');
	}
	return sign*num;

}

// unsigned integer to string
char* itoa(uint32_t num, char* buffer, int base) {
	int i = 0;
	//num = abs(num);
	int len = 8;

	if (base == 2)
		len = 32;
	
	if (num == 0 && base == 2) {
		while(i < len)
			buffer[i++] = '0';
		buffer[i] = '\0';
		return buffer;
	}
/*	if (num == 0 && base == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return buffer;
	}*/

	// go in reverse order
	while (num != 0 && len--) {
		int remainder = num % base;
		// case for hexadecimal
		buffer[i++] = (remainder > 9)? (remainder - 10) + 'A' : remainder + '0';
		num = num / base;
	}

	while(len-- && base != 10)
		buffer[i++] = '0';

	buffer[i] = '\0';

	return strrev(buffer);
}

// signed integer to string
char* sitoa(int num, char* buffer, int base) {
	int i = 0;
	int sign = 1;
	int len = 8;
	//num = abs(num);
	if (num == 0 || base == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return buffer;
	}
	if (num < 0) {
		sign = 0;
		num = abs(num);
	}

	// go in reverse order
	while (num != 0 && len--) {
		int remainder = num % base;
		// case for hexadecimal
		buffer[i++] = (remainder > 9)? (remainder - 10) + 'A' : remainder + '0';
		num = num / base;
	}

//	if (sign == 0) buffer[i++] = '-';

	buffer[i] = '\0';

	return strrev(buffer);
}

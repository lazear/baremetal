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

# define PRECISION 5
// adapted from http://stackoverflow.com/questions/2302969/how-to-implement-char-ftoafloat-num-without-sprintf-library-function-i/2303011#2303011
char*  ftoa(double num, char* str)
{
   int whole_part = num;
   int digit;
   int log_value = dlog10(num), index = log_value;
   long wt =0;
   double fraction_part  = num - whole_part;

	// go in reverse order
   	int i = 0;
	while (whole_part != 0) {
		int remainder = whole_part % 10;
		str[i++] = remainder + '0';
		whole_part = whole_part / 10;
	}
	strrev(str);

   index = log_value + 1;
   str[index] = '.';


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

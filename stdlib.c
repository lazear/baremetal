/*
stdlib.c

2007-2016 (C) Michael Lazear
Implementation of stdlib for baremetal
*/

#include <types.h>


double atof();

int atoi(char* s) {

}


char* hex(int d) {
	char buffer[8];
	itoa(&buffer, 16, d, 8, 'a');
	return buffer;
}

void itoa(char *buf, int base, int d, signed int length, char flags)
{
   char *p = buf;
   char *p1, *p2;
   unsigned long ud = d;
   int divisor = 10;

   /* If %d is specified and D is minus, put `-' in the head. */
   if(base == 10 && d < 0)
   {
      *p++ = '-';
      buf++;
      ud = -d;
   }
   else if( base == 16 )
      divisor = 16;
   else if( base < 16 )
	   divisor = base;

   /* Divide UD by DIVISOR until UD == 0. */
	do
	{
		int remainder = ud % divisor;
		// funky if
		*p++ = (remainder < 10) ? remainder + '0' : remainder + flags - 10;
		length--;
	}
	while(ud /= divisor);

   
	while(length > 0)
	{
		*p++ = '0';
		length--;
	}

	/* Terminate BUF. */
	*p = 0;

	/* Reverse BUF. */
	p1 = buf;
	p2 = p - 1;
	while(p1 < p2)
	{
		char tmp = *p1;

		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}

#include <types.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

	printf("Hello %4s%20s\n",  "Michael", "Lazear");
	return 0;
}

int functwo(int x) {
	return pow(x, 2);
}

#include <types.h>

int main(int argc, char* argv[]) {

	puts("Hello");
	char* r = malloc(32);

	sitoa(functwo(-0x366172), r, 16);
	puts(r);

	for(;;);
	return 0;

}

int functwo(int x) {
	return x+2;
}

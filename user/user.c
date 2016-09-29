#include <types.h>
#include <stdio.h>

int main(int argc, char* argv[]) {


	char* r = malloc(32);

	sitoa(functwo(-0x366172), r, 16);
	puts(r);

	STREAM* s = k_new_stream(32);
	k_stream_write(s, "Hello, world", 13);
	k_stream_seek(s, 3);
	puts(k_stream_read(s, 5));
	fflush(s);
	ftoa(pow(10, 2.3)*123, s->data);
	puts(k_stream_read(s, 10));
	printf("Hello %4s%20s\n",  "Michael", "Lazear");
	

	return 0;

}

int functwo(int x) {
	return pow(x, 2);
}

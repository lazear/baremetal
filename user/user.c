#include <types.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

	printf("USERLAND ELF EXECUTABLE!\n");
	// int esp;
	// asm("mov %esp, %eax");
	// asm("mov %%eax, %0" : "=r"(esp));

	// printf("stack %#x",  esp);

	return 0;
}

int functwo(int x) {
	return pow(x, 2);
}

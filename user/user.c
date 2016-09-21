

void syscall_print(char* message) {
	asm volatile("mov %0, %%edx" : : "a"(message));
	asm volatile("mov $1, %eax");
	asm volatile("int $0x80");
}


int main(int argc, char* argv[]) {

	char* buf = malloc(10);

	char ib[32];
	itoa(0xDEADBABE, ib, 16);
	syscall_print(ib);
	//syscall_print(argv[0]);
	//syscall_print("?");
	syscall_print("Hello world, iteration 2!");
	//for(;;);
	return 0;

}

int functwo(int x) {
	return x+2;
}

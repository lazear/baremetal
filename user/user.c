
int main(int argc, char* argv[]) {
	//syscall_print(argv[argc-1]);
	return 0;

}

int functwo(int x) {
	return x+2;
}

void syscall_print(char* message) {
	asm volatile("mov %0, %%edx" : : "a"(message));
	asm volatile("mov $1, %eax");
	asm volatile("int $0x80");
}

global read_stack_pointer
read_stack_pointer:
	mov eax, esp
	add eax, 4
	ret
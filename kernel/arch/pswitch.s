;switch.s

;[BITS 32]


; switch(context struct1, context struct2)
; struct context {
; uint32 edi
; uint32 esi
; uint32 ebx
; uint32 ebp
; uint32 eip;
; }
global pswitch
pswitch:
	mov eax, [esp+4]
	mov edx, [esp+8]

	push ebp
	push ebx
	push esi
	push edi

	mov [eax], esp
	mov esp, edx

	pop edi
	pop esi
	pop ebx
	pop ebp
	ret
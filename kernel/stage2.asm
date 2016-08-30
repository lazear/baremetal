[BITS 16]
[ORG 0x1000]

start:
	; mov ax, 0x8000
	; mov es, ax
	; mov ax, 0x4F01			; move information to es:di
	; mov cx, 0x0118
	; xor di, di
	; int 10h

	; mov ax, 0x4F02          ; set VBE mode
	; ; 431A is 1280x 10. 4118 is 1080x720 or whatever
	; mov bx, 0x4118         ; Bit 14 is set(Linear frame buffer) 15 is clear
	; int 0x10

	mov ax, [superblock+56]			; EXT2_MAGUC
	cmp ax, 0xEF53			
	jne .error

	; To locate an inode: block group = (inode - 1) / s_inodes_per_group
	; block table index = (inode-1) % s_inodes_per_group
	; containing block = (bg->inode_table + (index * 128) / BLOCK_SIZE)
	; INODE 5 is located in the first block table, index 0. + 4*128 = 0x200

	mov ax, [superblock + 1024 + 8]		; Block_Group_Descriptor->inode_table
	mov cx, 2
	mul cx
	push ax


.read_disk:	
	mov ah, 0               ; Reset drive command
	int 13h                 ; Call interrupt 13h
	mov [drive], dl         ; Store boot disk
	or ah, ah               ; Check for error code
	jnz .error           ; Try again if error

	mov ax, 0				; Clear AX
	mov es, ax              ; ES segment = 0                
	mov bx, 0x2000          ; Destination address = 0000:1000
	mov ah, 02h             ; Read sector command
	mov al, 2            	; Number of sectors to read (0x12 = 18 sectors)
	mov dl, [drive]         ; Load boot disk
	mov ch, 0               ; Cylinder = 0
	pop ax
	mov cx, ax				; Sector
	mov dh, 0               ; Head = 1
	int 13h                 ; Call interrupt 13h
	or ah, ah               ; Check for error code
	jnz .error         ; Try again if error
	cli                     ; Disable interrupts


	mov bx, 0x2100
	add bx, 40
	mov ax, [bx]


	jmp $

	;jmp $



.print:
	mov bp, .string
	mov ah, 0x13
	mov al, 0x01
	mov bh, 0x00
	mov bl, 0xF
	mov cx, .strlen
	int 0x10
	jmp enter_pm


.error:
	mov ah, 09h
	mov al, '!'
	mov bh, 0
	mov bl, 0xA;
	mov cx, 160*25
	int 0x10
	jmp $

.string	db 13, 'EXT2 Loaded'
.strlen equ $-.string

drive db 0

enter_pm:
	in al, 0x92				; enable a20
	or al, 2
	out 0x92, al

	xor ax, ax              ; Clear AX register
	mov ds, ax              ; Set DS-register to 0 

	lgdt [gdt_desc]         ; Load the Global Descriptor Table

	;----------Entering Protected Mode----------;

	mov eax, cr0            ; Copy the contents of CR0 into EAX
	or eax, 1               ; Set bit 0     (0xFE = Real Mode)
	mov cr0, eax            ; Copy the contents of EAX into CR0

	jmp 08h:kernel_segments ; Jump to code segment, offset kernel_segments


[BITS 32]                       ; We now need 32-bit instructions
	kernel_segments:
	mov ax, 10h             ; Save data segment identifyer
	mov ds, ax              ; Setup data segment
	mov ss, ax              ; Setup stack segment
	;mov esp, 090000h        ; Move the stack pointer to 090000h

	mov eax, 0xB8000
	mov cx, 'H'
	mov [eax], cx
	inc eax
	mov cx, 0x2
	mov [eax], cx



; 	mov edi, [0x80000 + 28h]   
; 	jmp $
; 	mov edx, edi
;    	mov eax, 0xFFFFFFFF

;    	add edx, 1024*720
;    	mov [edx], eax
;    	mov ecx, 0

; .loop:
	
;    	mov [edx+ecx], eax
;    	inc ecx

;    	cmp ecx, 1024*24
;    	jne .loop


	;jmp 08h:0x1000          ; Jump to section 08h (code), offset 01000h
	jmp $






	;----------Global Descriptor Table----------;

gdt:                            ; Address for the GDT

gdt_null:                       ; Null Segment
	dd 0
	dd 0


KERNEL_CODE             equ $-gdt		; 0x08
gdt_kernel_code:
	dw 0FFFFh               ; Limit 0xFFFF
	dw 0                    ; Base 0:15
	db 0                    ; Base 16:23
	db 09Ah                 ; Present, Ring 0, Code, Non-conforming, Readable
	db 0CFh                 ; Page-granular
	db 0                    ; Base 24:31

KERNEL_DATA             equ $-gdt		; 0x10
gdt_kernel_data:                        
	dw 0FFFFh               ; Limit 0xFFFF
	dw 0                    ; Base 0:15
	db 0                    ; Base 16:23
	db 092h                 ; Present, Ring 0, Data, Expand-up, Writable
	db 0CFh                 ; Page-granular
	db 0                    ; Base 24:32

gdt_end:                        ; Used to calculate the size of the GDT

gdt_desc:                       ; The GDT descriptor
	dw gdt_end - gdt - 1    ; Limit (size)
	dd gdt                  ; Address of the GDT


times 512-($-$$) db 0           ; Fill up the file with zeros
superblock:

; Fat12 Bootloader

[BITS 16]                       ; We need 16-bit intructions for Real mode

[ORG 0x7C00]                    ; The BIOS loads the boot sector into memory location 0x7C00

jmp word read_disk            ; Load the OS Kernel

read_disk:
        mov ah, 0               ; Reset drive command
        int 13h                 ; Call interrupt 13h
        mov [drive], dl         ; Store boot disk
        or ah, ah               ; Check for error code
        jnz read_disk           ; Try again if error
        
        mov ax, 0				; Clear AX
        mov es, ax              ; ES segment = 0                
        mov bx, 0x1000          ; Destination address = 0000:1000
        mov ah, 02h             ; Read sector command
        mov al, 5            	; Number of sectors to read (0x12 = 18 sectors)
        mov dl, [drive]         ; Load boot disk
        mov ch, 0               ; Cylinder = 0
        mov cl, 2               ; Starting Sector = 3
        mov dh, 0               ; Head = 1
        int 13h                 ; Call interrupt 13h
        or ah, ah               ; Check for error code
        jnz read_disk         ; Try again if error
        cli                     ; Disable interrupts



        jmp 0x1000

drive db 0
times 510-($-$$) db 0           ; Fill up the file with zeros
dw 0AA55h                   	; Last 2 bytes = Boot sector identifyer


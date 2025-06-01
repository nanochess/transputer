	;
	; Transputer interface for my Pascal compiler and Ray Tracer.
	;
	; by Oscar Toledo G.
	; https://nanochess.org/
	;
	; Creation date: May/31/2025.
	;

	org 0x0100

	;
	; This is written for an ISA board compatible with Inmos B004
	; 
	; 0x0150 - Connected to Inmos C011 communication chip.
	; 0x0160 - Reset pin for transputer board
	; 0x0161 - Analyse pin for transputer board
	;
	; I'm supposing the C011 chips is connected to link 0 of the transputer chip.
	;
	; Also that you are using an Inmos T805 transputer (the one I used when I
	; developed my software)
	;

arguments:      equ 0x0080

	push cs
	push cs
	pop ds
	pop es
	call parse_arguments

	call reset_transputer

	mov dx,arguments        ; File name.
	call load_file

	call move_arguments

	; New position in dx:ax (file length)
	; Give this info to the boot loader.
	mov ax,[size]
	mov bl,al
	and bl,0x0f
	or [boot_image+64],bl
	mov cl,4
	shr ax,cl
	mov bl,al
	and bl,0x0f
	or [boot_image+63],bl
	shr ax,cl
	mov bl,al
	and bl,0x0f
	or [boot_image+62],bl
	shr ax,cl
	mov bl,al
	and bl,0x0f
	or [boot_image+61],bl

	;
	; Send the bootstrap code to the transputer
	;
	mov cx,boot_image_end-boot_image
	mov al,cl
	call write_byte
	mov bx,boot_image
.1:
	mov al,[bx]
	call write_byte
	inc bx
	loop .1

	call wait_a_little
	call wait_a_little

	;
	; Send the main program to the transputer
	;
	mov cx,[size]
	mov bx,input_data
.2:     
	mov al,[bx]
	call write_byte
	inc bx
	loop .2

	call wait_a_little
	call wait_a_little

	mov al,[arguments]
	or al,al
	mov al,0
	je .3
	mov al,1
.3:
	mov byte [mode],al
	mov byte [status],0
	mov word [pointer],0
	mov word [output],buffer

main_loop:
	mov dx,0x0153
	in al,dx
	test al,0x01            ; Ready to send data?
	je .19                  ; No, jump.

	mov al,[mode]
	cmp al,1                ; Pascal mode?
	je .20                  ; Yes, jump.
				; Simple mode then.
	mov ah,0x01
	int 0x16                ; Key available?
	je .19                  ; No, jump.

	mov ah,0x00             ; Read key.
	int 0x16
	mov dx,0x151
	out dx,al               ; Send to transputer.
.19:
	jmp .1

.20:
	;
	; Send more data to the transputer input.
	;
	cld
	mov si,[pointer]        ; Get file pointer.
	mov al,[status]
	test al,1
	je .3
	and al,0xfe
	mov [status],al
	mov al,0x0a
	jmp .6
.3:
	test si,si
	jne .2
	mov al,[0x0080]         ; Another file ready?
	test al,al
	jne .12
	mov al,0x1a             ; Send end-of-file
	jmp .6

.12:
	mov dx,arguments
	call load_file

	call move_arguments

	mov si,[pointer]
.2:     lodsb
	cmp al,0x0d
	je .5
	cmp al,0x0a
	jne .6
	mov al,[status]
	or al,1
	mov [status],al
	mov al,0x0d
	inc word [size]

.6:     mov dx,0x0151
	out dx,al

.5:     mov [pointer],si
	dec word [size]
	jne main_loop
	xor si,si
	mov [pointer],si
	jmp main_loop   

	;
	; Handle transputer output
	;
.1:
	mov dx,0x0152
	in al,dx
	test al,0x01
	jne .21
	jmp main_loop
.21:
	mov dx,0x0150
	in al,dx
	cmp byte [mode],0
	jne .26
	mov ah,0x0e
	mov bx,0x0007
	int 0x10
	jmp main_loop
.26:
	mov di,[output]
	stosb
	mov [output],di
	cmp al,0x04
	je .7
	cmp al,0x0a     ; End of line?
	je .7           ; Yes, jump.
	cmp al,0x41     ; If last byte is letter...
	jb .15
	cmp al,0x7b
	jnb .15
	cmp al,0x5b
	jb .25
	cmp al,0x61
	jnb .25
.15:
	jmp main_loop
.25:
	mov al,[buffer]
	cmp al,0x1b     ; ...and first byte is Esc?
	jne .15         ; No, keep saving data.
.7:
	mov si,buffer
	lodsb
	cmp al,0x01     ; Source code?
	jne .8
	mov ah,0x40
	mov bx,0x0002   ; STDERR
	mov cx,[output]
	sub cx,si
	mov dx,si
	int 0x21
	push cs
	pop ds
	push cs
	pop es
	mov word [output],buffer
	jmp main_loop

.8:     cmp al,0x02     ; Error?
	jne .9
	mov ah,0x40
	mov bx,0x0002   ; STDERR
	mov cx,[output]
	sub cx,si
	mov dx,si
	int 0x21
	push cs
	pop ds
	push cs
	pop es
	mov word [output],buffer
	jmp main_loop

.9:     cmp al,0x03     ; End of compilation (error)?
	jne .10
	mov ah,0x40
	mov bx,0x0002   ; STDERR
	mov dx,message_1_start
	mov cx,message_1_end
	sub cx,dx
	int 0x21
	int 0x20

.10:    cmp al,0x04     ; End of compilation (all good)?
	jne .11
	mov ah,0x40
	mov bx,0x0002   ; STDERR
	mov dx,message_2_start
	mov cx,message_2_end
	sub cx,dx
	int 0x21
	int 0x20
.11:
	cmp al,0x1b     ; Escape sequence?
	je .22
	jmp .14
.22:
	mov bx,[output]
	mov al,[bx-1]
	cmp al,0x47     ; G
	jne .16
	mov ax,$0013    ; Graphics mode
	int $10
	jmp .17
.16:
	cmp al,0x54     ; T
	jne .18
	mov ax,$0002    ; Text mode
	int $10
	jmp .17
.18:
	cmp al,0x50     ; P
	je .24
	jmp .17
.24:

	mov al,[si]
	cmp al,'['
	jne $+3
	inc si

	call atoi
	cmp ax,320
	jnb .23
	push ax
	mov al,[si]
	cmp al,';'
	jne $+3
	inc si

	call atoi
	pop bx
	cmp ax,200
	jnb .23
	mov cx,320
	mul cx
	add ax,bx
	push ax

	mov al,[si]
	cmp al,';'
	jne $+3
	inc si
	call atoi
	mov cx,30
	mul cx
	push ax

	mov al,[si]
	cmp al,';'
	jne $+3
	inc si
	call atoi
	mov cx,59
	mul cx
	push ax

	mov al,[si]
	cmp al,';'
	jne $+3
	inc si
	call atoi
	mov cx,11
	mul cx
	pop bx
	add ax,bx
	pop bx
	add ax,bx
	xor dx,dx
	mov cx,1600
	div cx
	add al,$10
	pop bx

	push ds
	mov cx,0xa000
	mov ds,cx
	mov [bx],al
	pop ds
.23:
	mov word [output],buffer
	jmp main_loop
.14:    
	dec si
	mov ah,0x40
	mov bx,0x0001   ; STDOUT
	mov dx,si
	mov cx,[output]
	sub cx,dx
	int 0x21
.17:
	mov word [output],buffer
	jmp main_loop   

atoi:
	xor ax,ax
.1:
	mov bl,[si]
	cmp bl,0x30
	jb .2
	cmp bl,0x3a
	jnb .2
	sub bl,0x30
	mov bh,0
	mov cx,10
	mul cx
	add ax,bx
	inc si
	jmp .1
.2:
	ret

message_0_start:
	db "Unable to open file",0x0d,0x0a
message_0_end:

message_1_start:
	db "Compilation error!",0x0d,0x0a
message_1_end:

message_2_start:
	db "Compilation successful!",0x0d,0x0a
message_2_end:

	;
	; Parse arguments.
	;
parse_arguments:
	cld
	mov si,0x0081
	mov di,0x0080
.1:
	lodsb
	cmp al,0x0d
	je .5
	cmp al,' '
	je .1
.2:
	stosb
	lodsb
	cmp al,0x0d
	je .3
	cmp al,' '
	jne .2
.3:     push ax
	xor al,al
	stosb
	pop ax
	cmp al,0x0d
	jne .1
.5:
	xor al,al
	stosb
	ret

	;
	; Move arguments.
	;
move_arguments:
	cld
	mov si,0x0080
	mov di,0x0080
.1:     lodsb
	cmp al,0
	je .4
.5:
	lodsb
	cmp al,0
	jne .5

.2:     lodsb
	cmp al,0
	je .4
	stosb
.3:
	lodsb
	stosb
	cmp al,0
	jne .3
	jmp .2

.4:     stosb
	ret

dos_error:
	mov ah,0x40
	mov bx,0x0002   ; STDERR
	mov dx,message_0_start
	mov cx,message_0_end
	sub cx,dx
	int 0x21
	int 0x20

	;
	; Load a complete file into memory.
	;
load_file:
	push cs
	pop ds
	push cs
	pop es
	mov ax,0x3d00           ; Open file, read only.
	int 0x21
	jb dos_error
	mov [input],ax

	mov ax,0x4202           ; Go to end of file.
	mov bx,[input]
	xor cx,cx
	xor dx,dx
	int 0x21
	mov [size],ax

	mov ax,0x4200           ; Go to start of file.
	mov bx,[input]
	xor cx,cx
	xor dx,dx
	int 0x21

	mov ah,0x3f             ; Read file.
	mov bx,[input]
	mov cx,[size]
	mov dx,input_data
	mov [pointer],dx
	int 0x21

	mov bx,[input]
	mov ah,0x3e             ; Close file.
	int 0x21

	xor bx,bx
	mov [input],bx
	push cs
	pop ds
	push cs
	pop es
	ret

	;
	; Reset transputer.
	;
reset_transputer:
	; Initial state
	mov dx,0x0160
	mov al,0
	out dx,al
	mov dx,0x0161
	mov al,0        ; We don't want "analyse"
	out dx,al
	call wait_a_little
	call wait_a_little

	; Reset
	mov dx,0x0160
	mov al,1
	out dx,al
	call wait_a_little
	call wait_a_little
	call wait_a_little
	call wait_a_little

	; Deassert reset
	mov dx,0x0160
	mov al,0
	out dx,al
	call wait_a_little
	call wait_a_little
	call wait_a_little
	call wait_a_little
	call wait_a_little
	call wait_a_little
	call wait_a_little
	call wait_a_little
	ret

	;
	; Wait 1/18.2 sec.
	;
wait_a_little:
	mov ah,0x00     ; Read real-time clock. 
	int 0x1a        
	mov bx,dx
.1:
	mov ah,0x00     ; Read real-time clock.
	int 0x1a
	cmp dx,bx
	je .1
	ret

	;
	; Write byte via link
	;
write_byte:
	push ax
	mov dx,0x0153
.1:     in al,dx
	test al,1
	je .1
	pop ax
	mov dx,0x0151
	out dx,al
	ret

	;
	; Read byte via link
	;
read_byte:
	push ax
	mov dx,0x0152
.1:     in al,dx
	test al,1
	je .1
	pop ax
	mov dx,0x0150
	in al,dx
	ret

boot_image:
	db 0x24, 0xf2                  ; mint 
	db 0x22, 0x20, 0x2f, 0x2f, 0x88   ; adc 0x20ff8 
	db 0x23, 0xfc                  ; gajw 
	db 0x22, 0xf9                  ; testerr 
	db 0x25, 0xf7                  ; clrhalterr 
	db 0x29, 0xfc                  ; fptesterr 
	db 0x24, 0xf2                  ; mint 
	db 0x21, 0xf8                  ; sthf 
	db 0x24, 0xf2                  ; mint 
	db 0x21, 0xfc                  ; stlf 
	db 0x24, 0xf2                  ; mint 
	db 0x24, 0xf2                  ; mint 
	db 0xe9                        ; stnl 9 
	db 0x24, 0xf2                  ; mint 
	db 0x24, 0xf2                  ; mint 
	db 0xea                        ; stnl 10 
	db 0x40                        ; ldc 0 
	db 0x25, 0xf4                  ; sttimer 
	db 0x27, 0xfa                  ; timerdisableh 
	db 0x27, 0xfb                  ; timerdisablel 
	db 0x40                        ; ldc 0 
	db 0xd0                        ; stl 0 
	db 0x49                        ; ldc 9 
	db 0xd1                        ; stl 1 
	db 0x70                        ; ldl 0 
	db 0x24, 0xf2                  ; mint 
	db 0xfa                        ; wsub 
	db 0x21, 0xf2                  ; resetch 
	db 0x10                        ; ldlp 0 
	db 0x4a                        ; ldc 10 
	db 0x22, 0xf1                  ; lend 
	db 0x24, 0xf2                  ; mint 
	db 0x24, 0x50                  ; ldnlp 64 
	db 0x24, 0xf2                  ; mint 
	db 0x54                        ; ldnlp 4 
	db 0x20, 0x20, 0x20, 0x40      ; ldc initial_code_length 
	db 0xf7                        ; in 
	db 0x24, 0x9c                  ; call 0x80000100 
	db 0x2f, 0xff ;0x60, 0x0c
boot_image_end:

input:          equ $
pointer:        equ $+2
size:           equ $+4
mode:           equ $+6
status:         equ $+7
output:         equ $+8

buffer:         equ $+0x000a
buffer_end:     equ $+0x040a
input_data:     equ $+0x040a

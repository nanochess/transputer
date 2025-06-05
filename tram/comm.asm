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

	call bootstrap

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

message_1_start:
	db "Compilation error!",0x0d,0x0a
message_1_end:

message_2_start:
	db "Compilation successful!",0x0d,0x0a
message_2_end:

	%include "startup.asm"

input:          equ $
pointer:        equ $+2
size:           equ $+4
mode:           equ $+6
status:         equ $+7
output:         equ $+8

buffer:         equ $+0x000a
buffer_end:     equ $+0x040a
input_data:     equ $+0x040a

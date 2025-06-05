	;
	; Transputer interface for my small operating system.
	;
	; by Oscar Toledo G.
	; https://nanochess.org/
	;
	; Creation date: Jun/04/2025.
	;

	org 0x0100

	;
	; This program allows to use my basic operating system as intended at
	; the time (1995) It uses a 1.44mb floppy disk as the drive for the
	; operating system. You can do complete cycles of editing, compilation,
	; assembly, and execution without exiting from the operating system.
	;
	; Usage instructions:
	; * Copy the os/disk.img file into a real 1.44 mb floppy.
	; * Make sure you have os/maestro.cmg in the same directory as this program.
	; * Run this program as: comm2 maestro.cmg
	;

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

	mov ax,0x0002
	int 0x10

	mov byte [status],0
	mov word [pointer],0
	mov word [output],buffer

handle_service:
	mov bx,buffer
	xor bp,bp
main_loop:
	;
	; Wait for the transputer to ask for a service
	;
	mov dx,0x0152
	in al,dx
	test al,0x01
	je main_loop
	mov dx,0x0150
	in al,dx		; Read byte
	mov [bx],al
	inc bx
	test bp,bp		; Known limit?
	je read_service		; No, jump.
	cmp bx,bp		; Reached limit?
	jb main_loop		; No, wait for more data.
handle_now:
	jmp di

	; The first time we need to know the message size
read_service:
	cmp bx,buffer+2         ; Wait for two bytes.
	jb main_loop
	mov si,buffer
	call read_pseudohex     ; Read the service code.        
	cmp al,0x01
	jb .2
	cmp al,0x09
	jb .3
.2:
	mov bx,message_1
	call show_message
	int 0x20

.3:     mov ah,0
	shl ax,1
	shl ax,1
	mov bx,services_length-4  ; Index into the table.
	add bx,ax
	mov ax,[bx]
	mov di,[bx+2]           ; di = service routine.
	cmp ax,2
	je handle_now
	add ax,buffer
	xchg ax,bp              ; bp = expected pointer.
	mov bx,buffer+2
	jmp main_loop

	;
	; Read a pseudo-hexadecimal byte
	;
read_pseudohex:
	lodsb
	shl al,1
	shl al,1
	shl al,1
	shl al,1		; High nibble.
	mov ah,al
	lodsb
	and al,0x0f		; Low nibble.
	or al,ah
	ret

	;
	; Read a sector from the floppy disk.
	;
read_sector:
	mov si,buffer+2
	call read_pseudohex
	mov ch,al		; Track
	call read_pseudohex
	mov dh,al		; Head
	call read_pseudohex
	mov cl,al		; Sector
.2:     mov dl,0
	mov ax,0x0201
	push ds
	push es
	mov bx,buffer+1
	int 0x13
	pop es
	pop ds
	jnb .1
	cmp ah,0x06     	; Disk change?
	je .2
	stc
.1:
	mov al,0
	adc al,0
	mov [buffer],al
	cld
	mov si,buffer
	mov cx,513
send_response:
	lodsb
	call write_byte
	loop send_response
	jmp handle_service

	;
	; Write a sector to the floppy disk.
	;
write_sector:
	mov si,buffer+2
	call read_pseudohex
	mov ch,al		; Track
	call read_pseudohex
	mov dh,al		; Head
	call read_pseudohex
	mov cl,al		; Sector
	mov dl,0
	push cx
	push dx
	mov bx,buffer
	mov cx,512
.0:     call read_pseudohex
	mov [bx],al
	inc bx
	loop .0
	pop dx
	pop cx
.2:
	mov ax,0x0301
	push ds
	push es
	mov bx,buffer
	int 0x13
	pop es
	pop ds
	jnb .1
	cmp ah,0x06     ; Disk change
	je .2
	stc
.1:
	mov al,0
	adc al,0
	mov [buffer],al
	cld
	mov si,buffer
	mov cx,1
	jmp send_response

	;
	; Format a track (floppy disk 1.44mb)
	;
format_track:
	mov si,buffer+2
	call read_pseudohex
	mov ch,al		; Track
	call read_pseudohex
	mov dh,al		; Head
	mov bx,buffer
	mov ah,1
.2:
	mov [bx],ch		; Track
	inc bx
	mov [bx],dh		; Head
	inc bx
	mov [bx],ah		; Sector
	inc bx
	mov byte [bx],2		; 512-bytes
	inc bx
	inc ah
	cmp ah,19
	jne .2

	mov ax,0x0512
	mov cl,0x01
	mov dl,0x00
	mov bx,buffer
	push ds
	push es
	int 0x13
	pop es
	pop ds
	jnb .1

.1:
	mov al,0
	adc al,0
	mov [buffer],al
	cld
	mov si,buffer
	mov cx,1
	jmp send_response
	
	;
	; Update one line on the screen.
	;
update_screen:
	mov si,buffer+2
	lodsb
	mov ah,160
	mul ah
	mov di,ax
	push es
	mov ax,0xb800
	mov es,ax
	mov cx,80
.1:
	call read_pseudohex
	stosb
	call read_pseudohex
	stosb
	loop .1

	pop es
	jmp handle_service

	;
	; Get pressed key status.
	;
pressed_key:
	mov ah,0x01
	int 0x16
	je .1
	mov ah,0x00
	int 0x16
	mov bx,scancode
.4:
	mov dh,[bx]
	test dh,dh
	jz .2
	cmp ah,dh
	jne .3
	mov al,[bx+1]
	jmp .2

.3:	inc bx
	inc bx
	jmp .4
.1:
	mov al,0x00
.2:
	mov [buffer],al
	cld
	mov si,buffer
	mov cx,1
	jmp send_response

scancode:
	db 0x3b,0x01
	db 0x3c,0x02
	db 0x3d,0x03
	db 0x3e,0x04
	db 0x3f,0x05
	db 0x40,0x06
	db 0x41,0x07
	db 0x42,0x08
	db 0x43,0x09
	db 0x44,0x0a
	db 0x85,0x0b
	db 0x86,0x0c
	db 0x47,0x17
	db 0x48,0x18
	db 0x49,0x19
	db 0x4b,0x14
	db 0x4c,0x15
	db 0x4d,0x16
	db 0x4f,0x11
	db 0x50,0x12
	db 0x51,0x13
	db 0x52,0x10
	db 0x53,0x0e
	db 0x00
	
	;
	; Get current time.
	;
get_time:
	mov ah,0x02
	int 0x1a
	mov al,dh
	call bcd2bin
	mov [buffer],al
	mov al,cl
	call bcd2bin
	mov [buffer+1],al
	mov al,ch
	call bcd2bin
	mov [buffer+2],al
	mov ah,0x04
	int 0x1a
	mov al,dl
	call bcd2bin
	mov [buffer+3],al
	mov al,dh
	call bcd2bin
	mov [buffer+4],al
	mov al,ch
	call bcd2bin
	mov ah,100
	mul ah
	mov bx,ax
	mov al,cl
	call bcd2bin
	mov ah,0
	add ax,bx
	sub ax,1900
	mov [buffer+5],al
	cld
	mov si,buffer
	mov cx,6
	jmp send_response

	;
	; AL contains a BCD byte.
	; (One digit in the top nibble, one digit in the bottom nibble)
bcd2bin:
	mov ah,al
	and al,0x0f     ; Extract the lower nibble (0-9)
	shr ah,1
	shr ah,1
	shr ah,1
	and ah,0xfe     ; Shift the upper nibble (0-9) keep x2
	add al,ah       ; x2
	add al,ah       ; x4
	add al,ah       ; x6
	add al,ah       ; x8
	add al,ah       ; x10
	ret

	;
	; Set the current cursor position.
	;
cursor_position:
	mov si,buffer+2
	call read_pseudohex
	push ax
	call read_pseudohex
	mov bh,al
	pop ax
	mov bl,al
	mov ax,bx
	xor dx,dx
	mov cx,80
	div cx
	mov dh,al
	mov bh,0
	mov ah,0x02
	int 0x10
	jmp handle_service

	;
	; Set the cursor shape.
	;
cursor_shape:
	mov si,buffer+2
	call read_pseudohex
	push ax
	call read_pseudohex
	mov cl,al
	pop ax
	mov ch,al
	mov ah,0x01
	int 0x10
	jmp handle_service

	;
	; Show a message 
	;
show_message:
	mov al,[bx]
	or al,al
	je .1
	push bx
	mov ah,0x0e
	mov bx,0x0007
	int 0x10
	pop bx
	inc bx
	jmp show_message
.1:
	ret

message_1:
	db "Unhandled service code",0x0d,0x0a

services_length:
	dw 8,read_sector                ; 01: Read sector
	dw 1032,write_sector            ; 02: Write sector
	dw 6,format_track               ; 03: Format track (floppy disk)
	dw 323,update_screen            ; 04: Update screen
	dw 2,pressed_key                ; 05: Pressed key
	dw 2,get_time                   ; 06: Get time
	dw 6,cursor_position            ; 07: Cursor position
	dw 6,cursor_shape               ; 08: Cursor shape

	%include "startup.asm"

input:          equ $
pointer:        equ $+2
size:           equ $+4
mode:           equ $+6
status:         equ $+7
output:         equ $+8

buffer:         equ $+0x000a
buffer_end:     equ $+0x000a+1152
input_data:     equ $+0x000a+1152

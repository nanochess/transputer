	;
	; Start up code and bootstrap for transputer
	;
	; by Oscar Toledo G.
	;
	; Creation date: Jun/04/2025.
	;

arguments:      equ 0x0080

	;
	; Bootstrap code.
	; Get initial image filename from the command line.
	;
bootstrap:
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
	jmp wait_a_little

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
	; Report DOS error and exit.
	;
dos_error:
	mov ah,0x40
	mov bx,0x0002   ; STDERR
	mov dx,message_0_start
	mov cx,message_0_end
	sub cx,dx
	int 0x21
	int 0x20

message_0_start:
	db "Unable to open file",0x0d,0x0a
message_0_end:

	;
	; >>>>>>>>>>>>>>>>>>>>
	;    Start of the transputer code.
	; >>>>>>>>>>>>>>>>>>>>
	;

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


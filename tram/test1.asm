	;
	; Programa de prueba de transputer
	;
	; por Oscar Toledo G.
	; https://nanochess.org/
	;
	; Fecha de creación: 21-may-2025.
	;

	;
	; Código de inicialización.
	;
	; De mi emulador de transputer en
	; https://github.com/nanochess/transputer
	;
	; Ensamblar con tasm, disponible en el mismo lugar.
	;
	mint
	adc 0x0ffc
	gajw
	testerr
	clrhalterr
	fptesterror
	mint
	sthf
	mint
	stlf
	mint
	mint
	stnl 9
	mint
	mint
	stnl 10
	ldc 0
	sttimer
	timerdisableh
	timerdisablel
	ldc 0
	stl 0
	ldc 9
	stl 1
	ldl 0
	mint
	wsub
	resetch
	ldlp 0
	ldc 10
	lend
	;
	; Comienza nuestro código
	; 
	mint
	ldc 0x6f6e616e	; ASCII "nano"
	outword
detenerse:
	j detenerse

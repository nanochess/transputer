{ Programa: Compilador de Pascal para el Inmos T805 }
{                                                   }
{ Módulo  : Controlador Maestro                     }
{                                                   }
{ (c) Copyright 1993-1994 Oscar Toledo G.           }
{          Todos los derechos reservados.           }

{ Procesa un bloque }
PROCEDURE bloque;
VAR ultsim2,apulit2,pila2,posproc2:INTEGER;
BEGIN
 ultsim2:=ultsim;
 apulit2:=apulit;
 pila2:=pila;
 posproc2:=posproc;
 posproc:=parb;
 pila:=1;
 IF profact>maxniv THEN error(50);
 IF lexema=CLABEL THEN plabel;
 IF lexema=CCONST THEN pconst;
 dentrotype:=TRUE;
 IF lexema=CTYPE  THEN ptype;
 dentrotype:=FALSE;
 IF lexema=CVAR   THEN pvar;
 pparinf[profact]:=pila;
 WHILE (lexema=CPROCEDURE) OR
       (lexema=CFUNCTION) DO
  IF lexema=CPROCEDURE THEN pproc ELSE pfunc;
 IF lexema=CBEGIN THEN pbegin ELSE error(25);
 posproc:=posproc2;
 pila:=pila2;
 apulit:=apulit2;
 ultsim:=ultsim2
END;

{ Procesa el encabezado PROGRAM }
PROCEDURE sprogram;
BEGIN
 compsig(CPROGRAM,37);
 compsig(CIDENT,4);
 obtlex;
 IF lexema=CPARENI THEN
 BEGIN
  REPEAT
   compsig(CIDENT,4);
   obtlex
  UNTIL lexema<>CCOMA;
  compara(CPAREND,22);
  obtlex
 END;
 compara(CPUNTOYCOMA,52);
 obtlex;
 writeln('j INIC');
 val[0]:=geneti;
 bloque(0);
 writeln('INIC:');
 writeln('ajw -(NUMCONS+NUMCONSLIB)');
 writeln('ldc CONSLIB-CONS1');
 writeln('ldpi');
 writeln('CONS1:');
 writeln('ldlp 0');
 writeln('ldc NUMCONSLIB*4');
 writeln('move');
 IF apucons>0 THEN
 BEGIN
{ Carga las constantes del programa (si existen) }
  writeln('ldc CONS-CONS2');
  writeln('ldpi');
  writeln('CONS2:');
  writeln('ldlp NUMCONSLIB');
  writeln('ldc NUMCONS*4');
  writeln('move');
 END;
{ Inicialización para los procedimientos NEW, DISPOSE, EOF y EOLN }
 writeln('call DINAMICA');
 writeln('ajw -5');
 writeln('stl 1'); { Apuntador a lista de bloques (NEW y DISPOSE) }
 writeln('stl 2'); { Apuntador a memoria libre }
 writeln('mint');  { Indica que no se ha asignado ningun bloque }
 writeln('stl 3');
 writeln('mint');  { Indica que no hay ningun caracter en el buffer }
 writeln('stl 4');
 writeln('ldc 0x3F579834');
 writeln('stl 0'); { Número aleatorio de comienzo }
{ Llama al programa principal con un apuntador a la tabla de constantes }
 writeln('ldlp 5+NUMCONSLIB');
 writeln('call P1');
 writeln('ajw NUMCONS+NUMCONSLIB+5');
{ Retorna al llamador }
 writeln('ret')
END;

{ Genera la lista de constantes compiladas }
PROCEDURE listacons;
VAR contador,contador2:INTEGER;
BEGIN
 writeln('CONS:');
 FOR contador:=0 TO apucons-1 DO
 BEGIN
  write('db ');
  FOR contador2:=1 TO 4 DO
  BEGIN
   write(ord(constantes[contador].bytes[contador2]):0);
   IF contador2<4 THEN write(',');
  END;
  writeln;
 END;
 writeln('NUMCONS:');
 writeln('equ ',apucons:0);
{ Esta etiqueta es muy útil para insertar datos extras junto con un }
{ programa Pascal y accederlos con _FINPROG }
 writeln('FINPROG:')
END;

{ Inicialización de tipos básicos }
PROCEDURE inicializa;
BEGIN
 cadena:='INTEGER';         { Tipo INTEGER }
 declara('T');
 num[ultsim]:=4;
 tipo[ultsim]:=ultsim+1;
 cadena:='';                { Subrango -2147483647 .. 2147483647 }
 declara('B');
 min[ultsim]:=$80000000;
 max[ultsim]:=$7FFFFFFF;
 num[ultsim]:=4;
 tipo[ultsim]:=2;
 cadena:='CHAR';            { Tipo CHAR }
 declara('T');
 num[ultsim]:=1;
 tipo[ultsim]:=ultsim+1;
 cadena:='';                { Subrango 0 .. 255 }
 declara('B');
 min[ultsim]:=0;
 max[ultsim]:=255;
 num[ultsim]:=1;
 tipo[ultsim]:=4;
 cadena:='REAL';            { Tipo REAL }
 declara('T');
 num[ultsim]:=4;
 tipo[ultsim]:=ultsim+1;
 cadena:='';                { es un tipo básico }
 declara('I');
 num[ultsim]:=4;
 tipo[ultsim]:=6;
 cadena:='BOOLEAN';         { Tipo BOOLEAN }
 declara('T');
 num[ultsim]:=4;
 tipo[ultsim]:=ultsim+1;
 cadena:='';                { enumerador }
 declara('E');
 tipo[ultsim]:=2;
 num[ultsim]:=4;
 min[ultsim]:=0;
 max[ultsim]:=1;
 prox[ultsim]:=ultsim+1;
 cadena:='FALSE';           { 0=FALSE }
 declara('C');
 num[ultsim]:=4;
 tipo[ultsim]:=8;
 val[ultsim]:=0;
 prox[ultsim]:=ultsim+1;
 cadena:='TRUE';            { 1=TRUE }
 declara('C');
 num[ultsim]:=4;
 tipo[ultsim]:=8;
 val[ultsim]:=1;
 prox[ultsim]:=0;
 cadena:='';                { Tipo vacio para NIL }
 declara('D');
 num[ultsim]:=4;
 tipo[ultsim]:=ultsim;
 min[ultsim]:=1;
 cadena:='MAXINT';          { MAXINT }
 declara('C');
 num[ultsim]:=4;
 tipo[ultsim]:=2;
 val[ultsim]:=$7FFFFFFF;
 cadena:='MININT';          { MININT }
 declara('C');
 num[ultsim]:=4;
 tipo[ultsim]:=2;
 val[ultsim]:=$80000000;
 cadena:='BYTE';            { Tipo BYTE }
 declara('T');
 num[ultsim]:=1;
 tipo[ultsim]:=4;
 cadena:='WORD';            { Tipo WORD }
 declara('T');
 num[ultsim]:=4;
 tipo[ultsim]:=2;
 cadena:='TEXT';            { TEXT = FILE OF CHAR }
 declara('O');
 num[ultsim]:=40;
 tipo[ultsim]:=4;
 cadena:='';                { Uso para tipos de conjuntos }
 declara('S');
 num[ultsim]:=32;
 tipo[ultsim]:=ultsim;
 cadena:='';                { Uso para operaciones STRING }
 declara('N');
 num[ultsim]:=256;
 tipo[ultsim]:=ultsim
END;

{ Programa principal }

BEGIN
 inicanalex;         { Inicializa el analizador léxico }
 profact:=0;         { Profundidad de anidamiento }
 apulit:=1;          { Apuntador al buffer literal }
 literal[0]:=chr(0); { Ahorra espacio en literales vacios }
 proxeti:=0;         { Proxima etiqueta a usar }
 ultsim:=0;          { ×ltimo identificador definido }
 numpar:=0;          { Número de parametros del procedimiento }
 clase[0]:='G';      { Esto evita problemas en la busqueda }
 apucons:=0;         { Número constantes definidas }
 aajw:=FALSE;        { Todavía no hay ajw a optimizar }
 oajw:=0;
 optcc:=FALSE;       { Optimación corto-circuito desactivada }
 escontrol:=FALSE;   { No esta dentro de IF, WHILE o REPEAT }
 afps:=FALSE;        { Todavía no hay asignación res. función real }
 astl:=FALSE;        { Todavía no hay asignación res. función entera }
 aenl:=FALSE;        { Todavía no se carga el enlace estático }
 twith:=0;           { Indica que no hay ningun WITH }
 pwith:=0;           { Apuntador al apuntador para el WITH }
 cadact:=0;          { Ninguna cadena en el programa }
 cadlit:=0;          { Almacenamiento de cadenas vacio }
 proxcad:=1;         { Proxima cadena a sacar del almacenamiento }
 inicializa;         { Inicializa tipos básicos }
 sprogram;           { Procesa bloque PROGRAM }
 compara(CPUNTO,9);  { Verífica que exista el punto de final }
 listacons;          { Lista las constantes }
 write(chr(4))       { Indica fin de compilación }
END.


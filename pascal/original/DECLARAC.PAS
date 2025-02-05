{ Programa: Compilador de Pascal para el Inmos T805 }
{                                                   }
{ Módulo  : Analizador de Declaraciones             }
{                                                   }
{ (c) Copyright 1993-1994 Oscar Toledo G.           }
{          Todos los derechos reservados.           }

{ Procesa BEGIN }
PROCEDURE pbegin;
VAR
 contador:INTEGER;
 claseres:CHAR;
BEGIN
 gepf(val[posproc]);    { Etiqueta del procedimiento }
 sevalegoto:=TRUE;      { Se autorizan los goto }
 apret:=0;              { El apuntador relativo en su posición inicial }
 aenl:=FALSE;           { El enlace estático aun no esta cargado }
 gpila(-pila);          { Espacio para variables locales }
 sbegin;                { Procesa bloque }
 gpila(pila);           { Deasigna variables locales }
 pila:=0;               { Vacia la pila }
 IF clase[posproc]='F' THEN   { Optimiza resultado función }
 BEGIN
  contador:=posproc;
  WHILE prox[contador]<>0 DO contador:=prox[contador];
  IF tipo[contador]=6 THEN
  BEGIN
   IF afps THEN afps:=FALSE
   ELSE
   BEGIN
    yoptimiza(0,0);
    writeln('ldlp ',abs(val[contador]):0);
    writeln('fpldnlsn')
   END;
  END
  ELSE
  BEGIN
   IF astl THEN astl:=FALSE
   ELSE
   BEGIN
    yoptimiza(0,0);
    claseres:=clase[tipo[contador]];
    IF (claseres<>'S') AND (claseres<>'N') THEN
    BEGIN
     IF (claseres='A') OR (claseres='R') OR
        (claseres='O') THEN writeln('ldlp ',abs(val[contador]):0)
                       ELSE writeln('ldl ',abs(val[contador]):0)
    END;
   END;
  END;
 END;
 yoptimiza(0,0);         { Vacia cualquier optimización }
 writeln('ret');         { Retorno del procedimiento }
 vaciacad                { Vacia el almacenamiento de cadenas }
END;

PROCEDURE dfvar;
BEGIN
 compara(CIDENT,4);
 declara('V');
 obtlex
END;

FUNCTION atype:INTEGER; FORWARD;

{ Aquí se procesa el tipo de las variables }
PROCEDURE dtipof(inicio,final:INTEGER);
VAR contador,apuntador:INTEGER;
BEGIN
 IF tpaso<>3 THEN
 BEGIN
  compara(CDOSPUNTOS,5);
  obtlex;
  apuntador:=atype
 END
 ELSE apuntador:=0;
 FOR contador:=inicio TO final DO
 BEGIN
  tipo[contador]:=apuntador;
  val[contador]:=pila;
  paso[contador]:=tpaso;
  CASE tpaso OF
   0,1,2: num[contador]:=num[apuntador];
   3,4: num[contador]:=8
  END;
  CASE tpaso OF
   0,2: pila:=pila+((num[contador]+3) DIV 4);
   1: pila:=pila+1;
   3,4: pila:=pila+2
  END;
 END;
END;

{ Procesa las variables/parametros de un procedimiento/función }
PROCEDURE dvar1;
VAR
 coma:BOOLEAN;
 primvar:INTEGER;
BEGIN
 primvar:=0;
 coma:=FALSE;
 REPEAT
  IF coma THEN obtlex;
  dfvar;
  IF primvar=0 THEN primvar:=ultsim;
  numpar:=numpar+1;
  numvar:=numvar+1;
  coma:=TRUE
 UNTIL lexema<>CCOMA;
 IF tpaso<>3 THEN dtipof(primvar,primvar+(numvar-1))
END;

{ Procesa lo que sigue despues de un VAR }
PROCEDURE dvar;
BEGIN
 numvar:=0;
 tpaso:=0;
 dvar1
END;

{ Aquí procesa los parametros de un procedimiento/función }
PROCEDURE dvarp;
BEGIN
 numvar:=0;
 CASE lexema OF
  CVAR:        { Paso por dirección }
   BEGIN
    obtlex;
    tpaso:=1
   END;
  CPROCEDURE:  { Pasa un procedimiento cómo parametro }
   BEGIN
    obtlex;
    tpaso:=3
   END;
  CFUNCTION:   { Pasa una función cómo parametro }
   BEGIN
    obtlex;
    tpaso:=4
   END
  ELSE tpaso:=0
 END;
 dvar1
END;

{ Procesa las declaraciones VAR del bloque }
PROCEDURE pvar;
BEGIN
 obtlex;
 REPEAT
  dvar;
  compara(CPUNTOYCOMA,10);
  obtlex
 UNTIL (lexema=CPROCEDURE) OR
       (lexema=CFUNCTION) OR
       (lexema=CBEGIN)
END;

PROCEDURE bloque(parb:INTEGER); FORWARD;

{ Procesa los parametros de un procedimiento/funcion }
PROCEDURE par;
VAR
 pilaant,contador:INTEGER;
 simproc,parproc:INTEGER;
 primero,resultado:INTEGER;
 resta,libres:INTEGER;
BEGIN
 tipo[ultsim]:=0;
 profact:=profact+1;
 simproc:=ultsim;
 resfun:=numpar;
 pilaant:=pila;           { Salva la pila anterior }
 obtlex;
 pila:=4;
{ Analiza los parametros (si los hay) }
 IF lexema=CPARENI THEN
 BEGIN
  REPEAT
   obtlex;
   dvarp
  UNTIL lexema<>CPUNTOYCOMA;
  compara(CPAREND,22);
  obtlex
 END;
{ Sí es función verifíca el tipo de resultado }
 IF proced THEN resultado:=0
 ELSE
 BEGIN
  cadena:=funcion;
  numvar:=1;
  tpaso:=2;      { Indica que es resultado de función }
  declara('V');
  resultado:=ultsim;
  dtipof(ultsim,ultsim);   { Cualquier resultado de STRING[##] es STRING }
  IF clase[tipo[resultado]]='N' THEN
  BEGIN
   pila:=pila-((num[tipo[resultado]]+3) DIV 4);
   num[tipo[resultado]]:=256;
   max[sub[tipo[resultado]]]:=255
  END;
 END;
 compara(CPUNTOYCOMA,5);
 obtlex;
 parproc:=0;
 primero:=simproc;
 prox[primero]:=0;
{ Invierte el desplazamiento para indicar a las rutinas de carga de }
{ variables que son parametros, también hace que las variables enteras, }
{ apuntadores, o paso por dirección se pasen en los registros del }
{ procesador. }
 resta:=0;
 libres:=2;
 FOR contador:=simproc TO ultsim DO
 BEGIN
  IF clase[contador]='V' THEN
  BEGIN
   IF ((tipobase(tipo[contador])=2) OR
       (tipobase(tipo[contador])=4) OR
       (clase[tipo[contador]]='D') OR
       (paso[contador]=1)) AND
      (libres<>0) AND (paso[contador]<=1) THEN
   BEGIN
    val[contador]:=4-libres;
    resta:=resta+1;
    libres:=libres-1
   END
   ELSE val[contador]:=val[contador]-resta;
   prox[primero]:=contador;
   primero:=contador;
   IF contador<>resultado THEN parproc:=parproc+1;
   val[contador]:=-val[contador]
  END;
 END;
 num[simproc]:=parproc; { Número total de parametros }
 val[simproc]:=geneti;  { Etiqueta del procedimiento }
{ Verifica FORWARD }
 IF lexema=CFORWARD THEN
 BEGIN
  obtlex;
  paso[simproc]:=1
 END
 ELSE
 BEGIN
  paso[simproc]:=0;
  bloque(simproc)
 END;
 contador:=simproc;
 WHILE prox[contador]<>0 DO  { Desactiva los parametros del proc. }
 BEGIN
  contador:=prox[contador];
  clase[contador]:='Z'
 END;
 pila:=pilaant;
 profact:=profact-1;
 compara(CPUNTOYCOMA,10);
 obtlex
END;

{ Tratamiento de procedimientos/funciones con FORWARD redeclarados }
PROCEDURE tratafor(pos:INTEGER);
VAR
 contpar:INTEGER;
BEGIN
 paso[pos]:=0;
 numpar:=num[pos];
 profact:=profact+1;
 compsig(CPUNTOYCOMA,10);
 obtlex;
 contpar:=pos;
 WHILE prox[contpar]<>0 DO  { Activa los pars. del procedimiento }
 BEGIN
  contpar:=prox[contpar];
  clase[contpar]:='V'
 END;
 bloque(pos);
 contpar:=pos;
 WHILE prox[contpar]<>0 DO  { Desactiva los pars. del procedimiento }
 BEGIN
  contpar:=prox[contpar];
  clase[contpar]:='Z'
 END;
 profact:=profact-1;
 compara(CPUNTOYCOMA,10);
 obtlex
END;

{ Verifíca FORWARD }
PROCEDURE vforward;
BEGIN
 busca;
 especial:=possim;
 IF possim<>0 THEN
  IF prof[possim]=profact THEN
   IF paso[possim]=0 THEN error(27);
END;

{ Procesa PROCEDURE }
PROCEDURE pproc;
VAR contador:INTEGER;
BEGIN
 compsig(CIDENT,4);
 vforward;
 IF especial<>0 THEN tratafor(especial)
 ELSE
 BEGIN
  numpar:=0;
  declara('P');
  proced:=TRUE;
  par
 END;
END;

{ Procesa FUNCTION }
PROCEDURE pfunc;
VAR contador:INTEGER;
BEGIN
 compsig(CIDENT,4);
 vforward;
 IF especial<>0 THEN tratafor(especial)
 ELSE
 BEGIN
  numpar:=1;
  declara('F');
  proced:=FALSE;
  funcion:=cadena;
  par
 END;
END;

{ Procesa un tipo enumerado }
FUNCTION denum:INTEGER;
VAR minsub,maxsub,posenum,contador:INTEGER;
BEGIN
 cadena:='';
 declara('E');     { Tipo enumerado }
 posenum:=ultsim;
 minsub:=0;        { Mínimo actual }
 maxsub:=-1;       { Máximo actual }
 REPEAT
  compsig(CIDENT,4);
  busca;
  IF possim<>0 THEN error(27);
  declara('C');            { Declara una constante }
  prox[ultsim-1]:=ultsim;  { Agrega a la lista }
  maxsub:=maxsub+1;        { Incrementa el máximo actual }
  val[ultsim]:=maxsub;     { Le da un valor }
  tipo[ultsim]:=posenum;   { Indica su tipo }
  obtlex
 UNTIL lexema<>CCOMA;
 compara(CPAREND,22);
 min[posenum]:=minsub;     { Mínimo enumerado }
 max[posenum]:=maxsub;     { Máximo enumerado }
 num[posenum]:=4;          { Tamaño en bytes }
 tipo[posenum]:=2;         { Tipo INTEGER }
 denum:=posenum;
 obtlex
END;

{ Procesa una lista de franjas }
PROCEDURE listafra(VAR ultrecor,numfranja,tamrec,pilarec:INTEGER);
VAR
 finlista:BOOLEAN;
 numcoma,franja,tiporec,contador:INTEGER;
BEGIN
 numcoma:=0;
 finlista:=FALSE;
 REPEAT
  compara(CIDENT,4);         { Verifica nombre de franja }
  declara('J');              { Lo declara como franja }
  prox[ultrecor]:=ultsim;    { Lo agrega a la lista de franjas }
  ultrecor:=ultsim;
  obtlex;
  numfranja:=numfranja+1;    { Incrementa el contador de franjas }
  numcoma:=numcoma+1;
  IF lexema=CCOMA THEN obtlex ELSE finlista:=TRUE
 UNTIL finlista;
 compara(CDOSPUNTOS,5);
 obtlex;
 franja:=ultsim;
 tiporec:=atype;             { Calcula el tipo }
 FOR contador:=franja-(numcoma-1) TO franja DO
 BEGIN
  tipo[contador]:=tiporec;   { Asigna el mismo tipo a las franjas }
  val[contador]:=pilarec;    { Su posición relativa }
  num[contador]:=num[tiporec]; { y su tamaño en bytes }
  tamrec:=tamrec+((num[tiporec]+3) DIV 4)*4;  { Tamaño del record }
  pilarec:=pilarec+((num[tiporec]+3) DIV 4)
 END;
END;

{ Procesa la parte variable de un RECORD }
PROCEDURE drecvar(VAR ultrecor,numfranja,tamrec,pilarec:INTEGER);
VAR
 franja,tiporec,incpila,posprec,maximo,maxpila,tipovar:INTEGER;
 variante:BOOLEAN;
BEGIN
 compsig(CIDENT,4);
 busca;
 IF NOT ((possim<>0) AND       { Sí no es tipo declara una franja }
    (clase[possim]='T')) THEN
 BEGIN
  declara('J');                { Declara la franja }
  prox[ultrecor]:=ultsim;      { Añade a la lista }
  ultrecor:=ultsim;
  obtlex;
  numfranja:=numfranja+1;      { Incrementa el número de franjas }
  compara(CDOSPUNTOS,5);
  obtlex;
  franja:=ultsim;
  tiporec:=atype;              { Calcula su tipo }
  tipo[franja]:=tiporec;       { Indica el tipo }
  val[franja]:=pilarec;        { Su posición relativa }
  num[franja]:=num[tiporec];   { Su tamaño en bytes }
  tamrec:=tamrec+((num[tiporec]+3) DIV 4)*4;  { Nuevo tamaño del RECORD }
  pilarec:=pilarec+((num[tiporec]+3) DIV 4);
  tipovar:=tipobase(tiporec);  { Tipo del CASE }
 END
 ELSE
 BEGIN
  tipovar:=tipobase(tipo[possim]);  { Tipo del CASE }
  obtlex
 END;
 compara(COF,26);
 maximo:=tamrec;
 maxpila:=pilarec;
 REPEAT
  obtlex;
  const1;                            { Saca la constante }
  IF tipoi=6 THEN error(14);         { No puede ser REAL }
  IF tipoi<>tipovar THEN error(14);  { Debe ser del mismo tipo }
  compsig(CDOSPUNTOS,5);
  compsig(CPARENI,31);
  posprec:=pilarec;
  incpila:=tamrec;
  variante:=FALSE;
  REPEAT
   obtlex;
        IF lexema=CCASE  THEN variante:=TRUE
   ELSE IF lexema=CIDENT THEN
    listafra(ultrecor,numfranja,incpila,posprec)
   ELSE error(12)
  UNTIL (lexema<>CCOMA) OR (variante=TRUE);
  IF variante=TRUE THEN drecvar(ultrecor,numfranja,incpila,posprec);
  compara(CPAREND,22);
  obtlex;
  IF incpila>maximo THEN       { Busca el caso más largo }
  BEGIN
   maximo:=incpila;
   maxpila:=posprec
  END;
 UNTIL lexema<>CPUNTOYCOMA;
 pilarec:=maxpila;
 tamrec:=maximo;
END;

{ Procesa un RECORD }
FUNCTION drecord:INTEGER;
VAR
 posrecor,ultrecor,contador:INTEGER;
 numfranja,pilarec,tamrec:INTEGER;
 variante:BOOLEAN;
BEGIN
 cadena:='';
 declara('R');             { Declara un RECORD }
 numfranja:=0;
 pilarec:=0;
 tamrec:=0;
 posrecor:=ultsim;
 ultrecor:=posrecor;
 variante:=FALSE;
 REPEAT
  obtlex;
       IF lexema=CCASE  THEN variante:=TRUE  { Verifica parte variante }
  ELSE IF lexema=CIDENT THEN listafra(ultrecor,numfranja,tamrec,pilarec)
 UNTIL (lexema<>CPUNTOYCOMA) OR (variante=TRUE);
 IF variante=TRUE THEN drecvar(ultrecor,numfranja,tamrec,pilarec);
 compara(CEND,17);
 num[posrecor]:=tamrec;    { Tamaño total del RECORD }
 min[posrecor]:=1;         { Número mínimo de franja }
 max[posrecor]:=numfranja; { Número total de franjas }
 drecord:=posrecor;
 obtlex;
END;

{ Procesa los tamaños de las matrices (recursiva) }
PROCEDURE darray2(VAR posarray:INTEGER);
VAR minim,maxim,tipoarray,contador,tarray:INTEGER;
BEGIN
 tarray:=atype;
 IF (clase[tarray]<>'B') AND (clase[tarray]<>'E') THEN error(36);
 cadena:='';
 declara('A');
 posarray:=ultsim;
 IF lexema=CCOMA THEN         { Multidimensional ? }
 BEGIN
  obtlex;
  darray2(tipoarray);         { Saca el tamaño de la matriz inferior }
 END
 ELSE
 BEGIN
  compara(CLLAVED,34);        { Fin de matriz }
  compsig(COF,26);
  obtlex;
  tipoarray:=atype;           { Saca el tipo de elemento }
 END;
                              { Tamaño total }
 num[posarray]:=((max[tarray]-min[tarray])+1)*num[tipoarray];
 tipo[posarray]:=tipoarray;   { Tipo de elemento }
 sub[posarray]:=tarray;       { Tipo de indice }
END;

{ Procesa ARRAY }
FUNCTION darray:INTEGER;
BEGIN
 compsig(CLLAVEI,33);
 obtlex;
 darray2(darray)
END;

{ Procesa subrango }
FUNCTION dsub:INTEGER;
VAR tipomin,tipomax,minsub,maxsub,contador:INTEGER;
BEGIN
 const1;
 IF tipoi=6 THEN error(14);   { No se puede hacer un subrango REAL }
 tipomin:=tipoi;
 minsub:=numero;
 compsig(CPUNTOPUNTO,39);    { sintaxis }
 obtlex;
 const1;
 IF tipoi=6 THEN error(14);
 tipomax:=tipoi;
 maxsub:=numero;
 IF tipomin<>tipomax THEN error(14);  { Los tipos deben ser iguales }
 IF minsub>maxsub THEN error(15);  { El máximo >= mínimo sino error }
 cadena:='';
 declara('B');                    { Declara el subrango }
 min[ultsim]:=minsub;             { Mínimo }
 max[ultsim]:=maxsub;             { Máximo }
 tipo[ultsim]:=tipomin;           { Tipo }
 num[ultsim]:=num[tipo[ultsim]];  { Tamaño }
 dsub:=ultsim;
 obtlex
END;

{ Si encuentra un identificador, verifica si es para un subrango o }
{ para identificar un tipo }
FUNCTION ditype:INTEGER;
BEGIN
 busca;
 IF possim=0 THEN error(11);
 IF clase[possim]='C' THEN ditype:=dsub
 ELSE
 BEGIN
  IF clase[possim]<>'T' THEN error(36);
  ditype:=tipo[possim];
  obtlex;
 END;
END;

{ Procesa un apuntador a tipo }
FUNCTION dapunta:INTEGER;
VAR contador,tamcad,posapun:INTEGER;
BEGIN
 cadena:='';
 declara('D');             { Declara un apuntador }
 posapun:=ultsim;
 num[posapun]:=4;          { Tamaño: 4 bytes }
 compsig(CIDENT,4);
 IF dentrotype THEN        { Se valen apuntadores a tipos indefinidos }
 BEGIN
  tipo[posapun]:=apulit;   { Apunta al nombre del tipo }
  min[posapun]:=0;         { Indica que no ha sido procesado }
  tamcad:=length(cadena);
  contador:=1;             { Copia el nombre del tipo }
  WHILE contador<=tamcad DO
  BEGIN
   literal[apulit]:=cadena[contador];
   contador:=contador+1;
   apulit:=apulit+1
  END;
  literal[apulit]:=chr(0);
  apulit:=apulit+1
 END
 ELSE
 BEGIN
  busca;
  IF clase[possim]<>'T' THEN error(29);  { No esta definido el tipo }
  tipo[posapun]:=tipo[possim];           { Tipo verdadero }
  min[posapun]:=1                        { Indica que ya ha sido procesado }
 END;
 obtlex;
 dapunta:=posapun
END;

{ Procesa un declarador de conjunto }
FUNCTION dset:INTEGER;
VAR
 contador:INTEGER;
 tiposet,posset:INTEGER;
BEGIN
 compsig(COF,26);
 cadena:='';
 declara('S');
 posset:=ultsim;
 obtlex;
 tiposet:=atype;
 IF (clase[tiposet]='B') OR (clase[tiposet]='E') THEN
 BEGIN
  min[posset]:=min[tiposet];
  max[posset]:=max[tiposet];
  IF (min[posset]-max[posset])+1>255 THEN error(14);
  num[posset]:=32;           { Conjuntos de hasta 256 elementos }
  tipo[posset]:=tiposet
 END
 ELSE error(14);
 dset:=posset
END;

{ Procesa un declarador FILE }
FUNCTION dfile:INTEGER;
VAR
 contador:INTEGER;
 tipofile,posfile:INTEGER;
BEGIN
 compsig(COF,26);
 cadena:='';
 declara('O');
 posfile:=ultsim;
 obtlex;
 tipofile:=atype;
 num[posfile]:=40;  { La primera palabra contiene un apuntador al }
                    { siguiente objeto a leerse, la siguiente }
                    { palabra indica en que objeto se esta, la sig. }
                    { indica cuantos objetos existen, la sig. indica }
                    { el tamaño del objeto en bytes, la sig. indica }
                    { el número de manejo del archivo, la sig. indica }
                    { un número de control del archivo y luego un }
                    { STRING[15] para el nombre. }
 tipo[posfile]:=tipofile;
 dfile:=posfile
END;

{ Procesa un declarador de cadena (parecido a una matriz) }
FUNCTION dstring:INTEGER;
VAR
 contador:INTEGER;
 posstring:INTEGER;
 possub:INTEGER;
 tamano:INTEGER;
BEGIN
 cadena:='';
 declara('B');                 { Declara un subrango para usar }
 possub:=ultsim;
 declara('N');                 { Declara un tipo STRING }
 posstring:=ultsim;
 obtlex;
 IF lexema=CLLAVEI THEN        { Si lleva tamaño averiguar cual es }
 BEGIN
  obtlex;
  const1;                      { Debe ser una constante el tamaño }
  IF tipoi<>2 THEN error(14);  { INTEGER }
  IF numero<1 THEN error(30);  { y por lo menos de longitud 1 }
  IF numero>255 THEN error(30);{ Cadena demasiado larga }
  tamano:=numero;
  compsig(CLLAVED,34);
  obtlex;
 END
 ELSE tamano:=255;             { Tamaño por defecto de 255 caracteres }
 sub[posstring]:=possub;       { Declara su tipo de subindice }
 num[posstring]:=tamano+1;     { Su tamaño }
 tipo[posstring]:=4;           { Elementos CHAR }
 min[possub]:=0;               { Mínimo subrango }
 max[possub]:=tamano;          { Máximo subrango }
 tipo[possub]:=2;              { Tipo del subrango }
 num[possub]:=num[tipo[possub]]; { Tamaño del subrango }
 dstring:=posstring            { Apunta al tipo STRING }
END;

{ Esta función crea una estructura de tipo y devuelve un apuntador }
{ a la estructura }
FUNCTION atype;
BEGIN
 CASE lexema OF
  CPARENI: atype:=denum;
  CAPUNTA: atype:=dapunta;
  CNUM,CCAD,CMENOS,CMAS: atype:=dsub;
  CIDENT : atype:=ditype
  ELSE
  BEGIN
   IF lexema=CPACKED THEN obtlex;
   CASE lexema OF
    CARRAY : atype:=darray;
    CRECORD: atype:=drecord;
    CSET   : atype:=dset;
    CFILE  : atype:=dfile;
    CSTRING: atype:=dstring
    ELSE error(36)
   END
  END
 END
END;

{ Re-acondicionamiento de tipos indefinidos (apuntadores) }
PROCEDURE rtype;
VAR contador,contador2:INTEGER;
BEGIN
 FOR contador:=ultsim DOWNTO 1 DO
 BEGIN
  IF (clase[contador]='D') AND   { Verifica que no haya sido procesado }
     (min[contador]=0) THEN
  BEGIN
   cadena:='';
   contador2:=tipo[contador];
   WHILE literal[contador2]<>chr(0) DO
   BEGIN
    cadena:=cadena+literal[contador2];
    contador2:=contador2+1
   END;
   busca;
   IF clase[possim]<>'T' THEN error(29);  { No esta definido el tipo }
   tipo[contador]:=tipo[possim];          { Tipo verdadero }
   min[contador]:=1                       { Ahora esta procesado }
  END;
 END;
END;

{ Definición de tipos }
PROCEDURE dtype;
VAR contador,simtype:INTEGER;
BEGIN
 compara(CIDENT,4);
 declara('T');
 simtype:=ultsim;
 compsig(CIGUAL,3);
 obtlex;
 tipo[simtype]:=atype;
 num[simtype]:=num[tipo[simtype]];
END;

PROCEDURE ptype;
BEGIN
 obtlex;
 REPEAT
  dtype;
  compara(CPUNTOYCOMA,10);
  obtlex
 UNTIL (lexema=CVAR) OR
       (lexema=CPROCEDURE) OR
       (lexema=CFUNCTION) OR
       (lexema=CBEGIN);
 rtype
END;

{ Procesa una declaración de constante }
PROCEDURE dconst;
VAR
 contador,tamcad:INTEGER;
 cadena2,cadena3:STRING;
BEGIN
 compara(CIDENT,4);
 cadena2:=cadena;
 compsig(CIGUAL,3);
 obtlex;
 const1;
 cadena3:=cadena;
 cadena:=cadena2;
 declara('C');
 cadena:=cadena3;
 tamcad:=length(cadena);
 IF lexema=CCAD THEN   { Constante de cadena }
 BEGIN
  IF tamcad=1 THEN     { Una cadena de tamaño 1 es un caracter }
  BEGIN
   val[ultsim]:=ord(cadena[1]);
   tipo[ultsim]:=4
  END
  ELSE
  BEGIN
   tipo[ultsim]:=ultsim+1;
   min[ultsim]:=apulit;
   contador:=1;
   WHILE contador<=tamcad DO
   BEGIN
    literal[apulit]:=cadena[contador];
    apulit:=apulit+1;
    contador:=contador+1
   END;
   literal[apulit]:=chr(0);
   apulit:=apulit+1;
   contador:=contador-1;
   cadena:='';
   declara('A');
   tipo[ultsim]:=4;
   num[ultsim]:=contador;
   sub[ultsim]:=ultsim+1;
   declara('B');
   tipo[ultsim]:=2;
   min[ultsim]:=1;
   max[ultsim]:=contador;
   num[ultsim]:=num[tipo[ultsim]];
  END;
 END
 ELSE IF lexema=CNUMREAL THEN  { Constante REAL }
 BEGIN
  usoesp:=numreal;
  tipo[ultsim]:=6;
  min[ultsim]:=agreal
 END
 ELSE
 BEGIN                 { Constante INTEGER }
  val[ultsim]:=numero;
  tipo[ultsim]:=2
 END;
 obtlex
END;

PROCEDURE pconst;
BEGIN
 obtlex;
 REPEAT
  dconst;
  compara(CPUNTOYCOMA,10);
  obtlex
 UNTIL (lexema=CTYPE) OR
       (lexema=CVAR) OR
       (lexema=CPROCEDURE) OR
       (lexema=CFUNCTION) OR
       (lexema=CBEGIN)
END;

{ Procesa una declaración de etiquetas }
PROCEDURE dlabel;
BEGIN
 compara(CNUM,2);
 declara('L');
 val[ultsim]:=geneti;
 obtlex
END;

PROCEDURE plabel;
VAR fin:BOOLEAN;
BEGIN
 obtlex;
 REPEAT
  fin:=FALSE;
  REPEAT
   dlabel;
   IF lexema<>CCOMA THEN fin:=TRUE ELSE obtlex
  UNTIL fin;
  compara(CPUNTOYCOMA,10);
  obtlex
 UNTIL (lexema=CCONST) OR
       (lexema=CTYPE) OR
       (lexema=CVAR) OR
       (lexema=CPROCEDURE) OR
       (lexema=CFUNCTION) OR
       (lexema=CBEGIN)
END;


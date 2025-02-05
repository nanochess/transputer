{ Programa: Compilador de Pascal para el Inmos T805 }
{                                                   }
{ Módulo  : Análisis y Procesamiento de Expresiones }
{                                                   }
{ (c) Copyright 1993-1994 Oscar Toledo G.           }
{          Todos los derechos reservados.           }

{ Los siguientes dos procedimientos son muy usados para }
{ comprobación de sintaxis }
PROCEDURE compara(lex:comlex; errnum:INTEGER);
BEGIN
 IF lexema<>lex THEN error(errnum)
END;

PROCEDURE compsig(lex:comlex; errnum:INTEGER);
BEGIN
 obtlex;
 IF lexema<>lex THEN error(errnum)
END;

{ Rutinas generales de comprobación de tipos }

{ Verifica que ambos tipos sean INTEGER }
FUNCTION enteros(tipoizq,tipoder:INTEGER):BOOLEAN;
BEGIN
 enteros:=(tipoizq=2) AND (tipoder=2)
END;

{ Verifica que ambos tipos sean BOOLEAN }
FUNCTION boleanos(tipoizq,tipoder:INTEGER):BOOLEAN;
BEGIN
 boleanos:=(tipoizq=8) AND (tipoder=8)
END;

{ Verifica que los tipos sean INTEGER o REAL }
FUNCTION reales(tipoizq,tipoder:INTEGER):BOOLEAN;
BEGIN
 reales:=((tipoizq=6) AND ((tipoder=6) OR (tipoder=2))) OR
         ((tipoder=6) AND ((tipoizq=6) OR (tipoizq=2)))
END;

{ Verifica que los tipos sean aceptables para operaciones de bit }
FUNCTION bit(tipoizq,tipoder:INTEGER):BOOLEAN;
BEGIN
 bit:=((tipoizq=2) AND ((tipoder=2) OR (tipoder=4))) OR
      ((tipoder=2) AND ((tipoizq=2) OR (tipoizq=4)))
END;

{ Verifica que los tipos sean aceptables para operaciones de conjuntos }
FUNCTION conjuntos(tipoizq,tipoder:INTEGER):BOOLEAN;
BEGIN
 conjuntos:=(clase[tipoizq]='S') AND (clase[tipoder]='S')
END;

{ Verifica que el tipo pueda ser usado cómo una cadena }
FUNCTION cadtipo(tipoi:INTEGER):BOOLEAN;
BEGIN
 cadtipo:=(clase[tipoi]='N') OR ((clase[tipoi]='A') AND (tipo[tipoi]=4)) OR
          (tipoi=4)
END;

FUNCTION cadenas(tipoizq,tipoder:INTEGER):BOOLEAN;
BEGIN
 cadenas:=cadtipo(tipoizq) AND cadtipo(tipoder)
END;

{ Saca el tipo base de un tipo }
FUNCTION tipobase(tipop:INTEGER):INTEGER;
BEGIN
 IF clase[tipop]='B' THEN tipobase:=tipo[tipop] ELSE tipobase:=tipop;
END;

{ Comprueba si tipoder es asignable a tipoizq }
FUNCTION comtipos(tipoizq,tipoder:INTEGER):BOOLEAN;
BEGIN
 tipoizq:=tipobase(tipoizq);
 tipoder:=tipobase(tipoder);
 IF tipoizq=tipoder THEN comtipos:=TRUE
 ELSE IF (tipoizq=6) AND (tipoder=2) THEN comtipos:=TRUE
 ELSE IF (clase[tipoizq]='D') AND (clase[tipoder]='D') THEN comtipos:=TRUE
 ELSE IF (clase[tipoizq]='S') AND (clase[tipoder]='S') THEN comtipos:=TRUE
 ELSE IF (clase[tipoizq]='N') AND cadtipo(tipoder) THEN comtipos:=TRUE
 ELSE IF (clase[tipoizq]='A') AND (clase[tipoder]='A') AND
         (tipo[tipoizq]=4) AND (tipo[tipoder]=4) AND
         (num[tipoizq]=num[tipoder]) THEN comtipos:=TRUE
 ELSE comtipos:=FALSE;
END;

{ Comprueba si tipoizq es comparable con tipoder }
FUNCTION checatipos(tipoizq,tipoder:INTEGER):BOOLEAN;
BEGIN
 IF tipoizq=tipoder THEN checatipos:=TRUE
 ELSE IF reales(tipoizq,tipoder) THEN checatipos:=TRUE
 ELSE IF (clase[tipoizq]='D') AND (clase[tipoder]='D') THEN checatipos:=TRUE
 ELSE IF (clase[tipoizq]='S') AND (clase[tipoder]='S') THEN checatipos:=TRUE
 ELSE IF (clase[tipoizq]='A') AND (clase[tipoder]='A') AND
         (tipo[tipoizq]=4) AND (tipo[tipoder]=4) AND
         (num[tipoizq]=num[tipoder]) THEN checatipos:=TRUE
 ELSE IF cadenas(tipoizq,tipoder) THEN checatipos:=TRUE
 ELSE checatipos:=FALSE;
END;

{ Busca el identificador actual en la tabla de identificadores }
PROCEDURE busca;
VAR
 contador,contador2:INTEGER;
 noencontro,tamcad:INTEGER;
BEGIN
 tamcad:=length(cadena);
 possim:=ultsim;
 noencontro:=1;
 WHILE (noencontro=1) AND (possim<>0) DO
 BEGIN
  CASE clase[possim] OF
  'L','C','T','V','F','P': { Sólo LABEL, CONST, TYPE, VAR, FUNC y PROC }
  BEGIN
   contador:=sim[possim];
   contador2:=1;
   noencontro:=0;
   WHILE noencontro=0 DO
   BEGIN
    IF literal[contador]=chr(0) THEN
     IF contador2>tamcad THEN noencontro:=2
     ELSE noencontro:=1
    ELSE IF contador2>tamcad THEN noencontro:=1
    ELSE IF literal[contador]<>cadena[contador2] THEN noencontro:=1
    ELSE
    BEGIN
     contador2:=contador2+1;
     contador:=contador+1;
    END;
   END;
   IF noencontro=1 THEN possim:=possim-1;
  END
  ELSE possim:=possim-1
  END;
 END;
END;

{ Devuelve el valor de una constante }
PROCEDURE const2;
VAR contador:INTEGER;
BEGIN
      IF lexema=CNUM     THEN tipoi:=2
 ELSE IF lexema=CNUMREAL THEN tipoi:=6
 ELSE IF lexema=CIDENT   THEN
 BEGIN
  busca;
  IF possim=0 THEN error(2);
  IF clase[possim]<>'C' THEN error(2);
  numero:=val[possim];
  tipoi:=tipo[possim];
  IF clase[tipo[possim]]='A' THEN
  BEGIN
   lexema:=CCAD;
   cadena:='';
   contador:=min[possim];
   numero:=ord(literal[contador]);
   WHILE literal[contador]<>chr(0) DO
   BEGIN
    cadena:=cadena+literal[contador];
    contador:=contador+1
   END;
   tipoi:=4
  END;
 END
 ELSE
 BEGIN
  compara(CCAD,2);
  numero:=ord(cadena[1]);
  tipoi:=4
 END;
END;

PROCEDURE const1;
BEGIN
 IF lexema=CMENOS THEN
 BEGIN
  obtlex;
  const2;
  IF tipoi=4 THEN error(14);
  IF tipoi=2 THEN numero:=-numero;
  IF tipoi=6 THEN numreal.valor:=-numreal.valor;
 END
 ELSE IF lexema=CMAS THEN
 BEGIN
  obtlex;
  const2;
  IF tipoi=4 THEN error(14);
 END
 ELSE const2;
END;

{ Declara un identificador }
PROCEDURE declara(claset:CHAR);
VAR contador,tamcad:INTEGER;
BEGIN
 tamcad:=length(cadena);
 ultsim:=ultsim+1;
 IF ultsim>tam THEN error(44);
 IF cadena='' THEN sim[ultsim]:=0
 ELSE
 BEGIN
  sim[ultsim]:=apulit;
  contador:=1;
  WHILE contador<=tamcad DO
  BEGIN
   literal[apulit]:=cadena[contador];
   contador:=contador+1;
   apulit:=apulit+1
  END;
  literal[apulit]:=chr(0);
  apulit:=apulit+1;
  IF apulit>maxlit THEN error(46)
 END;
 clase[ultsim]:=claset;  { Clase de identificador }
 prof[ultsim]:=profact;  { Profundidad de anidamiento }
 prox[ultsim]:=0         { Sucesor }
END;

{ Agrega un número real a la tabla de constantes }
FUNCTION agreal:INTEGER;
VAR
 doble:BOOLEAN;
 contador,repetido:INTEGER;
BEGIN
 doble:=FALSE;
 FOR contador:=0 TO apucons-1 DO    { Verifíca número repetido }
  IF constantes[contador].valorint=usoesp.valorint THEN
  BEGIN
   repetido:=contador;
   doble:=TRUE;
  END;
 IF doble THEN agreal:=repetido
 ELSE
 BEGIN
  agreal:=apucons;
  constantes[apucons]:=usoesp;
  apucons:=apucons+1;
  if apucons>maxcon THEN error(45)
 END;
END;

{ Conversión a real, convierte números enteros a reales, }
{ o variables normales }
PROCEDURE convreal(VAR nodo,tipo:INTEGER);
VAR nombre:narbol;
BEGIN
 IF tipo=2 THEN
 BEGIN
  nombre:=arbol[nodo].nom;
  IF nombre=NINUM THEN
  BEGIN
   usoesp.valor:=arbol[nodo].esp;
   IF usoesp.valorint=0 THEN creanodo(NRCERO,0,0,0)
                        ELSE creanodo(NRNUM,0,0,agreal);
  END
  ELSE creanodo(NINTAREAL,nodo,0,0);
  nodo:=nodoact;
  tipo:=6
 END
 ELSE IF tipo<>6 THEN error(14);
END;

{ Convierte una matriz de caracteres o un caracter a cadena }
PROCEDURE convcadena(VAR nodo,tipo:INTEGER);
BEGIN
 IF clase[tipo]<>'N' THEN
 BEGIN
  IF tipo=4 THEN
  BEGIN
   creanodo(NCLIMPIAA,nodo,0,0);
   nodo:=nodoact;
   tipo:=18
  END
  ELSE IF clase[tipo]='A' THEN
  BEGIN
   IF (max[sub[tipo]]-min[sub[tipo]])+1=0 THEN
        creanodo(NCLIMPIA,0,0,0)
   ELSE creanodo(NARRAYACAD,nodo,0,(max[sub[tipo]]-min[sub[tipo]])+1);
   nodo:=nodoact;
   tipo:=18
  END;
 END;
END;

{ Conversión de tipos para asignaciones }
PROCEDURE convasigna(VAR tipoi,tipod:INTEGER);
BEGIN
 tipoi:=tipobase(tipoi);
 tipod:=tipobase(tipod);
 IF NOT comtipos(tipoi,tipod) THEN error(14);
 IF clase[tipoi]='N' THEN convcadena(nodoact,tipod);
 IF clase[tipod]='S' THEN creanodo(NSDIR,nodoact,0,0);
 IF (tipoi=6) AND (tipod=2) THEN convreal(nodoact,tipod)
END;

PROCEDURE expr2; FORWARD;

PROCEDURE factor; FORWARD;

{ Procesa un identificador de constante }
PROCEDURE proccons;
BEGIN
 IF clase[tipo[possim]]<>'A' THEN
 BEGIN
  IF tipo[possim]=6 THEN
       creanodo(NRNUM,0,0,min[possim])   { Constante real }
  ELSE creanodo(NINUM,0,0,val[possim])   { Constante entera }
 END
 ELSE  creanodo(NCAD,0,0,min[possim]);  { Cadena }
 tipoi:=tipo[possim];
 obtlex
END;

{ Procesa una franja de tipo RECORD }
FUNCTION buscarec(posfranja:INTEGER):INTEGER;
VAR
 nomfranja,comfranja:STRING;
 contador:INTEGER;
 noencontrado:BOOLEAN;
BEGIN
 nomfranja:=cadena;
 posfranja:=prox[posfranja];
 noencontrado:=TRUE;
 WHILE noencontrado DO
 BEGIN
  deslit(sim[posfranja],comfranja);
  IF nomfranja=comfranja THEN noencontrado:=FALSE
  ELSE
  BEGIN
   posfranja:=prox[posfranja];
   IF posfranja=0 THEN noencontrado:=FALSE
  END;
 END;
 buscarec:=posfranja;
END;

PROCEDURE listarec;
VAR posfranja:INTEGER;
BEGIN
 IF (lexema=CIDENT) AND          { Verifica que el tipo actual sea RECORD }
    (clase[tipoi]='R') THEN
 BEGIN
  posfranja:=buscarec(tipoi);    { Busca la franja en la lista de franjas }
  obtlex;
  IF posfranja<>0 THEN
  BEGIN
   creanodo(NINUM,0,0,val[posfranja]);   { La accesa }
   creanodo(NIFRANJA,nodoact-1,nodoact,0);
   tipoi:=tipo[posfranja];
  END
  ELSE error(7);
 END
 ELSE error(7);
END;

{ Procesa un indicador de apuntador }
PROCEDURE listaapu;
BEGIN
 IF (clase[tipoi]='D') OR
    (clase[tipoi]='O') THEN  { Se asegura de que sea apuntador o FILE }
 BEGIN
  obtlex;
  creanodo(NICARGA,nodoact,0,0);
  tipoi:=tipo[tipoi]
 END
 ELSE error(24)
END;

{ Procesa una lista de subindices, y optimiza el acceso }
PROCEDURE listasub;
VAR
 nodovar,nodoant,tipomat:INTEGER;
 byteselem:INTEGER;
 suma:BOOLEAN;
 subnodo:narbol;
BEGIN
{ Busca el tamaño del elemento básico }
 byteselem:=tipoi;
 WHILE (clase[byteselem]='A') OR (clase[byteselem]='N') DO
  byteselem:=tipo[byteselem];
 byteselem:=num[tipobase(byteselem)];
 IF (byteselem MOD 4)=0 THEN byteselem:=4 ELSE byteselem:=1;
{ Calcula los nodos }
 nodovar:=nodoact;
 nodoant:=-1;
{ Procesa los subindices }
 REPEAT
  IF (clase[tipoi]='A') OR
     (clase[tipoi]='N') THEN   { Verifica que el tipo sea ARRAY o STRING }
  BEGIN
   tipomat:=tipoi;
   obtlex;
   expr2;
{ Verifica que el tipo del subindice sea el correcto }
   IF NOT comtipos(sub[tipomat],tipoi) THEN error(14);
   tipoi:=tipomat;
{ Si el subindice minimo no es 0 lo resta }
   IF min[sub[tipoi]]<>0 THEN
   BEGIN
    creanodo(NINUM,0,0,min[sub[tipoi]]);
    creanodo(NIRESTA,nodoact-1,nodoact,0)
   END;
{ Multiplica por el tamaño del elemento }
   IF (num[tipo[tipoi]] DIV byteselem)>1 THEN
   BEGIN
    creanodo(NINUM,0,0,num[tipo[tipoi]] DIV byteselem);
    creanodo(NMULRAPIDA,nodoact,nodoact-1,0)
   END;
   IF nodoant<>-1 THEN creanodo(NISUMA,nodoant,nodoact,0);
   nodoant:=nodoact;
   tipoi:=tipo[tipoi]
  END
  ELSE error(8)
 UNTIL lexema<>CCOMA;
 compara(CLLAVED,34);
 obtlex;
 IF nodoant<>-1 THEN
 BEGIN
  IF byteselem=4 THEN creanodo(NINDEXAPAL,nodoant,nodovar,0)
                 ELSE creanodo(NINDEXABYTE,nodoant,nodovar,0)
 END
 ELSE nodoact:=nodovar;
END;

{ Procesa la entrada y crea un nodo que da la dirección de la variable }
PROCEDURE nodvar;
VAR posfranja:INTEGER;
BEGIN
 compara(CIDENT,4);
 posfranja:=0;
 IF twith<>0 THEN                { Verifica si esta dentro de un WITH }
 BEGIN
  posfranja:=buscarec(twith);    { Busca si es una franja RECORD valida }
  IF posfranja<>0 THEN           { Si es }
  BEGIN
   creanodo(NCARGAWITH,0,0,0);   { Carga el desplazamiento }
   tipoi:=twith;                 { Toma su tipo de referencia }
   listarec                      { y procesa una franja RECORD }
  END;
 END;
 IF posfranja=0 THEN
 BEGIN
  busca;                         { Si no es WITH, entonces busca en tabla }
  IF possim=0 THEN error(11);    { Si no esta, da error indefinido }
  IF clase[possim]<>'V' THEN error(12);  { Debe ser una variable }
  creanodo(NDIRVAR,0,0,possim);  { Crea un apuntador a una variable }
  tipoi:=tipo[possim];           { Toma el tipo de la variable }
  obtlex;
 END;
 WHILE (lexema=CLLAVEI) OR       { Verifíca continuaciones validas }
       (lexema=CPUNTO) OR
       (lexema=CAPUNTA) DO
 BEGIN
       IF lexema=CLLAVEI THEN listasub
  ELSE IF lexema=CPUNTO  THEN
  BEGIN
   obtlex;
   listarec
  END
  ELSE listaapu;
 END;
END;

{ Obtiene la dirección de la variable y la carga }
PROCEDURE procvar;
BEGIN
 nodvar;
 IF (clase[tipoi]<>'R') AND (clase[tipoi]<>'A') AND
    (clase[tipoi]<>'O') THEN
 BEGIN
       IF tipoi=4          THEN creanodo(NICARGACHAR,nodoact,0,0)
  ELSE IF tipoi=6          THEN creanodo(NRCARGA,nodoact,0,0)
  ELSE IF clase[tipoi]='S' THEN creanodo(NSCARGA,nodoact,0,0)
  ELSE IF clase[tipoi]='N' THEN creanodo(NCCARGA,nodoact,0,0)
  ELSE                          creanodo(NICARGA,nodoact,0,0);
 END;
END;

{ Procesa un paso de procedimiento/función cómo parametro }
PROCEDURE pasafunc;
BEGIN
 { Debe pasar en una palabra la dirección de la función/procedimiento }
 { En la otra palabra debe pasar la dirección del enlace estático }

 { La posición de la función se obtiene mediante ldpi }
 { El enlace estático se calcula durante la ejecución }

 { Se debe crear un nuevo nodo en el arbol que se llame NPASAFUNC }
END;

{ Procesa una llamada a una función }
PROCEDURE procfunc;
VAR
 numpar,tipoi2:INTEGER;
 posfunc,pospar,ultimo:INTEGER;
BEGIN
 posfunc:=possim;
 pospar:=prox[posfunc];
 ultimo:=0;
 IF num[posfunc]>0 THEN
 BEGIN
  compsig(CPARENI,31);
  numpar:=0;
  REPEAT
   obtlex;
   CASE paso[pospar] OF
    0,2: expr2;
    1: nodvar;
    3,4: pasafunc
   END;
   tipoi2:=tipo[pospar];
   convasigna(tipoi2,tipoi);
   creanodo(NNULO,0,ultimo,pospar);
   ultimo:=nodoact;
   pospar:=prox[pospar];
   numpar:=numpar+1
  UNTIL lexema<>CCOMA;
  compara(CPAREND,22);
  IF numpar<>num[posfunc] THEN error(35)
 END;
 creanodo(NFUNCION,0,ultimo,posfunc);
 obtlex;
 tipoi:=tipo[pospar]
END;

PROCEDURE lfunc;
VAR posfunc:INTEGER;
BEGIN
 posfunc:=possim;
 procfunc;
 WHILE prox[posfunc]<>0 DO posfunc:=prox[posfunc];
 tipoi:=tipo[posfunc];
END;

{ Sintaxis de funciones interconstruidas }
PROCEDURE funcinter;
BEGIN
 compsig(CPARENI,31);
 obtlex;
 expr2;
 compara(CPAREND,22);
 obtlex
END;

{ Conversión a tipo pedido }
PROCEDURE proctipo;
VAR tipox:INTEGER;
BEGIN
 tipox:=tipo[possim];
 IF (clase[tipox]<>'B') AND (clase[tipox]<>'E') THEN error(14);
 funcinter;
 IF (clase[tipoi]<>'B') AND (clase[tipoi]<>'E') THEN error(14);
 tipoi:=tipox
END;

{ Verifíca llamadas a funciones interconstruidas }
FUNCTION funcesp:BOOLEAN;
VAR
 funcion:STRING;
 contador,nodo:INTEGER;
 nodoizq,nododer,nodostr:INTEGER;
BEGIN
 funcesp:=TRUE;
 funcion:=cadena;
 IF (funcion='ABS') OR (funcion='SQR') THEN
 BEGIN
  funcinter;
  tipoi:=tipobase(tipoi);
  IF (tipoi<>2) AND (clase[tipoi]<>'E') AND (tipoi<>6) THEN error(14);
  IF tipoi<>6 THEN
   IF funcion='ABS' THEN creanodo(NABS,nodoact,0,0)
                    ELSE creanodo(NSQR,nodoact,0,0)
  ELSE
   IF funcion='ABS' THEN creanodo(NABSR,nodoact,0,0)
                    ELSE creanodo(NSQRR,nodoact,0,0)
 END
 ELSE IF (funcion='EOLN') OR (funcion='EOF') THEN
 BEGIN
  obtlex;
  IF funcion='EOLN' THEN creanodo(NEOLN,0,0,0)
                    ELSE creanodo(NEOF,0,0,0);
  tipoi:=8
 END
 ELSE IF (funcion='ARCTAN') OR (funcion='COS') OR
         (funcion='SIN') OR (funcion='SQRT') OR
         (funcion='EXP') OR (funcion='LN') THEN
 BEGIN
  funcinter;
  convreal(nodoact,tipoi);
  IF tipoi<>6 THEN error(14);
       IF funcion='ARCTAN' THEN creanodo(NARCTAN,nodoact,0,0)
  ELSE IF funcion='COS' THEN creanodo(NCOS,nodoact,0,0)
  ELSE IF funcion='SIN' THEN creanodo(NSIN,nodoact,0,0)
  ELSE IF funcion='SQRT' THEN creanodo(NSQRT,nodoact,0,0)
  ELSE IF funcion='EXP' THEN creanodo(NEXP,nodoact,0,0)
  ELSE creanodo(NLN,nodoact,0,0);
  tipoi:=6
 END
 ELSE IF (funcion='PRED') OR (funcion='SUCC') THEN
 BEGIN
  funcinter;
  tipoi:=tipobase(tipoi);
  IF (tipoi<>2) AND (tipoi<>4) AND (clase[tipoi]<>'E') THEN error(14);
  creanodo(NINUM,0,0,1);
  IF funcion='PRED' THEN creanodo(NIRESTA,nodoact-1,nodoact,0)
                    ELSE creanodo(NISUMA,nodoact-1,nodoact,0);
 END
 ELSE IF (funcion='ROUND') OR (funcion='TRUNC') THEN
 BEGIN
  funcinter;
  IF tipoi<>6 THEN error(14);
  IF funcion='ROUND' THEN creanodo(NROUND,nodoact,0,0)
                     ELSE creanodo(NTRUNC,nodoact,0,0);
  tipoi:=2
 END
 ELSE IF funcion='CHR' THEN
 BEGIN
  funcinter;
  IF tipobase(tipoi)<>2 THEN error(14);
  tipoi:=4
 END
 ELSE IF funcion='ODD' THEN
 BEGIN
  funcinter;
  IF (tipobase(tipoi)<>2) AND (clase[tipoi]<>'E') THEN error(14);
  creanodo(NINUM,0,0,1);
  creanodo(NAND,nodoact-1,nodoact,0);
  tipoi:=8
 END
 ELSE IF funcion='ORD' THEN
 BEGIN
  funcinter;
  tipoi:=tipobase(tipoi);
  IF (tipoi<>2) AND (tipoi<>4) AND
     (clase[tipoi]<>'E') THEN error(14);
  tipoi:=2
 END
 ELSE IF funcion='CALL' THEN
 BEGIN
  funcinter;
  tipoi:=tipobase(tipoi);
  IF tipoi<>2 THEN error(14);
  creanodo(NCALL,nodoact,0,0);
  tipoi:=2
 END
 ELSE IF funcion='PI' THEN
 BEGIN
  usoesp.valor:=3.141592;
  creanodo(NRNUM,0,0,agreal);
  obtlex;
  tipoi:=6
 END
 ELSE IF funcion='RANDOM' THEN
 BEGIN
  creanodo(NRANDOM,0,0,0);
  tipoi:=6;
  obtlex;
  IF lexema=CPARENI THEN
  BEGIN
   nodo:=nodoact;
   obtlex;
   expr2;
   compara(CPAREND,22);
   obtlex;
   convreal(nodoact,tipoi);
   creanodo(NRMUL,nodo,nodoact,0);
   creanodo(NTRUNC,nodoact,0,0);
   tipoi:=2
  END;
 END
 ELSE IF funcion='LENGTH' THEN
 BEGIN
  funcinter;
  tipoi:=tipobase(tipoi);
  IF NOT cadtipo(tipoi) THEN error(14);
  convcadena(nodoact,tipoi);
  creanodo(NCDIR,nodoact,0,0);
  creanodo(NICARGACHAR,nodoact,0,0);
  tipoi:=2
 END
 ELSE IF funcion='COPY' THEN
 BEGIN
  compsig(CPARENI,31);
  obtlex;
  expr2;
  tipoi:=tipobase(tipoi);
  IF NOT cadtipo(tipoi) THEN error(14);
  convcadena(nodoact,tipoi);
  nodostr:=nodoact;
  compara(CCOMA,6);
  obtlex;
  expr2;
  tipoi:=tipobase(tipoi);
  IF tipoi<>2 THEN error(14);
  nodoizq:=nodoact;
  compara(CCOMA,6);
  obtlex;
  expr2;
  tipoi:=tipobase(tipoi);
  IF tipoi<>2 THEN error(14);
  nododer:=nodoact;
  compara(CPAREND,22);
  obtlex;
  creanodo(NCOPY,nodoizq,nododer,nodostr);
  tipoi:=18
 END
 ELSE IF funcion='POS' THEN
 BEGIN
  compsig(CPARENI,31);
  obtlex;
  expr2;
  tipoi:=tipobase(tipoi);
  IF NOT cadtipo(tipoi) THEN error(14);
  convcadena(nodoact,tipoi);
  nodoizq:=nodoact;
  compara(CCOMA,6);
  obtlex;
  expr2;
  tipoi:=tipobase(tipoi);
  IF NOT cadtipo(tipoi) THEN error(14);
  convcadena(nodoact,tipoi);
  compara(CPAREND,22);
  obtlex;
  nododer:=nodoact;
  creanodo(NPOS,nodoizq,nododer,0);
  tipoi:=2
 END
 ELSE IF funcion='CONCAT' THEN
 BEGIN
  compsig(CPARENI,31);
  obtlex;
  expr2;
  tipoi:=tipobase(tipoi);
  IF NOT cadtipo(tipoi) THEN error(14);
  convcadena(nodoact,tipoi);
  compara(CCOMA,6);
  WHILE lexema=CCOMA DO
  BEGIN
   nodoizq:=nodoact;
   obtlex;
   expr2;
   tipoi:=tipobase(tipoi);
   IF NOT cadtipo(tipoi) THEN error(14);
   convcadena(nodoact,tipoi);
   creanodo(NCONCAT,nodoizq,nodoact,0)
  END;
  compara(CPAREND,22);
  obtlex;
  tipoi:=18
 END
 ELSE funcesp:=FALSE;
END;

{ Procesa un identificador }
PROCEDURE eident;
VAR posfranja:INTEGER;
BEGIN
 posfranja:=0;
 IF twith<>0 THEN
 BEGIN
  posfranja:=buscarec(twith);
  IF posfranja<>0 THEN procvar;
 END;
 IF posfranja=0 THEN
 BEGIN
  busca;
  IF possim=0 THEN
  BEGIN
   IF NOT funcesp THEN error(11)
  END
  ELSE
  CASE clase[possim] OF
   'F': lfunc;      { Llamada a función }
   'C': proccons;   { Una constante }
   'V': procvar;    { Una variable }
   'T': proctipo;   { Conversión a tipo X }
   'P': error(21)   { Un procedimiento no vale en una expresión }
   ELSE error(12)   { Algo ilegal }
  END;
 END;
END;

{ Procesa números enteros en expresiones }
PROCEDURE enum;
BEGIN
 creanodo(NINUM,0,0,numero);
 obtlex;
 tipoi:=2
END;

{ Procesa números reales en expresiones }
PROCEDURE enumreal;
BEGIN
 IF numreal.valorint=0 THEN creanodo(NRCERO,0,0,0)
 ELSE
 BEGIN
  usoesp:=numreal;
  creanodo(NRNUM,0,0,agreal);
 END;
 obtlex;
 tipoi:=6
END;

{ Procesa cadenas en expresiones }
PROCEDURE estr;
VAR contador,tamcad:INTEGER;
BEGIN
 tamcad:=length(cadena);
 IF tamcad=1 THEN     { Si el tamaño es 1 entonces es un caracter }
 BEGIN
  creanodo(NINUM,0,0,ord(cadena[1]));
  tipoi:=4
 END
 ELSE
 BEGIN
  creanodo(NCAD,0,0,apuarb);
  contador:=1;
  WHILE contador<=tamcad DO
  BEGIN
   literal[apuarb]:=cadena[contador];
   apuarb:=apuarb+1;
   contador:=contador+1
  END;
  literal[apuarb]:=chr(0);
  apuarb:=apuarb+1;
  contador:=contador-1;
  cadena:='';
  declara('A');          { Declara un tipo ARRAY }
  tipo[ultsim]:=4;       { Tipo CHAR }
  num[ultsim]:=contador; { Tamaño }
  sub[ultsim]:=ultsim+1; { Apuntador al subrango }
  declara('B');          { Subrango }
  tipo[ultsim]:=2;       { Tipo INTEGER }
  min[ultsim]:=1;        { Empieza en 1 }
  max[ultsim]:=contador; { Hasta el tamaño de la cadena }
  num[ultsim]:=num[tipo[ultsim]]; { Tamaño del tipo INTEGER }
  tipoi:=ultsim-1
 END;
 obtlex;
END;

PROCEDURE enot;
BEGIN
 obtlex;
 factor;
 tipoi:=tipobase(tipoi);
 IF tipoi=8 THEN                      { Verifica NOT booleano }
 BEGIN
  creanodo(NNOTB,nodoact,0,0);
  tipoi:=8
 END
 ELSE IF (tipoi=2) OR (tipoi=4) THEN  { Verifica NOT de bits }
 BEGIN
  creanodo(NNOT,nodoact,0,0);
  tipoi:=2
 END
 ELSE error(14);
END;

PROCEDURE enil;
BEGIN
 creanodo(NNIL,0,0,0);
 obtlex;
 tipoi:=11  { Indica apuntador }
END;

{ Procesa una expresión entre parentesis }
PROCEDURE epar;
BEGIN
 obtlex;
 expr2;
 compara(CPAREND,22);
 obtlex
END;

{ Procesa un definidor de conjunto }
PROCEDURE eset;
VAR nodoset,nodobit:INTEGER;
BEGIN
 creanodo(NSLIMPIA,0,0,0);
 nodoset:=nodoact;
 REPEAT
  obtlex;
  IF lexema<>CLLAVED THEN
  BEGIN
   expr2;
   IF lexema=CPUNTOPUNTO THEN
   BEGIN
    nodobit:=nodoact;
    obtlex;
    expr2;
    creanodo(NSGENBITS,nodobit,nodoact,nodoset)
   END
   ELSE creanodo(NSGENBIT,nodoact,0,nodoset);
   nodoset:=nodoact
  END
 UNTIL lexema<>CCOMA;
 compara(CLLAVED,34);
 obtlex;
 tipoi:=17
END;

PROCEDURE efinprog;
BEGIN
 creanodo(NFINPROG,0,0,0);             { Carga la dir. del fin del prog. }
 obtlex;
 tipoi:=2                              { El tipo es INTEGER }
END;

PROCEDURE factor;
BEGIN
 CASE lexema OF
  CIDENT   : eident;
  CNUM     : enum;
  CNUMREAL : enumreal;
  CCAD     : estr;
  CNOT     : enot;
  CNIL     : enil;
  CPARENI  : epar;
  CLLAVEI  : eset;
  C_FINPROG: efinprog
        ELSE error(23)
 END;
END;

{ Optimización para multiplicar o dividir por 2.0 }
PROCEDURE veropt(VAR nodo:INTEGER; VAR hecho:BOOLEAN; mul:BOOLEAN);
VAR nombre:narbol;
BEGIN
 IF (usoesp.valor=0.125) OR (usoesp.valor=0.25) OR (usoesp.valor=0.5) OR
    (usoesp.valor=2.0  ) OR (usoesp.valor=4.0 ) OR (usoesp.valor=8.0) THEN
 BEGIN
  hecho:=TRUE;
  IF usoesp.valor<1.0 THEN nombre:=NDIV2 ELSE nombre:=NPOR2;
  IF NOT mul THEN IF nombre=NDIV2 THEN nombre:=NPOR2 ELSE nombre:=NDIV2;
  REPEAT
   creanodo(nombre,nodo,0,0);
   nodo:=nodoact;
   IF usoesp.valor<1.0 THEN usoesp.valor:=usoesp.valor*2.0
                       ELSE usoesp.valor:=usoesp.valor/2.0;
  UNTIL usoesp.valor=1.0;
 END;
END;

PROCEDURE optdm(operador:narbol; VAR nodoizq,nododer:INTEGER;
                mul:BOOLEAN);
VAR
 nombre:narbol;
 hecho:BOOLEAN;
BEGIN
 hecho:=FALSE;
 nombre:=arbol[nododer].nom;
 IF nombre=NRNUM THEN
 BEGIN
  usoesp:=constantes[arbol[nododer].esp];
  veropt(nodoizq,hecho,mul);
 END
 ELSE IF mul THEN
 BEGIN
  nombre:=arbol[nodoizq].nom;
  IF nombre=NRNUM THEN
  BEGIN
   usoesp:=constantes[arbol[nodoizq].esp];
   veropt(nododer,hecho,mul);
  END;
 END;
 IF NOT hecho THEN creanodo(operador,nodoizq,nododer,0);
END;

PROCEDURE termino;
VAR
 operador2:comlex;
 operador:narbol;
 tipoizq,tipoder:INTEGER;
 nodoizq,nododer:INTEGER;
 menos:INTEGER;
BEGIN
 factor;
 WHILE (lexema=CPOR) OR (lexema=CDIVIDE) OR
       (lexema=CDIV) OR (lexema=CMOD) OR
       (lexema=CSHL) OR (lexema=CSHR) OR
       (lexema=CAND) DO
 BEGIN
  operador2:=lexema;
  tipoizq:=tipobase(tipoi);
  nodoizq:=nodoact;
  obtlex;
  factor;
  tipoder:=tipobase(tipoi);
  nododer:=nodoact;
  CASE operador2 OF
   CPOR   : operador:=NIMUL;
   CDIVIDE: operador:=NRDIV;
   CMOD   : operador:=NIMOD;
   CDIV   : operador:=NIDIV;
   CSHL   : operador:=NSHL;
   CSHR   : operador:=NSHR;
   CAND   : operador:=NAND
  END;
  IF operador2=CPOR THEN
  BEGIN
   IF enteros(tipoizq,tipoder) THEN
   BEGIN
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=2
   END
   ELSE IF reales(tipoizq,tipoder) THEN
   BEGIN
    operador:=NRMUL;
    convreal(nodoizq,tipoizq);
    convreal(nododer,tipoder);
    optdm(operador,nodoizq,nododer,TRUE);
    tipoi:=6
   END
   ELSE IF conjuntos(tipoizq,tipoder) THEN
   BEGIN
    creanodo(NSINTER,nodoizq,nododer,0);
    tipoi:=17
   END
   ELSE error(14);
  END
  ELSE IF operador2=CDIVIDE THEN
  BEGIN
   IF reales(tipoizq,tipoder) OR enteros(tipoizq,tipoder) THEN
   BEGIN
    operador:=NRDIV;
    convreal(nodoizq,tipoizq);
    convreal(nododer,tipoder);
    optdm(operador,nodoizq,nododer,FALSE);
    tipoi:=6
   END
   ELSE error(14);
  END
  ELSE IF (operador2=CDIV) OR (operador2=CMOD) THEN
  BEGIN
   IF enteros(tipoizq,tipoder) THEN
   BEGIN
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=2
   END
   ELSE error(14);
  END
  ELSE IF (operador2=CSHL) OR (operador2=CSHR) THEN
  BEGIN
   IF bit(tipoizq,tipoder) THEN
   BEGIN
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=2
   END
   ELSE error(14);
  END
  ELSE
  BEGIN
   IF boleanos(tipoizq,tipoder) THEN
   BEGIN
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=8
   END
   ELSE IF bit(tipoizq,tipoder) THEN
   BEGIN
    creanodo(operador,nodoizq,nododer,1);
    tipoi:=2
   END
   ELSE error(14);
  END;
 END;
END;

PROCEDURE simple;
VAR
 operador:narbol;
 operador2:comlex;
 optimiza:narbol;
 tipoizq,tipoder:INTEGER;
 nodoizq,nododer:INTEGER;
 menos:INTEGER;
BEGIN
 menos:=0;
 IF lexema=CMENOS THEN
 BEGIN
  obtlex;
  menos:=1
 END
 ELSE IF lexema=CMAS THEN obtlex;
 termino;
 IF menos=1 THEN
 BEGIN
  IF tipobase(tipoi)=2 THEN
  BEGIN
   optimiza:=arbol[nodoact].nom;
   IF optimiza=NINUM THEN arbol[nodoact].esp:=-arbol[nodoact].esp
                     ELSE creanodo(NNEG,nodoact,0,0)
  END
  ELSE IF tipoi=6 THEN
  BEGIN
   optimiza:=arbol[nodoact].nom;
   IF optimiza=NRNUM THEN
   BEGIN
    usoesp:=constantes[arbol[nodoact].esp];
    usoesp.valor:=-usoesp.valor;
    arbol[nodoact].esp:=agreal
   END
   ELSE
   BEGIN
    creanodo(NRCERO,0,0,0);
    creanodo(NRRESTA,nodoact,nodoact-1,0)
   END
  END
  ELSE error(14);
 END;
 WHILE (lexema=CMAS) OR (lexema=CMENOS) OR
       (lexema=COR) OR (lexema=CXOR) DO
 BEGIN
  operador2:=lexema;
  tipoizq:=tipobase(tipoi);
  nodoizq:=nodoact;
  obtlex;
  termino;
  tipoder:=tipobase(tipoi);
  nododer:=nodoact;
  CASE operador2 OF
   CMAS  : operador:=NISUMA;
   CMENOS: operador:=NIRESTA;
   COR   : operador:=NOR;
   CXOR  : operador:=NXOR
  END;
  IF (operador2=CMAS) OR (operador2=CMENOS) THEN
  BEGIN
   IF enteros(tipoizq,tipoder) THEN
   BEGIN
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=2
   END
   ELSE IF reales(tipoizq,tipoder) THEN
   BEGIN
    IF operador=NIRESTA THEN operador:=NRRESTA
                        ELSE operador:=NRSUMA;
    convreal(nodoizq,tipoizq);
    convreal(nododer,tipoder);
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=6
   END
   ELSE IF conjuntos(tipoizq,tipoder) THEN
   BEGIN
    IF operador2=CMAS THEN creanodo(NSUNION,nodoizq,nododer,0)
                      ELSE creanodo(NSDIFER,nodoizq,nododer,0);
    tipoi:=17
   END
   ELSE IF (operador2=CMAS) AND cadenas(tipoizq,tipoder) THEN
   BEGIN
    convcadena(nodoizq,tipoizq);
    convcadena(nododer,tipoder);
    creanodo(NCONCAT,nodoizq,nododer,0);
    tipoi:=18
   END
   ELSE error(14);
  END
  ELSE
  BEGIN
   IF boleanos(tipoizq,tipoder) THEN
   BEGIN
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=8
   END
   ELSE IF bit(tipoizq,tipoder) THEN
   BEGIN
    creanodo(operador,nodoizq,nododer,1);
    tipoi:=2
   END
   ELSE error(14);
  END;
 END;
END;

PROCEDURE expr2;
VAR
 operador2:comlex;
 operador:narbol;
 tipoizq,tipoder:INTEGER;
 nodoizq,nododer:INTEGER;
 temp:INTEGER;
BEGIN
 simple;
 IF (lexema=CIGUAL) OR (lexema=CNOIGUAL) OR
    (lexema=CMENOR) OR (lexema=CMENORIGUAL) OR
    (lexema=CMAYOR) OR (lexema=CMAYORIGUAL) THEN
 BEGIN
  operador2:=lexema;
  tipoizq:=tipobase(tipoi);
  nodoizq:=nodoact;
  obtlex;
  simple;
  tipoder:=tipobase(tipoi);
  nododer:=nodoact;
  IF NOT checatipos(tipoizq,tipoder) THEN error(14)
  ELSE
  BEGIN
   IF (operador2=CMENOR) OR (operador2=CMAYORIGUAL) THEN
   BEGIN
    temp:=nodoizq;    { Invierte el nodo izq. y el nodo der. }
    nodoizq:=nododer; { Esto optimiza las comparaciones < y >= }
    nododer:=temp;
    temp:=tipoizq;
    tipoizq:=tipoder;
    tipoder:=temp
   END;
   operador:=narbol(ord(operador2)+ord(NIIGUAL)-ord(CIGUAL));
   IF enteros(tipoizq,tipoder) OR (tipoizq=4) OR
      (clase[tipoizq]='E') OR (clase[tipoizq]='D') THEN
   BEGIN
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=8
   END
   ELSE IF reales(tipoizq,tipoder) THEN
   BEGIN
    operador:=narbol(ord(operador)+ord(NRIGUAL)-ord(NIIGUAL));
    convreal(nodoizq,tipoizq);
    convreal(nododer,tipoder);
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=8
   END
   ELSE IF clase[tipoizq]='A' THEN
   BEGIN
    operador:=narbol(ord(operador)+ord(NAIGUAL)-ord(NIIGUAL));
    creanodo(operador,nodoizq,nododer,num[tipoizq]);
    tipoi:=8
   END
   ELSE IF clase[tipoizq]='S' THEN
   BEGIN
    IF (operador2=CMENOR) OR
       (operador2=CMAYOR) THEN error(43);
    IF (operador2=CIGUAL) OR
       (operador2=CNOIGUAL) THEN
    BEGIN
     creanodo(NSCOMPA,nodoizq,nododer,0);
     IF operador2=CNOIGUAL THEN creanodo(NNOTB,nodoact,0,0);
    END
    ELSE
    BEGIN
     creanodo(NSDIFER,nodoizq,nododer,0);
     creanodo(NSALGO,nodoact,0,0);
    END;
    tipoi:=8
   END
   ELSE IF cadenas(tipoizq,tipoder) THEN
   BEGIN
    operador:=narbol(ord(operador)+ord(NCIGUAL)-ord(NIIGUAL));
    convcadena(nodoizq,tipoizq);
    convcadena(nododer,tipoder);
    creanodo(operador,nodoizq,nododer,0);
    tipoi:=8
   END
  END;
 END
 ELSE IF lexema=CIN THEN
 BEGIN
  tipoi:=tipobase(tipoi);
  IF (tipoi<>2) AND (tipoi<>4) AND
     (clase[tipoi]<>'E') AND
     (clase[tipoi]<>'S') THEN error(14);
  IF clase[tipoi]<>'S' THEN
  BEGIN
   creanodo(NSLIMPIA,0,0,0);
   creanodo(NSGENBIT,nodoact-1,0,nodoact)
  END;
  nodoizq:=nodoact;
  obtlex;
  expr2;
  IF clase[tipoi]<>'S' THEN error(14);
  creanodo(NSDIFER,nodoizq,nodoact,0);
  creanodo(NSALGO,nodoact,0,0);
  tipoi:=8
 END;
END;

PROCEDURE expresion;
BEGIN
 nodoact:=0;
 apuarb:=apulit;
 expr2;
 codigo
END;


{ Programa: Compilador de Pascal para el Inmos T805 }
{                                                   }
{ Módulo  : Analizador Léxico                       }
{                                                   }
{ (c) Copyright 1993-1994 Oscar Toledo G.           }
{          Todos los derechos reservados.           }

{ Inicializa el analizador léxico }
PROCEDURE inicanalex;
BEGIN
 pal[1]:='AND';
 pal[2]:='ARRAY';
 pal[3]:='ASM';
 pal[4]:='BEGIN';
 pal[5]:='CASE';
 pal[6]:='CONST';
 pal[7]:='DIV';
 pal[8]:='DO';
 pal[9]:='DOWNTO';
 pal[10]:='ELSE';
 pal[11]:='END';
 pal[12]:='FILE';  { No terminado }
 pal[13]:='FOR';
 pal[14]:='FORWARD';
 pal[15]:='FUNCTION';
 pal[16]:='GOTO';
 pal[17]:='IF';
 pal[18]:='IN';
 pal[19]:='LABEL';
 pal[20]:='MOD';
 pal[21]:='NIL';
 pal[22]:='NOT';
 pal[23]:='OF';
 pal[24]:='OR';
 pal[25]:='PACKED';  { Sin efecto }
 pal[26]:='PROCEDURE';
 pal[27]:='PROGRAM';
 pal[28]:='RECORD';
 pal[29]:='REPEAT';
 pal[30]:='SET';
 pal[31]:='SHL';
 pal[32]:='SHR';
 pal[33]:='STRING';
 pal[34]:='THEN';
 pal[35]:='TO';
 pal[36]:='TYPE';
 pal[37]:='UNTIL';
 pal[38]:='VAR';
 pal[39]:='WHILE';
 pal[40]:='WITH';
 pal[41]:='XOR';
 pal[42]:='_FINPROG';
 linact:=0;   { Inicializa el contador de lineas }
 caract:=' '; { Forza la lectura de un caracter  }
 aplin:=1;    { Empieza en el 1er. caracter de la linea ficticia }
 tamlin:=1;   { Tamaño ficticio de linea (para forzar carga de linea) }
END;

{ Obtiene una linea de la entrada }
PROCEDURE obtlin;
VAR car:CHAR;
BEGIN
 linact:=linact+1;
 write(chr(1),linact:4,' ');
 linea:='';
 WHILE NOT (eoln OR eof) DO
 BEGIN
  read(car);
  write(car);
  IF car<>chr(8) THEN linea:=linea+car
  ELSE linea:=copy(linea,1,length(linea)-1)
 END;
 writeln;
 IF NOT eof THEN readln { Absorbe el fin de linea }
 ELSE linea:=linea+chr(26); { Caracter indicador de fin de archivo }
 linea:=linea+' ';
 tamlin:=length(linea);
 aplin:=0;
 IF length(linea)>4 THEN
   IF (linea[1]='{') AND (linea[2]='$') AND
      (linea[3]='1') AND (linea[4]='}') THEN optcc:=TRUE
END;

{ Obtiene un caracter }
PROCEDURE obtcar;
BEGIN
 IF aplin>=tamlin THEN obtlin; { Excedio la linea, obtener otra linea }
 aplin:=aplin+1;               { Avanza el apuntador }
 caract:=linea[aplin]          { Toma el caracter }
END;

{ Obtiene un lexema }
PROCEDURE obtlex;
VAR
 caractm:CHAR;
 min,cen,max:INTEGER;
 conteo,conteo2:INTEGER;
 fraccion,divisor,signo,exponente:INTEGER;
 numproc:REAL;
 er:BOOLEAN;
BEGIN
 WHILE caract=' ' DO obtcar;    { Ignora espacios }
 lexema:=CERROR;
 numero:=0;
 cadena:='';
 caractm:=caract;
 IF (caractm>='a') AND (caractm<='z') THEN
  caractm:=chr(ord(caractm)-32);
 IF ((caractm>='A') AND         { Detecta identificadores }
     (caractm<='Z')) OR (caractm='_') THEN
 BEGIN
  REPEAT
   IF length(cadena)<255 THEN cadena:=cadena+caractm ELSE errorlex(1);
   obtcar;
   caractm:=caract;
   IF (caractm>='a') AND (caractm<='z') THEN
    caractm:=chr(ord(caractm)-32)
  UNTIL ((caractm<'0') OR (caractm>'9')) AND
        ((caractm<'A') OR (caractm>'Z')) AND (caractm<>'_');
  min:=1;                       { Busqueda binaria para saber }
  max:=palres;                  { si es una palabra reservada }
  REPEAT
   cen:=(min+max) DIV 2;
   IF cadena<=pal[cen] THEN max:=cen-1;
   IF cadena>=pal[cen] THEN min:=cen+1
  UNTIL min>max;
  IF (min-1)<=max THEN lexema:=CIDENT
                  ELSE lexema:=comlex(min-2)
 END
 ELSE IF caract='{' THEN        { Ignora los comentarios }
 BEGIN
  er:=FALSE;
  WHILE (caract<>'}') AND NOT er DO
  BEGIN
   obtcar;
   IF aplin=tamlin THEN         { Detecta fin de linea (error) }
   BEGIN
    errorlex(4);
    er:=TRUE
   END;
  END;
  obtcar;
  obtlex
 END
 ELSE IF caract='''' THEN       { Procesa una cadena alfanúmerica }
 BEGIN
  lexema:=CCAD;
  conteo2:=1;
  WHILE conteo2=1 DO
  BEGIN
   obtcar;
   IF aplin=tamlin THEN         { Detecta fin de linea (error) }
   BEGIN
    errorlex(2);
    conteo2:=0
   END
   ELSE
   BEGIN
    IF caract='''' THEN         { Detecta doble ' }
    BEGIN
     obtcar;
     IF caract<>'''' THEN conteo2:=0
    END;
    IF conteo2=1 THEN
     IF length(cadena)<255 THEN cadena:=cadena+caract ELSE errorlex(1);
   END;
  END;
 END
 ELSE IF caract='"' THEN        { Procesa una cadena alfanúmerica }
 BEGIN
  lexema:=CCAD;
  conteo2:=1;
  WHILE conteo2=1 DO
  BEGIN
   obtcar;
   IF aplin=tamlin THEN         { Detecta fin de linea (error) }
   BEGIN
    errorlex(7);
    conteo2:=0
   END
   ELSE
   BEGIN
    IF caract='"' THEN          { Detecta doble " }
    BEGIN
     obtcar;
     IF caract<>'"' THEN conteo2:=0
    END;
    IF conteo2=1 THEN
     IF length(cadena)<255 THEN cadena:=cadena+caract ELSE errorlex(1);
   END;
  END;
 END
 ELSE IF (caract>='0') AND      { Procesa números enteros/reales }
         (caract<='9') THEN
 BEGIN
  lexema:=CNUM;                 { Hasta aquí es un número entero }
  REPEAT                        { Procesa el número }
   cadena:=cadena+caract;
   numero:=numero*10+(ord(caract)-48);
   obtcar
  UNTIL (caract<'0') OR (caract>'9');
  IF (linea[aplin]='.') AND     { Sí encuentra un punto se asegura }
     (linea[aplin+1]<>'.') THEN { que no sea de un subrango.       }
  BEGIN
   obtcar;
   IF (caract<'0') OR           { Se necesita un digito despues del punto }
      (caract>'9') THEN errorlex(3)
   ELSE
   BEGIN
    numproc:=numero;            { Se encontro que es número real }
    lexema:=CNUMREAL;
    fraccion:=0;                { Calcula la fracción }
    divisor:=1;
    REPEAT
     fraccion:=fraccion*10+(ord(caract)-48);
     divisor:=divisor*10;
     obtcar
    UNTIL (caract<'0') OR (caract>'9');
    numproc:=numproc+fraccion/divisor;
   END;
  END;
  IF (caract='E') OR (caract='e') THEN  { Procesa el exponente }
  BEGIN
   IF lexema<>CNUMREAL THEN       { Verifica posible conversión }
   BEGIN
    numproc:=numero;
    lexema:=CNUMREAL
   END;
   obtcar;
   signo:=0;                      { Verifica signo del exponente }
   IF caract='-' THEN signo:=1;
   IF (caract='-') OR (caract='+') THEN obtcar;
   IF (caract<'0') OR (caract>'9') THEN errorlex(3)
   ELSE
   BEGIN
    exponente:=0;
    REPEAT
     exponente:=exponente*10+(ord(caract)-48);
     obtcar
    UNTIL (caract<'0') OR (caract>'9');
    IF signo=1 THEN exponente:=-exponente;
    IF exponente<0 THEN
    BEGIN
     REPEAT
      numproc:=numproc/10;
      exponente:=exponente+1
     UNTIL exponente=0;
    END
    ELSE IF exponente>0 THEN
    BEGIN
     REPEAT
      numproc:=numproc*10;
      exponente:=exponente-1
     UNTIL exponente=0;
    END;
   END;
  END;
  IF lexema=CNUMREAL THEN numreal.valor:=numproc;
 END
 ELSE IF caract='$' THEN       { Procesa números hexadecimales }
 BEGIN
  lexema:=CNUM;
  obtcar;
  caractm:=caract;
  IF (caractm>='a') AND (caractm<='z') THEN
   caractm:=chr(ord(caractm)-32);
  IF (caractm>='0') AND (caractm<='9') OR
     (caractm>='A') AND (caractm<='F') THEN
  BEGIN
   REPEAT
    conteo:=ord(caractm)-48;
    IF conteo>9 THEN conteo:=conteo-7;
    numero:=(numero SHL 4)+conteo; { Los parentesis no son necesarios }
    obtcar;
    caractm:=caract;
    IF (caractm>='a') AND (caractm<='z') THEN
     caractm:=chr(ord(caractm)-32);
   UNTIL ((caractm<'0') OR (caractm>'9')) AND
         ((caractm<'A') OR (caractm>'F'))
  END
  ELSE errorlex(5);
 END
 ELSE
 BEGIN
  caractm:=caract;
  obtcar;
  CASE caractm OF
   '+': lexema:=CMAS;
   '-': lexema:=CMENOS;
   '*': lexema:=CPOR;
   '/': lexema:=CDIVIDE;
   '^': lexema:=CAPUNTA;
   '(': lexema:=CPARENI;
   ')': lexema:=CPAREND;
   '[': lexema:=CLLAVEI;
   ']': lexema:=CLLAVED;
   ';': lexema:=CPUNTOYCOMA;
   ',': lexema:=CCOMA;
   '=': lexema:=CIGUAL;
   ':':                    { Verifica := }
    IF caract='=' THEN
    BEGIN
     lexema:=CASIGNA;
     obtcar
    END
    ELSE lexema:=CDOSPUNTOS;
   '.':                    { Verifica .. }
    IF caract='.' THEN
    BEGIN
     lexema:=CPUNTOPUNTO;
     obtcar
    END
    ELSE lexema:=CPUNTO;
   '<':                    { Verifica <= o <> }
    IF caract='=' THEN
    BEGIN
     lexema:=CMENORIGUAL;
     obtcar
    END
    ELSE IF caract='>' THEN
    BEGIN
     lexema:=CNOIGUAL;
     obtcar
    END
    ELSE lexema:=CMENOR;
   '>':                    { Verifica >= }
    IF caract='=' THEN
    BEGIN
     lexema:=CMAYORIGUAL;
     obtcar
    END
    ELSE lexema:=CMAYOR
   ELSE
   BEGIN
    IF caractm=chr(26) THEN lexema:=CFINDEARCHIVO
    ELSE
    BEGIN
     errorlex(6);
     lexema:=CERROR
    END
   END
  END
 END
END;


{ Programa: Compilador de Pascal para el Inmos T805 }
{                                                   }
{ Módulo  : Generador de Codigo                     }
{                                                   }
{ (c) Copyright 1993-1994 Oscar Toledo G.           }
{          Todos los derechos reservados.           }

{ Vacia las cadenas usadas hasta el momento }
PROCEDURE vaciacad;
VAR concad,contador,poslit:INTEGER;
BEGIN
 FOR concad:=proxcad TO cadact DO
 BEGIN
  writeln('P',tabcad[concad].etiqueta:0,':');
  poslit:=tabcad[concad].posicion;
  contador:=0;
  WHILE litcad[poslit]<>chr(0) DO
  BEGIN
   IF contador=0 THEN write('db ')
   ELSE write(',');
   write(ord(litcad[poslit]):0);
   contador:=contador+1;
   IF contador=8 THEN
   BEGIN
    writeln;
    contador:=0
   END;
   poslit:=poslit+1
  END;
  IF contador<>0 THEN writeln;
 END;
 proxcad:=cadact+1;
END;

{ Optimización especial }

{ yoptimiza(0,0)  saca cuálquier optimización guardada.   }
{ yoptimiza(0,1)  saca sólo optimización asignaciones     }
{                 del resultado de funciones.             }
{ yoptimiza(1,x)  indica que va un ajw x, la optimización }
{                 se encarga de juntar ajw juntos.        }
{ yoptimiza(2,0)  indica que va un fpstnlsn local para    }
{                 resultado de función.                   }
{ yoptimiza(3,0)  indica que va un stl local para         }
{                 resultado de función.                   }

{ Calcula el desplazamiento relativo a la base hecho hasta el momento }
FUNCTION vdes:INTEGER;
BEGIN
 vdes:=0-apret-pila
END;

PROCEDURE yoptimiza(clase,valor:INTEGER);
BEGIN
 CASE clase OF
 0:
  BEGIN
   IF aajw AND (oajw<>0) AND (valor=0) THEN
   BEGIN
    writeln('ajw ',oajw:0);
    apret:=apret+oajw;
    oajw:=0;
    aajw:=FALSE
   END;
   IF afps THEN
   BEGIN
    writeln('ldlp ',vdes+pila+afps2:0);
    writeln('fpstnlsn');
   END;
   IF astl THEN writeln('stl ',vdes+pila+astl2:0);
   afps:=FALSE;
   astl:=FALSE
  END;
 1:
  BEGIN
   oajw:=oajw+valor;
   aajw:=TRUE;
  END;
 2:
  BEGIN
   afps:=TRUE;
   afps2:=valor
  END;
 3:
  BEGIN
   astl:=TRUE;
   astl2:=valor
  END
 END;
END;

{ Crea una nueva etiqueta }
FUNCTION geneti:INTEGER;
BEGIN
 proxeti:=proxeti+1;
 geneti:=proxeti
END;

{ Saca la posición del enlace estático }
FUNCTION enl:INTEGER;
BEGIN
 IF aenl THEN enl:=0
         ELSE enl:=pila+1;
END;

{ Copia el enlace }
PROCEDURE copiaenlace;
BEGIN
 IF NOT aenl THEN
 BEGIN
  IF enl+vdes<0 THEN yoptimiza(0,0);
  writeln('ldl ',enl+vdes:0);
  aenl:=TRUE;
  IF enl+vdes<0 THEN yoptimiza(0,0);
  writeln('stl ',enl+vdes:0);
 END;
END;

{ Genera etiqueta de procedimiento/función }
PROCEDURE gepf(eti:INTEGER);
BEGIN
 writeln('P',eti:0,':')
END;

{ Ilustra una etiqueta (con vaciado de optimización) }
PROCEDURE geti(eti:INTEGER);
BEGIN
 yoptimiza(0,0);
 copiaenlace;
 writeln('P',eti:0,':')
END;

{ Ajusta el apuntador al espacio de trabajo }
PROCEDURE gpila(pila:INTEGER);
BEGIN
 yoptimiza(1,pila)
END;

{ Salto condicional a una etiqueta }
PROCEDURE gcj(etiqueta:INTEGER);
BEGIN
 yoptimiza(0,0);
 copiaenlace;
 writeln('cj P',etiqueta:0)
END;

{ Salto incondicional a una etiqueta }
PROCEDURE gj(etiqueta:INTEGER);
BEGIN
 yoptimiza(0,0);
 copiaenlace;
 writeln('j P',etiqueta:0);
 vaciacad     { Vacia el almacenamiento (se hace tan pronto se puede) }
END;

{ Llamada a una etiqueta }
PROCEDURE gcall(etiqueta:INTEGER);
BEGIN
 yoptimiza(0,0);
 writeln('call P',etiqueta:0)
END;

{ Saca la base de las variables no-locales }
PROCEDURE gbase(actual, simbolo:INTEGER);
VAR contador:INTEGER;
BEGIN
 IF actual=simbolo THEN
 BEGIN
{ Sí se va a obtener la dirección del enlace estático }
{ entonces hay que cargarlo sí no estaba cargado }
  copiaenlace;
  IF enl+vdes<0 THEN yoptimiza(0,0);
  writeln('ldlp ',enl+vdes:0)
 END
 ELSE
 BEGIN
  IF enl+vdes<0 THEN yoptimiza(0,0);
  writeln('ldl ',enl+vdes:0);
  FOR contador:=2 TO actual-simbolo DO writeln('ldnl 0');
 END;
END;

{ Crea un nodo del arbol de expresiones }
PROCEDURE creanodo(operador:narbol; i,d,valor:INTEGER);
VAR contador:INTEGER;
BEGIN
 nodoact:=nodoact+1;
 IF nodoact>maxarb THEN error(49);
 arbol[nodoact].nom:=operador; { Nodo }
 arbol[nodoact].izq:=i;        { Apuntador al descendiente izquierdo }
 arbol[nodoact].der:=d;        { Apuntador al descendiente derecho }
 arbol[nodoact].reg:=0;        { Número de registros enteros requeridos}
 arbol[nodoact].regf:=0;       { Número de regs. de punto flotante req.}
 arbol[nodoact].esp:=valor     { Franja multi-usos }
END;

{ Extrae una cadena del buffer literal }
PROCEDURE deslit(lit:INTEGER; VAR nombre:STRING);
BEGIN
 nombre:='';
 WHILE literal[lit]<>chr(0) DO
 BEGIN
  nombre:=nombre+literal[lit];
  lit:=lit+1
 END;
END;

{ Obtiene la dirección de una variable }
PROCEDURE obtdir(postab:INTEGER);
VAR nivel,contador,dir:INTEGER;
BEGIN
 nivel:=profact-prof[postab];
 IF val[postab]>=0 THEN dir:=val[postab]
 ELSE dir:=pparinf[prof[postab]]+abs(val[postab]); { Parametro formal }
 IF nivel=0 THEN
 BEGIN
  IF vdes+dir<0 THEN yoptimiza(0,0);
  IF paso[postab]=1 THEN write('ldl ')
  ELSE write('ldlp ');
  writeln(vdes+dir:0)
 END
 ELSE
 BEGIN
  gbase(profact,prof[postab]);
  IF paso[postab]=1 THEN write('ldnl ')
  ELSE write('ldnlp ');
  writeln(dir:0);
 END;
END;

{ Lee una variable simple }
PROCEDURE leevar(postab:INTEGER);
VAR nivel,contador,dir:INTEGER;
BEGIN
 IF paso[postab]=1 THEN
 BEGIN
  obtdir(postab);
  writeln('ldnl 0')
 END
 ELSE
 BEGIN
  nivel:=profact-prof[postab];
  IF val[postab]>=0 THEN dir:=val[postab]
  ELSE dir:=pparinf[prof[postab]]+abs(val[postab]); { Parametro formal }
  IF nivel=0 THEN
  BEGIN
   IF vdes+dir<0 THEN yoptimiza(0,0);
   writeln('ldl ',vdes+dir:0)
  END
  ELSE
  BEGIN
   gbase(profact,prof[postab]);
   writeln('ldnl ',dir:0);
  END;
 END;
END;

{ Guarda en una variable simple }
PROCEDURE guavar(postab:INTEGER);
VAR nivel,contador,dir:INTEGER;
BEGIN
 IF paso[postab]=1 THEN
 BEGIN
  obtdir(postab);
  writeln('stnl 0')
 END
 ELSE
 BEGIN
  nivel:=profact-prof[postab];
  IF val[postab]>=0 THEN dir:=val[postab]
  ELSE dir:=pparinf[prof[postab]]+abs(val[postab]); { Parametro formal }
  IF nivel=0 THEN
  BEGIN
   IF vdes+dir<0 THEN yoptimiza(0,0);
   writeln('stl ',vdes+dir:0)
  END
  ELSE
  BEGIN
   gbase(profact,prof[postab]);
   writeln('stnl ',dir:0);
  END;
 END;
END;

{ Crea una variable temporal y devuelve su dirección }
PROCEDURE createmp(VAR dir:INTEGER);
BEGIN
 gpila(-1);
 dir:=apret+oajw+pila
END;

{ Borra la última variable temporal }
PROCEDURE borratemp;
BEGIN
 gpila(1)
END;

{ Carga un número real de la tabla de constantes }
PROCEDURE careal(poscons:INTEGER);
BEGIN
 gbase(profact,-1);
 IF poscons>0 THEN writeln('ldnlp ',poscons:0);
 writeln('fpldnlsn');
END;

{ Carga la dirección de una cadena, averigua que no sea redefinida }
PROCEDURE carcad(poslit:INTEGER);
VAR
 etiqcad,etiqins,contador,contador2,contador3:INTEGER;
 existente,igual,final:BOOLEAN;
BEGIN
 contador:=1;
 existente:=FALSE;
 WHILE (contador<=cadact) AND NOT existente DO
 BEGIN
  contador2:=tabcad[contador].posicion;
  contador3:=poslit;
  igual:=TRUE;
  final:=FALSE;
  REPEAT
   IF litcad[contador2]<>literal[contador3] THEN igual:=FALSE
   ELSE
   BEGIN
    IF litcad[contador2]=chr(0) THEN final:=TRUE
    ELSE
    BEGIN
     contador2:=contador2+1;
     contador3:=contador3+1
    END
   END
  UNTIL NOT igual OR final;
  IF final THEN
  BEGIN
   etiqcad:=tabcad[contador].etiqueta;
   existente:=TRUE
  END;
  contador:=contador+1
 END;
 IF NOT existente THEN
 BEGIN
  etiqcad:=geneti;
  cadact:=cadact+1;
  IF cadact>maxcad THEN error(47);
  tabcad[cadact].etiqueta:=etiqcad;
  tabcad[cadact].posicion:=cadlit;
  REPEAT
   litcad[cadlit]:=literal[poslit];
   cadlit:=cadlit+1;
   poslit:=poslit+1
  UNTIL litcad[cadlit-1]=chr(0);
  IF cadlit>maxcadl THEN error(48)
 END;
 etiqins:=geneti;
 writeln('ldc P',etiqcad:0,'-P',etiqins:0);
 writeln('ldpi');
 writeln('P',etiqins:0,':');
END;

PROCEDURE gencodigo(nodo:INTEGER); FORWARD;

{ Genera codigo para una llamada a función/procedimiento }
PROCEDURE llamafunc(nodo:INTEGER);
VAR
 posfunc,espacio,numpar,contador,paract,tipopar,clasellamada,comienzo:INTEGER;
 pars:ARRAY[1..64] OF INTEGER;
 optimiza:narbol;
 pospar:INTEGER;
 reg1,reg2,nodoizq,nododer:INTEGER;
BEGIN
 posfunc:=arbol[nodo].esp;
 IF clase[posfunc]='F' THEN clasellamada:=1
                       ELSE clasellamada:=0;
 numpar:=num[posfunc];
 espacio:=0;
 paract:=posfunc;
 FOR contador:=1 TO numpar+clasellamada DO   { Cuenta el espacio necesario }
 BEGIN                                       { para los parametros. }
  paract:=prox[paract];
  IF (val[paract]<>-2) AND (val[paract]<>-3) THEN
   IF paso[paract]=1 THEN espacio:=espacio+1
                     ELSE espacio:=espacio+((num[paract]+3) DIV 4);
 END;
 gpila(-espacio);                { Asigna el espacio }
 pospar:=apret+oajw+pila;
 paract:=prox[posfunc];
 FOR contador:=numpar DOWNTO 1 DO
 BEGIN
  nodo:=arbol[nodo].der;         { Ordena los parametros }
  pars[contador]:=nodo;
 END;
 reg1:=0;    { No ha encontrado ningun parametro para pasar en los }
 reg2:=0;    { registros del procesador }
 FOR contador:=1 TO numpar DO
 BEGIN
       IF val[paract]=-2 THEN reg2:=contador
  ELSE IF val[paract]=-3 THEN reg1:=contador
  ELSE
  BEGIN
   tipopar:=tipo[arbol[pars[contador]].esp];
   optimiza:=arbol[pars[contador]-1].nom;
        IF optimiza=NRCARGA THEN
   BEGIN
    arbol[pars[contador]-1].nom:=NICARGA;
    gencodigo(pars[contador]-1);
    tipopar:=2
   END
   ELSE IF optimiza=NRCERO THEN
   BEGIN
    writeln('ldc 0');
    tipopar:=2
   END
   ELSE IF optimiza=NRNUM THEN
   BEGIN
    gbase(profact,-1);
    writeln('ldnl ',arbol[pars[contador]-1].esp:0);
    tipopar:=2
   END
   ELSE gencodigo(pars[contador]-1);
   IF vdes+pospar<0 THEN yoptimiza(0,0);
   IF paso[paract]=1 THEN writeln('stl ',vdes+pospar:0)
   ELSE
   BEGIN
    IF tipopar=6 THEN
    BEGIN
     writeln('ldlp ',vdes+pospar:0);
     writeln('fpstnlsn')
    END
    ELSE IF (clase[tipopar]='A') OR (clase[tipopar]='R') OR
            (clase[tipopar]='O') OR (clase[tipopar]='S') THEN
    BEGIN
     writeln('ldlp ',vdes+pospar:0);
     writeln('ldc ',num[paract]:0);
     writeln('move')
    END
    ELSE IF clase[tipopar]='N' THEN
    BEGIN
     writeln('ldlp ',vdes+pospar:0);
     writeln('ldc ',num[paract]:0);
     yoptimiza(0,0);
     writeln('call ASIGNASTR');
     apret:=apret+64
    END
    ELSE writeln('stl ',vdes+pospar:0);
   END;
   IF paso[paract]=1 THEN pospar:=pospar+1
                     ELSE pospar:=pospar+((num[paract]+3) DIV 4);
  END;
  paract:=prox[paract];          { Proximo parametro }
 END;
 IF reg1<>0 THEN
 BEGIN
  nodoizq:=pars[reg1]-1;
  nododer:=pars[reg2]-1;
  IF (arbol[nododer].reg>arbol[nodoizq].reg) AND
     (arbol[nodoizq].reg<3) THEN
  BEGIN
   gencodigo(nododer);
   gencodigo(nodoizq);
   writeln('rev')
  END
  ELSE IF (arbol[nodoizq].reg>=arbol[nododer].reg) AND
          (arbol[nododer].reg<3) THEN
  BEGIN
   gencodigo(nodoizq);
   gencodigo(nododer)
  END
  ELSE
  BEGIN
   gencodigo(nododer);
   createmp(nododer);
   IF vdes+nododer<0 THEN yoptimiza(0,0);
   writeln('stl ',vdes+nododer:0);
   gencodigo(nodoizq);
   writeln('ldl ',vdes+nododer:0);
   borratemp
  END
 END
 ELSE IF reg2<>0 THEN gencodigo(pars[reg2]-1);
 gbase(profact,prof[posfunc]);
 gcall(val[posfunc]);            { Llama a la función }
 IF (clasellamada=1) AND         { Verifica si el resultado es un conjunto }
    ((clase[tipo[paract]]='S') OR { o una cadena, si es así lo acumula en el }
     (clase[tipo[paract]]='N')) THEN { espacio de trabajo }
  espacio:=espacio-((num[paract]+3) DIV 4);
 gpila(espacio)                  { Deasigna el espacio }
END;

{ Calcula el nemonico para nodos binarios }
PROCEDURE calcula(nodo,rev:INTEGER);
BEGIN
 CASE arbol[nodo].nom OF
  NISUMA: writeln('bsub');
  NIMUL: writeln('prod');
  NMULRAPIDA: writeln('prod');
  NIIGUALESP: writeln('diff');
  NIIGUAL:
   BEGIN
    writeln('diff');
    writeln('eqc 0')
   END;
  NINOIGUAL:
   BEGIN
    writeln('diff');
    writeln('eqc 0');
    writeln('eqc 0')
   END;
  NRSUMA: writeln('fpadd');
  NRMUL: writeln('fpmul');
  NRIGUAL: writeln('fpeq');
  NRNOIGUAL:
   BEGIN
    writeln('fpeq');
    writeln('eqc 0')
   END;
  NAIGUAL:
   BEGIN
    writeln('ldc ',arbol[nodo].esp:0);
    writeln('call IGUAL')
   END;
  NANOIGUAL:
   BEGIN
    writeln('ldc ',arbol[nodo].esp:0);
    writeln('call NOIGUAL')
   END;
  NAND: writeln('and');
  NOR: writeln('or');
  NXOR: writeln('xor');
  NINDEXABYTE: writeln('bsub')
  ELSE
  BEGIN
   IF rev=1 THEN
   BEGIN
    CASE arbol[nodo].nom OF
     NRDIV,NRRESTA,NRMENOR,NRMAYORIGUAL,NRMENORIGUAL,NRMAYOR:
      writeln('fprev')
     ELSE writeln('rev')
    END
   END;
   CASE arbol[nodo].nom OF
    NIRESTA: writeln('diff');
    NIDIV: writeln('div');
    NIMOD: writeln('rem');
    NSHL: writeln('shl');
    NSHR: writeln('shr');
    NINDEXAPAL: writeln('wsub');
    NRCARGAI: writeln('fpldnlsni');
    NIMAYOR,NIMENOR: writeln('gt');
    NIMENORIGUAL,NIMAYORIGUAL:
     BEGIN
      writeln('gt');
      writeln('eqc 0')
     END;
    NRDIV: writeln('fpdiv');
    NRRESTA: writeln('fpsub');
    NRMAYOR,NRMENOR: writeln('fpgt');
    NRMENORIGUAL,NRMAYORIGUAL:
     BEGIN
      writeln('fpgt');
      writeln('eqc 0')
     END;
    NAMAYOR,NAMENOR:
     BEGIN
      writeln('ldc ',arbol[nodo].esp:0);
      writeln('call MAYOR')
     END;
    NAMENORIGUAL,NAMAYORIGUAL:
     BEGIN
      writeln('ldc ',arbol[nodo].esp:0);
      writeln('call MENORIGUAL')
     END
    ELSE
    BEGIN
     IF arbol[nodo].nom<>NNULO THEN yoptimiza(0,0);
     CASE arbol[nodo].nom OF
      NSGENBITS: writeln('call GENBITS');
      NCOPY: writeln('call COPYSTR');
      NDELETE: writeln('call DELETESTR')
     END
    END
   END
  END
 END
END;

{ Verifica si el nodo tiene dos descendientes de punto flotante }
FUNCTION espuntoflotante(nodoa:INTEGER):BOOLEAN;
VAR nodo:narbol;
BEGIN
 nodo:=arbol[nodoa].nom;
 espuntoflotante:=(nodo=NRSUMA) OR (nodo=NRRESTA) OR
                  (nodo=NRMUL) OR (nodo=NRDIV) OR
                  (nodo=NRIGUAL) OR (nodo=NRNOIGUAL) OR
                  (nodo=NRMENOR) OR (nodo=NRMAYOR) OR
                  (nodo=NRMENORIGUAL) OR (nodo=NRMAYORIGUAL)
END;

{ Generación de codigo por el nodo izquierdo y luego el derecho }
PROCEDURE genizq(nodo:INTEGER);
VAR nombre,nombre2:narbol; poscons:INTEGER;
BEGIN
 gencodigo(arbol[nodo].izq);
 nombre:=arbol[nodo].nom;
 nombre2:=arbol[arbol[nodo].der].nom;
{ Optimiza comparaciones, sumas y restas con constantes enteras }
 IF ((nombre=NISUMA) OR (nombre=NIRESTA) OR
     (nombre=NIIGUAL) OR (nombre=NINOIGUAL)) AND
    (nombre2=NINUM) THEN
 BEGIN
  IF (nombre=NISUMA) OR
     (nombre=NIRESTA) THEN write('adc ')
                      ELSE write('eqc ');
  writeln(arbol[arbol[nodo].der].esp:0);
  IF nombre=NINOIGUAL THEN writeln('eqc 0');
 END
 ELSE
 BEGIN
{ Optimiza sumas y multiplicaciones con números reales }
  IF ((nombre=NRSUMA) OR (nombre=NRMUL)) AND
     ((nombre2=NRCARGA) OR (nombre2=NRNUM)) THEN
  BEGIN
   IF nombre2=NRNUM THEN
   BEGIN
    gbase(profact,-1);
    poscons:=arbol[arbol[nodo].der].esp;
    IF poscons>0 THEN writeln('ldnlp ',poscons:0);
   END
   ELSE gencodigo(arbol[arbol[nodo].der].izq);
   IF nombre=NRSUMA THEN writeln('fpldnladdsn')
                    ELSE writeln('fpldnlmulsn');
  END
  ELSE
  BEGIN
   gencodigo(arbol[nodo].der);
   calcula(nodo,0)
  END;
 END;
END;

{ Generación de codigo por el nodo derecho y luego el izquierdo }
PROCEDURE gender(nodo:INTEGER);
VAR nombre,nombre2:narbol; poscons:INTEGER;
BEGIN
 gencodigo(arbol[nodo].der);
 nombre:=arbol[nodo].nom;
 nombre2:=arbol[arbol[nodo].izq].nom;
{ Optimiza comparaciones, sumas y restas con constantes enteras }
 IF ((nombre=NISUMA) OR (nombre=NIRESTA) OR
     (nombre=NIIGUAL) OR (nombre=NINOIGUAL)) AND
    (nombre2=NINUM) THEN
 BEGIN
  IF (nombre=NISUMA) OR
     (nombre=NIRESTA) THEN write('adc ')
                      ELSE write('eqc ');
  writeln(arbol[arbol[nodo].izq].esp:0);
  IF nombre=NINOIGUAL THEN writeln('eqc 0');
 END
 ELSE
 BEGIN
{ Optimiza sumas y multiplicaciones con números reales }
  IF ((nombre=NRSUMA) OR (nombre=NRMUL)) AND
     ((nombre2=NRCARGA) OR (nombre2=NRNUM)) THEN
  BEGIN
   IF nombre2=NRNUM THEN
   BEGIN
    gbase(profact,-1);
    poscons:=arbol[arbol[nodo].izq].esp;
    IF poscons>0 THEN writeln('ldnlp ',poscons:0);
   END
   ELSE gencodigo(arbol[arbol[nodo].izq].izq);
   IF nombre=NRSUMA THEN writeln('fpldnladdsn')
                    ELSE writeln('fpldnlmulsn');
  END
  ELSE
  BEGIN
   gencodigo(arbol[nodo].izq);
   calcula(nodo,1)
  END;
 END;
END;

{ Obtiene la dirección de una variable (con optimización para ldnlp) }
FUNCTION dirvar(nodo,sumafranjas:INTEGER):INTEGER;
VAR nivel,contador,dir,postab:INTEGER;
BEGIN
 postab:=arbol[nodo].esp;
 nivel:=profact-prof[postab];
 IF val[postab]>=0 THEN dir:=val[postab]
 ELSE dir:=pparinf[prof[postab]]+abs(val[postab]); { Parametro formal }
 IF nivel=0 THEN
 BEGIN
  IF vdes+dir<0 THEN yoptimiza(0,0);
  IF paso[postab]=1 THEN
  BEGIN
   writeln('ldl ',vdes+dir:0);
   dirvar:=sumafranjas
  END
  ELSE
  BEGIN
   writeln('ldlp ',vdes+dir+sumafranjas:0);
   dirvar:=0
  END
 END
 ELSE
 BEGIN
  gbase(profact,prof[postab]);
  IF paso[postab]=1 THEN
  BEGIN
   write('ldnl ',dir:0);
   dirvar:=sumafranjas
  END
  ELSE dirvar:=dir+sumafranjas
 END;
END;

{ Calcula el desplazamiento total de varios ldnlp }
{ para integrarlos en uno sólo }
FUNCTION totaldes(nodo:INTEGER):INTEGER;
VAR sumafranjas:INTEGER; nombre:narbol;
BEGIN
 sumafranjas:=0;
 REPEAT
  sumafranjas:=sumafranjas+arbol[arbol[nodo].der].esp;
  nodo:=arbol[nodo].izq;
  nombre:=arbol[nodo].nom;
 UNTIL nombre<>NIFRANJA;
 IF nombre=NDIRVAR THEN sumafranjas:=dirvar(nodo,sumafranjas)
                   ELSE gencodigo(nodo);
 totaldes:=sumafranjas
END;

{ Rutina principal de generación de codigo (recursiva) }
PROCEDURE gencodigo;
VAR
 nombre,nombre2,nombre3:narbol;
 contador,tipoesp:INTEGER;
 dirtemp:INTEGER;
 optimizado:BOOLEAN;
 sumafranjas:INTEGER;
 posicion:INTEGER;
BEGIN
 nombre:=arbol[nodo].nom;   { Primero nodos terminales }
 IF (nombre=NSGENBIT) OR (nombre=NSGENBITS) OR
    (nombre=NCOPY) OR (nombre=NDELETE) THEN
  gencodigo(arbol[nodo].esp);
      IF nombre=NFUNCION THEN llamafunc(nodo)
 ELSE IF nombre=NSUNION THEN
 BEGIN
  gencodigo(arbol[nodo].izq);
  gencodigo(arbol[nodo].der);
  yoptimiza(0,0);
  writeln('call UNIONSET');
  apret:=apret+8
 END
 ELSE IF nombre=NSDIFER THEN
 BEGIN
  gencodigo(arbol[nodo].izq);
  gencodigo(arbol[nodo].der);
  yoptimiza(0,0);
  writeln('call DIFERSET');
  apret:=apret+8
 END
 ELSE IF nombre=NSINTER THEN
 BEGIN
  gencodigo(arbol[nodo].izq);
  gencodigo(arbol[nodo].der);
  yoptimiza(0,0);
  writeln('call INTERSET');
  apret:=apret+8
 END
 ELSE IF nombre=NSCOMPA THEN
 BEGIN
  gencodigo(arbol[nodo].izq);
  gencodigo(arbol[nodo].der);
  yoptimiza(0,0);
  writeln('call COMPASET');
  apret:=apret+16
 END
 ELSE IF nombre=NCONCAT THEN
 BEGIN
  gencodigo(arbol[nodo].izq);
  gencodigo(arbol[nodo].der);
  yoptimiza(0,0);
  writeln('call CONCATSTR');
  apret:=apret+64
 END
 ELSE IF nombre=NPOS THEN
 BEGIN
  gencodigo(arbol[nodo].izq);
  gencodigo(arbol[nodo].der);
  yoptimiza(0,0);
  writeln('call POSSTR');
  apret:=apret+128
 END
 ELSE IF (nombre=NCIGUAL) OR (nombre=NCNOIGUAL) THEN
 BEGIN
  gencodigo(arbol[nodo].izq);
  gencodigo(arbol[nodo].der);
  yoptimiza(0,0);
  writeln('call IGUALSTR');
  IF nombre=NCNOIGUAL THEN writeln('eqc 0');
  apret:=apret+128
 END
 ELSE IF (nombre=NCMAYOR) OR (nombre=NCMENOR) OR
         (nombre=NCMENORIGUAL) OR (nombre=NCMAYORIGUAL) THEN
 BEGIN
  gencodigo(arbol[nodo].izq);
  gencodigo(arbol[nodo].der);
  yoptimiza(0,0);
  writeln('call MAYORSTR');
  IF (nombre=NCMENORIGUAL) OR (nombre=NCMAYORIGUAL) THEN
   writeln('eqc 0');
  apret:=apret+128
 END
 ELSE IF nombre=NINSERT THEN
 BEGIN
  gencodigo(arbol[nodo].izq);
  gencodigo(arbol[nodo].der);
  gencodigo(arbol[nodo].esp);
  yoptimiza(0,0);
  writeln('call INSERTSTR');
  apret:=apret+64
 END
 ELSE IF (arbol[nodo].izq=0) AND (arbol[nodo].der=0) THEN
 BEGIN
       IF nombre=NINUM THEN writeln('ldc ',arbol[nodo].esp:0)
  ELSE IF nombre=NNIL THEN writeln('mint')
  ELSE IF nombre=NRNUM THEN careal(arbol[nodo].esp)
  ELSE IF nombre=NRCERO THEN writeln('fpldzerosn')
  ELSE IF nombre=NCAD THEN carcad(arbol[nodo].esp)
  ELSE IF nombre=NDIRVAR THEN obtdir(arbol[nodo].esp)
  ELSE IF nombre=NCARGAFOR THEN writeln('ldl ',vdes+arbol[nodo].esp:0)
  ELSE IF nombre=NFINPROG THEN
  BEGIN
   posicion:=geneti;
   writeln('ldc FINPROG-P',posicion:0);
   writeln('ldpi');
   writeln('P',posicion:0,':')
  END
  ELSE IF nombre=NCARGAWITH THEN
  BEGIN
   IF vdes+pwith<0 THEN yoptimiza(0,0);
   writeln('ldl ',vdes+pwith:0)
  END
  ELSE IF nombre=NEOLN THEN
  BEGIN
   gbase(profact,-1);
   writeln('ldnlp -NUMCONSLIB');
   writeln('call EOLN')
  END
  ELSE IF nombre=NEOF THEN
  BEGIN
   gbase(profact,-1);
   writeln('ldnlp -NUMCONSLIB');
   writeln('call EOF')
  END
  ELSE IF nombre=NSLIMPIA THEN
  BEGIN
   yoptimiza(0,0);
   writeln('call LIMPIASET');
   apret:=apret-8
  END
  ELSE IF nombre=NRANDOM THEN
  BEGIN
   gbase(profact,-1);
   writeln('ldnlp -NUMCONSLIB');
   writeln('call RANDOM')
  END
  ELSE IF nombre=NCLIMPIA THEN
  BEGIN
   yoptimiza(0,0);
   writeln('call LIMPIASTR');
   apret:=apret-64
  END
 END
 ELSE IF nombre=NASIGNA THEN  { Las asignaciones son un caso especial }
 { Esto es debido a que el nodo derecho (la dirección de la variable) }
 { siempre es un valor entero, y el nodo izquierdo puede ser un }
 { entero o un REAL, además así se pueden optimizar otros casos }
 BEGIN
  nombre2:=arbol[arbol[nodo].der].nom;
  tipoesp:=arbol[nodo].esp;
  { Asignación a variable simple CHAR, INTEGER, REAL o apuntador }
  IF ((tipoesp=2) OR (tipoesp=4) OR (tipoesp=6) OR
     (clase[tipoesp]='D') OR (clase[tipoesp]='E')) AND
     (nombre2=NDIRVAR) THEN
  BEGIN
   nombre3:=arbol[arbol[nodo].izq].nom;
        IF nombre3=NRCERO THEN
   BEGIN
    writeln('ldc 0');
    tipoesp:=2
   END
   ELSE IF nombre3=NRCARGA THEN
   BEGIN
    arbol[arbol[nodo].izq].nom:=NICARGA;
    gencodigo(arbol[nodo].izq);
    tipoesp:=2
   END
   ELSE IF nombre3=NRNUM THEN
   BEGIN
    gbase(profact,-1);
    writeln('ldnl ',arbol[arbol[nodo].izq].esp:0);
    tipoesp:=2
   END
   ELSE gencodigo(arbol[nodo].izq);
   posicion:=arbol[arbol[nodo].der].esp;
{ Busca si es una asignación para resultado de función }
   IF paso[posicion]=2 THEN
   BEGIN
    IF tipoesp=6 THEN yoptimiza(2,abs(val[posicion]))
    ELSE              yoptimiza(3,abs(val[posicion]))
   END
   ELSE
   BEGIN
    IF tipoesp=6 THEN
    BEGIN
     obtdir(posicion);
     writeln('fpstnlsn')
    END
    ELSE IF tipoesp=4 THEN
    BEGIN
     obtdir(posicion);
     writeln('sb')
    END
    ELSE guavar(posicion);
   END
  END
  ELSE
  BEGIN
   optimizado:=FALSE;
   IF (arbol[arbol[nodo].der].reg<3) AND
      (arbol[arbol[nodo].der].regf<3) THEN
   BEGIN
    nombre3:=arbol[arbol[nodo].izq].nom;
         IF nombre3=NRCERO THEN
    BEGIN
     writeln('ldc 0');
     tipoesp:=2
    END
    ELSE IF nombre3=NRCARGA THEN
    BEGIN
     arbol[arbol[nodo].izq].nom:=NICARGA;
     gencodigo(arbol[nodo].izq);
     tipoesp:=2
    END
    ELSE IF nombre3=NRNUM THEN
    BEGIN
     gbase(profact,-1);
     writeln('ldnl ',arbol[arbol[nodo].izq].esp:0);
     tipoesp:=2
    END
    ELSE gencodigo(arbol[nodo].izq);
    nombre3:=arbol[arbol[nodo].der].nom;
    IF (nombre3=NIFRANJA) AND (tipoesp=2) THEN
    BEGIN
     optimizado:=TRUE;
     sumafranjas:=totaldes(arbol[nodo].der);
    END
    ELSE gencodigo(arbol[nodo].der);
   END
   ELSE
   BEGIN
    nombre3:=arbol[arbol[nodo].izq].nom;
    IF (nombre3=NRCERO) OR
       (nombre3=NRCARGA) OR
       (nombre3=NRNUM) THEN tipoesp:=2;
    nombre3:=arbol[arbol[nodo].der].nom;
    IF (nombre3=NIFRANJA) AND (tipoesp=2) THEN
    BEGIN
     optimizado:=TRUE;
     sumafranjas:=totaldes(arbol[nodo].der);
    END
    ELSE gencodigo(arbol[nodo].der);
    createmp(dirtemp);
    IF vdes+dirtemp<0 THEN yoptimiza(0,0);
    writeln('stl ',vdes+dirtemp:0);
    nombre3:=arbol[arbol[nodo].izq].nom;
         IF nombre3=NRCERO THEN writeln('ldc 0')
    ELSE IF nombre3=NRCARGA THEN
    BEGIN
     arbol[arbol[nodo].izq].nom:=NICARGA;
     gencodigo(arbol[nodo].izq)
    END
    ELSE IF nombre3=NRNUM THEN
    BEGIN
     gbase(profact,-1);
     writeln('ldnl ',arbol[arbol[nodo].izq].esp:0)
    END
    ELSE gencodigo(arbol[nodo].izq);
    IF vdes+dirtemp<0 THEN yoptimiza(0,0);
    writeln('ldl ',vdes+dirtemp:0);
    borratemp;
   END;
   IF (tipoesp=2) OR (tipoesp=4) OR (tipoesp=6) OR
      (clase[tipoesp]='D') OR (clase[tipoesp]='E') THEN
   BEGIN
         IF tipoesp=6  THEN writeln('fpstnlsn')
    ELSE IF tipoesp=4  THEN writeln('sb')
    ELSE IF optimizado THEN writeln('stnl ',sumafranjas:0)
    ELSE                    writeln('stnl 0');
   END
   ELSE
   BEGIN
    IF clase[tipoesp]='N' THEN
    BEGIN
     writeln('ldc ',num[tipoesp]:0);
     yoptimiza(0,0);
     writeln('call ASIGNASTR');
     apret:=apret+64
    END
    ELSE
    BEGIN
     writeln('ldc ',num[tipoesp]:0);
     writeln('move')
    END;
   END;
  END;
 END
 ELSE IF nombre=NIFRANJA THEN   { Desplazamiento de franja }
 BEGIN
  sumafranjas:=totaldes(nodo);
  IF sumafranjas>0 THEN writeln('ldnlp ',sumafranjas:0)
 END
 ELSE
 BEGIN
  IF arbol[nodo].der=0 THEN     { Un nodo con un sólo descendiente }
  BEGIN
   optimizado:=FALSE;
   IF nombre=NICARGA THEN       { Cargas de variables }
   BEGIN
    nombre2:=arbol[arbol[nodo].izq].nom;
    IF nombre2=NDIRVAR THEN
    BEGIN
     optimizado:=TRUE;
     leevar(arbol[arbol[nodo].izq].esp)
    END
    ELSE IF nombre2=NIFRANJA THEN
    BEGIN
     optimizado:=TRUE;
     sumafranjas:=totaldes(arbol[nodo].izq);
     writeln('ldnl ',sumafranjas:0)
    END
   END
   ELSE IF nombre=NRCARGA THEN { Cargas de variables reales }
   BEGIN
    nombre2:=arbol[arbol[nodo].izq].nom;
    IF nombre2=NINDEXAPAL THEN    { Con indexado }
    BEGIN
     arbol[arbol[nodo].izq].nom:=NRCARGAI;
     optimizado:=TRUE;
     gencodigo(arbol[nodo].izq)
    END
   END
   ELSE IF nombre=NINTAREAL THEN  { Conversión entero a real }
   BEGIN
    nombre2:=arbol[arbol[nodo].izq].nom;
         IF nombre2=NICARGA THEN { Optimiza cargas }
    BEGIN
     optimizado:=TRUE;
     gencodigo(arbol[arbol[nodo].izq].izq);
     writeln('fpi32tor32');
    END
    ELSE IF nombre2=NROUND THEN { Optimiza con redondeo }
    BEGIN
     optimizado:=TRUE;
     gencodigo(arbol[arbol[nodo].izq].izq);
     writeln('fpint')
    END
    ELSE IF nombre2=NTRUNC THEN { Optimiza con truncado }
    BEGIN
     optimizado:=TRUE;
     gencodigo(arbol[arbol[nodo].izq].izq);
     writeln('fpurz');
     writeln('fpint')
    END
   END;
   IF NOT optimizado THEN
   BEGIN
    IF arbol[nodo].izq<>0 THEN gencodigo(arbol[nodo].izq);
    IF nombre=NINTAREAL THEN
    BEGIN
     createmp(dirtemp);
     IF vdes+dirtemp<0 THEN yoptimiza(0,0);
     writeln('stl ',vdes+dirtemp:0);
     writeln('ldlp ',vdes+dirtemp:0);
     writeln('fpi32tor32');
     borratemp;
    END
    ELSE IF nombre=NCALL THEN writeln('call LLAMADA')
    ELSE IF nombre=NPOR2 THEN writeln('fpumulby2')
    ELSE IF nombre=NDIV2 THEN writeln('fpudivby2')
    ELSE IF nombre=NICARGACHAR THEN writeln('lb')
    ELSE IF nombre=NRCARGA THEN writeln('fpldnlsn')
    ELSE IF nombre=NICARGA THEN writeln('ldnl 0')
    ELSE IF nombre=NABS THEN writeln('call ABS')
    ELSE IF nombre=NABSR THEN writeln('fpuabs')
    ELSE IF nombre=NSQR THEN
    BEGIN
     writeln('dup');
     writeln('mul')
    END
    ELSE IF nombre=NSQRR THEN
    BEGIN
     writeln('fpdup');
     writeln('fpmul')
    END
    ELSE IF (nombre=NROUND) OR (nombre=NTRUNC) THEN
    BEGIN
     IF nombre=NROUND THEN writeln('fpint');
     createmp(dirtemp);
     IF vdes+dirtemp<0 THEN yoptimiza(0,0);
     writeln('ldlp ',vdes+dirtemp:0);
     writeln('fpstnli32');
     writeln('ldl ',vdes+dirtemp:0);
     borratemp;
    END
    ELSE IF nombre=NNOTB THEN writeln('eqc 0')
    ELSE IF nombre=NNOT THEN writeln('not')
    ELSE IF nombre=NNEG THEN
    BEGIN
     writeln('not');
     writeln('adc 1')
    END
    ELSE IF nombre=NSQRT THEN
    BEGIN
     writeln('fpusqrtfirst');
     writeln('fpusqrtstep');
     writeln('fpusqrtstep');
     writeln('fpusqrtlast')
    END
    ELSE IF nombre=NSCARGA THEN
    BEGIN
     yoptimiza(0,0);
     writeln('call CARGASET');
     apret:=apret-8
    END
    ELSE IF nombre=NSGENBIT THEN
    BEGIN
     yoptimiza(0,0);
     writeln('call GENBIT')
    END
    ELSE IF nombre=NSALGO THEN
    BEGIN
     yoptimiza(0,0);
     writeln('call ALGOENSET');
     apret:=apret+8
    END
    ELSE IF nombre=NSDIR THEN
    BEGIN
     yoptimiza(0,0);
     writeln('ldlp 0');
     gpila(8)
    END
    ELSE IF nombre=NCLIMPIAA THEN
    BEGIN
     yoptimiza(0,0);
     writeln('call LIMPIASTRA');
     apret:=apret-64
    END
    ELSE IF nombre=NCCARGA THEN
    BEGIN
     yoptimiza(0,0);
     writeln('call CARGASTR');
     apret:=apret-64
    END
    ELSE IF nombre=NCDIR THEN
    BEGIN
     yoptimiza(0,0);
     writeln('ldlp 0');
     gpila(64)
    END
    ELSE IF nombre=NARRAYACAD THEN
    BEGIN
     yoptimiza(0,0);
     writeln('ldc ',arbol[nodo].esp:0);
     writeln('call ARRAYASTR');
     apret:=apret-64
    END
    ELSE IF nombre=NNULO THEN        { No hace nada }
    ELSE                             { Rutinas trigonometricas }
    BEGIN
     gbase(profact,-1);
     writeln('ldnlp -NUMCONSLIB');
     CASE nombre OF
      NCOS   : writeln('call COS');
      NSIN   : writeln('call SIN');
      NARCTAN: writeln('call ARCTAN');
      NEXP   : writeln('call EXP');
      NLN    : writeln('call LN')
          ELSE error(255)
     END;
    END;
   END;
  END
  ELSE
  BEGIN
        IF NOT espuntoflotante(nodo) AND
           (arbol[arbol[nodo].der].reg>arbol[arbol[nodo].izq].reg) AND
           (arbol[arbol[nodo].izq].reg<3) THEN gender(nodo)
   ELSE IF NOT espuntoflotante(nodo) AND
           (arbol[arbol[nodo].izq].reg>=arbol[arbol[nodo].der].reg) AND
           (arbol[arbol[nodo].der].reg<3) THEN genizq(nodo)
   ELSE IF espuntoflotante(nodo) AND
           (arbol[arbol[nodo].der].regf>arbol[arbol[nodo].izq].regf) AND
           (arbol[arbol[nodo].izq].regf<3) THEN gender(nodo)
   ELSE IF espuntoflotante(nodo) AND
           (arbol[arbol[nodo].izq].regf>=arbol[arbol[nodo].der].regf) AND
           (arbol[arbol[nodo].der].regf<3) THEN genizq(nodo)
   ELSE
   BEGIN
    optimizado:=FALSE;     { Requiere variables temporales }
    gencodigo(arbol[nodo].der);
    createmp(dirtemp);     { Ahora averigua el tipo de operandos }
    IF espuntoflotante(nodo) THEN
    BEGIN
     IF vdes+dirtemp<0 THEN yoptimiza(0,0);
     writeln('ldlp ',vdes+dirtemp:0);
     writeln('fpstnlsn');
     gencodigo(arbol[nodo].izq);
     IF vdes+dirtemp<0 THEN yoptimiza(0,0);
     writeln('ldlp ',vdes+dirtemp:0);
     optimizado:=TRUE;
          IF nombre=NRMUL THEN writeln('fpldnlmulsn')
     ELSE IF nombre=NRSUMA THEN writeln('fpldnladdsn')
     ELSE
     BEGIN
      optimizado:=FALSE;
      writeln('fpldnlsn')
     END;
    END
    ELSE
    BEGIN
     IF vdes+dirtemp<0 THEN yoptimiza(0,0);
     writeln('stl ',vdes+dirtemp:0);
     gencodigo(arbol[nodo].izq);
     IF vdes+dirtemp<0 THEN yoptimiza(0,0);
     writeln('ldl ',vdes+dirtemp:0);
    END;
    borratemp;
    IF optimizado=FALSE THEN calcula(nodo,0)
   END;
  END;
 END;
END;

{ Algoritmo de etiquetación de nodos (recursivo) }
PROCEDURE etiqueta(nodo:INTEGER);
VAR
 nombre,nombre2:narbol;
 min,max,minf,maxf:INTEGER;
 contador,nodofunc:INTEGER;
 optimizado:BOOLEAN;
BEGIN
 yoptimiza(0,1);   { Guarda cuálquier asignación de resultado de función }
 nombre:=arbol[nodo].nom;
{ Las funciones invariablemente ocupan los 3 registros. }
 IF nombre=NFUNCION THEN
 BEGIN
  arbol[nodo].reg:=3;
  arbol[nodo].regf:=3;
  nodofunc:=nodo;
  FOR contador:=1 TO num[arbol[nodo].esp] DO
  BEGIN
   nodofunc:=arbol[nodofunc].der;  { Etiqueta las expresiones de los }
   etiqueta(nodofunc-1)            { argumentos. }
  END;
 END
 ELSE IF (nombre=NEOLN) OR (nombre=NEOF) OR (nombre=NSLIMPIA) THEN
 BEGIN
  arbol[nodo].reg:=3;
  arbol[nodo].regf:=3
 END
{ Todos los demás terminales ocupan sólo un registro }
 ELSE IF (arbol[nodo].izq=0) AND (arbol[nodo].der=0) THEN
 BEGIN
  IF (nombre=NRCERO) OR (nombre=NRNUM) THEN arbol[nodo].regf:=1;
  IF nombre<>NRCERO THEN arbol[nodo].reg:=1
 END
 ELSE
 BEGIN
  optimizado:=FALSE;
{ Optimiza las sumas, restas y comparaciones de enteros }
{ con constantes, aquí por el lado derecho }
  IF (nombre=NISUMA) OR (nombre=NIRESTA) OR
     (nombre=NIIGUAL) OR (nombre=NINOIGUAL) THEN
  BEGIN
   nombre2:=arbol[arbol[nodo].der].nom;
   IF nombre2=NINUM THEN
   BEGIN
    etiqueta(arbol[nodo].izq);
    IF nombre=NIRESTA THEN
     arbol[arbol[nodo].der].esp:=-arbol[arbol[nodo].der].esp;
    arbol[arbol[nodo].der].reg:=0;
    optimizado:=TRUE
   END
{ Aquí por el lado izquierdo }
   ELSE IF nombre<>NIRESTA THEN
   BEGIN
    nombre2:=arbol[arbol[nodo].izq].nom;
    IF nombre2=NINUM THEN
    BEGIN
     etiqueta(arbol[nodo].der);
     arbol[arbol[nodo].izq].reg:=0;
     optimizado:=TRUE
    END;
   END;
  END;
{ La suma de dirección de franjas no ocupa ningun registro aparte, }
{ ya que se usa un ldnlp simple }
  IF nombre=NIFRANJA THEN
  BEGIN
   optimizado:=TRUE;
   etiqueta(arbol[nodo].izq);
   arbol[arbol[nodo].der].reg:=0;
  END;
  IF NOT optimizado THEN
  BEGIN
   IF arbol[nodo].izq<>0 THEN etiqueta(arbol[nodo].izq);
   IF arbol[nodo].der<>0 THEN etiqueta(arbol[nodo].der);
  END;
{ Saca el número de registros empleados por el nodo }
  IF arbol[nodo].der<>0 THEN
  BEGIN
   max:=arbol[arbol[nodo].izq].reg;
   min:=arbol[arbol[nodo].der].reg;
   IF (min=0) AND (max=0) THEN max:=0
   ELSE IF min>max THEN max:=min
   ELSE IF min=max THEN max:=max+1;
   maxf:=arbol[arbol[nodo].izq].regf;
   minf:=arbol[arbol[nodo].der].regf;
   IF (minf=0) AND (maxf=0) THEN maxf:=0
   ELSE IF minf>maxf THEN maxf:=minf
   ELSE IF minf=maxf THEN maxf:=maxf+1
  END
  ELSE
  BEGIN
   max:=arbol[arbol[nodo].izq].reg;
   maxf:=arbol[arbol[nodo].izq].regf
  END;
  IF (nombre=NSGENBIT) OR (nombre=NSGENBITS) OR
     (nombre=NCOPY) OR (nombre=NDELETE) OR (nombre=NINSERT) THEN
   etiqueta(arbol[nodo].esp);
{ Las funciones SQR, y SQRR duplican el registro para luego }
{ multiplicarlo por si mismo, por eso ocupan un registro más. }
{ Las funciones de cadenas requieren un registro más para llevar el }
{ tamaño de la cadena }
  IF nombre=NSQR THEN max:=max+1
  ELSE IF (nombre=NSQRR) OR (nombre=NINTAREAL) OR
          (nombre=NRCARGA) OR (nombre=NRCARGAI) THEN maxf:=maxf+1
  ELSE IF nombre=NSQRT THEN maxf:=3
  ELSE IF (nombre=NAIGUAL) OR (nombre=NANOIGUAL) OR
          (nombre=NAMENOR) OR (nombre=NAMENORIGUAL) OR
          (nombre=NAMAYOR) OR (nombre=NAMAYORIGUAL) THEN
  BEGIN
   maxf:=3;
   max:=max+1
  END
  ELSE
{ Las otras funciones interconstruidas requieren todos los registros }
  IF (nombre=NARCTAN) OR (nombre=NCOS) OR
     (nombre=NSIN) OR (nombre=NEXP) OR
     (nombre=NLN) OR (nombre=NSGENBIT) OR
     (nombre=NSGENBITS) OR (nombre=NSCARGA) OR
     (nombre=NRANDOM) OR (nombre=NCLIMPIA) OR
     (nombre=NCLIMPIAA) OR (nombre=NCCARGA) THEN
  BEGIN
   max:=3;
   maxf:=3
  END
  ELSE IF (nombre=NCOPY) OR (nombre=NINSERT) OR
          (nombre=NDELETE) OR (nombre=NARRAYACAD) OR
          (nombre=NCIGUAL) OR (nombre=NCNOIGUAL) OR
          (nombre=NCMENOR) OR (nombre=NCMAYORIGUAL) OR
          (nombre=NCMAYOR) OR (nombre=NCMENORIGUAL) THEN
  BEGIN
   max:=3;
   maxf:=3
  END;
{ Guarda el número de registros ocupados }
  arbol[nodo].reg:=max;
  arbol[nodo].regf:=maxf;
 END;
END;

PROCEDURE optimacion(nodo:INTEGER); FORWARD;

{ Optima cada uno de los parametros de la función }
PROCEDURE optimafunc(nodo:INTEGER);
VAR contador:INTEGER;
BEGIN
 FOR contador:=1 TO num[arbol[nodo].esp] DO
 BEGIN
  nodo:=arbol[nodo].der;
  optimacion(nodo-1);
 END
END;

{ Optimación del arbol }
PROCEDURE optimacion;
VAR
 nombre:narbol;
 optimado:BOOLEAN;
 val1,val2:INTEGER;
BEGIN
 nombre:=arbol[nodo].nom;   { Primero nodos terminales }
 IF nombre=NINDEXABYTE THEN
 BEGIN
  nombre:=NISUMA;
  arbol[nodo].nom:=NISUMA
 END;
 IF (nombre=NSGENBIT) OR (nombre=NSGENBITS) OR
    (nombre=NCOPY) OR (nombre=NDELETE) THEN
  optimacion(arbol[nodo].esp);
      IF nombre=NFUNCION THEN optimafunc(nodo)
 ELSE IF (nombre=NSUNION) OR (nombre=NSDIFER) OR
         (nombre=NSINTER) OR (nombre=NSCOMPA) OR
         (nombre=NCONCAT) OR (nombre=NPOS) OR
         (nombre=NASIGNA) THEN
 BEGIN
  optimacion(arbol[nodo].izq);
  optimacion(arbol[nodo].der)
 END
 ELSE IF nombre=NINSERT THEN
 BEGIN
  optimacion(arbol[nodo].izq);
  optimacion(arbol[nodo].der);
  optimacion(arbol[nodo].esp)
 END
 ELSE IF (arbol[nodo].izq=0) AND (arbol[nodo].der=0) THEN
 BEGIN
 END
 ELSE
 BEGIN
  IF arbol[nodo].der=0 THEN
  BEGIN
   optimacion(arbol[nodo].izq)
  END
  ELSE
  BEGIN
   optimacion(arbol[nodo].izq);
   optimacion(arbol[nodo].der);
   IF (arbol[arbol[nodo].izq].nom=NINUM) AND
      (arbol[arbol[nodo].der].nom=NINUM) THEN
   BEGIN
    val1:=arbol[arbol[nodo].izq].esp;
    val2:=arbol[arbol[nodo].der].esp;
    optimado:=TRUE;
    CASE nombre OF
     NISUMA: val1:=val1+val2;
     NIRESTA: val1:=val1-val2;
     NIMUL,NMULRAPIDA: val1:=val1*val2;
     NIDIV: val1:=val1 DIV val2;
     NIMOD: val1:=val1 MOD val2;
     NSHL: val1:=val1 SHL val2;
     NSHR: val1:=val1 SHR val2;
     NIFRANJA: val1:=val1+val2*4;
     NINDEXAPAL: val1:=val1*4+val2
     ELSE optimado:=FALSE
    END;
    IF optimado THEN
    BEGIN
     arbol[nodo].nom:=NINUM;
     arbol[nodo].izq:=0;
     arbol[nodo].der:=0;
     arbol[nodo].esp:=val1
    END;
   END
   ELSE IF arbol[arbol[nodo].der].nom=NINUM THEN
   BEGIN
    val1:=arbol[arbol[nodo].der].esp;
    CASE nombre OF
     NISUMA:
      IF val1=0 THEN arbol[nodo]:=arbol[arbol[nodo].izq]
      ELSE IF (val1 MOD 4)=0 THEN
      BEGIN
       arbol[nodo].nom:=NIFRANJA;
       arbol[arbol[nodo].der].esp:=arbol[arbol[nodo].der].esp DIV 4
      END;
     NIRESTA:
      IF val1=0 THEN arbol[nodo]:=arbol[arbol[nodo].izq];
     NIMUL,NMULRAPIDA:
      IF val1=1 THEN arbol[nodo]:=arbol[arbol[nodo].izq]
      ELSE IF val1=0 THEN
      BEGIN
       arbol[nodo].nom:=NINUM;
       arbol[nodo].izq:=0;
       arbol[nodo].der:=0;
       arbol[nodo].esp:=0
      END
    END
   END
   ELSE IF arbol[arbol[nodo].izq].nom=NINUM THEN
   BEGIN
    val1:=arbol[arbol[nodo].izq].esp;
    CASE nombre OF
     NISUMA:
      IF val1=0 THEN arbol[nodo]:=arbol[arbol[nodo].der]
      ELSE IF (val1 MOD 4)=0 THEN
      BEGIN
       arbol[nodo].nom:=NIFRANJA;
       val2:=arbol[nodo].izq;
       arbol[nodo].izq:=arbol[nodo].der;
       arbol[nodo].der:=val2;
       arbol[arbol[nodo].der].esp:=arbol[arbol[nodo].der].esp DIV 4
      END;
     NIMUL,NMULRAPIDA:
      IF val1=1 THEN arbol[nodo]:=arbol[arbol[nodo].der]
      ELSE IF val1=0 THEN
      BEGIN
       arbol[nodo].nom:=NINUM;
       arbol[nodo].izq:=0;
       arbol[nodo].der:=0;
       arbol[nodo].esp:=0
      END;
     NINDEXAPAL:
      BEGIN
       arbol[nodo].nom:=NIFRANJA;
       val2:=arbol[nodo].izq;
       arbol[nodo].izq:=arbol[nodo].der;
       arbol[nodo].der:=val2
      END
    END
   END
  END
 END
END;

PROCEDURE cortocircuito(nodo,sand,sor:INTEGER);
VAR
 etiq,nodoo:INTEGER;
 nueva,opt:BOOLEAN;
BEGIN
 nueva:=FALSE;
 IF arbol[nodo].nom=NAND THEN
 BEGIN
  IF arbol[arbol[nodo].izq].nom=NAND THEN
   cortocircuito(arbol[nodo].izq,sand,sor)
  ELSE IF arbol[arbol[nodo].izq].nom=NOR THEN
  BEGIN
   etiq:=geneti;
   nueva:=TRUE;
   cortocircuito(arbol[nodo].izq,sand,etiq)
  END
  ELSE
  BEGIN
   IF arbol[arbol[nodo].izq].nom=NINOIGUAL THEN
    arbol[arbol[nodo].izq].nom:=NIIGUALESP;
   gencodigo(arbol[nodo].izq)
  END;
  gcj(sand);
  IF nueva THEN geti(etiq);
  IF arbol[arbol[nodo].der].nom=NAND THEN
   cortocircuito(arbol[nodo].der,sand,sor)
  ELSE IF arbol[arbol[nodo].der].nom=NOR THEN
   cortocircuito(arbol[nodo].der,sand,sor)
  ELSE
  BEGIN
   IF arbol[arbol[nodo].der].nom=NINOIGUAL THEN
    arbol[arbol[nodo].der].nom:=NIIGUALESP;
   gencodigo(arbol[nodo].der)
  END
 END
 ELSE IF arbol[nodo].nom=NOR THEN
 BEGIN
  opt:=FALSE;
  IF arbol[arbol[nodo].izq].nom=NAND THEN
  BEGIN
   etiq:=geneti;
   nueva:=TRUE;
   cortocircuito(arbol[nodo].izq,etiq,sor)
  END
  ELSE IF arbol[arbol[nodo].izq].nom=NOR THEN
   cortocircuito(arbol[nodo].izq,sand,sor)
  ELSE
  BEGIN
   opt:=TRUE;
   nodoo:=arbol[nodo].izq;
   CASE arbol[nodoo].nom OF
    NNOTB: nodoo:=arbol[nodoo].izq;
    NIIGUAL: arbol[nodoo].nom:=NIIGUALESP;
    NINOIGUAL: arbol[nodoo].nom:=NIIGUAL;
    NRNOIGUAL: arbol[nodoo].nom:=NRIGUAL;
    NRMENORIGUAL: arbol[nodoo].nom:=NRMAYOR;
    NRMAYORIGUAL: arbol[nodoo].nom:=NRMENOR;
    NIMENORIGUAL: arbol[nodoo].nom:=NIMAYOR;
    NIMAYORIGUAL: arbol[nodoo].nom:=NIMENOR
   ELSE
    opt:=FALSE
   END;
   gencodigo(nodoo)
  END;
  IF NOT opt THEN writeln('eqc 0');
  gcj(sor);
  IF nueva THEN geti(etiq);
  IF arbol[arbol[nodo].der].nom=NAND THEN
   cortocircuito(arbol[nodo].der,sand,sor)
  ELSE IF arbol[arbol[nodo].der].nom=NOR THEN
   cortocircuito(arbol[nodo].der,sand,sor)
  ELSE
  BEGIN
   IF arbol[arbol[nodo].der].nom=NINOIGUAL THEN
    arbol[arbol[nodo].der].nom:=NIIGUALESP;
   gencodigo(arbol[nodo].der)
  END
 END
END;

PROCEDURE codigo;
BEGIN
 optimacion(nodoact); { Optima el arbol }
 etiqueta(nodoact);   { Etiqueta el arbol }
 IF (((arbol[nodoact].nom=NAND) AND (arbol[nodoact].esp=0)) OR
     ((arbol[nodoact].nom=NOR) AND (arbol[nodoact].esp=0))) AND
    optcc AND escontrol THEN cortocircuito(nodoact,etiqand,etiqor)
 ELSE gencodigo(nodoact)   { Genera el codigo para el arbol }
END;


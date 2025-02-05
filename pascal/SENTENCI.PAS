{ Programa: Compilador de Pascal para el Inmos T805 }
{                                                   }
{ Módulo  : Analizador de Sentencias                }
{                                                   }
{ (c) Copyright 1993-1994 Oscar Toledo G.           }
{          Todos los derechos reservados.           }

PROCEDURE sentencia; FORWARD;

PROCEDURE snum;   { Procesa una etiqueta }
BEGIN
 busca;           { Busca redefiniciones }
 IF possim<>0 THEN
 BEGIN
  IF NOT sevalegoto THEN error(51);
  IF prof[possim]=profact THEN  { Verifica que este en el lugar correcto }
  BEGIN
   geti(val[possim]);
   compsig(CDOSPUNTOS,5);
   obtlex
  END
  ELSE error(1);
 END;
END;

PROCEDURE sbegin;
BEGIN
 REPEAT
  obtlex;
  sentencia
 UNTIL lexema<>CPUNTOYCOMA;
 compara(CEND,17);
 obtlex
END;

PROCEDURE sgoto;
BEGIN
 yoptimiza(0,0);
 copiaenlace;
 compsig(CNUM,2);
 busca;
 IF possim=0 THEN error(1);           { Etiqueta indefinida }
 IF clase[possim]<>'L' THEN error(1); { No es una etiqueta }
 gbase(profact,prof[possim]);         { Obtiene el nivel correcto }
 writeln('gajw');
 gj(val[possim]);
 obtlex
END;

PROCEDURE srepeat;
VAR etiqueta:INTEGER;
BEGIN
 etiqueta:=geneti;
 geti(etiqueta);             { Etiqueta del bucle }
 REPEAT
  obtlex;
  sentencia
 UNTIL lexema<>CPUNTOYCOMA;
 compara(CUNTIL,10);         { Condición }
 obtlex;
 IF optcc THEN
 BEGIN
   escontrol:=TRUE;
   etiqand:=etiqueta;
   etiqor:=geneti
 END;
 expresion;
 IF tipoi<>8 THEN error(14); { Debe ser BOOLEAN }
 gcj(etiqueta);
 IF optcc THEN
 BEGIN
   geti(etiqor);
   escontrol:=FALSE
 END
END;

PROCEDURE sif;
VAR etiqueta1,etiqueta2:INTEGER;
BEGIN
 obtlex;
 etiqueta1:=geneti;
 IF optcc THEN
 BEGIN
   escontrol:=TRUE;
   etiqand:=etiqueta1;
   etiqor:=geneti
 END;
 expresion;                  { Expresión }
 IF tipoi<>8 THEN error(14); { Debe ser BOOLEAN }
 compara(CTHEN,16);
 obtlex;
 gcj(etiqueta1);
 IF optcc THEN
 BEGIN
  geti(etiqor);
  escontrol:=FALSE
 END;
 sentencia;
 IF lexema<>CELSE THEN geti(etiqueta1)
 ELSE
 BEGIN
  etiqueta2:=geneti;
  gj(etiqueta2);
  geti(etiqueta1);
  obtlex;
  sentencia;
  geti(etiqueta2)
 END;
END;

PROCEDURE swhile;
VAR etiqueta1,etiqueta2:INTEGER;
BEGIN
 etiqueta1:=geneti;
 etiqueta2:=geneti;
 geti(etiqueta1);            { Etiqueta del bucle }
 obtlex;
 IF optcc THEN
 BEGIN
   escontrol:=TRUE;
   etiqand:=etiqueta2;
   etiqor:=geneti
 END;
 expresion;                  { Expresión de control }
 IF tipoi<>8 THEN error(14); { Debe ser BOOLEAN }
 gcj(etiqueta2);             { Salta si es 0 }
 IF optcc THEN
 BEGIN
   geti(etiqor);
   escontrol:=FALSE
 END;
 compara(CDO,18);
 obtlex;
 sentencia;
 gj(etiqueta1);              { Hace el bucle }
 geti(etiqueta2)             { Etiqueta de salida }
END;

PROCEDURE scase;
VAR
 tipocase,proxcase:INTEGER;
 etsalida,etvarios:INTEGER;
 varios,coma:BOOLEAN;
 fincase,numcase:INTEGER;
 dirtemp:INTEGER;
 antiguo:BOOLEAN;
BEGIN
 antiguo:=sevalegoto;
 sevalegoto:=FALSE;
 obtlex;
 expresion;
 IF (clase[tipoi]='B') OR    { Sólo con subrangos, o enumerados }
    (clase[tipoi]='E') THEN
 BEGIN
  tipocase:=tipoi;
  createmp(dirtemp);
  IF vdes+dirtemp<0 THEN yoptimiza(0,0);
  writeln('stl ',vdes+dirtemp:0);  { Guarda la expresión }
  compara(COF,26);
  etsalida:=geneti;
  fincase:=0;
  REPEAT
   varios:=FALSE;
   coma:=TRUE;
   REPEAT
    obtlex;
    const1;   { Verifica que lo que sigue al case sea una constante }
    IF tipoi<>tipocase THEN error(14);  { El tipo debe ser el mismo }
    IF vdes+dirtemp<0 THEN yoptimiza(0,0);
    writeln('ldl ',vdes+dirtemp:0);     { Toma la expresión }
    numcase:=numero;
    obtlex;
    IF lexema=CDOSPUNTOS THEN coma:=FALSE  { Fin de casos }
    ELSE
    BEGIN
     compara(CCOMA,5);
     IF NOT varios THEN etvarios:=geneti;
     writeln('ldc ',numcase:0);  { Número }
     writeln('diff');  { diff es más rapido que eqc n; eqc 0 }
     gcj(etvarios);
     varios:=TRUE;
    END
   UNTIL NOT coma;
   proxcase:=geneti;
   writeln('eqc ',numcase:0);
   gcj(proxcase);   { Salta si no es ninguno }
   IF varios THEN geti(etvarios); { Etiqueta para varios }
   obtlex;
   sentencia;
        IF lexema= CELSE       THEN fincase:=1 { imitando Modula-2 }
   ELSE IF lexema<>CPUNTOYCOMA THEN fincase:=2
   ELSE
   BEGIN
    gj(etsalida);
    geti(proxcase)
   END
  UNTIL fincase<>0;
  IF fincase=1 THEN
  BEGIN
   gj(etsalida);
   geti(proxcase);
   obtlex;
   sentencia
  END
  ELSE geti(proxcase);
  compara(CEND,17);
  geti(etsalida);
  obtlex
 END
 ELSE error(14);
 borratemp;
 sevalegoto:=antiguo
END;

PROCEDURE sfor;
VAR
 incremento:INTEGER;
 tipofor:INTEGER;
 vtemp1,vtemp2:INTEGER;
 texpr1,texpr2:INTEGER;
 etiqbucle:INTEGER;
 etiqsalida:INTEGER;
 antiguo:BOOLEAN;
BEGIN
 antiguo:=sevalegoto;
 sevalegoto:=FALSE;
 etiqbucle:=geneti;
 etiqsalida:=geneti;
 createmp(texpr1);
 createmp(texpr2);
 obtlex;
 nodoact:=0;
 apuarb:=apulit;
 nodvar;
 vtemp1:=nodoact;
 vtemp2:=apuarb;
 tipofor:=tipobase(tipoi);
 IF (tipofor<>2) AND (tipofor<>4) AND
    (clase[tipofor]<>'E') THEN error(14); { Debe ser un tipo valido }
 compara(CASIGNA,13);
 obtlex;
 expr2;                           { Expresión de comienzo }
 codigo;
 IF vdes+texpr1<0 THEN yoptimiza(0,0);
 writeln('stl ',vdes+texpr1:0);
 IF NOT comtipos(tipofor,tipoi) THEN error(14); { Los tipos d/ser iguales }
      IF lexema=CTO     THEN incremento:= 1     { Verifica el incremento }
 ELSE IF lexema=CDOWNTO THEN incremento:=-1
 ELSE error(28);
 obtlex;
 nodoact:=vtemp1;
 apuarb:=vtemp2;
 expr2;
 codigo;
 IF NOT comtipos(tipofor,tipoi) THEN error(14);
 IF vdes+texpr2<0 THEN yoptimiza(0,0);
 writeln('stl ',vdes+texpr2:0);   { Expresión final }
 compara(CDO,18);
 obtlex;
 IF incremento=1 THEN
 BEGIN
  writeln('ldl ',vdes+texpr1:0);
  writeln('ldl ',vdes+texpr2:0)
 END
 ELSE
 BEGIN
  writeln('ldl ',vdes+texpr2:0);
  writeln('ldl ',vdes+texpr1:0)
 END;
 writeln('gt');
 writeln('eqc 0');
 gcj(etiqsalida);
 geti(etiqbucle);
 nodoact:=vtemp1;
 apuarb:=vtemp2;
 creanodo(NCARGAFOR,0,0,texpr1);
 creanodo(NASIGNA,nodoact,nodoact-1,tipofor);
 codigo;
 sentencia;
 writeln('ldl ',vdes+texpr1:0);
 writeln('ldl ',vdes+texpr2:0);
 writeln('diff');
 writeln('eqc 0');
 writeln('ldl ',vdes+texpr1:0);
 writeln('adc ',incremento:0);
 writeln('stl ',vdes+texpr1:0);
 gcj(etiqbucle);                 { Continua el bucle }
 geti(etiqsalida);
 borratemp;
 borratemp;
 sevalegoto:=antiguo
END;

PROCEDURE fwrite(par:INTEGER);
VAR tipowrite,nodoizq,nododer,formateo:INTEGER;
BEGIN
 obtlex;
 IF lexema=CPARENI THEN
 BEGIN
  REPEAT
   obtlex;
   nodoact:=0;
   apuarb:=apulit;
   expr2;
   tipoi:=tipobase(tipoi);
   IF (tipoi<>2) AND (tipoi<>4) AND (tipoi<>6) AND (tipoi<>8) AND
      (clase[tipoi]<>'A') THEN error(14);
   tipowrite:=tipoi;
   nodoizq:=nodoact;
   IF lexema=CDOSPUNTOS THEN
   BEGIN
    obtlex;
    expr2;
    IF tipoi<>2 THEN error(14);
    nododer:=nodoact;
   END
   ELSE
   BEGIN
         IF tipowrite=2 THEN formateo:=12
    ELSE IF tipowrite=4 THEN formateo:=1
    ELSE IF tipowrite=6 THEN formateo:=12
    ELSE IF tipowrite=8 THEN formateo:=0
    ELSE                     formateo:=num[tipowrite];
    creanodo(NINUM,0,0,formateo);
    nododer:=nodoact;
   END;
   creanodo(NNULO,nodoizq,nododer,0);
   codigo;
        IF tipowrite=2 THEN writeln('call E0')
   ELSE IF tipowrite=4 THEN writeln('call E1')
   ELSE IF tipowrite=6 THEN writeln('call E2')
   ELSE IF tipowrite=8 THEN writeln('call E3')
   ELSE writeln('call E4');
  UNTIL lexema<>CCOMA;
  compara(CPAREND,22);
  obtlex;
 END
 ELSE IF par>0 THEN error(35);
END;

PROCEDURE fwriteln;
BEGIN
 fwrite(0);
 writeln('call E5')
END;

PROCEDURE fread(par:INTEGER);
VAR valtemp:INTEGER;
BEGIN
 obtlex;
 IF lexema=CPARENI THEN
 BEGIN
  REPEAT
   obtlex;
   nodoact:=0;
   apuarb:=apulit;
   nodvar;
   codigo;
   createmp(valtemp);
   IF vdes+valtemp<0 THEN yoptimiza(0,0);
   writeln('stl ',vdes+valtemp:0);
   tipoi:=tipobase(tipoi);
   gbase(profact,-1);
   writeln('ldnlp -NUMCONSLIB');
        IF tipoi=2 THEN writeln('call L0')
   ELSE IF tipoi=6 THEN writeln('call L2')
   ELSE IF tipoi=4 THEN writeln('call L1')
   ELSE error(14);
   writeln('ldl ',vdes+valtemp:0);
   IF arbol[nodoact].nom=NDIRVAR THEN
    IF num[tipo[arbol[nodoact].esp]]=1 THEN tipoi:=2;
        IF tipoi=6 THEN writeln('fpstnlsn')
   ELSE IF tipoi=2 THEN writeln('stnl 0')
   ELSE IF tipoi=4 THEN writeln('sb');
   borratemp
  UNTIL lexema<>CCOMA;
  compara(CPAREND,22);
  obtlex
 END
 ELSE IF par>0 THEN error(35);
END;

PROCEDURE freadln;
BEGIN
 fread(0);
 yoptimiza(0,0);
 gbase(profact,-1);
 writeln('ldnlp -NUMCONSLIB');
 writeln('call L3')
END;

PROCEDURE fnew;
BEGIN
 compsig(CPARENI,31);
 obtlex;
 nodoact:=0;
 apuarb:=apulit;
 nodvar;
 IF clase[tipoi]<>'D' THEN error(14);
 compara(CPAREND,22);
 obtlex;
 creanodo(NINUM,0,0,num[tipo[tipoi]]);
 creanodo(NNULO,nodoact-1,nodoact,0);
 codigo;
 gbase(profact,-1);
 writeln('ldnlp -NUMCONSLIB');
 writeln('call NEW')
END;

PROCEDURE fdispose;
BEGIN
 compsig(CPARENI,31);
 obtlex;
 nodoact:=0;
 apuarb:=apulit;
 nodvar;
 IF clase[tipoi]<>'D' THEN error(14);
 compara(CPAREND,22);
 obtlex;
 creanodo(NICARGA,nodoact,0,0);
 codigo;
 gbase(profact,-1);
 writeln('ldnlp -NUMCONSLIB');
 writeln('call DISPOSE')
END;

PROCEDURE frandomize;
BEGIN
 obtlex;
 yoptimiza(0,0);
 gbase(profact,-1);
 writeln('ldnlp -NUMCONSLIB');
 writeln('call RANDOMIZE')
END;

PROCEDURE finsert;
VAR nodovar,nodoizq,nododer,nodostr,tipovar:INTEGER;
BEGIN
 compsig(CPARENI,31);
 obtlex;
 nodoact:=0;
 apuarb:=apulit;
 expr2;
 tipoi:=tipobase(tipoi);
 IF NOT cadtipo(tipoi) THEN error(14);
 convcadena(nodoact,tipoi);
 nodoizq:=nodoact;
 compara(CCOMA,6);
 obtlex;
 nodvar;
 nodovar:=nodoact;
 tipovar:=tipoi;
 IF clase[tipoi]<>'N' THEN error(14);
 creanodo(NCCARGA,nodoact,0,0);
 nododer:=nodoact;
 compara(CCOMA,6);
 obtlex;
 expr2;
 tipoi:=tipobase(tipoi);
 IF tipoi<>2 THEN error(14);
 nodostr:=nodoact;
 creanodo(NINSERT,nodoizq,nododer,nodostr);
 compara(CPAREND,22);
 obtlex;
 creanodo(NASIGNA,nodoact,nodovar,tipovar);
 codigo
END;

PROCEDURE fdelete;
VAR nodovar,nodoizq,nododer,nodostr,tipovar:INTEGER;
BEGIN
 compsig(CPARENI,31);
 obtlex;
 nodoact:=0;
 apuarb:=apulit;
 nodvar;
 nodovar:=nodoact;
 tipovar:=tipoi;
 IF clase[tipoi]<>'N' THEN error(14);
 creanodo(NCCARGA,nodoact,0,0);
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
 creanodo(NDELETE,nodoizq,nododer,nodostr);
 compara(CPAREND,22);
 obtlex;
 creanodo(NASIGNA,nodoact,nodovar,tipovar);
 codigo
END;

PROCEDURE asigna;
VAR nodo2,tipoi2:INTEGER;
BEGIN
 nodoact:=0;
 apuarb:=apulit;
 nodvar;
 tipoi2:=tipoi;
 nodo2:=nodoact;
 compara(CASIGNA,13);
 obtlex;
 expr2;
 convasigna(tipoi2,tipoi);
 creanodo(NASIGNA,nodoact,nodo2,tipoi2);
 codigo
END;

PROCEDURE proc;
BEGIN
 nodoact:=0;
 apuarb:=apulit;
 procfunc;
 codigo
END;

PROCEDURE sident;
BEGIN
 busca;
 IF possim=0 THEN  { Busca en funciones interconstruidas }
 BEGIN
       IF cadena='WRITE' THEN fwrite(1)
  ELSE IF cadena='WRITELN' THEN fwriteln
  ELSE IF cadena='READ' THEN fread(1)
  ELSE IF cadena='READLN' THEN freadln
  ELSE IF cadena='NEW' THEN fnew
  ELSE IF cadena='DISPOSE' THEN fdispose
  ELSE IF cadena='RANDOMIZE' THEN frandomize
  ELSE IF cadena='INSERT' THEN finsert
  ELSE IF cadena='DELETE' THEN fdelete
  ELSE asigna;     { Quizas se usa con WITH }
 END
 ELSE
 BEGIN
       IF clase[possim]='V' THEN asigna  { Es una variable }
  ELSE IF clase[possim]='P' THEN proc    { o un procedimiento }
  ELSE error(12);                        { pero no otra cosa }
 END;
END;

PROCEDURE sasm;               { Ensamblaje en linea }
BEGIN
 copiaenlace;                 { Estas dos lineas forzan a que }
 yoptimiza(0,0);              { la pila este en el formato correcto }
 WHILE caract=' ' DO obtcar;  { Ignora los espacios }
 WHILE aplin<tamlin DO        { Copia la linea literalmente }
 BEGIN
  write(caract);
  obtcar
 END;
 writeln;
 obtlex;
 sentencia;
END;

PROCEDURE swith;
VAR pwitha,twitha:INTEGER;
BEGIN
 twitha:=twith;                       { Salva el estado anterior }
 pwitha:=pwith;
 obtlex;
 nodoact:=0;                          { Genera la dirección del WITH }
 apuarb:=apulit;
 nodvar;
 IF clase[tipoi]<>'R' THEN error(14); { Verifíca que sea RECORD }
 WHILE lexema=CCOMA DO
 BEGIN
  obtlex;
  listarec;
  IF clase[tipoi]<>'R' THEN error(14)
 END;
 twith:=tipoi;
 codigo;
 compara(CDO,18);
 obtlex;
 createmp(pwith);                     { Crea un lugar para la dirección }
 IF vdes+pwith<0 THEN yoptimiza(0,0);
 writeln('stl ',vdes+pwith:0);
 sentencia;                           { Procesa las sentencias }
 borratemp;
 twith:=twitha;                       { Recupera el estado anterior }
 pwith:=pwitha
END;

PROCEDURE sentencia;
BEGIN
 IF lexema=CNUM THEN snum;
 CASE lexema OF
  CIDENT : sident;
  CIF    : sif;
  CWHILE : swhile;
  CFOR   : sfor;
  CREPEAT: srepeat;
  CCASE  : scase;
  CBEGIN : sbegin;
  CGOTO  : sgoto;
  CWITH  : swith;
  CASM   : sasm
 END;
END;


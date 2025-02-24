/*
** Small C compiler for transputer.
** Now with expresion trees and optimized code generator.
**
** by Oscar Toledo Gutierrez.
**
** Original compiler by Ron Cain.
**
** 12-jun-1995
*/
 
#define BANNER  "*** Compilador de C para G-10 ***"
#define AUTHOR  "   por Oscar Toledo Gutierrez."
#define VERSION "          Version 1.00"

#define NO      0
#define SI      1

/* Definiciones para ejecucion solitaria */

#define NULL 0
#define eol 10

/* Define parametros de la tabla de nombres */

#define symsiz  24
#define symtbsz 8400
#define numglbs 300
#define startglb symtab
#define endglb  startglb+numglbs*symsiz
#define startloc endglb+symsiz
#define endloc  symtab+symtbsz-symsiz

/* Define formato de los nombres */

#define name    0
#define ident   17
#define type    18
#define storage 19
#define offset  20

/* Tama¤o m ximo de los nombres */

#define namesize 17
#define namemax  16

/* Valores posibles para "ident" */

#define variable 1
#define array    2
#define pointer  3
#define function 4

/* Valores posibles para "type" */

#define cchar   1
#define cint    2

/* Valores posibles para "storage" */

#define statik  1
#define stkloc  2

/* Define la cola de "while" */

#define wqtabsz 100
#define wqsiz   4
#define wqmax   wq+wqtabsz-wqsiz

/* Define los desplazamientos en la cola de while's */

#define wqsym   0
#define wqsp    1
#define wqloop  2
#define wqlab   3

/* Define el almacenamiento de cadenas */

#define litabsz 1024
#define litmax  litabsz-1

/* Define la linea de entrada */

#define linesize 512
#define linemax linesize-1
#define mpmax   linemax

/* Define el almacenamiento de macros */

#define macqsize 4096
#define macmax  macqsize-1

/* Define los tipos de sentencias */

#define stif     1
#define stwhile  2
#define streturn 3
#define stbreak  4
#define stcont   5
#define stasm    6
#define stexp    7

/* Define como cortar un nombre muy largo para el ensamblador */

#define asmpref 7
#define asmsuff 7

/* Reserva espacio para las variables */

char symtab[symtbsz];   /* Tabla de nombres */
char *glbptr, *locptr;  /* Apuntadores a las sigs. entradas libres */

int wq[wqtabsz];        /* Cola de bucles */
int *wqptr;             /* Apuntador a la sig. entrada */

char litq[litabsz];     /* Almacenamiento de cadenas */
int litptr;             /* Apuntador a la sig. entrada */

char macq[macqsize];    /* Buffer de macros */
int macptr;             /* Indice en el buffer */

char line[linesize];    /* Buffer de analisis */
char mline[linesize];   /* Buffer sin preprocesar */
int lptr, mptr;         /* Apuntadores respectivos */

/* Almacenamiento miscelaneo */

int nxtlab,             /* Siguiente etiqueta disponible */
    litlab,             /* Etiqueta para el buffer de cadenas */
    Zsp,                /* Apuntador de pila del compilador */
    argstk,             /* Pila de argumentos */
    ncmp,               /* No. de bloques abiertos */
    errcnt,             /* No. de errores detectados */
    errstop,            /* Indica si se detiene en caso de error */
    eof,                /* Indica el final del archivo de entrada */
    input,              /* Archivo de entrada */
    output,             /* Archivo de salida */
    input2,             /* Archivo #include */
    ctext,              /* Indica si incluye el prog. en la salida */
    cmode,              /* Indica si esta compilando C */
    lastst,             /* Ultima sentencia ejecutada */
    saveout,            /* Indica desvio a la consola */
    fnstart,            /* Linea de comienzo de la funcion actual */
    lineno,             /* Linea en el archivo actual */
    infunc,             /* Indica si esta dentro de una funcion */
    savestart,          /* Copia de "fnstart" */
    saveline,           /* Copia de "lineno" */
    saveinfn;           /* Copia de "infunc" */

char *currfn,           /* Apuntador a la definicion de la funci¢n actual */
     *savecurr;         /* Copia de "currfn" para #include */
char quote[2];          /* Cadena literal para '"' */
char *cptr;             /* Apuntador de trabajo */
int *iptr;              /* Apuntador de trabajo */
int posglobal;          /* Posicion para variables estaticas */
int usaexpr;            /* Indica si se usa el resultado de la expr. */

/*
** El compilador comienza su ejecucion aqui.
*/
main()
{
  hello();              /* Presentacion */
  see();                /* Determina las opciones */
  openin();             /* Primer archivo a procesar */
  while (input != 0) {  /* Procesa todos los archivos que se pidan */
    glbptr = startglb;  /* Limpia la tabla global */
    locptr = startloc;  /* Limpia la tabla local */
    wqptr = wq;         /* Limpia la cola de bucles */
    macptr =            /* Limpia la tabla de macros */
    Zsp =               /* Apuntador de pila */
    errcnt =            /* No hay errores */
    eof =               /* No se ha alcanzado el fin del archivo */
    input2 =            /* No hay #include */
    saveout =           /* No se ha desviado la salida */
    ncmp =              /* No hay bloques abiertos */
    lastst =
    fnstart =           /* La funcion actual empez¢ en la linea 0 */
    lineno =            /* No se han leido lineas del archivo */
    infunc =            /* No esta dentro de una funci¢n */
    nxtlab =            /* Inicia n£meros de etiquetas */
    quote[1] =
    0;
    quote[0] = '"';     /* Crea una cadena con una comilla */
    posglobal = 2;
    currfn = NULL;      /* Ninguna funci¢n a£n */
    cmode = 1;          /* Activa el preprocesamiento */
    openout();
    header();
    parse();
    if (ncmp)
      error("Falta llave de cierre");
    trailer();
    closeout();
    errorsummary();
    openin();
  }
}

/*
** Cancela la compilaci¢n.
*/
abort()
{
  if (input2)
    endinclude();
  if (input)
    fclose(input);
  closeout();
  toconsole();
  pl("Compilaci¢n cancelada.");
  nl();
  exit();
}


/*
** Procesa todo el texto de entrada.
**
** En este nivel, solo declaraciones estaticas,
** #define, #include, y definiciones de funcion
** son legales.
*/
parse()
{
  while (eof == 0) {    /* Trabaja hasta que no haya mas entrada */
    if (amatch("char", 4)) {
      declglb(cchar);
      ns();
    } else if (amatch("int", 3)) {
      declglb(cint);
      ns();
    } else if (match("#asm"))
      doasm();
    else if (match("#include"))
      doinclude();
    else if (match("#define"))
      addmac();
    else
      newfunc();
    blanks();           /* Rastrea fin de archivo */
  }
}

/*
** Vacia el almacenamiento de cadenas
*/
dumplits()
{
  int j, k;

  if (litptr == 0)
    return;             /* No hay nada, volver... */
  printlabel(litlab);   /* Imprime la etiqueta */
  col();
  nl();
  k = 0;                /* Inicia un indice... */
  while (k < litptr) {  /* para vaciar el almacenamiento */
    defbyte();          /* Define byte */
    j = 5;              /* Bytes por linea */
    while (j--) {
      outdec(litq[k++] & 255);
      if ((j == 0) | (k >= litptr)) {
        nl();           /* Otra linea */
        break;
      }
      outbyte(',');     /* Separa los bytes */
    }
  }
}

/*
** Reporta los errores
*/
errorsummary()
{
  nl();
  outstr("Hubo ");
  outdec(errcnt);       /* No. total de errores */
  outstr(" errores en la compilaci¢n.");
  nl();
}

/*
** Presentacion.
*/
hello()
{
  nl();
  nl();
  pl(BANNER);
  nl();
  pl(AUTHOR);
  nl();
  nl();
  pl(VERSION);
  nl();
  nl();
}

see()
{
  /* Checa si el usuario quiere ver todos los errores */
  pl("Desea una pausa despues de un error (S/N) ? ");
  gets(line);
  errstop = 0;
  if ((ch() == 'S') | (ch() == 's'))
    errstop = 1;

  pl("Desea que aparezca el listado C (S/N) ? ");
  gets(line);
  ctext = 0;
  if ((ch() == 'S') | (ch() == 's'))
    ctext = 1;
}

/*
** Obtiene el nombre del archivo de salida.
*/
openout()
{
  output = 0;           /* Por defecto la salida a la consola */
  while (output == 0) {
    kill();
    pl("Archivo de salida ? ");
    gets(line);         /* Obtiene el nombre */
    if (ch() == 0)
      break;            /* Ninguno... */
    if ((output = fopen(line, "w")) == NULL) {  /* Intenta crear */
      output = 0;       /* No pudo crearse */
      error("No se pudo crear el archivo");
    }
  }
  kill();               /* Limpia la linea */
}

/*
** Obtiene el archivo de entrada
*/
openin()
{
  input = 0;            /* Ninguno aun */
  while (input == 0) {
    kill();             /* Limpia la linea de entrada */
    pl("Archivo de entrada ? ");
    gets(line);         /* Obtiene un nombre */
    if (ch() == 0)
      break;
    if ((input = fopen(line, "r")) != NULL)
      newfile();
    else {
      input = 0;        /* No se pudo leer */
      pl("No se pudo leer el archivo");
    }
  }
  kill();               /* Limpia la linea */
}

/*
** Inicia el contador de lineas.
*/
newfile()
{
  lineno = 0;           /* Ninguna linea leida */
  fnstart = 0;          /* Ninguna funcion aun */
  currfn = NULL;
  infunc = 0;
}

/*
** Abre un archivo #include
*/
doinclude()
{
  blanks();             /* Salta los espacios */

  toconsole();
  outstr("#include ");
  outstr(line + lptr);
  nl();
  tofile();

  if (input2)
    error("No se pueden anidar archivos");
  else if ((input2 = fopen(line + lptr, "r")) == NULL) {
    input2 = 0;
    error("No se pudo leer el archivo");
  } else {
    saveline = lineno;
    savecurr = currfn;
    saveinfn = infunc;
    savestart = fnstart;
    newfile();
  }
  kill();               /* La siguiente entrada sera del */
                        /* nuevo archivo. */
}

/*
** Cierra un archivo #include
*/
endinclude()
{
  toconsole();
  outstr("#fin include");
  nl();
  tofile();

  input2 = 0;
  lineno = saveline;
  currfn = savecurr;
  infunc = saveinfn;
  fnstart = savestart;
}

/*
** Cierra el archivo de salida.
*/
closeout()
{
  tofile();             /* Si esta desviado, volver al archivo */
  if (output)
    fclose(output);     /* Si esta abierto, cerrarlo */
  output = 0;           /* Marcar como cerrado */
}

/*
** Declara una variable est tica.
**
** Crea una entrada en la tabla, para que las
** referencias subsiguientes la llamen por nombre.
*/
declglb(typ)            /* typ es cchar o cint */
  int typ;
{
  int k, j;
  char sname[namesize];

  while (1) {
    while (1) {
      if (endst())
        return;         /* Procesa la linea */
      k = 1;            /* Asume 1 elemento */
      if (match("*"))   /* ¨ Apuntador ? */
        j = pointer;    /* Si */
      else
        j = variable;   /* No */
      if (symname(sname) == 0)  /* ¨ Nombre correcto ? */
        illname();              /* No... */
      if (findglb(sname))       /* ¨ Ya estaba en la tabla ? */
        multidef(sname);
      if (match("[")) {         /* ¨ Matriz ? */
        k = needsub();          /* Obtiene el tama¤o */
        if (k)
          j = array;            /* !0= Matriz */
        else
          j = pointer;          /* 0= Apuntador */
      }
      addglb(sname, j, typ, posglobal); /* Agrega el nombre */
      if ((cptr[type] == cint) |
          (cptr[ident] == pointer))
        k = k * 4;
      posglobal = posglobal + ((k + 3) / 4);
      break;
    }
    if (match(",") == 0)
      return;                   /* ¨ M s ? */
  }
}

/*
** Procesa declaraciones de variables locales.
*/
declloc()
{
  int k, j, pila, typ;
  char sname[namesize];

  pila = Zsp;
  while (1) {
    if (amatch("int", 3))
      typ = cint;
    else if (amatch("char", 4))
      typ = cchar;
    else
      break;
    while (1) {
      if (endst())
	break;
      if (match("*"))
	j = pointer;
      else
	j = variable;
      if (symname(sname) == 0)
	illname();
      if (findloc(sname))
	multidef(sname);
      if (match("[")) {
	k = needsub();
	if (k) {
	  j = array;
	  if (typ == cint)
	    k = k * 4;
	} else {
	  j = pointer;
	  k = 4;
	}
      } else if ((typ == cchar)
		 & (j != pointer))
	k = 1;
      else
	k = 4;
      /* Modifica la pila */
      k = (k + 3) / 4;
      pila = pila - k;
      addloc(sname, j, typ, pila);
      if (match(",") == 0)
        break;
    }
    ns();
  }
  Zsp = modstk(pila);
}

/*
** Obtiene el tama¤o de una matriz.
**
** Invocada cuando una declaraci¢n es seguida
** por "[".
*/
needsub()
{
  int num[1];

  if (match("]"))
    return 0;                   /* Tama¤o nulo */
  if (number(num) == 0) {       /* Busca el n£mero */
    error("Debe ser un n£mero");/* No es un n£mero */
    num[0] = 1;                 /* Forza a 1 */
  }
  if (num[0] < 0) {
    error("Tama¤o negativo");
    num[0] = (-num[0]);
  }
  needbrack("]");       /* Forza una dimensi¢n */
  return num[0];        /* y retorna el tama¤o */
}

/*
** Compila una funci¢n.
**
** Invocada por "parse", esta rutina intenta compilar una funcion
** a partir de la entrada.
*/
newfunc()
{
  char n[namesize];
  int argtop;

  if (symname(n) == 0) {
    if (eof == 0)
      error("Declaracion o funcion ilegal");
    kill();                     /* Inv lida la linea */
    return;
  }
  fnstart = lineno;             /* Recuerda en que linea comenzo la funci¢n */
  infunc = 1;                   /* Indica que esta dentro de una funci¢n */
  if (currfn = findglb(n)) {    /* ¨ Ya estaba en la tabla de nombres ? */
    if (currfn[ident] != function)
      multidef(n);              /* Ya hay una variable con ese nombre */
    else if (currfn[offset] == function)
      multidef(n);              /* Se redefinio una funci¢n */
    else
      currfn[offset] = function;/* Una funci¢n referenciada antes */
  }

  /* No estaba en la tabla, definir c¢mo una funci¢n */

  else
    currfn = addglb(n, function, cint, function);

  toconsole();
  outstr("Compilando ");
  outstr(currfn + name);
  outstr("()...");
  nl();
  tofile();

  /* Checa que haya parentesis de apertura */
  if (match("(") == 0)
    error("Falta un parentesis de apertura");
  outname(n);                   /* Imprime el nombre de la funci¢n */
  col();
  nl();

  locptr = startloc;            /* Limpia la tabla de variables locales */
  argstk = 0;                   /* Inicia la cuenta de argumentos */
  while (match(")") == 0) {     /* Empieza a contar */

    /* Cualquier nombre legal incrementa la cuenta */

    if (symname(n)) {
      if (findloc(n))
	multidef(n);
      else {
	addloc(n, 0, 0, argstk + 2);
	++argstk;
      }
    } else {
      error("Nombre ilegal para el argumento");
      junk();
    }
    blanks();

    /* Si no es parentesis de cierre, debe ser coma */

    if (streq(line + lptr, ")") == 0) {
      if (match(",") == 0)
	error("Se requiere una coma");
    }
    if (endst())
      break;
  }

  argtop = argstk;
  while (argstk) {

    /* Ahora el usuario declara los tipos de los argumentos */

    if (amatch("char", 4)) {
      getarg(cchar, argtop);
      ns();
    } else if (amatch("int", 3)) {
      getarg(cint, argtop);
      ns();
    } else {
      error("Numero incorrecto de argumentos");
      break;
    }
  }

  Zsp = 0;              /* Inicializa el apuntador de la pila */

  litlab = getlabel();  /* Etiqueta para el buffer literal */
  litptr = 0;           /* Limpia el buffer literal */

  /* Procesa una sentencia, si es un retorno */
  /* entonces no limpia la pila */

  if(statement() != streturn) {
    modstk(0);
    zret();
  }
  dumplits();

  Zsp = 0;              /* Limpia la pila de nuevo */
  locptr = startloc;    /* Elimina todas las variables locales */
  infunc = 0;           /* Ahora no esta dentro de una funci¢n */
}

/*
** Declara los tipos de los argumentos.
*/
getarg(t, top)                  /* Tipo = cchar o cint */
  int t, top;
{                               /* tope = punto m s alto de la pila */
  char n[namesize], *argptr;
  int j;

  while (1) {
    if (match("*"))
      j = pointer;
    else
      j = variable;
    if (symname(n)) {
      if (match("[")) {         /* Ignora lo que esta entre [] */
	while (inbyte() != ']')
	  if (endst())
	    break;
	j = pointer;
      }
      if (argptr = findloc(n)) {

	/* Pone el tipo correcto al argumento */

	argptr[ident] = j;
	argptr[type] = t;

      } else
	error("Se requiere el nombre de un argumento");
    } else
      illname();

    --argstk;                   /* cuenta hacia atras */
    if (endst())
      return;
    if (match(",") == 0)
      error("Se requiere una coma");
  }
}

/*
** Analizador de sentencias.
**
** Llamado cuando la sintaxis requiere una
** sentencia, retorna un n£mero que indica
** la £ltima sentencia procesada.
*/
statement()
{
  if ((ch() == 0) & (eof))
    return;
  else if (match("{"))
    compound();
  else if (amatch("if", 2)) {
    doif();
    lastst = stif;
  } else if (amatch("while", 5)) {
    dowhile();
    lastst = stwhile;
  } else if (amatch("return", 6)) {
    doreturn();
    ns();
    lastst = streturn;
  } else if (amatch("break", 5)) {
    dobreak();
    ns();
    lastst = stbreak;
  } else if (amatch("continue", 8)) {
    docont();
    ns();
    lastst = stcont;
  } else if (match(";"))
    lastst = stexp;
  else if (match("#asm")) {
    doasm();
    lastst = stasm;
  }
  /* Asumir que es una expresi¢n */
  else {
    usaexpr = NO;
    expression();
    ns();
    lastst = stexp;
  }
  return lastst;
}

/*
** Checa punto y coma.
**
** Llamado cuando la sintaxis lo requiere.
*/
ns()
{
  if (match(";") == 0)
    error("Falta punto y coma");
}

/*
** Bloque de sentencias.
*/
compound()
{
  int local, pila;

  local = locptr;               /* Variables locales */
  pila = Zsp;                   /* Pila actual */
  ++ncmp;                       /* Un nuevo nivel */
  declloc();                    /* Procesa declaraciones locales */
  while (match("}") == 0)
    statement();                /* Procesa sentencias */
  --ncmp;                       /* Cierra el nivel */
  locptr = local;               /* Limpia las variables locales */
  if(lastst == streturn) return;
  if(lastst == stbreak) return;
  if(lastst == stcont) return;
  Zsp = modstk(pila);           /* Limpia la pila */
}

/*
** Sentencia "if"
*/
doif()
{
  int flab1, flab2;

  flab1 = getlabel();           /* Etiqueta para el salto falso */
  test(flab1);                  /* Prueba la expresi¢n y salta si es falsa */
  statement();                  /* Verdadera, procesa sentencias */
  if (amatch("else", 4) == 0)   /* ¨ if...else ? */
                                /* "if" simple ... imprimir etiqueta de falso */
  {
    printlabel(flab1);
    col();
    nl();
    return;                     /* Y vuelve */
  }
                                /* Una sentencia "if...else" */
  jump(flab2 = getlabel());     /* Salta alrededor del codigo de else */
  printlabel(flab1);
  col();
  nl();                         /* Imprime etiqueta falsa */
  statement();                  /* Procesa el else */
  printlabel(flab2);
  col();
  nl();                         /* Imprime etiqueta verdadera */
}

/*
** Sentencia "while"
*/
dowhile()
{
  int wq[4];                    /* Crea una entrada */

  wq[wqsym] = locptr;           /* Nivel de variables locales */
  wq[wqsp] = Zsp;               /* Nivel de la pila */
  wq[wqloop] = getlabel();      /* Etiqueta del bucle */
  wq[wqlab] = getlabel();       /* Etiqueta de salida */
  addwhile(wq);                 /* Agrega a la cola (para el break) */
  printlabel(wq[wqloop]);       /* Etiqueta del bucle */
  col();
  nl();
  test(wq[wqlab]);              /* Checa la expresi¢n */
  statement();                  /* Procesa una sentencia */
  if((lastst != streturn) &
     (lastst != stcont) &
     (lastst != stbreak))
    jump(wq[wqloop]);           /* Vuelve al bucle */
  printlabel(wq[wqlab]);        /* Etiqueta de salida */
  col();
  nl();
  delwhile();                   /* Borra de la cola */
}

/*
** Sentencia "return"
*/
doreturn()
{
  /* Checa si hay una expresi¢n */
  if (endst() == 0) {
    usaexpr = SI;
    expression();
  }
  modstk(0);                    /* Limpia la pila */
  zret();                       /* Sale de la funci¢n */
}

/*
** Sentencia "break"
*/
dobreak()
{
  int *ptr;

  /* Ve si hay un while abierto */
  if ((ptr = readwhile()) == 0)
    return;                     /* No */
  modstk((ptr[wqsp]));          /* Si, arregla la pila */
  jump(ptr[wqlab]);             /* Salta a la etiqueta de salida */
}

/*
** Sentencia "continue"
*/
docont()
{
  int *ptr;

  /* Ve si hay un while abierto */
  if ((ptr = readwhile()) == 0)
    return;                     /* No */
  modstk((ptr[wqsp]));          /* Si, arregla la pila */
  jump(ptr[wqloop]);            /* Salta a la etiqueta de salida */
}

/*
** Seudo-sentencia "asm"
**
** Entra en un modo en el que el lenguaje ensamblador
** es pasado intacto a traves del analizador.
*/
doasm()
{
  cmode = 0;                    /* Marca modo ensamblador */
  while (1) {
    inline();                   /* Obtiene y imprime lineas */
    if (match("#endasm"))
      break;                    /* hasta que... */
    if (eof)
      break;
    outstr(line);
    nl();
  }
  kill();                       /* Limpia la linea */
  cmode = 1;                    /* Vuelve al modo de an lisis */
}

junk()
{
  if (an(inbyte()))
    while (an(ch()))
      gch();
  else
    while (an(ch()) == 0) {
      if (ch() == 0)
	break;
      gch();
    }
  blanks();
}

endst()
{
  blanks();
  return ((streq(line + lptr, ";") | (ch() == 0)));
}

illname()
{
  error("Nombre ilegal");
  junk();
}

multidef(sname)
  char *sname;
{
  error("Nombre redefinido");
  comment();
  outstr(sname);
  nl();
}

needbrack(str)
  char *str;
{
  if (match(str) == 0) {
    error("Falta un ");
    comment();
    outstr(str);
    nl();
  }
}

needlval()
{
  error("Debe ser un valor-l");
}

findglb(sname)
  char *sname;
{
  char *ptr;

  ptr = startglb;
  while (ptr != glbptr) {
    if (astreq(sname, ptr, namemax))
      return ptr;
    ptr = ptr + symsiz;
  }
  return 0;
}

findloc(sname)
  char *sname;
{
  char *ptr;

  ptr = startloc;
  while (ptr != locptr) {
    if (astreq(sname, ptr, namemax))
      return ptr;
    ptr = ptr + symsiz;
  }
  return 0;
}

addglb(sname, id, typ, value)
  char *sname, id, typ;
  int value;
{
  char *ptr;

  if (cptr = findglb(sname))
    return cptr;
  if (glbptr >= endglb) {
    error("Tabla global llena");
    return 0;
  }
  cptr = ptr = glbptr;
  while (an(*ptr++ = *sname++));/* Copia el nombre */
  cptr[ident] = id;
  cptr[type] = typ;
  cptr[storage] = statik;
  cptr[offset] = value;
  cptr[offset + 1] = value >> 8;
  cptr[offset + 2] = value >> 16;
  cptr[offset + 3] = value >> 24;
  glbptr = glbptr + symsiz;
  return cptr;
}

addloc(sname, id, typ, value)
  char *sname, id, typ;
  int value;
{
  char *ptr;

  if (cptr = findloc(sname))
    return cptr;
  if (locptr >= endloc) {
    error("Tabla local llena");
    return 0;
  }
  cptr = ptr = locptr;
  while (an(*ptr++ = *sname++));/* Copia el nombre */
  cptr[ident] = id;
  cptr[type] = typ;
  cptr[storage] = stkloc;
  cptr[offset] = value;
  cptr[offset + 1] = value >> 8;
  cptr[offset + 2] = value >> 16;
  cptr[offset + 3] = value >> 24;
  locptr = locptr + symsiz;
  return cptr;
}


/* Prueba si la proxima cadena de entrada es un nombre legal */
symname(sname)
  char *sname;
{
  int k;
  char c;

  blanks();
  if (alpha(ch()) == 0)
    return 0;
  k = 0;
  while (an(ch()))
    sname[k++] = gch();
  sname[k] = 0;
  return 1;
}

/* Prueba si el caracter dado es una letra */
alpha(c)
  int c;
{
  c = c & 255;
  return (((c >= 'a') & (c <= 'z')) |
	  ((c >= 'A') & (c <= 'Z')) |
	  (c == '_'));
}

/* Prueba si el caracter dado es un n£mero */
numeric(c)
  int c;
{
  c = c & 255;
  return ((c >= '0') & (c <= '9'));
}

/* Prueba si el caracter dado es alfanum‚rico */
an(c)
  char c;
{
  return ((alpha(c)) | (numeric(c)));
}

/* Imprime un retorno de carro y una cadena a la consola */
pl(str)
  char *str;
{
  int k;

  k = 0;
  putchar(13);
  putchar(10);
  while (*str)
    putchar(*str++);
}

addwhile(ptr)
  int ptr[];

{
  int k;

  if (wqptr == wqmax) {
    error("Demasiados bucles activos");
    return;
  }
  k = 0;
  while (k < wqsiz)
    *wqptr++ = ptr[k++];
}

delwhile()
{
  if (readwhile())
    wqptr = wqptr - wqsiz;
}

readwhile()
{
  if (wqptr == wq) {
    error("No hay bucles activos");
    return 0;
  } else
    return (wqptr - wqsiz);
}

ch()
{
  return (line[lptr] & 255);
}

nch()
{
  if (ch() == 0)
    return 0;
  else
    return (line[lptr + 1] & 255);
}

gch()
{
  if (ch() == 0)
    return 0;
  else
    return (line[lptr++] & 255);
}

kill()
{
  lptr = 0;
  line[lptr] = 0;
}

inbyte()
{
  while (ch() == 0) {
    if (eof)
      return 0;
    inline();
    preprocess();
  }
  return gch();
}

inline()
{
  int k, unit;

  while (1) {
    if (input == 0) {
      eof = 1;
      return;
    }
    if ((unit = input2) == 0)
      unit = input;
    kill();
    while ((k = fgetc(unit)) > 0) {
      if (k == 13)
	continue;
      if ((k == eol) | (lptr >= linemax))
	break;
      line[lptr++] = k;
    }
    line[lptr] = 0;     /* Agrega un caracter nulo */
    lineno++;           /* Se ha leido una l¡nea m s */
    if (k <= 0) {
      fclose(unit);
      if (input2)
	endinclude();
      else
	input = 0;
    }
    if (lptr) {
      if (ctext & cmode) {
	comment();
	outstr(line);
	nl();
      }
      lptr = 0;
      return;
    }
  }
}

preprocess()
{
  int k;
  char c, sname[namesize];

  if (cmode == 0)
    return;
  mptr = lptr = 0;
  while (ch()) {
    if ((ch() == ' ') | (ch() == 9))
      predel();
    else if (ch() == '"')
      prequote();
    else if (ch() == 39)
      preapos();
    else if ((ch() == '/') & (nch() == '*'))
      precomm();
    else if (alpha(ch())) {
      k = 0;
      while (an(ch())) {
	if (k < namemax)
	  sname[k++] = ch();
	gch();
      }
      sname[k] = 0;
      if (k = findmac(sname))
	while (c = macq[k++])
	  keepch(c);
      else {
	k = 0;
	while (c = sname[k++])
	  keepch(c);
      }
    } else
      keepch(gch());
  }
  keepch(0);
  if (mptr >= mpmax)
    error("Linea muy larga");
  lptr = mptr = 0;
  while (line[lptr++] = mline[mptr++]);
  lptr = 0;
}

keepch(c)
  char c;
{
  mline[mptr] = c;
  if (mptr < mpmax)
    mptr++;
  return c;
}

predel()
{
  keepch(' ');
  while ((ch() == ' ') |
	 (ch() == 9))
    gch();
}

prequote()
{
  keepch(ch());
  gch();
  while ((ch() != '"') | ((line[lptr - 1] == 92) & (line[lptr - 2] != 92))) {
    if (ch() == 0) {
      error("Faltan comillas");
      break;
    }
    keepch(gch());
  }
  gch();
  keepch('"');
}

preapos()
{
  keepch(39);
  gch();
  while ((ch() != 39) | ((line[lptr - 1] == 92) & (line[lptr - 2] != 92))) {
    if (ch() == 0) {
      error("Falta un apostrofe");
      break;
    }
    keepch(gch());
  }
  gch();
  keepch(39);
}

precomm()
{
  lptr = lptr + 2;
  while (((ch() == '*') &
	  (nch() == '/')) == 0) {
    if (ch() == 0)
      inline();
    else
      ++lptr;
    if (eof)
      break;
  }
  lptr = lptr + 2;
}

addmac()
{
  char sname[namesize];
  int k;

  if (symname(sname) == 0) {
    illname();
    kill();
    return;
  }
  k = 0;
  while (putmac(sname[k++]));
  while (ch() == ' ' | ch() == 9)
    gch();
  while (putmac(gch()));
  if (macptr >= macmax)
    error("Tabla de macros llena");
}

putmac(c)
  char c;
{
  macq[macptr] = c;
  if (macptr < macmax)
    macptr++;
  return c;
}

findmac(sname)
  char *sname;
{
  int k;

  k = 0;
  while (k < macptr) {
    if (astreq(sname, macq + k, namemax)) {
      while (macq[k++]);
      return k;
    }
    while (macq[k++]);
    while (macq[k++]);
  }
  return 0;
}

/* Desvia la salida a la consola */
toconsole()
{
  saveout = output;
  output = 0;
}

/* Regresa la salida al archivo */
tofile()
{
  if (saveout)
    output = saveout;
  saveout = 0;
}

outbyte(c)
  char c;
{
  if (c == 0)
    return 0;
  if (output) {
    if ((fputc(c, output)) <= 0) {
      closeout();
      error("Error al escribir");
      abort();
    }
  } else
    putchar(c);
  return c;
}

nl()
{
  outbyte(13);
  outbyte(10);
}

outstr(ptr)
  char *ptr;
{
  while (outbyte(*ptr++));
}

/*
** Escribe texto destinado al ensamblador
*/
outasm(ptr)
  char *ptr;
{
  while (outbyte(*ptr++));
}

error(ptr)
  char ptr[];
{
  int k;
  char junk[81];

  toconsole();
  outstr("L¡nea ");
  outdec(lineno);
  outstr(", ");
  if (infunc == 0)
    outbyte('(');
  if (currfn == NULL)
    outstr("comienzo del archivo");
  else
    outstr(currfn + name);
  if (infunc == 0)
    outbyte(')');
  outstr(" + ");
  outdec(lineno - fnstart);
  outstr(": ");
  outstr(ptr);
  nl();

  outstr(line);
  nl();

  k = 0;                /* Busca la posici¢n del error */
  while (k < lptr) {
    if (line[k++] == 9)
      outbyte(9);
    else
      outbyte(' ');
  }
  outbyte('^');
  nl();
  ++errcnt;

  if (errstop) {
    pl("Continuar (Si, No, Pasar de largo) ? ");
    gets(junk);
    k = junk[0];
    if ((k == 'N') | (k == 'n'))
      abort();
    if ((k == 'P') | (k == 'p'))
      errstop = 0;
  }
  tofile();
}

ol(ptr)
  char ptr[];
{
  ot(ptr);
  nl();
}

ot(ptr)
  char ptr[];
{
  outasm(ptr);
}

streq(str1, str2)
  char str1[], str2[];
{
  int k;

  k = 0;
  while (str2[k]) {
    if ((str1[k]) != (str2[k]))
      return 0;
    k++;
  }
  return k;
}

astreq(str1, str2, len)
  char str1[], str2[];
  int len;
{
  int k;

  k = 0;
  while (k < len) {
    if ((str1[k]) != (str2[k]))
      break;
    if (str1[k] == 0)
      break;
    if (str2[k] == 0)
      break;
    k++;
  }
  if (an(str1[k]))
    return 0;
  if (an(str2[k]))
    return 0;
  return k;
}

match(lit)
  char *lit;
{
  int k;

  blanks();
  if (k = streq(line + lptr, lit)) {
    lptr = lptr + k;
    return 1;
  }
  return 0;
}

amatch(lit, len)
  char *lit;
  int len;
{
  int k;

  blanks();
  if (k = astreq(line + lptr, lit, len)) {
    lptr = lptr + k;
    while (an(ch()))
      inbyte();
    return 1;
  }
  return 0;
}

blanks()
{
  while (1) {
    while (ch() == 0) {
      inline();
      preprocess();
      if (eof)
	break;
    }
    if (ch() == ' ')
      gch();
    else if (ch() == 9)
      gch();
    else
      return;
  }
}

outdec(number)
  int number;
{
  if (number < 0) {
    outbyte('-');
    if (number < -9)
      outdec(-(number / 10));
    outbyte(-(number % 10) + '0');
  } else {
    if (number > 9)
      outdec(number / 10);
    outbyte((number % 10) + '0');
  }
}

/* Retorna el tama¤o de una cadena. */
strlen(s)
  char *s;
{
  char *t;

  t = s;
  while (*s)
    s++;
  return (s - t);
}

/* Conversi¢n a m yusculas. */
raise(c)
  char c;
{
  if ((c >= 'a') & (c <= 'z'))
    c = c - 'a' + 'A';
  return (c);
}

/*
** Evaluador de Expresiones.
**
** por Oscar Toledo Gutierrez.
**
** (c) Copyright 1995 Oscar Toledo G.
*/

#define N_OR      1
#define N_XOR     2
#define N_AND     3
#define N_IGUAL   4
#define N_CIGUAL  5
#define N_MAYOR   6
#define N_CSUMA   7
#define N_NULO    8
#define N_STNL    9
#define N_SMAYOR  10
#define N_FUNC    11
#define N_FUNCI   12
#define N_PAR     13
#define N_CD      14
#define N_CI      15
#define N_SUMA    16
#define N_RESTA   17
#define N_MUL     18
#define N_DIV     19
#define N_MOD     20
#define N_NEG     21
#define N_COM     22
#define N_INC     23
#define N_STL     24
#define N_PINC    25
#define N_LDNL    26
#define N_NOT     27
#define N_IXP     28
#define N_APFUNC  29
#define N_CONST   30
#define N_LIT     31
#define N_CBYTE   32
#define N_CPAL    33
#define N_LDLP    34
#define N_LDL     35
#define N_LDNLP   36
#define N_GBYTE   37
#define N_GPAL    38

#define TAM_ARBOL 128

int nodo_izq[TAM_ARBOL];
int nodo_der[TAM_ARBOL];
int oper[TAM_ARBOL];
int esp[TAM_ARBOL];
int regs[TAM_ARBOL];
int ultimo_nodo;
int raiz_arbol;

expression()
{
  int lval[2];

  ultimo_nodo = 0;
  if (heir1(lval))
    rvalue(lval);
  etiqueta(ultimo_nodo);
  raiz_arbol = ultimo_nodo;
  gen_codigo(ultimo_nodo);
}

heir1(lval)
  int lval[];
{
  int k, lval2[2];
  int der;
  char *ap;

  k = heir2(lval);
  blanks();
  if (ch() != '=')
    return k;
  ++lptr;
  der = ultimo_nodo;
  if (k == 0)
    needlval();
  if (heir1(lval2))
    rvalue(lval2);
  if (lval[1] == cint)
    crea_nodo(N_GPAL, ultimo_nodo, der, 0);
  else
    crea_nodo(N_GBYTE, ultimo_nodo, der, 0);
  return 0;
}

heir2(lval)
  int lval[];
{
  int k, lval2[2];
  int izq;

  k = heir3(lval);
  blanks();
  if (ch() != '|')
    return k;
  if (k)
    rvalue(lval);
  while (match("|")) {
    izq = ultimo_nodo;
    if (heir3(lval2))
      rvalue(lval2);
    crea_nodo(N_OR, izq, ultimo_nodo, 0);
  }
  return 0;
}

heir3(lval)
  int lval[];
{
  int k, lval2[2];
  int izq;

  k = heir4(lval);
  blanks();
  if (ch() != '^')
    return k;
  if (k)
    rvalue(lval);
  while (match("^")) {
    izq = ultimo_nodo;
    if (heir4(lval2))
      rvalue(lval2);
    crea_nodo(N_XOR, izq, ultimo_nodo, 0);
  }
  return 0;
}

heir4(lval)
  int lval[];
{
  int k, lval2[2];
  int izq;

  k = heir5(lval);
  blanks();
  if (ch() != '&')
    return k;
  if (k)
    rvalue(lval);
  while (match("&")) {
    izq = ultimo_nodo;
    if (heir5(lval2))
      rvalue(lval2);
    crea_nodo(N_AND, izq, ultimo_nodo, 0);
  }
  return 0;
}

heir5(lval) int lval[]; {
  int k, lval2[2];
  int izq;

  k = heir6(lval);
  blanks();
  if ((streq(line + lptr, "==") == 0) &
      (streq(line + lptr, "!=") == 0))
    return k;
  if (k)
    rvalue(lval);
  while (1) {
    izq = ultimo_nodo;
    if (match("==")) {
      if (heir6(lval2))
        rvalue(lval2);
      if(oper[ultimo_nodo] == N_CONST) {
        if(oper[izq] == N_CONST)
          crea_nodo(N_CONST, 0, 0, esp[izq] == esp[ultimo_nodo]);
        else
          crea_nodo(N_CIGUAL, izq, 0, esp[ultimo_nodo]);
      }
      else if(oper[izq] == N_CONST)
        crea_nodo(N_CIGUAL, ultimo_nodo, 0, esp[izq]);
      else crea_nodo(N_IGUAL, izq, ultimo_nodo, 0);
    } else if (match("!=")) {
      if (heir6(lval2))
        rvalue(lval2);
      if(oper[ultimo_nodo] == N_CONST) {
        if(oper[izq] == N_CONST)
          crea_nodo(N_CONST, 0, 0, esp[izq] != esp[ultimo_nodo]);
        else {
          crea_nodo(N_CIGUAL, izq, 0, esp[ultimo_nodo]);
          crea_nodo(N_NOT, ultimo_nodo, 0, 0);
        }
      }
      else if(oper[izq] == N_CONST) {
        crea_nodo(N_CIGUAL, ultimo_nodo, 0, esp[izq]);
        crea_nodo(N_NOT, ultimo_nodo, 0, 0);
      } else {
        crea_nodo(N_IGUAL, izq, ultimo_nodo, 0);
        crea_nodo(N_NOT, ultimo_nodo, 0, 0);
      }
    } else
      return 0;
  }
}

heir6(lval)
  int lval[];
{
  int k;

  k = heir7(lval);
  blanks();
  if ((streq(line + lptr, "<") == 0) &
      (streq(line + lptr, ">") == 0) &
      (streq(line + lptr, "<=") == 0) &
      (streq(line + lptr, ">=") == 0))
  return k;
  if (streq(line + lptr, ">>"))
    return k;
  if (streq(line + lptr, "<<"))
    return k;
  if (k)
    rvalue(lval);
  while (1) {
    if (match("<="))
      heir6wrk(1, lval);
    else if (match(">="))
      heir6wrk(2, lval);
    else if (streq(line + lptr, "<") &
            (streq(line + lptr, "<<") == 0)) {
      inbyte();
      heir6wrk(3, lval);
    } else if (streq(line + lptr, ">") &
              (streq(line + lptr, ">>") == 0)) {
      inbyte();
      heir6wrk(4, lval);
    } else
      return 0;
  }
}

heir6wrk(k, lval)
  int k, lval[];
{
  int lval2[2];
  int izq;

  izq = ultimo_nodo;
  if (heir7(lval2))
    rvalue(lval2);
  if (cptr = lval[0])
    if (cptr[ident] == pointer) {
      heir6op(izq, k);
      return;
    }
  if (cptr = lval2[0])
    if (cptr[ident] == pointer) {
      heir6op(izq, k);
      return;
    }
  if(k == 4) {
    if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
      crea_nodo(N_CONST, 0, 0, esp[izq] > esp[ultimo_nodo]);
    else
      crea_nodo(N_MAYOR, izq, ultimo_nodo, 0);
  }
  else if(k == 3) {
    if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
      crea_nodo(N_CONST, 0, 0, esp[izq] < esp[ultimo_nodo]);
    else
      crea_nodo(N_MAYOR, ultimo_nodo, izq, 0);
  }
  else if(k == 1) {
    if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
      crea_nodo(N_CONST, 0, 0, esp[izq] <= esp[ultimo_nodo]);
    else {
      crea_nodo(N_MAYOR, izq, ultimo_nodo, 0);
      crea_nodo(N_NOT, ultimo_nodo, 0);
    }
  } else {
    if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
      crea_nodo(N_CONST, 0, 0, esp[izq] >= esp[ultimo_nodo]);
    else {
      crea_nodo(N_MAYOR, ultimo_nodo, izq, 0);
      crea_nodo(N_NOT, ultimo_nodo, 0);
    }
  }
}

heir6op(izq, k)
  int izq, k;
{
  if(k == 4) crea_nodo(N_SMAYOR, izq, ultimo_nodo, 0);
  else if(k == 3) crea_nodo(N_SMAYOR, ultimo_nodo, izq, 0);
  else if(k == 1) {
    crea_nodo(N_SMAYOR, izq, ultimo_nodo, 0);
    crea_nodo(N_NOT, ultimo_nodo, 0);
  } else {
    crea_nodo(N_SMAYOR, ultimo_nodo, izq, 0);
    crea_nodo(N_NOT, ultimo_nodo, 0);
  }
}

heir7(lval)
  int lval[];
{
  int k, lval2[2];
  int izq;

  k = heir8(lval);
  blanks();
  if ((streq(line + lptr, ">>") == 0) &
      (streq(line + lptr, "<<") == 0))
    return k;
  if (k)
    rvalue(lval);
  while (1) {
    izq = ultimo_nodo;
    if (match(">>")) {
      if (heir8(lval2))
	rvalue(lval2);
      if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] >> esp[ultimo_nodo]);
      else
        crea_nodo(N_CD, izq, ultimo_nodo, 0);
    } else if (match("<<")) {
      if (heir8(lval2))
	rvalue(lval2);
      if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] << esp[ultimo_nodo]);
      else
        crea_nodo(N_CI, izq, ultimo_nodo, 0);
    } else
      return 0;
  }
}

heir8(lval)
  int lval[];
{
  int k, lval2[2];
  int izq;

  k = heir9(lval);
  blanks();
  if ((ch() != '+') & (ch() != '-'))
    return k;
  if (k)
    rvalue(lval);
  while (1) {
    izq = ultimo_nodo;
    if (match("+")) {
      if (heir9(lval2))
        rvalue(lval2);
      if (cptr = lval[0])
        if ((cptr[ident] == pointer) &
            (cptr[type] == cint))
          doublereg();
      if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] + esp[ultimo_nodo]);
      else if(oper[ultimo_nodo] == N_CONST)
        crea_nodo(N_CSUMA, izq, 0, esp[ultimo_nodo]);
      else if(oper[izq] == N_CONST)
        crea_nodo(N_CSUMA, ultimo_nodo, 0, esp[izq]);
      else
        crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
    } else if (match("-")) {
      if (heir9(lval2))
        rvalue(lval2);
      if (cptr = lval[0])
        if ((cptr[ident] == pointer) &
            (cptr[type] == cint))
          doublereg();
      if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] - esp[ultimo_nodo]);
      else if(oper[ultimo_nodo] == N_CONST)
        crea_nodo(N_CSUMA, izq, 0, -esp[ultimo_nodo]);
      else
        crea_nodo(N_RESTA, izq, ultimo_nodo, 0);
    } else
      return 0;
  }
}

heir9(lval)
  int lval[];
{
  int k, lval2[2];
  int izq;

  k = heir10(lval);
  blanks();
  if ((ch() != '*') & (ch() != '/') &
      (ch() != '%'))
    return k;
  if (k)
    rvalue(lval);
  while (1) {
    izq = ultimo_nodo;
    if (match("*")) {
      if (heir10(lval2))
        rvalue(lval2);
      if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] * esp[ultimo_nodo]);
      else
        crea_nodo(N_MUL, izq, ultimo_nodo, 0);
    } else if (match("/")) {
      if (heir10(lval2))
        rvalue(lval2);
      if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] / esp[ultimo_nodo]);
      else
        crea_nodo(N_DIV, izq, ultimo_nodo, 0);
    } else if (match("%")) {
      if (heir10(lval2))
        rvalue(lval2);
      if((oper[izq] == N_CONST) & (oper[ultimo_nodo] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] % esp[ultimo_nodo]);
      else
        crea_nodo(N_MOD, izq, ultimo_nodo, 0);
    } else
      return 0;
  }
}

heir10(lval)
  int lval[];
{
  int k;

  if (match("++")) {
    if (heir10(lval) == 0)
      needlval();
    heir10inc(lval);
    return 0;
  } else if (match("--")) {
    if (heir10(lval) == 0)
      needlval();
    heir10dec(lval);
    return 0;
  } else if (match("-")) {
    if(heir10(lval))
      rvalue(lval);
    if(oper[ultimo_nodo] == N_CONST)
      esp[ultimo_nodo] = -esp[ultimo_nodo];
    else
      crea_nodo(N_NEG, ultimo_nodo, 0, 0);
    return 0;
  } else if (match("~")) {
    if(heir10(lval))
      rvalue(lval);
    if(oper[ultimo_nodo] == N_CONST)
      esp[ultimo_nodo] = ~esp[ultimo_nodo];
    else
      crea_nodo(N_COM, ultimo_nodo, 0, 0);
    return 0;
  } else if (match("!")) {
    if(heir10(lval))
      rvalue(lval);
    if(oper[ultimo_nodo] == N_CONST)
      esp[ultimo_nodo] = !esp[ultimo_nodo];
    else
      crea_nodo(N_NOT, ultimo_nodo, 0, 0);
    return 0;
  } else if (match("*")) {
    heir10as(lval);
    return 1;
  } else if (match("&")) {
    if(heir10(lval) == 0) {
      error("Direcci¢n ilegal");
      return 0;
    }
    if (lval[1])
      return 0;
    heir10at(lval);
    return 0;
  }
  k = heir11(lval);
  if (match("++")) {
    if (k == 0)
      needlval();
    heir10id(lval);
    return 0;
  } else if (match("--")) {
    if (k == 0)
      needlval();
    heir10di(lval);
    return 0;
  } else
    return k;
}

heir10inc(lval)
  int lval[];
{
  char *ptr;

  ptr = lval[0];
  if ((ptr[ident] == pointer) &
      (ptr[type] == cint))
    crea_nodo(N_INC, ultimo_nodo, 0, 4);
  else
    crea_nodo(N_INC, ultimo_nodo, 0, 1);
}

heir10dec(lval)
  int lval[];
{
  char *ptr;

  ptr = lval[0];
  if ((ptr[ident] == pointer) &
      (ptr[type] == cint))
    crea_nodo(N_INC, ultimo_nodo, 0, -4);
  else
    crea_nodo(N_INC, ultimo_nodo, 0, -1);
}

heir10as(lval)
  int lval[];
{
  int k;
  char *ptr;

  k = heir10(lval);
  if (k)
    rvalue(lval);
  lval[1] = cint;
  if (ptr = lval[0])
    lval[1] = ptr[type];
  lval[0] = 0;
}

heir10at(lval)
  int lval[];
{
  char *ptr;

  lval[1] = ptr[type];
}

heir10id(lval)
  int lval[];
{
  char *ptr;

  ptr = lval[0];
  if ((ptr[ident] == pointer) &
      (ptr[type] == cint))
    crea_nodo(N_PINC, ultimo_nodo, 0, 4);
  else
    crea_nodo(N_PINC, ultimo_nodo, 0, 1);
}

heir10di(lval)
  int lval[];
{
  char *ptr;

  ptr = lval[0];
  if ((ptr[ident] == pointer) &
      (ptr[type] == cint))
    crea_nodo(N_PINC, ultimo_nodo, 0, -4);
  else
    crea_nodo(N_PINC, ultimo_nodo, 0, -1);
}

heir11(lval)
  int lval[];
{
  int k, etiq, izq;
  char *ptr;
  int lval2[2];

  k = primary(lval);
  ptr = lval[0];
  blanks();
  if ((ch() == '[') | (ch() == '('))
    while (1) {
      if (match("[")) {
        if (ptr == 0) {
          error("No se puede usar subscripto");
          junk();
          needbrack("]");
          return 0;
        } else if (ptr[ident] == pointer)
          rvalue(lval);
        else if (ptr[ident] != array) {
          error("No se puede usar subscripto");
          k = 0;
        }
        izq = ultimo_nodo;
        if (heir1(lval2))
          rvalue(lval2);
        needbrack("]");
        if (ptr[type] == cint) {
          if (oper[ultimo_nodo] == N_CONST)
            crea_nodo(N_LDNLP, izq, 0, esp[ultimo_nodo]);
          else
            crea_nodo(N_IXP, ultimo_nodo, izq, 0);
        } else {
          if (oper[ultimo_nodo] == N_CONST)
            crea_nodo(N_CSUMA, izq, 0, esp[ultimo_nodo]);
          else
            crea_nodo(N_SUMA, ultimo_nodo, izq, 0);
        }
        lval[1] = ptr[type];
        k = 1;
      } else if (match("(")) {
        if (ptr == 0)
          callfunction(0);
        else if (ptr[ident] != function) {
          rvalue(lval);
          callfunction(0);
        } else
          callfunction(ptr);
        k = lval[0] = 0;
      } else
        return k;
    }
  if (ptr == 0)
    return k;
  if (ptr[ident] == function) {
    crea_nodo(N_APFUNC, 0, 0, ptr);
    return 0;
  }
  return k;
}

primary(lval)
  int lval[];
{
  char *ptr, sname[namesize];
  int num[1];
  int k;

  if (match("(")) {
    k = heir1(lval);
    needbrack(")");
    return k;
  }
  if (symname(sname)) {
    if (ptr = findloc(sname)) {
      getloc(ptr);
      lval[0] = ptr;
      lval[1] = ptr[type];
      if (ptr[ident] == pointer)
        lval[1] = cint;
      if (ptr[ident] == array)
        return 0;
      return 1;
    }
    if (ptr = findglb(sname)) {
      if (ptr[ident] != function) {
        outpos(0, ptr);
        lval[0] = ptr;
        lval[1] = ptr[type];
        if (ptr[ident] == pointer)
          lval[1] = cint;
        if (ptr[ident] == array)
          return 0;
        return 1;
      }
    } else
      ptr = addglb(sname, function, cint, 0);
    lval[0] = ptr;
    lval[1] = 0;
    return 0;
  }
  if (constant(num))
    return (lval[0] = lval[1] = 0);
  else {
    error("Expresion invalida");
    crea_nodo(N_CONST, 0, 0, 0);
    junk();
    return 0;
  }
}

/*
** Compila una llamada a una funcion
**
** Invocada por "heir11", esta funcion llama a la funcion
** nombra o a una funcion indirecta.
*/
callfunction(ptr)
  char *ptr;
{
  int lval[2];
  int anterior, primero;
  int izq;

  anterior = primero = 0;
  blanks();             /* Ya ha sido tomado el parentesis inicial */
  if (ptr == 0)
    izq = ultimo_nodo;  /* Llamada indirecta */
  while (streq(line + lptr, ")") == 0) {
    if (endst())
      break;
    if (heir1(lval))
      rvalue(lval);     /* Obtiene un argumento */
    crea_nodo(N_PAR, ultimo_nodo, 0, 0);
    if (primero == 0)
      primero = ultimo_nodo;
    if (anterior != 0)
      esp[anterior] = ultimo_nodo;
    anterior = ultimo_nodo;
    if (match(",") == 0)
      break;
    blanks();
  }
  needbrack(")");
  if (ptr == 0)
    crea_nodo(N_FUNCI, primero, 0, izq);
  else
    crea_nodo(N_FUNC, primero, 0, ptr);
}

rvalue(lval)
  int lval[];
{
  if (lval[1] == cchar)
    crea_nodo(N_CBYTE, ultimo_nodo, 0, 0);
  else
    crea_nodo(N_CPAL, ultimo_nodo, 0, 0);
}

/* Carga la direcci¢n de una variable local */
getloc(sym)
  char *sym;
{
  crea_nodo(N_LDLP, 0, 0, ((sym[offset] & 255) +
			   ((sym[offset + 1] & 255) << 8) +
			   ((sym[offset + 2] & 255) << 16) +
			   ((sym[offset + 3] & 255) << 24)));
}

/* Carga la dir. de las variables estaticas */
enlace()
{
  crea_nodo(N_LDL, 0, 0, 1);
}

/* Operaciones con memoria global */
outpos(tipo, var)
  int tipo;
  char *var;
{
  int j;

  j = (var[offset] & 255) +
    ((var[offset + 1] & 255) << 8) +
    ((var[offset + 2] & 255) << 16) +
    ((var[offset + 3] & 255) << 24);
  enlace();
  crea_nodo(N_LDNLP, ultimo_nodo, 0, j);
}

/* Multiplica por palabra el registro primario */
doublereg()
{
  if(oper[ultimo_nodo] == N_CONST)
    esp[ultimo_nodo] = esp[ultimo_nodo] * 4;
  else {
    crea_nodo(N_CONST, 0, 0, 4);
    crea_nodo(N_MUL, ultimo_nodo - 1, ultimo_nodo, 0);
  }
}

test(label)
  int label;
{
  needbrack("(");
  usaexpr = SI;
  expression();
  needbrack(")");
  testjump(label);
}

constant(val)
  int val[];
{
  if (number(val))
    crea_nodo(N_CONST, 0, 0, val[0]);
  else if (pstr(val))
    crea_nodo(N_CONST, 0, 0, val[0]);
  else if (qstr(val))
    crea_nodo(N_LIT, 0, 0, val[0]);
  else
    return 0;
  return 1;
}

isxdigit(c) char c; {
  return (((c >= '0') & (c <= '9')) |
          ((c >= 'A') & (c <= 'F')) |
          ((c >= 'a') & (c <= 'f')));
}

number(val)
  int val[];
{
  int k, minus, base;
  char c;

  k = minus = 1;
  while (k) {
    k = 0;
    if (match("+"))
      k = 1;
    if (match("-")) {
      minus = -minus;
      k = 1;
    }
  }
  if (numeric(ch()) == 0)
    return 0;
  if(ch() == '0') {
    while(ch() == '0') gch();
    if(raise(ch()) == 'X') {
      gch();
      while(isxdigit(ch())) {
        c = raise(gch()) - '0';
        if(c > 9) c = c - 7;
        k = (k << 4) | c;
      }
    } else {
      while ((ch() >= '0') & (ch() <= '7'))
        k = k * 8 + (gch() - '0');
    }
  } else {
    while (numeric(ch()))
      k = k * 10 + (gch() - '0');
  }
  if (minus < 0)
    k = -k;
  val[0] = k;
  return 1;
}

pstr(val)
  int val[];
{
  int k;

  k = 0;
  if (match("'") == 0)
    return 0;
  while (ch() != 39)
    k = (k & 255) * 256 + (litchar() & 255);
  ++lptr;
  val[0] = k;
  return 1;
}

qstr(val)
  int val[];
{
  char c;

  if (match(quote) == 0)
    return 0;
  val[0] = litptr;
  while (ch() != '"') {
    if (ch() == 0)
      break;
    if (litptr >= litmax) {
      error("Espacio de almacenamiento de cadenas agotado");
    while (match(quote) == 0)
      if (gch() == 0)
        break;
      return 1;
    }
    litq[litptr++] = litchar();
  }
  gch();
  litq[litptr++] = 0;
  return 1;
}

litchar()
{
  int i, oct;

  if ((ch() != 92) | (nch() == 0))
    return gch();
  gch();
  if (ch() == 'n') {
    ++lptr;
    return 10;
  }
  if (ch() == 't') {
    ++lptr;
    return 9;
  }
  if (ch() == 'b') {
    ++lptr;
    return 8;
  }
  if (ch() == 'f') {
    ++lptr;
    return 12;
  }
  if (ch() == 'r') {
    ++lptr;
    return 13;
  }
  i = 3;
  oct = 0;
  while ((i-- > 0) & (ch() >= '0') & (ch() <= '7'))
    oct = (oct << 3) + gch() - '0';
  if (i == 2)
    return gch();
  else
    return oct;
}

crea_nodo(op, izq, der, val)
  int op, izq, der, val;
{
  if(op == N_CSUMA) {
    if(oper[izq] == N_CSUMA) {
      val = val + esp[izq];
      izq = nodo_izq[izq];
    }
  }
  else if(op == N_LDNLP) {
    if(oper[izq] == N_LDNLP) {
      val = val + esp[izq];
      izq = nodo_izq[izq];
    }
    else if(oper[izq] == N_LDLP) {
      val = val + esp[izq];
      izq = 0;
      op = N_LDLP;
    }
  }
  ++ultimo_nodo;
  if(ultimo_nodo == TAM_ARBOL) {
    error("Expresion muy compleja");
    abort();
  }
  nodo_izq[ultimo_nodo] = izq;
  nodo_der[ultimo_nodo] = der;
  oper[ultimo_nodo] = op;
  esp[ultimo_nodo] = val;
  regs[ultimo_nodo] = 0;
}

etiqueta(nodo)
  int nodo;
{
  int min, max;

  if (nodo_izq[nodo])
    etiqueta(nodo_izq[nodo]);
  if (nodo_der[nodo])
    etiqueta(nodo_der[nodo]);
  if ((oper[nodo] == N_FUNCI) | (oper[nodo] == N_PAR))
    if (esp[nodo])
      etiqueta(esp[nodo]);
  if ((oper[nodo] == N_FUNCI) | (oper[nodo] == N_FUNC) |
      (oper[nodo] == N_GBYTE) | (oper[nodo] == N_GPAL))
    regs[nodo] = 3;
  else if ((oper[nodo] == N_INC) | (oper[nodo] == N_PINC)) {
    regs[nodo] = 3;
    if (regs[nodo_izq[nodo]] == 1)
      if (oper[nodo_izq[nodo]] == N_LDLP)
	regs[nodo] = 2;
  } else if (nodo_izq[nodo] == 0)
    regs[nodo] = 1;
  else if (nodo_der[nodo] == 0)
    regs[nodo] = regs[nodo_izq[nodo]];
  else {
    min = regs[nodo_izq[nodo]];
    max = regs[nodo_der[nodo]];
    if (min > max)
      max = min;
    else if (min == max)
      max = max + 1;
    regs[nodo] = max;
  }
}

gen_oper(oper, rev)
  int oper, rev;
{
  if (oper == N_OR)
    ol("or");
  else if (oper == N_XOR)
    ol("xor");
  else if (oper == N_AND)
    ol("and");
  else if (oper == N_IGUAL) {
    ol("diff");
    ol("eqc 0");
  } else if (oper == N_SUMA)
    ol("bsub");
  else if (oper == N_MUL)
    ol("prod");
  else if (oper == N_NEG) {
    ol("not");
    ol("adc 1");
  } else if (oper == N_COM)
    ol("not");
  else if (oper == N_NOT)
    ol("eqc 0");
  else if (oper == N_CBYTE)
    ol("lb");
  else if (oper == N_CPAL)
    ol("ldnl 0");
  else if (oper == N_SMAYOR) {
    if (rev == 0)
      ol("rev");
    ol("mint");
    ol("xor");
    ol("rev");
    ol("mint");
    ol("xor");
    ol("gt");
  } else {
    if (rev)
      ol("rev");
    if (oper == N_MAYOR)
      ol("gt");
    else if (oper == N_CD)
      ol("shr");
    else if (oper == N_CI)
      ol("shl");
    else if (oper == N_RESTA)
      ol("diff");
    else if (oper == N_DIV)
      ol("div");
    else if (oper == N_MOD)
      ol("rem");
    else if (oper == N_IXP)
      ol("wsub");
  }
}

gen_codigo(nodo)
  int nodo;
{
  int temp, conteo, pals, par, rev, op, req, reqres;
  int regb, regc;

  if((nodo != raiz_arbol) | (usaexpr == SI)) reqres = SI;
  else reqres = NO;
  op = oper[nodo];
  if ((op == N_FUNC) | (op == N_FUNCI)) {
    pals = conteo = 0;
    regb = regc = 0;
    temp = nodo_izq[nodo];
    while (temp) {
      if(regb == 0) regb = nodo_izq[temp];
      else if(regc == 0) regc = nodo_izq[temp];
      else {
        ++pals;
        ++conteo;
      }
      temp = esp[temp];
    }
    Zsp = modstk(Zsp - pals);
    if(conteo) {
      temp = esp[esp[nodo_izq[nodo]]];
      par = 0;
      while (conteo--) {
        gen_codigo(nodo_izq[temp]);
        ins("stl ", par++);
        temp = esp[temp];
      }
    }
    if (oper[nodo] == N_FUNC) {
      if(regc == 0) {
        if(regb) gen_codigo(regb);
        }
      else {
        if ((regs[regc] >= regs[regb]) &
            (regs[regb] < 3)) {
          gen_codigo(regc);
          gen_codigo(regb);
        } else if ((regs[regb] > regs[regc]) &
                   (regs[regc] < 3)) {
          gen_codigo(regb);
          gen_codigo(regc);
          ol("rev");
        } else {
          gen_codigo(regb);
          zpush();
          gen_codigo(regc);
          zpop();
        }
      }
      ins("ldl ", 1 - Zsp);
      zcall(esp[nodo]);
    } else {
      Zsp = modstk(Zsp - 4);
      if(regb) {
        gen_codigo(regb);
        ol("stl 2");
        }
      if(regc) {
        gen_codigo(regc);
        ol("stl 3");
        }
      gen_codigo(esp[nodo]);
      ins("ldl ", 1 - Zsp);
      ol("ldc 3");
      ol("ldpi");
      ol("stl 0");
      ol("stl 1");
      ol("gcall");
      Zsp = Zsp + 4;
    }
    Zsp = modstk(Zsp + pals);
    return;
  }
  if ((op == N_GBYTE) | (op == N_GPAL)) {
    if(reqres) req = 2;
    else req = 3;
    if (regs[nodo_der[nodo]] < req) {
      gen_codigo(nodo_izq[nodo]);
      if(reqres) ol("dup");
      if ((op == N_GPAL) & (oper[nodo_der[nodo]] == N_LDLP))
	oper[nodo_der[nodo]] = N_STL;
      else if ((op == N_GPAL) & (oper[nodo_der[nodo]] == N_LDNLP))
	oper[nodo_der[nodo]] = N_STNL;
      else {
	gen_codigo(nodo_der[nodo]);
	if(op == N_GPAL)
          ol("stnl 0");
        else
          ol("sb");
        return;
      }
      gen_codigo(nodo_der[nodo]);
      return;
    }
    gen_codigo(nodo_der[nodo]);
    if(reqres) req = SI;
    else if (regs[nodo_izq[nodo]] < 3) req = NO;
    else req = SI;
    if(req) {
      zpush();
      gen_codigo(nodo_izq[nodo]);
      ol("dup");
      zpop();
    } else {
      gen_codigo(nodo_izq[nodo]);
      ol("rev");
    }
    if (op == N_GPAL)
      ol("stnl 0");
    else
      ol("sb");
    return;
  }
  if (op == N_APFUNC) {
    ot("ldc ");
    outname(esp[nodo]);
    outasm("-");
    printlabel(temp = getlabel());
    nl();
    ol("ldpi");
    printlabel(temp);
    col();
    nl();
    return;
  }
  if (op == N_CONST) {
    ins("ldc ", esp[nodo]);
    return;
  }
  if (op == N_LIT) {
    ot("ldc ");
    printlabel(litlab);
    outasm("-");
    printlabel(temp = getlabel());
    outasm("+");
    outdec(esp[nodo]);
    nl();
    ol("ldpi");
    printlabel(temp);
    col();
    nl();
    return;
  }
  if (op == N_LDLP) {
    ins("ldlp ", esp[nodo] - Zsp);
    return;
  }
  if (op == N_LDL) {
    ins("ldl ", esp[nodo] - Zsp);
    return;
  }
  if (op == N_STL) {
    ins("stl ", esp[nodo] - Zsp);
    return;
  }
  if ((op == N_INC) | (op == N_PINC)) {
    if (regs[nodo] == 2) {
      oper[nodo_izq[nodo]] = N_LDL;
      gen_codigo(nodo_izq[nodo]);
    } else {
      gen_codigo(nodo_izq[nodo]);
      ol("dup");
      ol("ldnl 0");
    }
    if (op == N_PINC)
      if(reqres)
        ol("dup");
    ins("adc ", esp[nodo]);
    if (op == N_INC)
      if(reqres)
        ol("dup");
    if (regs[nodo] == 2) {
      oper[nodo_izq[nodo]] = N_STL;
      gen_codigo(nodo_izq[nodo]);
    } else if (reqres) {
      ol("pop");
      ol("pop");
      ol("stnl 0");
    } else {
      ol("rev");
      ol("stnl 0");
    }
    return;
  }
  if (op == N_CPAL) {
    if (oper[nodo_izq[nodo]] == N_LDLP) {
      oper[nodo_izq[nodo]] = N_LDL;
      op = N_NULO;
    } else if (oper[nodo_izq[nodo]] == N_LDNLP) {
      oper[nodo_izq[nodo]] = N_LDNL;
      op = N_NULO;
    }
  }
  rev = 0;
  if (nodo_der[nodo]) {
    if ((regs[nodo_izq[nodo]] >= regs[nodo_der[nodo]]) &
	(regs[nodo_der[nodo]] < 3)) {
      gen_codigo(nodo_izq[nodo]);
      gen_codigo(nodo_der[nodo]);
    } else if ((regs[nodo_der[nodo]] > regs[nodo_izq[nodo]]) &
	       (regs[nodo_izq[nodo]] < 3)) {
      gen_codigo(nodo_der[nodo]);
      gen_codigo(nodo_izq[nodo]);
      rev = 1;
    } else {
      gen_codigo(nodo_der[nodo]);
      zpush();
      gen_codigo(nodo_izq[nodo]);
      zpop();
    }
  } else
    gen_codigo(nodo_izq[nodo]);
  if (op == N_CIGUAL) {
    ins("eqc ", esp[nodo]);
    return;
  }
  if (op == N_CSUMA) {
    if(esp[nodo]) ins("adc ", esp[nodo]);
    return;
  }
  if (op == N_LDNLP) {
    if(esp[nodo]) ins("ldnlp ", esp[nodo]);
    return;
  }
  if (op == N_LDNL) {
    ins("ldnl ", esp[nodo]);
    return;
  }
  if (op == N_STNL) {
    ins("stnl ", esp[nodo]);
    return;
  }
  gen_oper(op, rev);
}

ins(codigo, valor) char *codigo; int valor; {
  outasm(codigo);
  outdec(valor);
  nl();
}

/* Comienza una linea de comentarios para el ensamblador */
comment()
{
  outbyte(';');
}

/* Pone el prologo para el codigo generado. */
header()
{
  comment();
  outstr(BANNER);
  nl();
  comment();
  outstr(VERSION);
  nl();
  comment();
  outstr(AUTHOR);
  nl();
  comment();
  nl();
  ol("COMIENZO:");
  ol("j INICIO");
}

/* Pone el epilogo para el codigo generado. */
trailer()
{
  nl();
  comment();
  outstr(" Fin de compilacion");
  nl();
  outasm("INICIO");
  col();
  nl();
  ins("ajw ", -posglobal);
  if (posglobal > 2) {
    ol("ldlp 2");
    ol("stl 0");
    ins("ldc ", posglobal - 2);
    ol("stl 1");
    outasm("INICIO2");
    col();
    nl();
    ol("ldc 0");
    ol("ldl 0");
    ol("stnl 0");
    ol("ldl 0");
    ol("adc 4");
    ol("stl 0");
    ol("ldl 1");
    ol("adc -1");
    ol("stl 1");
    ol("ldl 1");
    ol("eqc 0");
    ol("cj INICIO2");
  }
  ol("ldlp 0");
  ol("call qmain");
  ins("ajw ", posglobal);
  ol("ret");
}

/*
** Imprime un nombre que no entre en conflicto con las
** palabras reservadas del ensamblador.
*/
outname(sname)
  char *sname;
{
  outasm("q");
  outasm(sname);
}

/* Pone el registro A en la pila */
zpush()
{
  ol("ajw -1");
  ol("stl 0");
  --Zsp;
}

/* Pone el tope de la pila en el reg. A */
zpop()
{
  ol("ldl 0");
  ol("ajw 1");
  ++Zsp;
}

/* Llama a la funci¢n especificada */
zcall(sname)
  char *sname;
{
  ot("call ");
  outname(sname);
  nl();
}

/* Retorna de una funci¢n */
zret()
{
  ol("ret");
}

/* Salta a la etiqueta interna especificada */
jump(label)
  int label;
{
  ins("j c", label);
}

/* Prueba el registro primario y salta si es falso */
testjump(label)
  int label;
{
  ins("cj c", label);
}

/* Retorna la siguiente etiqueta interna disponible */
getlabel()
{
  return (++nxtlab);
}

/* Imprime el n£mero especificado c¢mo una etiqueta */
printlabel(label)
  int label;
{
  outasm("c");
  outdec(label);
}

col()
{
  outbyte(58);
}

/* Seudo-operacion para definir un byte */
defbyte()
{
  ot("db ");
}

/* Modifica la posici¢n de la pila */
modstk(newsp)
  int newsp;
{
  int k;

  if (k = newsp - Zsp)
    ins("ajw ", k);
  return newsp;
}

/* Fin del Compilador de Mini-C */

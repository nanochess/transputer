/*
** Ensamblador G10.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 14 de junio de 1995.
** Revisión: 19 de junio de 1995.
** Revisión: 12 de julio de 1995. Corrección de un defecto en la creación del
**                                primer archivo temporal.
** Revisión: 13 de julio de 1995. Corrección de un defecto en el ensamblaje
**                                de muchos archivos separados.
** Revisión: 18 de julio de 1995. Soporte para nombres largos de archivo.
** Revisión: 20 de julio de 1995. Coloca los archivos temporales en el
**                                disco RAM, y luego copia al destino.
** Revisión: 26 de julio de 1995. Soporte para dos metodos de almacenar
**                                las etiquetas, un metodo rápido pero que
**                                ocupa más memoria, y un metodo lento.
** Revisión: 2 de agosto de 1995. Hago que use los operadores && y ||, esto
**                                lo hace un poco más rápido.
** Revisión: 12 de agosto de 1995. Soporte para un nuevo metodo de almacenar
**                                 las etiquetas, que ocupa menos memoria.
** Revisión: 24 de agosto de 1995. Correcciones menores de los nemonicos.
** Revisión: 8 de septiembre de 1995. Se detiene en el paso de enlace si
**                                    detecta etiquetas indefinidos, no hace
**                                    el paso de enlace si encontró errores
**                                    en el primer paso, también se comenta
**                                    un poco el codigo fuente, agrego una
**                                    variable (errores_detectados).
** Revisión: 22 de septiembre de 1995. Corrección de un defecto que cuando
**                                     ocurria un error, algunas veces no
**                                     cerraba correctamente los archivos.
** Revisión: 26 de septiembre de 1995. Hago que las etiquetas y los
**                                     indefinidos ocupen menos espacio en
**                                     memoria.
** Revisión: 27 de septiembre de 1995. Mejores estéticas y de rápidez.
** Revisión: 22 de noviembre de 1995. Mejoras para rápidez.
** Revisión: 2 de diciembre de 1995. Corrección de defectos en copia_archivo().
** Revisión: 29 de enero de 1996. Soporte para caracteres en expresiones y
**                                cadenas en def_byte().
** Revisión: 29 de enero de 1996. Ahora soporta espacios en expresiones.
** Revisión: 29 de enero de 1996. Embellecimientos menores.
** Revisión: 20 de junio de 1996. Corrección de un defecto que hacia que no
**                                detectara el limite de la memoria.
** Revisión: 20 de junio de 1996. Reducción en el espacio que ocupa un
**                                indefinido.
*/

/*
** Este ensamblador sólo puede ensamblar archivos de hasta 16 megabytes.
*/

#define NULL     0
#define EOF     (-1)
#define TAM_LIN  128
#define NO       0
#define SI       1

#define TAM_BUF  512

#define TAM_MEM  87040

#define TAM_ETIQ 6   /* 1 byte. 0= Es una dirección. 1= Es un dato. */
                     /* 4 bytes de dato/dirección. */
                     /* 1 byte para el tamaño del nombre. */
                     /* x bytes para el nombre (terminado con el bit 7 a 1) */
#define TAM_INDEF 4  /* 1 byte. Indica el codigo de instrucción (0-15) */
                     /*         16= es para un db, 17= es para un dw. */
                     /* 3 bytes de dirección donde debe ir */
                     /* x bytes de la expresión (terminada con el bit 7 a 1) */
                     /* 1 byte para el tamaño de la expresión. */

int err;
int num_arch;
int archivo;
char *pos_linea;

char preins[8];
char oriins[8];
int acumula;

int temp1;
int temp2;
int fin_de_archivo;
int linea_actual;
int total_lineas;
int num_pasos;
int bytes_generados;
int disponible;
int pos_ens;
char *limite;
char *pos_global;
char *primer_etiq;
char *final_etiq;
char *ultima_etiq;
int num_etiq;
char *primer_indef;
char *ultimo_indef;
int num_indef;
int ptemp1, ptemp2;
char nom[TAM_LIN];
char linea[TAM_LIN];
char separa[TAM_LIN];
char separa2[TAM_LIN];
char etiq_indef[TAM_LIN];
char btemp1[TAM_BUF];
char btemp2[TAM_BUF];
int tabla[429];
char t[TAM_MEM];

int errores_detectados;

/*
** La unidad A: siempre es disco flexible.
** La unidad B: siempre es el disco RAM.
*/
#define TEMPORAL "B:/temporal.ens.1"
#define TEMP2    "B:/temporal.ens.2"

main()
{
  puts("\r\n");
  color(15);
  puts("Ensamblador G10  (c) Copyright 1995 Oscar Toledo G.\r\n");
  puts("\r\n");
  inicia1();   /* Las 16 instrucciones básicas */
  inicia2();   /* Las 16 operaciones básicas */
  inicia3();   /* La mitad del conjunto de instrucciones */
  inicia4();   /* La otra mitad del conjunto */
  inicia5();   /* Instrucciones de microcodigo de la FPU */
  num_arch = 0;
  limite = t + TAM_MEM;
  disponible = 1;
  primer_etiq = t;
  ultima_etiq = NULL;
  final_etiq = t;
  num_etiq = 0;
  primer_indef = t + TAM_MEM;
  ultimo_indef = t + TAM_MEM;
  num_indef = 0;
  pos_ens = 0;
  temp1 = NULL;
  temp2 = NULL;
  archivo = NULL;
  while (1) {
    color(10);
    puts("\r                          \r");
    puts("Archivo de entrada > ");
    lee_linea(nom, TAM_LIN);
    if (*nom == 0) {
      if (num_arch == 0) {
        color(15);
        puts("\r\nEnsamblaje cancelado.\r\n");
        sale();
      }
      vtemp1();
      linea_actual = 0;
      puts("\r\nArchivo de salida > ");
      lee_linea(nom, TAM_LIN);
      if (*nom == 0) {
        color(15);
        puts("\r\nEnsamblaje cancelado.\r\n");
        sale();
      }
      puts("\r\n");
      enlaza();
      puts("\r                                             \r");
      color(11);
      decimal(bytes_generados);
      puts(" bytes generados.\r\n");
      decimal(total_lineas);
      puts(" líneas ensambladas.\r\n");
      decimal(num_etiq);
      puts(" etiquetas definidas.\r\n");
      decimal(TAM_MEM - (ultimo_indef - final_etiq));
      puts(" bytes de memoria usados.\r\n");
      exit(1);
    }
    archivo = fopen(nom, "r");
    if (archivo == NULL) {
      color(15);
      puts("Archivo inexistente.\r\n");
      continue;
    }
    if (temp1 == NULL) {
      temp1 = fopen(TEMPORAL, "w");
      if (temp1 == NULL) {
        puts("Error al crear el archivo temporal.\r\n");
        sale();
      }
      ptemp1 = 0;
    }
    fin_de_archivo = NO;
    linea_actual = 0;
    while (!fin_de_archivo)
      ensambla();
    fclose(archivo);
    total_lineas += linea_actual;
    archivo = NULL;
    ++num_arch;
  }
}

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

sale()
{
  if (temp1 != NULL)
    fclose(temp1);
  if (temp2 != NULL)
    fclose(temp2);
  if (archivo != NULL)
    fclose(archivo);
  remove(TEMPORAL);
  remove(TEMP2);
  exit(1);
}

falta_memoria()
{
  color(15);
  puts("\r\nNo hay memoria.\r\n");
  sale();
}

/*
** Separa cada línea de entrada en componentes.
*/
separa_componente()
{
  char *sep;

  while (*pos_linea == ' ')
    ++pos_linea;
  sep = separa;
  while (*pos_linea && *pos_linea != ' ')
    *sep++ = *pos_linea++;
  *sep = 0;
  while (*pos_linea == ' ')
    ++pos_linea;
}

/*
** Ensambla un archivo, todavía no hace el enlace.
*/
ensambla()
{
  int instruccion, etiq;
  char *final, *ap;

  obtiene_linea();
  if (linea_actual % 256 == 0 || fin_de_archivo) {
    color(11);
    putchar('\r');
    decimal(linea_actual);
    puts(" líneas procesadas.");
  }
  etiq = NO;
  while (1) {
    separa_componente();
    if (*separa == 0)
      break;
    if (*separa == ';')
      break;
    final = separa;
    while (*final)
      ++final;
    if (final != separa && *(final - 1) == ':') {
      *(final - 1) = 0;
      if (isalpha(*separa) || *separa == '_') {
        etiqueta();
        etiq = SI;
      } else
        error("Etiqueta inválida");
    } else if (compara("db", separa)) {
      def_byte();
      if (*pos_linea)
        error_extras();
      break;
    } else if (compara("dw", separa)) {
      def_pal32();
      if (*pos_linea)
        error_extras();
      break;
    } else if (compara("ds", separa)) {
      def_espacio();
      if (*pos_linea)
        error_extras();
      break;
    } else if (compara("equ", separa)) {
      if (etiq) {
        def_equiv();
        if (*pos_linea)
          error_extras();
      } else
        error("El equ debe ir con una etiqueta");
      break;
    } else {
      instruccion = 0;
      while (instruccion < 429) {
        ap = tabla[instruccion++];
        if (*ap == '?')
          continue;
        if (compara(ap, separa)) {
          if (--instruccion < 16)
            ins_op(instruccion);
          else if (instruccion < 272)
            ins_sim(instruccion - 16);
          else if (instruccion < 429)
            ins_ext(instruccion - 272);
          if (*pos_linea)
            error_extras();
          break;
        }
      }
      if (instruccion == 429)
        error("Instrucción indefinida");
      break;
    }
  }
  if (fin_de_archivo)
    return;
}

error_extras()
{
  error("Caracteres extras");
}

strlen(cad)
  char *cad;
{
  char *ori;

  ori = cad;
  while (*cad)
    ++cad;
  return (cad - ori);
}

strcpy(des, ori)
  char *des, *ori;
{
  while (*des++ = *ori++) ;
}

strcat(des, ori)
  char *des, *ori;
{
  while (*des)
    ++des;
  strcpy(des, ori);
}

strcmp(ori, des)
  char *ori, *des;
{
  int c;

  while (1) {
    if ((c = *ori++) < *des)
      return -1;
    if (c > *des++)
      return 1;
    if (c == 0)
      return 0;
  }
}

igualdad(ori, des)
  char *ori, *des;
{
  while (*ori == *des++)
    if (*ori++ & 0x80)
      return 1;
  return 0;
}

/*
** Procesa etiquetas.
**
** Detecta etiquetas redefinidas.
*/
etiqueta()
{
  char *nueva_etiq;
  char *pos, *pos1;
  int cuenta;

  nueva_etiq = separa + strlen(separa) - 1;
  *nueva_etiq = *nueva_etiq | 0x80;

  pos = primer_etiq;
  while (pos != final_etiq) {
    if (igualdad(separa, pos + 6)) {
      *nueva_etiq = *nueva_etiq & 0x7f;
      strcpy(separa2, "Etiqueta redefinida ");
      strcat(separa2, separa);
      error(separa2);
      return;
    }
    pos += *(pos + 5) + 6;
  }
  nueva_etiq = pos;
  *pos++ = 0;
  *pos++ = pos_ens;
  *pos++ = pos_ens >> 8;
  *pos++ = pos_ens >> 16;
  *pos++ = pos_ens >> 24;
  *pos++ = strlen(separa);
  pos1 = separa;
  while (*pos1)
    *pos++ = *pos1++;
  if ((final_etiq = pos) > ultimo_indef)
    falta_memoria();
  ultima_etiq = nueva_etiq;
  ++num_etiq;
}

/*
** Busca etiquetas.
*/
busca_etiq(nombre)
  char *nombre;
{
  char *pos, *pos2;

  pos2 = nombre + strlen(nombre) - 1;
  *pos2 = *pos2 | 0x80;

  pos = primer_etiq;
  while (pos != final_etiq) {
    if (igualdad(nombre, pos + 6)) {
      *pos2 = *pos2 & 0x7f;
      return pos;
    }
    pos += *(pos + 5) + 6;
  }
  *pos2 = *pos2 & 0x7f;
  return NULL;
}

/*
** Procesa instrucciones con operando.
*/
ins_op(ins)
  int ins;
{
  int valor;
  int arregla, salto;

  separa_componente();
  err = 0;
  pos_global = separa;
  valor = evalua_expresion();
  ignora_espacios();
  if (*pos_global)
    error_extras();
  if (err) {
    ag_indef(ins);
    etemp1(ins << 4);
    ++pos_ens;
    return;
  }
  if (ins == 0 || ins == 9 || ins == 10) {
    ag_indef(ins);
    arregla = 1;
    while (1) {
      acumula = 0;
      gen_ins(ins, salto = (valor - (pos_ens + arregla)));
      if (pos_ens + acumula + salto == valor)
        break;
      ++arregla;
    }
  } else {
    acumula = 0;
    gen_ins(ins, valor);
  }
  valor = 0;
  while (valor < acumula) {
    etemp1(preins[valor++]);
    ++pos_ens;
  }
}

/*
** Agrega un nuevo operando indefinido aún.
*/
ag_indef(clave)
  int clave;
{
  char *nuevo_indef;
  char *pos, *pos1;
  int u;

  u = TAM_INDEF + strlen(separa) + 1;
  pos = (ultimo_indef -= u);
  if (ultimo_indef < final_etiq)
    falta_memoria();
  *pos++ = clave;
  *pos++ = pos_ens;
  *pos++ = pos_ens >> 8;
  *pos++ = pos_ens >> 16;
  pos1 = separa;
  while (*pos1)
    *pos++ = *pos1++;
  *(pos-1) = *(pos-1) | 0x80;
  *pos++ = strlen(separa);
  ++num_indef;
}

isdigit(c)
  char c;
{
  return(c >= '0' && c <= '9');
}

isalpha(c)
  char c;
{
  return((c >= 'A' && c <= 'Z')
      || (c >= 'a' && c <= 'z'));
}

ignora_espacios()
{
  while (*pos_global == ' ')
    ++pos_global;
}

/*
** Evalua las expresiones.
*/
evalua_expresion()
{
  int valor1;

  valor1 = eval1();
  while (1) {
    ignora_espacios();
    if (*pos_global == '+') {
      ++pos_global;
      valor1 += eval1();
    } else if (*pos_global == '-') {
      ++pos_global;
      valor1 -= eval1();
    } else
      break;
  }
  return valor1;
}

eval1()
{
  ignora_espacios();
  if (*pos_global == '+')
    ++pos_global;
  else if (*pos_global == '-') {
    ++pos_global;
    return -eval2();
  }
  return eval2();
}

eval2()
{
  int valor1;

  valor1 = eval3();
  while (1) {
    ignora_espacios();
    if (*pos_global == '*') {
      ++pos_global;
      valor1 *= eval3();
    } else if (*pos_global == '/') {
      ++pos_global;
      valor1 /= eval3();
    } else if (*pos_global == '%') {
      ++pos_global;
      valor1 = valor1 % eval3();
    } else
      break;
  }
  return valor1;
}

eval3()
{
  char *ap;
  int valor;

  ignora_espacios();
  if (*pos_global == '(') {
    ++pos_global;
    valor = evalua_expresion();
    ignora_espacios();
    if (*pos_global != ')')
      error("Falta parentesis derecho");
    else
      ++pos_global;
    return valor;
  } else if (isdigit(*pos_global)) {
    if (toupper(*(pos_global + 1)) == 'X')
      return eval_hex();
    else
      return eval_dec();
  } else if (isalpha(*pos_global) || *pos_global == '_') {
    ap = separa2;
    while (*pos_global == '_' || isalpha(*pos_global) ||
          isdigit(*pos_global))
      *ap++ = *pos_global++;
    *ap = 0;
    if ((ap = busca_etiq(separa2)) != NULL) {
      valor = *(ap+1) | (*(ap+2) << 8) | (*(ap+3) << 16) | (*(ap+4) << 24);
      return valor;
    } else {
      strcpy(etiq_indef, separa2);
      err = 1;
      return 0;
    }
  } else if (*pos_global == '\'') {
    ++pos_global;
    valor = evalua_caracter();
    if (*pos_global != ')')
      error("Falta parentesis derecho");
    else
      ++pos_global;
    return valor;
  } else
    error("error de sintaxis");
}

evalua_caracter()
{
  int valor;

  if (*pos_global != 92)
    return *pos_global++;
  switch (*++pos_global) {
    case 'b': valor = 8;
              break;
    case 'r': valor = 13;
              break;
    case 'f': valor = 12;
              break;
    case 'n': valor = 10;
              break;
    case 't': valor = 9;
              break;
    case '0': valor = 0;
              break;
    default : valor = *pos_global;
              break;
  }
  ++pos_global;
  return valor;
}

isxdigit(c)
  int c;
{
  return ((c>='0' && c<='9') ||
          (c>='a' && c<='f') ||
          (c>='A' && c<='F'));
}

eval_hex()
{
  int c;
  int valor;

  valor = 0;
  pos_global = pos_global + 2;
  while (isxdigit(*pos_global)) {
    c = toupper(*pos_global++) - '0';
    if (c > 9)
      c = c - 7;
    valor = (valor << 4) | c;
  }
  return valor;
}

eval_dec()
{
  int valor;

  valor = 0;
  while (isdigit(*pos_global))
    valor = valor * 10 + (*pos_global++ - '0');
  return valor;
}

/*
** Genera una instrucción con valor, usa los prefijos optimos.
*/
gen_ins(oper, valor)
  int oper, valor;
{
  if (valor < 0)
    gen_ins(6, ~valor >> 4);
  else if(valor >= 16)
    gen_ins(2, valor >> 4);
  preins[acumula++] = (oper << 4) | (valor & 15);
}

/*
** Instrucciones simples.
*/
ins_sim(ins)
  int ins;
{
  if (ins > 15) {
    etemp1(0x20 + (ins >> 4));
    ++pos_ens;
  }
  etemp1(0xf0 + (ins & 15));
  ++pos_ens;
}

/*
** Instrucciones de microcodigo de la FPU.
*/
ins_ext(ins)
  int ins;
{
  if (ins > 15) {
    etemp1(0x20 + (ins >> 4));
    ++pos_ens;
  }
  etemp1(0x40 + (ins & 15));
  ++pos_ens;
  etemp1(0x2a);
  ++pos_ens;
  etemp1(0xfb);
  ++pos_ens;
}

/*
** Procesa la seudo-operacion db.
*/
def_byte()
{
  int valor;

  separa_componente();
  pos_global = separa;
  while (1) {
    err = 0;
    ignora_espacios();
    if (*pos_global == '"') {
      ++pos_global;
      while (*pos_global && *pos_global != '"') {
        etemp1(evalua_caracter());
        ++pos_ens;
      }
      if (*pos_global == '"')
        ++pos_global;
      else
        error("Faltan comillas al final");
    } else {
      valor = evalua_expresion();
      if (err)
        ag_indef(16);
      etemp1(valor & 255);
      ++pos_ens;
    }
    ignora_espacios();
    if (*pos_global != ',') {
      if (*pos_global)
        error("Falta una coma");
      return;
    }
    ++pos_global;
  }
}

/*
** Procesa la seudo-operacion dw.
*/
def_pal32()
{
  int valor;

  separa_componente();
  pos_global = separa;
  while (1) {
    err = 0;
    valor = evalua_expresion();
    ignora_espacios();
    if (err)
      ag_indef(17);
    etemp1(valor & 255);
    ++pos_ens;
    etemp1((valor >> 8) & 255);
    ++pos_ens;
    etemp1((valor >> 16) & 255);
    ++pos_ens;
    etemp1((valor >> 24) & 255);
    ++pos_ens;
    if (*pos_global != ',') {
      if (*pos_global)
        error("Falta una coma");
      return;
    }
    ++pos_global;
  }
}

/*
** Procesa la seudo-operacion ds.
*/
def_espacio()
{
  int valor;

  separa_componente();
  pos_global = separa;
  err = 0;
  valor = evalua_expresion();
  ignora_espacios();
  if (*pos_global)
    error_extras();
  if (err) {
    error("Se requiere un valor definido");
    return;
  }
  while (valor--) {
    etemp1(0);
    ++pos_ens;
  }
}

/*
** Procesa la seudo-operacion equ.
*/
def_equiv()
{
  int valor;

  separa_componente();
  pos_global = separa;
  err = 0;
  valor = evalua_expresion();
  ignora_espacios();
  if (*pos_global)
    error_extras();
  if (err) {
    error("Se requiere un valor definido");
    return;
  }
  if (ultima_etiq == NULL) {
    error("Error interno");
    return;
  }
  *ultima_etiq = 1;
  *(ultima_etiq + 1) = valor;
  *(ultima_etiq + 2) = valor >> 8;
  *(ultima_etiq + 3) = valor >> 16;
  *(ultima_etiq + 4) = valor >> 24;
}

compara(ins, separa)
  char *ins, *separa;
{
  while (*ins == *separa++)
    if (*ins++ == 0)
      return 1;
  return 0;
}

error(info)
  char *info;
{
  color(15);
  if (linea_actual == 0)
    puts(" ");
  puts(info);
  if (linea_actual) {
    puts(" en la línea ");
    decimal(linea_actual);
  }
  puts("\r\n");
  ++errores_detectados;
}

obtiene_linea()
{
  int conteo, car;

  conteo = 0;
  while (1) {
    if ((car = fgetc(archivo)) == EOF) {
      fin_de_archivo = SI;
      break;
    }
    if (car == '\r') continue;
    if (car == '\n') break;
    if (car == '\t') car = ' ';
    if (conteo != TAM_LIN - 1)
      linea[conteo++] = car;
  }
  linea[conteo] = 0;
  if (!fin_de_archivo)
    ++linea_actual;
  pos_linea = linea;
}

int algo;

enlaza()
{
  num_pasos = 0;
  if (errores_detectados) {
    color(15);
    error("No se puede hacer el paso de enlace");
    sale();
  }
  while (1) {
    temp1 = fopen(TEMPORAL, "r");
    ptemp1 = TAM_BUF;
    temp2 = fopen(TEMP2, "w");
    ptemp2 = 0;
    if (temp2 == NULL) {
      color(15);
      error("Error de acceso al disco");
      sale();
    }
    algo = NO;
    paso();
    vtemp2();
    fclose(temp1);
    temp1 = NULL;
    if (errores_detectados) {
      color(15);
      error("No se puede seguir con el paso de enlace");
      sale();
    }
    if (!algo) {
      remove(TEMPORAL);
      remove(nom);
      copia_archivo(TEMP2, nom);
      break;
    }
    temp1 = fopen(TEMP2, "r");
    ptemp1 = TAM_BUF;
    temp2 = fopen(TEMPORAL, "w");
    ptemp2 = 0;
    if (temp2 == NULL) {
      color(15);
      error("Error de acceso al disco");
      sale();
    }
    algo = NO;
    paso();
    vtemp2();
    fclose(temp1);
    temp1 = NULL;
    if (errores_detectados) {
      color(15);
      error("No se puede seguir con el paso de enlace");
      sale();
    }
    if (!algo) {
      remove(TEMP2);
      remove(nom);
      copia_archivo(TEMPORAL, nom);
      break;
    }
  }
}

copia_archivo(original, destino)
  char *original, *destino;
{
  int arc1, arc2;
  int byte;

  arc1 = fopen(original, "r");
  if (arc1 == NULL) {
    error("Error de acceso al disco");
    sale();
  }
  arc2 = fopen(destino, "w");
  if (arc2 == NULL) {
    fclose(arc1);
    error("Error de acceso al disco");
    sale();
  }
  bytes_generados = 0;
  while ((byte = fgetc(arc1)) != EOF) {
    if (fputc(byte, arc2) == EOF) {
      fclose(arc1);
      fclose(arc2);
      remove(destino);
      error("Error de acceso al disco");
      sale();
    }
    ++bytes_generados;
  }
  fclose(arc1);
  fclose(arc2);
  remove(original);
}

paso()
{
  char *lista, *lista2, *extra;
  int inicio;
  int ultimo;
  int pos;
  int valor, byte;
  int acu, ins;
  int cuantos;
  int a, porcentaje;

  ++num_pasos;
  puts("\r                                                          \r");

  ultimo = pos_ens;
  pos_ens = 0;

  cuantos = 0;

  inicio = 0;

  porcentaje = -1;

  lista = primer_indef;

  a = 0;
  while (lista != ultimo_indef) {
    lista -= *(lista - 1) + (TAM_INDEF + 1);
    pos = *(lista + 1) | (*(lista + 2) << 8) | (*(lista + 3) << 16);
    copia(inicio, pos);

    ins = *lista;

    if (ins == 16) {
      byte = ltemp1();
      inicio = pos + 1;
    } else if (ins == 17) {
      byte = ltemp1();
      byte = ltemp1();
      byte = ltemp1();
      byte = ltemp1();
      inicio = pos + 4;
    } else {
      acu = 0;
      while (1) {
        oriins[acu++] = byte = ltemp1();
        if ((byte & 0xf0) == 0x20)
          continue;
        if ((byte & 0xf0) == 0x60)
          continue;
        break;
      }
      inicio = pos + acu;
    }

    *(lista + 1) = pos_ens;
    *(lista + 2) = pos_ens >> 8;
    *(lista + 3) = pos_ens >> 16;

    err = 0;
    extra = lista + 4;
    lista2 = linea;
    while ((*extra & 0x80) == 0)
      *lista2++ = *extra++;
    *lista2++ = *extra & 0x7f;
    *lista2 = 0;
    pos_global = linea;
    valor = evalua_expresion();
    if (err) {
      strcpy(separa2, "Etiqueta indefinida ");
      strcat(separa2, etiq_indef);
      error(separa2);
      valor = 0;
    }

    if (ins == 16) {
      etemp2(valor);
      ++pos_ens;
    } else if (ins == 17) {
      etemp2(valor);
      ++pos_ens;
      etemp2(valor >> 8);
      ++pos_ens;
      etemp2(valor >> 16);
      ++pos_ens;
      etemp2(valor >> 24);
      ++pos_ens;
    } else if (ins == 0 || ins == 9 || ins == 10) {
      acumula = 0;
      gen_ins(ins, valor - (pos_ens + acu));
    } else {
      acumula = 0;
      gen_ins(ins, valor);
    }
    if (ins != 16 && ins != 17) {
      valor = 0;
      while (valor < acumula) {
        etemp2(preins[valor++]);
        ++pos_ens;
      }
      if (acumula != acu) {
        algo = SI;
        lista2 = primer_etiq;
        while (lista2 != final_etiq) {
          if (*lista2 == 0) {
            valor = *(lista2 + 1) | (*(lista2 + 2) << 8) |
                    (*(lista2 + 3) << 16) | (*(lista2 + 4) << 24);
            if (valor >= inicio + cuantos)
              valor = valor + (acumula - acu);
            *(lista2 + 1) = valor;
            *(lista2 + 2) = valor >> 8;
            *(lista2 + 3) = valor >> 16;
            *(lista2 + 4) = valor >> 24;
          }
          lista2 += *(lista2 + 5) + 6;
        }
        cuantos += acumula - acu;
      }
    }
    if ((++a * 100 / num_indef) != porcentaje) {
      color(11);
      puts("\rPaso ");
      decimal(num_pasos);
      puts(": Expansión de instrucciones al ");
      decimal(porcentaje = a * 100 / num_indef);
      putchar('%');
    }
  }

  copia(inicio, ultimo);
}

copia(inicio, final)
  int inicio, final;
{
  while (inicio++ < final) {
    etemp2(ltemp1());
    ++pos_ens;
  }
}

toupper(car)
  int car;
{
  if (car >= 'a' && car <= 'z')
    return car - 32;
  else
    return car;
}

lee_linea(pos, tam)
  char *pos;
  int tam;
{
  char *ahora;
  int car;

  ahora = pos;
  while (1) {
    car = getchar();
    if (car == 8) {
      if (ahora == pos)
        continue;
      putchar(8);
      --ahora;
      continue;
    } else if(car == 13) {
      puts("\r\n");
      *ahora = 0;
      return;
    } else {
      if (ahora == pos + tam - 1)
        continue;
      putchar(car);
      *ahora++ = car;
    }
  }
}

etemp1(dato)
  int dato;
{
  btemp1[ptemp1++] = dato;
  if (ptemp1 == TAM_BUF) {
    ptemp1 = 0;
    while (ptemp1 < TAM_BUF) {
      if (fputc(btemp1[ptemp1++], temp1) == EOF) {
        color(15);
        error("Disco lleno");
        break;
      }
    }
    ptemp1 = 0;
  }
}

vtemp1()
{
  int conteo;

  conteo = 0;
  while (conteo < ptemp1) {
    if (fputc(btemp1[conteo++], temp1) == EOF) {
      color(15);
      error("Disco lleno");
      break;
    }
  }
  fclose(temp1);
  temp1 = NULL;
}

etemp2(dato)
  int dato;
{
  btemp2[ptemp2++] = dato;
  if (ptemp2 == TAM_BUF) {
    ptemp2 = 0;
    while (ptemp2 < TAM_BUF) {
      if (fputc(btemp2[ptemp2++], temp2) == EOF) {
        color(15);
        error("Disco lleno");
        break;
      }
    }
    ptemp2 = 0;
  }
}

vtemp2()
{
  int conteo;

  conteo = 0;
  while (conteo < ptemp2) {
    if (fputc(btemp2[conteo++], temp2) == EOF) {
      color(15);
      error("Disco lleno");
      break;
    }
  }
  fclose(temp2);
  temp2 = NULL;
}

ltemp1()
{
  if (ptemp1 == TAM_BUF) {
    ptemp1 = 0;
    while (ptemp1 < TAM_BUF)
      btemp1[ptemp1++] = fgetc(temp1);
    ptemp1 = 0;
  }
  return btemp1[ptemp1++];
}

decimal(numero)
  int numero;
{
  if (numero < 0) {
    putchar('-');
    if (numero < -9)
      decimal(-(numero / 10));
    putchar(-(numero % 10) + '0');
  } else {
    if (numero > 9)
      decimal(numero / 10);
    putchar((numero % 10) + '0');
  }
}

inicia1() {
  tabla[0] = "j";
  tabla[1] = "ldlp";
  tabla[2] = "pfix";
  tabla[3] = "ldnl";
  tabla[4] = "ldc";
  tabla[5] = "ldnlp";
  tabla[6] = "nfix";
  tabla[7] = "ldl";
  tabla[8] = "adc";
  tabla[9] = "call";
  tabla[10] = "cj";
  tabla[11] = "ajw";
  tabla[12] = "eqc";
  tabla[13] = "stl";
  tabla[14] = "stnl";
  tabla[15] = "opr";
}

inicia2() {
  tabla[16] = "rev";
  tabla[17] = "lb";
  tabla[18] = "bsub";
  tabla[19] = "endp";
  tabla[20] = "diff";
  tabla[21] = "add";
  tabla[22] = "gcall";
  tabla[23] = "in";
  tabla[24] = "prod";
  tabla[25] = "gt";
  tabla[26] = "wsub";
  tabla[27] = "out";
  tabla[28] = "sub";
  tabla[29] = "startp";
  tabla[30] = "outbyte";
  tabla[31] = "outword";
}

inicia3() {
  tabla[32] = "seterr";
  tabla[33] = "?";
  tabla[34] = "resetch";
  tabla[35] = "csub0";
  tabla[36] = "?";
  tabla[37] = "stopp";
  tabla[38] = "ladd";
  tabla[39] = "stlb";
  tabla[40] = "sthf";
  tabla[41] = "norm";
  tabla[42] = "ldiv";
  tabla[43] = "ldpi";
  tabla[44] = "stlf";
  tabla[45] = "xdble";
  tabla[46] = "ldpri";
  tabla[47] = "rem";

  tabla[48] = "ret";
  tabla[49] = "lend";
  tabla[50] = "ldtimer";
  tabla[51] =
  tabla[52] =
  tabla[53] =
  tabla[54] =
  tabla[55] =
  tabla[56] = "?";
  tabla[57] = "testerr";
  tabla[58] = "testpranal";
  tabla[59] = "tin";
  tabla[60] = "div";
  tabla[61] = "?";
  tabla[62] = "dist";
  tabla[63] = "disc";

  tabla[64] = "diss";
  tabla[65] = "lmul";
  tabla[66] = "not";
  tabla[67] = "xor";
  tabla[68] = "bcnt";
  tabla[69] = "lshr";
  tabla[70] = "lshl";
  tabla[71] = "lsum";
  tabla[72] = "lsub";
  tabla[73] = "runp";
  tabla[74] = "xword";
  tabla[75] = "sb";
  tabla[76] = "gajw";
  tabla[77] = "savel";
  tabla[78] = "saveh";
  tabla[79] = "wcnt";

  tabla[80] = "shr";
  tabla[81] = "shl";
  tabla[82] = "mint";
  tabla[83] = "alt";
  tabla[84] = "altwt";
  tabla[85] = "altend";
  tabla[86] = "and";
  tabla[87] = "enbt";
  tabla[88] = "enbc";
  tabla[89] = "enbs";
  tabla[90] = "move";
  tabla[91] = "or";
  tabla[92] = "csngl";
  tabla[93] = "ccnt1";
  tabla[94] = "talt";
  tabla[95] = "ldiff";

  tabla[96] = "sthb";
  tabla[97] = "taltwt";
  tabla[98] = "sum";
  tabla[99] = "mul";
  tabla[100] = "sttimer";
  tabla[101] = "stoperr";
  tabla[102] = "cword";
  tabla[103] = "clrhalterr";
  tabla[104] = "sethalterr";
  tabla[105] = "testhalterr";
  tabla[106] = "dup";
  tabla[107] = "move2dinit";
  tabla[108] = "move2dall";
  tabla[109] = "move2dnonzero";
  tabla[110] = "move2dzero";
  tabla[111] = "?";

  tabla[112] =
  tabla[113] =
  tabla[114] = "?";
  tabla[115] = "unpacksn";
  tabla[116] =
  tabla[117] =
  tabla[118] =
  tabla[119] =
  tabla[120] =
  tabla[121] =
  tabla[122] =
  tabla[123] = "?";
  tabla[124] = "postnormsn";
  tabla[125] = "roundsn";
  tabla[126] =
  tabla[127] = "?";

  tabla[128] = "?";
  tabla[129] = "ldinf";
  tabla[130] = "fmul";
  tabla[131] = "cflerr";
  tabla[132] = "crcword";
  tabla[133] = "crcbyte";
  tabla[134] = "bitcnt";
  tabla[135] = "bitrevword";
  tabla[136] = "bitrevnbits";
  tabla[137] = "pop";
  tabla[138] = "timerdisableh";
  tabla[139] = "timerdisablel";
  tabla[140] = "timerenableh";
  tabla[141] = "timerenablel";
  tabla[142] = "ldmemstartval";
  tabla[143] = "?";

  tabla[144] = "?";
  tabla[145] = "wsubdb";
  tabla[146] = "fpldnldbi";
  tabla[147] = "fpchkerr";
  tabla[148] = "fpstnldb";
  tabla[149] = "?";
  tabla[150] = "fpldnlsni";
  tabla[151] = "fpadd";
  tabla[152] = "fpstnlsn";
  tabla[153] = "fpsub";
  tabla[154] = "fpldnldb";
  tabla[155] = "fpmul";
  tabla[156] = "fpdiv";
  tabla[157] = "?";
  tabla[158] = "fpldnlsn";
  tabla[159] = "fpremfirst";
}

inicia4() {
  tabla[160] = "fpremstep";
  tabla[161] = "fpnan";
  tabla[162] = "fpordered";
  tabla[163] = "fpnotfinite";
  tabla[164] = "fpgt";
  tabla[165] = "fpeq";
  tabla[166] = "fpi32tor32";
  tabla[167] = "?";
  tabla[168] = "fpi32tor64";
  tabla[169] = "?";
  tabla[170] = "fpb32tor64";
  tabla[171] = "?";
  tabla[172] = "fptesterr";
  tabla[173] = "fprtoi32";
  tabla[174] = "fpstnli32";
  tabla[175] = "fpldzerosn";

  tabla[176] = "fpldzerodb";
  tabla[177] = "fpint";
  tabla[178] = "?";
  tabla[179] = "fpdup";
  tabla[180] = "fprev";
  tabla[181] = "?";
  tabla[182] = "fpldnladddb";
  tabla[183] = "?";
  tabla[184] = "fpldnlmuldb";
  tabla[185] = "?";
  tabla[186] = "fpldnladdsn";
  tabla[187] = "fpentry";
  tabla[188] = "fpldnlmulsn";
  tabla[189] =
  tabla[190] =
  tabla[191] = "?";

  tabla[192] = "?";
  tabla[193] = "break";
  tabla[194] = "clrj0break";
  tabla[195] = "setj0break";
  tabla[196] = "testj0break";
  tabla[197] =
  tabla[198] =
  tabla[199] =
  tabla[200] =
  tabla[201] =
  tabla[202] =
  tabla[203] =
  tabla[204] =
  tabla[205] =
  tabla[206] =
  tabla[207] = "?";

  tabla[208] =
  tabla[209] =
  tabla[210] =
  tabla[211] =
  tabla[212] =
  tabla[213] =
  tabla[214] =
  tabla[215] =
  tabla[216] =
  tabla[217] =
  tabla[218] =
  tabla[219] =
  tabla[220] =
  tabla[221] =
  tabla[222] =
  tabla[223] = "?";

  tabla[224] =
  tabla[225] =
  tabla[226] =
  tabla[227] =
  tabla[228] =
  tabla[229] =
  tabla[230] =
  tabla[231] =
  tabla[232] =
  tabla[233] =
  tabla[234] =
  tabla[235] =
  tabla[236] =
  tabla[237] =
  tabla[238] =
  tabla[239] = "?";

  tabla[240] =
  tabla[241] =
  tabla[242] =
  tabla[243] =
  tabla[244] =
  tabla[245] =
  tabla[246] =
  tabla[247] =
  tabla[248] =
  tabla[249] =
  tabla[250] =
  tabla[251] =
  tabla[252] =
  tabla[253] =
  tabla[254] =
  tabla[255] = "?";

  tabla[256] =
  tabla[257] =
  tabla[258] =
  tabla[259] =
  tabla[260] =
  tabla[261] =
  tabla[262] =
  tabla[263] =
  tabla[264] =
  tabla[265] =
  tabla[266] =
  tabla[267] =
  tabla[268] =
  tabla[269] =
  tabla[270] =
  tabla[271] = "?";
}

inicia5() {
  tabla[272] = "?";
  tabla[273] = "fpusqrtfirst";
  tabla[274] = "fpusqrtstep";
  tabla[275] = "fpusqrtlast";
  tabla[276] = "fpurp";
  tabla[277] = "fpurm";
  tabla[278] = "fpurz";
  tabla[279] = "fpur32tor64";
  tabla[280] = "fpur64tor32";
  tabla[281] = "fpuexpdec32";
  tabla[282] = "fpuexpinc32";
  tabla[283] = "fpuabs";
  tabla[284] = "?";
  tabla[285] = "fpunoround";
  tabla[286] = "fpuchki32";
  tabla[287] = "fpuchki64";

  tabla[288] = "?";
  tabla[289] = "fpudivby2";
  tabla[290] = "fpumulby2";
  tabla[291] =
  tabla[292] =
  tabla[293] =
  tabla[294] =
  tabla[295] =
  tabla[296] =
  tabla[297] =
  tabla[298] =
  tabla[299] =
  tabla[300] =
  tabla[301] =
  tabla[302] =
  tabla[303] = "?";

  tabla[304] =
  tabla[305] = "?";
  tabla[306] = "fpurn";
  tabla[307] = "fpuseterr";
  tabla[308] =
  tabla[309] =
  tabla[310] =
  tabla[311] =
  tabla[312] =
  tabla[313] =
  tabla[314] =
  tabla[315] =
  tabla[316] =
  tabla[317] =
  tabla[318] =
  tabla[319] = "?";

  tabla[320] =
  tabla[321] =
  tabla[322] =
  tabla[323] =
  tabla[324] =
  tabla[325] =
  tabla[326] =
  tabla[327] =
  tabla[328] =
  tabla[329] =
  tabla[330] =
  tabla[331] =
  tabla[332] =
  tabla[333] =
  tabla[334] =
  tabla[335] = "?";

  tabla[336] =
  tabla[337] =
  tabla[338] =
  tabla[339] =
  tabla[340] =
  tabla[341] =
  tabla[342] =
  tabla[343] =
  tabla[344] =
  tabla[345] =
  tabla[346] =
  tabla[347] =
  tabla[348] =
  tabla[349] =
  tabla[350] =
  tabla[351] = "?";

  tabla[352] =
  tabla[353] =
  tabla[354] =
  tabla[355] =
  tabla[356] =
  tabla[357] =
  tabla[358] =
  tabla[359] =
  tabla[360] =
  tabla[361] =
  tabla[362] =
  tabla[363] =
  tabla[364] =
  tabla[365] =
  tabla[366] =
  tabla[367] = "?";

  tabla[368] =
  tabla[369] =
  tabla[370] =
  tabla[371] =
  tabla[372] =
  tabla[373] =
  tabla[374] =
  tabla[375] =
  tabla[376] =
  tabla[377] =
  tabla[378] =
  tabla[379] =
  tabla[380] =
  tabla[381] =
  tabla[382] =
  tabla[383] = "?";

  tabla[384] =
  tabla[385] =
  tabla[386] =
  tabla[387] =
  tabla[388] =
  tabla[389] =
  tabla[390] =
  tabla[391] =
  tabla[392] =
  tabla[393] =
  tabla[394] =
  tabla[395] =
  tabla[396] =
  tabla[397] =
  tabla[398] =
  tabla[399] = "?";

  tabla[400] =
  tabla[401] =
  tabla[402] =
  tabla[403] =
  tabla[404] =
  tabla[405] =
  tabla[406] =
  tabla[407] =
  tabla[408] =
  tabla[409] =
  tabla[410] =
  tabla[411] =
  tabla[412] =
  tabla[413] =
  tabla[414] =
  tabla[415] = "?";

  tabla[416] =
  tabla[417] =
  tabla[418] =
  tabla[419] =
  tabla[420] =
  tabla[421] =
  tabla[422] =
  tabla[423] =
  tabla[424] =
  tabla[425] =
  tabla[426] =
  tabla[427] = "?";
  tabla[428] = "fpuclearerr";
}

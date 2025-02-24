/*
** Ensamblador G10.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** 17-jun-1995.
*/

#define NULL     0
#define EOF     (-1)
#define TAM_LIN  128
#define NO       0
#define SI       1

#define TAM_BUF  2048

#define TAM_MEM  77777

#define TAM_ETIQ 17  /* 1 byte. 0= Es una direcci¢n. 1= Es un dato. */
                     /* 4 bytes de dato/direcci¢n. */
                     /* 4 bytes para la siguiente etiqueta */
                     /* 4 bytes para el descendiente izquierdo */
                     /* 4 bytes para el descendiente derecho */
                     /* x bytes para el nombre (terminado en 0) */
#define TAM_INDEF 9  /* 1 byte. Indica el codigo de instrucci¢n (0-15) */
                     /*         16= es para un db, 17= es para un dw. */
                     /* 4 bytes de direcci¢n donde debe ir */
                     /* 4 bytes para el siguiente indefinido */
                     /* x bytes para la expresi¢n (terminada en 0) */

int err;
int num_arch;
int archivo, pos_linea;

char preins[8];
char oriins[8];
int acumula;

int temp1;
int temp2;
int fin_de_archivo;
int linea_actual;
int disponible;
int pos_ens;
char *pos_global;
char *primer_etiq;
char *ultima_etiq;
int num_etiq;
char *primer_indef;
char *ultimo_indef;
int num_indef;
int ptemp1, ptemp2;
char nom[15];
char linea[TAM_LIN];
char separa[TAM_LIN];
char separa2[TAM_LIN];
char etiq_indef[TAM_LIN];
char btemp1[TAM_BUF];
char btemp2[TAM_BUF];
int tabla[528];
char t[TAM_MEM];

#define TEMPORAL "TEMP1E.$$$"

main() {
  puts("\r\n");
  puts("Ensamblador G10  16-jun-95  (c) Copyright 1995 Oscar Toledo G.\r\n");
  puts("\r\n");
  inicia1();   /* Las 16 instrucciones b sicas */
  inicia2();   /* Las 16 operaciones b sicas */
  inicia3();   /* La mitad del conjunto de instrucciones */
  inicia4();   /* La otra mitad del conjunto */
  inicia5();   /* Instrucciones de microcodigo de la FPU */
  num_arch = 0;
  disponible = 1;
  primer_etiq = NULL;
  ultima_etiq = NULL;
  num_etiq = 0;
  primer_indef = NULL;
  ultimo_indef = NULL;
  num_indef = 0;
  pos_ens = 0;
  temp1 = fopen(TEMPORAL, "w");
  if(temp1 == NULL) {
    puts("Error al crear el archivo temporal\r\n\r\n");
    sale();
  }
  ptemp1 = 0;
  temp2 = NULL;
  while(1) {
    puts("Archivo de entrada > ");
    lee_linea(nom, 15);
    if(*nom == 0) {
      vtemp1();
      if(num_arch == 0) {
        puts("\r\nEnsamblaje cancelado.\r\n\r\n");
        sale();
      }
      linea_actual = 0;
      puts("Archivo de salida > ");
      lee_linea(nom, 15);
      if(*nom == 0) {
        puts("\r\nEnsamblaje cancelado.\r\n\r\n");
        sale();
      }
      puts("\r\n");
      enlaza();
      puts("\r                          \r");
      decimal(num_etiq);
      puts(" etiqueta definidas.\r\n");
      decimal((disponible + 1023) / 1024);
      puts(" KB. ocupados.\r\n\r\n");
      exit(1);
    }
    archivo = fopen(nom, "r");
    if(archivo == NULL) {
      puts("Archivo inexistente.\r\n");
      continue;
    }
    fin_de_archivo = NO;
    linea_actual = 0;
    while(!fin_de_archivo)
      ensambla();
    fclose(archivo);
    ++num_arch;
  }
}

sale() {
  if(temp1 != NULL) fclose(temp1);
  if(temp2 != NULL) fclose(temp2);
  exit(1);
}

asigna(bytes) int bytes; {
  int pos_orig;
  pos_orig = disponible;
  disponible = disponible + bytes;
  if(disponible > TAM_MEM) {
    if(archivo != NULL) fclose(archivo);
    puts("\r\nNo hay memoria.\r\n");
    sale();
  }
  return t + pos_orig;
}

separa_componente() {
  char *sep;
  while(linea[pos_linea] == ' ') ++pos_linea;
  sep = separa;
  while((linea[pos_linea] != 0) & (linea[pos_linea] != ' ')) {
    *sep++ = linea[pos_linea++];
  }
  *sep = 0;
  while(linea[pos_linea] == ' ') ++pos_linea;
}

ensambla() {
  int instruccion, etiq;
  char *final, *ap;
  obtiene_linea();
  etiq = NO;
  while(1) {
    separa_componente();
    if(*separa == 0) break;
    if(*separa == ';') break;
    final = separa;
    while(*final) ++final;
    if((final != separa) & (*(final-1) == ':')) {
      *(final-1) = 0;
      if(isalpha(*separa) | (*separa == '_')) {
        etiqueta();
        etiq = SI;
      }
      else error("Etiqueta inv lida");
    }
    else if(compara("db", separa)) {
      def_byte();
      break;
    }
    else if(compara("dw", separa)) {
      def_pal32();
      break;
    }
    else if(compara("ds", separa)) {
      def_espacio();
      break;
    }
    else if(compara("equ", separa)) {
      if(etiq) def_equiv();
      else error("El equ debe ir con una etiqueta");
      break;
    }
    else {
      instruccion = 0;
      while(instruccion < 528) {
        ap = tabla[instruccion++];
        if(*ap == '?') continue;
        if(compara(ap, separa)) {
          --instruccion;
          if(instruccion < 16) ins_op(instruccion);
          else if(instruccion < 272) ins_sim(instruccion - 16);
          else if(instruccion < 528) ins_ext(instruccion - 272);
          break;
        }
      }
      if(instruccion == 528) {
        error("Instrucci¢n indefinida");
      }
      else break;
    }
  }
  if(fin_de_archivo) return;
}

strlen(cad) char *cad; {
  char *ori;
  ori = cad;
  while(*cad) ++cad;
  return(cad - ori);
}

strcpy(des, ori) char *des, *ori; {
  while(*des++ = *ori++) ;
}

strcat(des, ori) char *des, *ori; {
  while(*des) ++des;
  strcpy(des, ori);
}

strcmp(ori, des) char *ori, *des; {
  while(1) {
    if(*ori < *des) return -1;
    if(*ori > *des++) return 1;
    if(*ori++ == 0) return 0;
  }
}

etiqueta() {
  char *nueva_etiq;
  char *pos, *pos1;
  int u, cuenta, valor;
  pos = primer_etiq;
  pos1 = NULL;
  while(pos != NULL) {
    if((valor = strcmp(separa, pos + 17)) == 0) {
      error("Etiqueta redefinida");
      return;
    }
    if(valor == -1) {
      pos1 = pos + 9;
      pos = (*(pos + 9)) | (*(pos + 10) << 8) |
            (*(pos + 11) << 16) | (*(pos + 12) << 24);
    } else if(valor == 1) {
      pos1 = pos + 13;
      pos = (*(pos + 13)) | (*(pos + 14) << 8) |
            (*(pos + 15) << 16) | (*(pos + 16) << 24);
    }
  }
  u = pos = nueva_etiq = asigna(TAM_ETIQ + strlen(separa) + 1);
  if(pos1 != NULL) {
    *pos1 = u;
    *(pos1+1) = u >> 8;
    *(pos1+2) = u >> 16;
    *(pos1+3) = u >> 24;
  }
  *pos++ = 0;
  *pos++ = pos_ens;
  *pos++ = pos_ens >> 8;
  *pos++ = pos_ens >> 16;
  *pos++ = pos_ens >> 24;
  cuenta = 0;
  while(cuenta++ < 12)
    *pos++ = 0;
  pos1 = separa;
  while(*pos++ = *pos1++);
  if(ultima_etiq != NULL) {
    *(ultima_etiq + 5) = u;
    *(ultima_etiq + 6) = u >> 8;
    *(ultima_etiq + 7) = u >> 16;
    *(ultima_etiq + 8) = u >> 24;
  }
  if(primer_etiq == NULL) primer_etiq = nueva_etiq;
  ultima_etiq = nueva_etiq;
  ++num_etiq;
}

busca_etiq(nombre) char *nombre; {
  char *pos;
  int valor;
  pos = primer_etiq;
  while(pos != NULL) {
    if((valor = strcmp(nombre, pos + 17)) == 0) return pos;
    if(valor == -1) {
      pos = (*(pos + 9)) | (*(pos + 10) << 8) |
            (*(pos + 11) << 16) | (*(pos + 12) << 24);
    } else if(valor == 1) {
      pos = (*(pos + 13)) | (*(pos + 14) << 8) |
            (*(pos + 15) << 16) | (*(pos + 16) << 24);
    }
  }
  return NULL;
}

ins_op(ins) int ins; {
  int valor;
  int arregla, salto;
  separa_componente();
  err = 0;
  pos_global = separa;
  valor = evalua_expresion();
  if(err) {
    ag_indef(ins);
    etemp1(ins << 4);
    ++pos_ens;
    return;
  }
  if((ins == 0) | (ins == 9) | (ins == 10)) {
    ag_indef(ins);
    arregla = 1;
    while(1) {
      acumula = 0;
      gen_ins(ins, salto = (valor - (pos_ens + arregla)));
      if(pos_ens + acumula + salto == valor) break;
      ++arregla;
    }
  } else {
    acumula = 0;
    gen_ins(ins, valor);
  }
  valor = 0;
  while(valor < acumula) {
    etemp1(preins[valor++]);
    ++pos_ens;
  }
}

ag_indef(clave) int clave; {
  char *nuevo_indef;
  char *pos, *pos1;
  int u;
  u = pos = nuevo_indef = asigna(TAM_INDEF + strlen(separa) + 1);
  *pos++ = clave;
  *pos++ = pos_ens;
  *pos++ = pos_ens >> 8;
  *pos++ = pos_ens >> 16;
  *pos++ = pos_ens >> 24;
  *pos++ = 0;
  *pos++ = 0;
  *pos++ = 0;
  *pos++ = 0;
  pos1 = separa;
  while(*pos++ = *pos1++) ;
  if(ultimo_indef != NULL) {
    *(ultimo_indef + 5) = u;
    *(ultimo_indef + 6) = u >> 8;
    *(ultimo_indef + 7) = u >> 16;
    *(ultimo_indef + 8) = u >> 24;
  }
  if(primer_indef == NULL) primer_indef = nuevo_indef;
  ultimo_indef = nuevo_indef;
  ++num_indef;
}

isdigit(c) char c; {
  return((c >= '0') & (c <= '9'));
}

isalpha(c) char c; {
  return(((c >= 'A') & (c <= 'Z')) |
         ((c >= 'a') & (c <= 'z')));
}

evalua_expresion() {
  int valor1;
  valor1 = eval1();
  while((*pos_global == '+') | (*pos_global == '-')) {
    if(*pos_global == '+') {
      ++pos_global;
      valor1 = valor1 + eval1();
    } else {
      ++pos_global;
      valor1 = valor1 - eval1();
    }
  }
  return valor1;
}

eval1() {
  if(*pos_global == '+') ++pos_global;
  else if(*pos_global == '-') {
    ++pos_global;
    return -eval2();
  }
  return eval2();
}

eval2() {
  int valor1;
  valor1 = eval3();
  while((*pos_global == '*') |
        (*pos_global == '/') |
        (*pos_global == '%')) {
    if(*pos_global == '*') {
      ++pos_global;
      valor1 = valor1 * eval3();
    } else if(*pos_global == '/') {
      ++pos_global;
      valor1 = valor1 / eval3();
    } else {
      ++pos_global;
      valor1 = valor1 % eval3();
    }
  }
  return valor1;
}

eval3() {
  char *ap;
  int valor;
  if(*pos_global == '(') {
    ++pos_global;
    valor = evalua_expresion();
    if(*pos_global != ')')
      error("Falta parentesis derecho");
    else ++pos_global;
    return valor;
  } else if(isdigit(*pos_global)) {
    if((*(pos_global + 1) == 'X') |
       (*(pos_global + 1) == 'x'))
      return eval_hex();
    else return eval_dec();
  }
  else if(isalpha(*pos_global) | (*pos_global == '_')) {
    ap = separa2;
    while((*pos_global == '_') | isalpha(*pos_global) |
          isdigit(*pos_global)) {
      *ap++ = *pos_global++;
    }
    *ap = 0;
    if((ap = busca_etiq(separa2)) != NULL) {
      valor = *(ap+1) | (*(ap+2) << 8) | (*(ap+3) << 16) | (*(ap+4) << 24);
      return valor;
    }
    else {
      strcpy(etiq_indef, separa2);
      err = 1;
      return 0;
    }
  }
  else error("error de sintaxis");
}

isxdigit(c) int c; {
  return (((c>='0') & (c<='9')) |
          ((c>='a') & (c<='f')) |
          ((c>='A') & (c<='F')));
}

eval_hex() {
  int c;
  int valor;
  valor = 0;
  pos_global = pos_global + 2;
  while(isxdigit(*pos_global)) {
    c = toupper(*pos_global++) - '0';
    if(c > 9) c = c - 7;
    valor = (valor << 4) | c;
  }
  return valor;
}

eval_dec() {
  int c;
  int valor;
  valor = 0;
  while(isdigit(*pos_global)) {
    c = *pos_global++ - '0';
    valor = valor * 10 + c;
  }
  return valor;
}

gen_ins(oper, valor) int oper; int valor; {
  if(valor < 0) gen_ins(6, ~valor >> 4);
  else if(valor >= 16) gen_ins(2, valor >> 4);
  preins[acumula++] = (oper << 4) | (valor & 15);
}

ins_sim(ins) int ins; {
  if(ins > 15) {
    etemp1(0x20 + (ins >> 4));
    ++pos_ens;
  }
  etemp1(0xf0 + (ins & 15));
  ++pos_ens;
}

ins_ext(ins) int ins; {
  if(ins > 15) {
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

def_byte() {
  int valor;
  separa_componente();
  pos_global = separa;
  while(1) {
    err = 0;
    valor = evalua_expresion();
    if(err) ag_indef(16);
    etemp1(valor & 255);
    ++pos_ens;
    if(*pos_global != ',') {
      if(*pos_global)
        error("Falta una coma");
      return;
    }
    ++pos_global;
  }
}

def_pal32() {
  int valor;
  separa_componente();
  pos_global = separa;
  while(1) {
    err = 0;
    valor = evalua_expresion();
    if(err) ag_indef(17);
    etemp1(valor & 255);
    ++pos_ens;
    etemp1((valor >> 8) & 255);
    ++pos_ens;
    etemp1((valor >> 16) & 255);
    ++pos_ens;
    etemp1((valor >> 24) & 255);
    ++pos_ens;
    if(*pos_global != ',') {
      if(*pos_global)
        error("Falta una coma");
      return;
    }
    ++pos_global;
  }
}

def_espacio() {
  int valor;
  separa_componente();
  pos_global = separa;
  err = 0;
  valor = evalua_expresion();
  if(err) {
    error("Se requiere un valor definido");
    return;
  }
  while(valor--) {
    etemp1(0);
    ++pos_ens;
  }
}

def_equiv() {
  int valor;
  separa_componente();
  pos_global = separa;
  err = 0;
  valor = evalua_expresion();
  if(err) {
    error("Se requiere un valor definido");
    return;
  }
  if(ultima_etiq == NULL) {
    error("Error interno");
    return;
  }
  *ultima_etiq = 1;
  *(ultima_etiq+1) = valor;
  *(ultima_etiq+2) = valor >> 8;
  *(ultima_etiq+3) = valor >> 16;
  *(ultima_etiq+4) = valor >> 24;
}

compara(ins, separa) char *ins, *separa; {
  while(*ins == *separa++) {
    if(*ins++ == 0) return 1;
  }
  return 0;
}

error(info) char *info; {
  puts(info);
  if(linea_actual) {
    puts(" en la l¡nea ");
    decimal(linea_actual);
  }
  puts("\r\n");
}

obtiene_linea() {
  int conteo, car;
  conteo = 0;
  while(1) {
    car = fgetc(archivo);
    if(car == EOF) {
      fin_de_archivo = SI;
      break;
    }
    if(car == '\r') continue;
    if(car == '\n') break;
    if(car == '\t') car = ' ';
    if(conteo != TAM_LIN - 1)
      linea[conteo++] = car;
  }
  linea[conteo] = 0;
  ++linea_actual;
  pos_linea = 0;
}

int algo;

enlaza() {
  while(1) {
    temp1 = fopen(TEMPORAL, "r");
    ptemp1 = TAM_BUF;
    temp2 = fopen(nom, "w");
    ptemp2 = 0;
    if(temp2 == NULL) {
      error("Error de acceso al disco");
      sale();
    }
    algo = NO;
    paso();
    vtemp2();
    fclose(temp1);
    temp1 = NULL;
    if(!algo) break;
    temp1 = fopen(nom, "r");
    ptemp1 = TAM_BUF;
    temp2 = fopen(TEMPORAL, "w");
    ptemp2 = 0;
    if(temp2 == NULL) {
      error("Error de acceso al disco");
      sale();
    }
    algo = NO;
    paso();
    vtemp2();
    fclose(temp1);
    temp1 = NULL;
/*
    if(!algo) {
      unlink(nom);
      rename(TEMPORAL, nom);
      break;
    }
*/
  }
}

paso() {
  char *lista, *lista2;
  int inicio;
  int ultimo;
  int pos;
  int valor, byte;
  int acu, ins;
  int arregla, salto;
  int cuantos;
  int a;

  ultimo = pos_ens;
  pos_ens = 0;

  cuantos = 0;

  inicio = 0;

  lista = primer_indef;

  a = 0;
  while(lista != NULL) {
    pos = *(lista + 1) | (*(lista + 2) << 8) |
          (*(lista + 3) << 16) | (*(lista + 4) << 24);
    copia(inicio, pos);

    ins = *lista;

    if(ins == 16) {
      byte = ltemp1();
      inicio = pos + 1;
    } else if(ins == 17) {
      byte = ltemp1();
      byte = ltemp1();
      byte = ltemp1();
      byte = ltemp1();
      inicio = pos + 4;
    } else {
      acu = 0;
      while(1) {
        oriins[acu++] = byte = ltemp1();
        if((byte & 0xf0) == 0x20) continue;
        if((byte & 0xf0) == 0x60) continue;
        break;
      }
      inicio = pos + acu;
    }

    *(lista + 1) = pos_ens;
    *(lista + 2) = pos_ens >> 8;
    *(lista + 3) = pos_ens >> 16;
    *(lista + 4) = pos_ens >> 24;

    err = 0;
    pos_global = lista + 9;
    valor = evalua_expresion();
    if(err) {
      strcpy(separa2, "Etiqueta indefinida ");
      strcat(separa2, etiq_indef);
      error(separa2);
      valor = 0;
    }

    if(ins == 16) {
      etemp2(valor);
      ++pos_ens;
    } else if(ins == 17) {
      etemp2(valor);
      ++pos_ens;
      etemp2(valor >> 8);
      ++pos_ens;
      etemp2(valor >> 16);
      ++pos_ens;
      etemp2(valor >> 24);
      ++pos_ens;
    } else if((ins == 0) | (ins == 9) | (ins == 10)) {
      acumula = 0;
      gen_ins(ins, valor - (pos_ens + acu));
    } else {
      acumula = 0;
      gen_ins(ins, valor);
    }
    if((ins != 16) & (ins != 17)) {
      valor = 0;
      while(valor < acumula) {
        etemp2(preins[valor++]);
        ++pos_ens;
      }
      if(acumula != acu) {
        algo = SI;
        lista2 = primer_etiq;
        while(lista2 != NULL) {
          if(*lista2 == 0) {
            valor = *(lista2+1) | (*(lista2+2) << 8) |
                    (*(lista2+3) << 16) | (*(lista2+4) << 24);
            if(valor >= inicio + cuantos)
              valor = valor + (acumula - acu);
            *(lista2+1) = valor;
            *(lista2+2) = valor >> 8;
            *(lista2+3) = valor >> 16;
            *(lista2+4) = valor >> 24;
          }
          lista2 = *(lista2+5) | (*(lista2+6) << 8) |
                  (*(lista2+7) << 16) | (*(lista2+8) << 24);
        }
        cuantos = cuantos + (acumula - acu);
      }
    }

    lista = *(lista + 5) | (*(lista + 6) << 8) |
           (*(lista + 7) << 16) | (*(lista + 8) << 24);
    decimal(++a * 100 / num_indef);
    puts("%  \r");
  }

  copia(inicio, ultimo);
}

copia(inicio, final) int inicio, final; {
  while(inicio++ < final) {
    etemp2(ltemp1());
    ++pos_ens;
  }
}

toupper(car) int car; {
  if((car >= 'a') & (car <= 'z')) return car - 32;
  else return car;
}

lee_linea(pos, tam) char *pos; int tam; {
  char *ahora;
  int car;
  ahora = pos;
  while(1) {
    car = getchar();
    if(car == 8) {
      if(ahora == pos) continue;
      putchar(8);
      --ahora;
      continue;
    } else if(car == 13) {
      puts("\r\n");
      *ahora = 0;
      return;
    } else {
      if(ahora == pos + tam - 1) continue;
      putchar(car);
      *ahora++ = car;
    }
  }
}

etemp1(dato) int dato; {
  btemp1[ptemp1++] = dato;
  if(ptemp1 == TAM_BUF) {
    ptemp1 = 0;
    while(ptemp1 < TAM_BUF) {
      if(fputc(btemp1[ptemp1++], temp1) == EOF) {
        error("Disco lleno");
        break;
      }
    }
    ptemp1 = 0;
  }
}

vtemp1() {
  int conteo;
  conteo = 0;
  while(conteo < ptemp1) {
    if(fputc(btemp1[conteo++], temp1) == EOF) {
      error("Disco lleno");
      break;
    }
  }
  fclose(temp1);
  temp1 = NULL;
}

etemp2(dato) int dato; {
  btemp2[ptemp2++] = dato;
  if(ptemp2 == TAM_BUF) {
    ptemp2 = 0;
    while(ptemp2 < TAM_BUF) {
      if(fputc(btemp2[ptemp2++], temp2) == EOF) {
        error("Disco lleno");
        break;
      }
    }
    ptemp2 = 0;
  }
}

vtemp2() {
  int conteo;
  conteo = 0;
  while(conteo < ptemp2) {
    if(fputc(btemp2[conteo++], temp2) == EOF) {
      error("Disco lleno");
      break;
    }
  }
  fclose(temp2);
  temp2 = NULL;
}

ltemp1() {
  if(ptemp1 == TAM_BUF) {
    ptemp1 = 0;
    while(ptemp1 < TAM_BUF)
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
  tabla[147] = "fpchkerror";
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
  tabla[172] = "fptesterror";
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
  tabla[307] = "fpuseterror";
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
  tabla[428] = "fpuclearerror";
  tabla[429] =
  tabla[430] =
  tabla[431] = "?";

  tabla[432] =
  tabla[433] =
  tabla[434] =
  tabla[435] =
  tabla[436] =
  tabla[437] =
  tabla[438] =
  tabla[439] =
  tabla[440] =
  tabla[441] =
  tabla[442] =
  tabla[443] =
  tabla[444] =
  tabla[445] =
  tabla[446] =
  tabla[447] = "?";

  tabla[448] =
  tabla[449] =
  tabla[450] =
  tabla[451] =
  tabla[452] =
  tabla[453] =
  tabla[454] =
  tabla[455] =
  tabla[456] =
  tabla[457] =
  tabla[458] =
  tabla[459] =
  tabla[460] =
  tabla[461] =
  tabla[462] =
  tabla[463] = "?";

  tabla[464] =
  tabla[465] =
  tabla[466] =
  tabla[467] =
  tabla[468] =
  tabla[469] =
  tabla[470] =
  tabla[471] =
  tabla[472] =
  tabla[473] =
  tabla[474] =
  tabla[475] =
  tabla[476] =
  tabla[477] =
  tabla[478] =
  tabla[479] = "?";

  tabla[480] =
  tabla[481] =
  tabla[482] =
  tabla[483] =
  tabla[484] =
  tabla[485] =
  tabla[486] =
  tabla[487] =
  tabla[488] =
  tabla[489] =
  tabla[490] =
  tabla[491] =
  tabla[492] =
  tabla[493] =
  tabla[494] =
  tabla[495] = "?";

  tabla[496] =
  tabla[497] =
  tabla[498] =
  tabla[499] =
  tabla[500] =
  tabla[501] =
  tabla[502] =
  tabla[503] =
  tabla[504] =
  tabla[505] =
  tabla[506] =
  tabla[507] =
  tabla[508] =
  tabla[509] =
  tabla[510] =
  tabla[511] = "?";

  tabla[512] =
  tabla[513] =
  tabla[514] =
  tabla[515] =
  tabla[516] =
  tabla[517] =
  tabla[518] =
  tabla[519] =
  tabla[520] =
  tabla[521] =
  tabla[522] =
  tabla[523] =
  tabla[524] =
  tabla[525] =
  tabla[526] =
  tabla[527] =
  tabla[528] = "?";
}

/*
** Monitor visualizador para el G10
**
** (c) Copyright 1995 Oscar Toledo Gutierrez
**
** Creacion: 9 de junio de 1995.
*/

/*
** Que la orden E ilustre el contenido final de los registros.
*/

#define NOMBRE  "Monitor Visual para el G10"
#define VERSION "       Version 1.00"
#define AUTOR   "por Oscar Toledo Gutierrez"

#define NULL    0
#define EOF     (-1)
#define NO      0
#define SI      1
#define ESC     27

char orden;
int car, digito;
int cancela;
int direccion;
int tabla[529];

main() {
  inicia1();
  inicia2();
  inicia3();
  presenta();
  il("? para ver una lista de ordenes");
  nl();
  il("Esc para cancelar ordenes");
  nl();
  nl();
  while(1) {
    color(15);
    il("- ");
    orden = toupper(car = getchar());
    if(orden == '?') ayuda();
    else if(orden == 'E') ejecuta();
    else if(orden == 'G') guarda();
    else if(orden == 'L') lista();
    else if(orden == 'S') sale();
    else if(orden == 'M') mueve();
    else if(orden == 'R') rellena();
    else if(orden == 'O') ordena();
    else if(orden == 'B') borra();
    else if(orden == 'C') compara();
    else if(orden == 'V') busca();
    else if(orden == 'N') miniens();
    else if(orden == 'X') leer();
    else if(orden == 'Y') grabar();
    else if(orden == 'I') memoria();
    else {
      color(15);
      putchar(car);
      color(12);
      il("  Orden erronea.");
      nl();
    }
  }
}

color(col) int col; {
  putchar(0x1b);
  putchar(0x01);
  putchar(col);
}

toupper(car) int car; {
  if((car >= 'a') & (car <= 'z')) return car - 32;
  else return car;
}

ayuda() {
  color(15);
  il("? Ayuda.");
  nl();
  nl();
  color(11);
  il("Ordenes disponibles:");
  nl();
  nl();
  color(10);
  il("B dir1 dir2         Borra dir1-dir2 con FF.");
  nl();
  il("C dir1 dir2 dir3    Compara dir1-dir2 con dir3.");
  nl();
  il("E dir               Llama la direccion especificada.");
  nl();
  il("G dir               Guarda datos en la direccion especificada.");
  nl();
  il("I                   Ilustra la memoria disponible.");
  nl();
  il("L dir               Lista datos de la direccion especificada.");
  nl();
  il("M dir1 dir2 dir3    Mueve datos de dir1-dir2 en dir3.");
  nl();
  il("N dir               Mini-ensamblador en la direccion especificada.");
  nl();
  il("O dir               Ordena datos de la direccion especificada.");
  nl();
  il("R dir1 dir2 byte    Rellena dir1-dir2 con el byte especificado.");
  nl();
  il("S                   Sale del monitor.");
  nl();
  il("V dir1 dir2 bytes   Busca los bytes en dir1-dir2.");
  nl();
  il("X nombre            Lee 'nombre' en la memoria, indica la direccion.");
  nl();
  il("Y dir1 dir2 nombre  Graba dir1-dir2 en 'nombre'.");
  nl();
  nl();
  color(13);
  il("La direccion debe tener el siguiente formato:   xxxx:xxxx");
  nl();
  nl();
}

ejecuta() {
  int a, b, c, wptr;
  color(15);
  il("Ejecutar la direccion ");
  color(14);
  procdir();
  if(cancela) {
    aborta();
    return;
  }
  nl();
  nl();
  _llamada(direccion, &a, &b, &c, &wptr);
  color(7);
  il("El programa termino normalmente.");
  nl();
  color(15);
  il("A= ");
  color(14);
  hexlargo(a);
  color(15);
  il("   B= ");
  color(14);
  hexlargo(b);
  color(15);
  il("   C= ");
  color(14);
  hexlargo(c);
  color(15);
  il("   Wptr= ");
  color(14);
  hexlargo(wptr);
  nl();
}

guarda() {
  char *dir;
  int byte;
  char valor;
  int pos;

  color(15);
  il("Guardar en la direccion ");
  color(14);
  procdir();
  if(cancela) {
    aborta();
    return;
  }
  nl();
  dir = direccion;
  while(1) {
    color(14);
    hexlargo(dir);
    color(10);
    il("  ");
    byte = 0;
    pos = dir;
    while(pos & 15) {
      il("  ");
      ++byte;
      if((byte & 3) == 0) il("  ");
      else il(" ");
      --pos;
    }
    while(byte < 16) {
      dig();
      if(cancela) break;
      valor = digito << 4;
      putchar(car);
      dig();
      if(cancela) break;
      valor = valor | digito;
      putchar(car);
      *dir++ = valor;
      ++byte;
      if((byte & 3) == 0) il("  ");
      else il(" ");
    }
    nl();
    if(cancela) break;
  }
}

lista() {
  char *dir, *ori;
  int lineas, byte, valor;
  int pos, suma;

  color(15);
  il("Listar la direccion ");
  color(14);
  procdir();
  if(cancela) {
    aborta();
    return;
  }
  nl();
  dir = direccion;
  while(1) {
    lineas = 0;
    suma = 0;
    while(lineas < 16) {
      color(14);
      hexlargo(dir);
      color(10);
      ori = dir;
      il("  ");
      byte = 0;
      pos = dir;
      while(pos & 15) {
        il("  ");
        ++byte;
        if((byte & 3) == 0) il("  ");
        else il(" ");
        --pos;
      }
      while(byte < 16) {
        hexbyte(*dir);
        suma = suma + *dir;
        ++dir;
        ++byte;
        if((byte & 3) == 0) il("  ");
        else il(" ");
      }
      dir = ori;
      byte = 0;
      pos = dir;
      while(pos & 15) {
        putchar(' ');
        ++byte;
        --pos;
      }
      color(111);
      while(byte < 16) {
        valor = *dir++;
        if(valor < 32) valor = '.';
        putchar(valor);
        ++byte;
      }
      color(7);
      nl();
      ++lineas;
    }
    color(15);
    il("Suma de bytes= ");
    color(14);
    hexbyte(suma >> 8);
    hexbyte(suma);
    nl();
    color(15);
    il("Pulse espacio para continuar, cualquier otra tecla para terminar...");
    valor = getchar();
    putchar(13);
    il("                                                                   ");
    putchar(13);
    if(valor == ' ') continue;
    break;
  }
}

mueve() {
  char *inicio, *final, *destino;

  color(15);
  il("Mueve de ");
  color(14);
  procdir();
  inicio = direccion;
  if(cancela) {
    aborta();
    return;
  }
  color(15);
  il(" hasta ");
  color(14);
  procdir();
  final = direccion;
  if(cancela) {
    aborta();
    return;
  }
  color(15);
  il(" en ");
  color(14);
  procdir();
  destino = direccion;
  if(cancela) {
    aborta();
    return;
  }
  nl();
  color(11);
  il("Moviendo...");
  while(1) {
    *destino++ = *inicio;
    if(inicio == final) break;
    ++inicio;
  }
  putchar(13);
  il("           ");
  putchar(13);
}

rellena() {
  char *inicio, *final;
  char byte;

  color(15);
  il("Rellena de ");
  color(14);
  procdir();
  inicio = direccion;
  if(cancela) {
    aborta();
    return;
  }
  color(15);
  il(" hasta ");
  color(14);
  procdir();
  final = direccion;
  if(cancela) {
    aborta();
    return;
  }
  color(15);
  il(" con ");
  color(10);
  dig();
  if(cancela) {
    aborta();
    return;
  }
  byte = digito << 4;
  putchar(car);
  dig();
  if(cancela) {
    aborta();
    return;
   }
  byte = byte | digito;
  putchar(car);
  nl();
  color(11);
  il("Rellenando...");
  while(1) {
    *inicio = byte;
    if(inicio == final) break;
    ++inicio;
  }
  putchar(13);
  il("             ");
  putchar(13);
}

borra() {
  char *inicio, *final;
  char byte;

  color(15);
  il("Borra de ");
  color(14);
  procdir();
  inicio = direccion;
  if(cancela) {
    aborta();
    return;
  }
  color(15);
  il(" hasta ");
  color(14);
  procdir();
  final = direccion;
  if(cancela) {
    aborta();
    return;
  }
  nl();
  color(11);
  il("Borrando...");
  while(1) {
    *inicio = 255;
    if(inicio == final) break;
    ++inicio;
  }
  putchar(13);
  il("           ");
  putchar(13);
}

ordena() {
  int lineas, valor;

  color(15);
  il("Ordenar la direccion ");
  color(14);
  procdir();
  if(cancela) {
    aborta();
    return;
  }
  nl();
  while(1) {
    lineas = 0;
    while(lineas < 24) {
      color(14);
      hexlargo(direccion);
      il("  ");
      des();
      nl();
      ++lineas;
    }
    color(15);
    il("Pulse espacio para continuar, cualquier otra tecla para terminar...");
    valor = getchar();
    putchar(13);
    il("                                                                   ");
    putchar(13);
    if(valor == ' ') continue;
    break;
  }
}

des() {
  char *dir;
  int valor;
  int clase;
  int val;
  int byte;
  int cuantos;

  valor = cuantos = 0;
  dir = direccion;
  color(10);
  while(1) {
    if(cuantos == 8) {
      direccion = dir;
      return;
    }
    byte = *dir++;
    ++cuantos;
    hexbyte(byte);
    putchar(' ');
    clase = (byte >> 4) & 15;
    val = byte & 15;
    if(clase == 2) {
      valor = (valor | val) << 4;
      continue;
    } else if(clase == 6) {
      valor = ~(valor | val) << 4;
      continue;
    } else {
      valor = valor | val;
      break;
    }
  }
  if((clase == 4) & (valor < 256) & (*dir == 42) & (*(dir+1) == 251)) {
    clase = 16;
    byte = *dir++;
    ++cuantos;
    hexbyte(byte);
    putchar(' ');
    byte = *dir++;
    ++cuantos;
    hexbyte(byte);
    putchar(' ');
  }
  while(cuantos < 8) {
    putchar(' ');
    putchar(' ');
    putchar(' ');
    ++cuantos;
  }
  putchar(' ');
  if(clase == 15) {
    if(valor == 380) il("lddevid");
    else if(valor == 511) il("softreset");
    else if(valor > 255) il("?");
    else il(tabla[valor + 17]);
  } else if(clase == 16) {
    il(tabla[valor + 273]);
  } else {
    il(tabla[clase + 1]);
    il(" ");
    if((clase == 0) | (clase == 9) | (clase == 10))
      hexlargo(dir + valor);
    else sacahex(valor);
  }
  direccion = dir;
}

compara() {
  char *inicio, *final;
  char *destino;

  color(15);
  il("Compara de ");
  color(14);
  procdir();
  inicio = direccion;
  if(cancela) {
    aborta();
    return;
  }
  color(15);
  il(" hasta ");
  color(14);
  procdir();
  final = direccion;
  if(cancela) {
    aborta();
    return;
  }
  color(15);
  il(" con ");
  color(14);
  procdir();
  destino = direccion;
  if(cancela) {
    aborta();
    return;
  }
  nl();
  while(1) {
    if(*inicio != *destino) {
      color(14);
      hexlargo(inicio);
      il(" - ");
      hexlargo(destino);
      nl();
    }
    if(inicio == final) break;
    ++inicio;
    ++destino;
  }
}

busca() {
  char *inicio, *final, *ori;
  char bytes[255];
  int cuantos;
  int byte, bucle;

  color(15);
  il("Busca de ");
  color(14);
  procdir();
  inicio = direccion;
  if(cancela) {
    aborta();
    return;
  }
  color(15);
  il(" hasta ");
  color(14);
  procdir();
  final = direccion;
  if(cancela) {
    aborta();
    return;
  }
  nl();
  color(15);
  il("Introduzca los bytes a buscar, y luego pulse Enter o espacio:");
  nl();
  color(10);
  cuantos = 0;
  while(1) {
    dig();
    if(cancela == 2) break;
    if(cancela) {
      nl();
      return;
    }
    byte = digito << 4;
    putchar(car);
    dig();
    if(cancela == 2) break;
    if(cancela) {
      nl();
      return;
    }
    byte = byte | digito;
    putchar(car);
    bytes[cuantos++] = byte;
    if(cuantos == 255) break;
  }
  nl();
  if(cuantos == 0) return;
  while(1) {
    ori = inicio;
    bucle = 0;
    while(bucle != cuantos) {
      if(*ori++ != bytes[bucle++]) {
        ++inicio;
        break;
      }
      if(bucle == cuantos) {
        color(14);
        hexlargo(inicio);
        nl();
        inicio = ori;
      }
    }
    if(inicio == final) break;
  }
}

leelinea(pos, tam) char *pos; int tam; {
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
      nl();
      *ahora = 0;
      return;
    } else if(car == ESC) {
      nl();
      *pos = 0;
      return;
    } else {
      if(ahora == pos + tam - 1) continue;
      putchar(car);
      *ahora++ = car;
    }
  }
}

miniens() {
  char ensambla[50];

  color(15);
  il("Mini-ensamblador en la direccion ");
  color(14);
  procdir();
  if(cancela) {
    aborta();
    return;
  }
  nl();
  while(1) {
    color(14);
    hexlargo(direccion);
    il("  ");
    color(10);
    leelinea(ensambla, 50);
    if(ensambla[0] == 0) break;
    ens(ensambla);
  }
}

comp(cad1, cad2) char *cad1, *cad2; {
  while(1) {
    if(*cad1 != *cad2++) return 0;
    if(*cad1++ == 0) return 1;
  }
}

isxdigit(a) int a; {
  return(((a>='0') & (a<='9')) |
         ((a>='A') & (a<='F')) |
         ((a>='a') & (a<='f')));
}

evalhex(pos, res) char *pos; int *res; {
  int valor, final, car, neg;
  final = 0;
  while(1) {
    neg = 0;
    if(*pos == '-') {
      neg = 1;
      ++pos;
    }
    else if(*pos == '+') {
      neg = 0;
      ++pos;
    }
    valor = 0;
    if(!isxdigit(*pos)) {
      color(15);
      il("Se requiere un n£mero hexadecimal");
      nl();
    } else {
      while(isxdigit(car = toupper(*pos))) {
        if(car > '9') car = car - 7;
        car = car - '0';
        valor = (valor << 4) | car;
        ++pos;
      }
    if(neg == 1) valor = -valor;
    final = final + valor;
    if((*pos == '+') | (*pos == '-')) continue;
    break;
    }
  }
  *res = pos;
  return final;
}

prefijo(dir, oper, valor) char *dir; int oper; int valor; {
  if(valor < 0) dir = prefijo(dir, 6, ~valor >> 4);
  else if(valor >= 16) dir = prefijo(dir, 2, valor >> 4);
  *dir++ = (oper << 4) | (valor & 15);
  return dir;
}

tolower(car) int car; {
  if((car >= 'A') & (car <= 'Z')) return car + 32;
  else return car;
}

ens(linea) char *linea; {
  char nemonico[50];
  char *lin, *nem, *dir;
  char *dir_ant;
  int conteo, dir_req;
  int operando;
  dir = direccion;
  lin = linea;
  nem = nemonico;
  while(*lin == ' ') ++lin;
  while((*lin != 0) & (*lin != ' '))
    *nem++ = tolower(*lin++);
  *nem = 0;
  while(*lin == ' ') ++lin;
  if(comp("db", nemonico)) conteo = 600;
  else if(comp("dw", nemonico)) conteo = 601;
  else if(comp("ds", nemonico)) conteo = 602;
  else {
    conteo = 0;
    while(conteo <= 528) {
      if(comp(tabla[conteo], nemonico)) break;
      ++conteo;
    }
  }
  if(conteo == 529) {
    if(comp("softreset", nemonico)) {
      *dir++ = 35;
      *dir++ = 47;
      *dir++ = 255;
    } else if(comp("lddevid", nemonico)) {
      *dir++ = 35;
      *dir++ = 39;
      *dir++ = 252;
    }
  } else {
    if(conteo == 600) {
      while(1) {
        operando = evalhex(lin, &lin);
        *dir++ = operando;
        if(*lin != ',') break;
      }
    } else if(conteo == 601) {
      while(1) {
        operando = evalhex(lin, &lin);
        *dir++ = operando;
        *dir++ = operando >> 8;
        *dir++ = operando >> 16;
        *dir++ = operando >> 24;
        if(*lin != ',') break;
      }
    } else if(conteo == 602) {
      operando = evalhex(lin, &lin);
      dir = dir + operando;
    } else if(conteo > 272) {
      conteo = conteo - 273;
      if(conteo > 15) *dir++ = 32 | ((conteo >> 4) & 15);
      *dir++ = 64 | (conteo & 15);
      *dir++ = 42;
      *dir++ = 251;
    } else if(conteo > 16) {
      conteo = conteo - 17;
      if(conteo > 15) *dir++ = 32 | ((conteo >> 4) & 15);
      *dir++ = 240 | (conteo & 15);
    } else {
      operando = evalhex(lin, &lin);
      --conteo;
      if((conteo != 0) & (conteo != 9) & (conteo != 10)) {
        dir = prefijo(dir, conteo, operando);
      } else {
        dir_ant = dir;
        dir_req = operando - (dir + 1);
        dir = prefijo(dir, conteo, dir_req);
        while(dir + dir_req != operando) {
          --dir_req;
          dir = dir_ant;
          dir = prefijo(dir, conteo, dir_req);
        }
      }
    }
  }
  direccion = dir;
}

leer() {
  char nombre[32];
  char *destino, *info;
  int archivo;
  int car;

  color(15);
  il("Nombre del archivo para leer: ");
  color(14);
  leelinea(nombre, 32);
  if(nombre[0] == 0) return;
  info = destino = (_finprog() + 15) & ~15;
  archivo = fopen(nombre, "r");
  color(15);
  il("Leyendo...");
  if(archivo == NULL) {
    putchar(13);
    color(12);
    il("No se pudo leer el archivo");
    nl();
    return;
  }
  while((car = fgetc(archivo)) != EOF)
    *destino++ = car;
  fclose(archivo);
  color(15);
  putchar(13);
  il("Archivo cargado en ");
  color(14);
  hexlargo(info);
  nl();
  color(14);
  sacahex(destino - info);
  color(15);
  il(" bytes leidos.");
  nl();
}

grabar() {
  char nombre[32];
  char *inicio, *final, *ori;
  int archivo;
  int car;

  color(15);
  il("Grabar de ");
  color(14);
  procdir();
  ori = inicio = direccion;
  if(cancela) {
    aborta();
    return;
  }
  color(15);
  il(" hasta ");
  color(14);
  procdir();
  final = direccion;
  if(cancela) {
    aborta();
    return;
  }
  nl();
  color(15);
  il("Nombre del archivo: ");
  color(14);
  leelinea(nombre, 32);
  if(nombre[0] == 0) return;
  archivo = fopen(nombre, "w");
  color(15);
  il("Grabando... ");
  if(archivo == NULL) {
    putchar(13);
    color(12);
    il("No se pudo crear el archivo");
    nl();
    return;
  }
  while(1) {
    if(fputc(*inicio, archivo) == EOF) {
      putchar(13);
      color(12);
      il("Error al escribir... ");
      break;
    }
    if(inicio++ == final) break;
  }
  color(14);
  sacahex(inicio - ori);
  color(15);
  il(" bytes escritos.");
  fclose(archivo);
  nl();
}

memoria() {
  int inicio;
  int wptr;
  wptr = &wptr - 256;
  inicio = _finprog();
  color(15);
  il("Memoria disponible.");
  nl();
  color(15);
  il("A partir de aqu¡ se pueden guardar datos: ");
  color(14);
  hexlargo(inicio);
  nl();
  color(15);
  il("Limite de memoria (Wptr actual)         : ");
  color(14);
  hexlargo(wptr);
  nl();
}

sacahex(valor) int valor; {
  int bajo;
  if((valor >> 31) & 1) {
    putchar('-');
    valor = -valor;
  }
  bajo = valor & 15;
  valor = valor >> 4;
  if(valor != 0) sacahex(valor);
  hex(bajo);
}

hexlargo(valor) int valor; {
  hexbyte(valor >> 24);
  hexbyte(valor >> 16);
  putchar(':');
  hexbyte(valor >> 8);
  hexbyte(valor);
}

hexbyte(byte) int byte; {
  hex(byte >> 4);
  hex(byte);
}

hex(valor) int valor; {
  valor = valor & 15;
  if(valor > 9) putchar(valor + 'A' - 10);
  else putchar(valor + '0');
}

aborta() {
  color(15);
  putchar(13);
  il("- Orden cancelada                                                      ");
  nl();
}

procdir() {
  int conteo;

  cancela = NO;
  conteo = direccion = 0;
  while(conteo < 8) {
    dig();
    if(cancela) return;
    direccion = (direccion << 4) | digito;
    putchar(car);
    ++conteo;
    if(conteo == 4) putchar(':');
  }
}

dig() {
  while(1) {
    digito = toupper(car = getchar());
    if(car == ESC) {
      cancela = SI;
      return;
    }
    if((car == 13) | (car == ' ')) {
      cancela = 2;
      return;
    }
    if(digito < '0') continue;
    if(digito > 'F') continue;
    if((digito > '9') & (digito < 'A')) continue;
    digito = digito - '0';
    if(digito > 9) digito = digito - 7;
    break;
  }
}

sale() {
  color(15);
  il("Sale del monitor.");
  nl();
  nl();
  color(10);
  exit(1);
}

presenta() {
  nl();
  nl();
  color(12);
  il(NOMBRE);
  nl();
  nl();
  color(15);
  il(VERSION);
  nl();
  nl();
  color(10);
  il(AUTOR);
  nl();
  nl();
}

nl() {
  putchar(13);
  putchar(10);
}

il(cad) char *cad; {
  while(*cad)
    putchar(*cad++);
}

inicia1() {
  tabla[0] = "db";
  tabla[1] = "j";
  tabla[2] = "ldlp";
  tabla[3] = "pfix";
  tabla[4] = "ldnl";
  tabla[5] = "ldc";
  tabla[6] = "ldnlp";
  tabla[7] = "nfix";
  tabla[8] = "ldl";
  tabla[9] = "adc";
  tabla[10] = "call";
  tabla[11] = "cj";
  tabla[12] = "ajw";
  tabla[13] = "eqc";
  tabla[14] = "stl";
  tabla[15] = "stnl";
  tabla[16] = "opr";
  tabla[17] = "rev";
  tabla[18] = "lb";
  tabla[19] = "bsub";
  tabla[20] = "endp";
  tabla[21] = "diff";
  tabla[22] = "add";
  tabla[23] = "gcall";
  tabla[24] = "in";
  tabla[25] = "prod";
  tabla[26] = "gt";
  tabla[27] = "wsub";
  tabla[28] = "out";
  tabla[29] = "sub";
  tabla[30] = "startp";
  tabla[31] = "outbyte";
  tabla[32] = "outword";
  tabla[33] = "seterr";
  tabla[34] = "?";
  tabla[35] = "resetch";
  tabla[36] = "csub0";
  tabla[37] = "?";
  tabla[38] = "stopp";
  tabla[39] = "ladd";
  tabla[40] = "stlb";
  tabla[41] = "sthf";
  tabla[42] = "norm";
  tabla[43] = "ldiv";
  tabla[44] = "ldpi";
  tabla[45] = "stlf";
  tabla[46] = "xdble";
  tabla[47] = "ldpri";
  tabla[48] = "rem";
  tabla[49] = "ret";
  tabla[50] = "lend";
  tabla[51] = "ldtimer";
  tabla[52] =
  tabla[53] =
  tabla[54] =
  tabla[55] =
  tabla[56] =
  tabla[57] = "?";
  tabla[58] = "testerr";
  tabla[59] = "testpranal";
  tabla[60] = "tin";
  tabla[61] = "div";
  tabla[62] = "?";
  tabla[63] = "dist";
  tabla[64] = "disc";
  tabla[65] = "diss";
  tabla[66] = "lmul";
  tabla[67] = "not";
  tabla[68] = "xor";
  tabla[69] = "bcnt";
  tabla[70] = "lshr";
  tabla[71] = "lshl";
  tabla[72] = "lsum";
  tabla[73] = "lsub";
  tabla[74] = "runp";
  tabla[75] = "xword";
  tabla[76] = "sb";
  tabla[77] = "gajw";
  tabla[78] = "savel";
  tabla[79] = "saveh";
  tabla[80] = "wcnt";
  tabla[81] = "shr";
  tabla[82] = "shl";
  tabla[83] = "mint";
  tabla[84] = "alt";
  tabla[85] = "altwt";
  tabla[86] = "altend";
  tabla[87] = "and";
  tabla[88] = "enbt";
  tabla[89] = "enbc";
  tabla[90] = "enbs";
  tabla[91] = "move";
  tabla[92] = "or";
  tabla[93] = "csngl";
  tabla[94] = "ccnt1";
  tabla[95] = "talt";
  tabla[96] = "ldiff";
  tabla[97] = "sthb";
  tabla[98] = "taltwt";
  tabla[99] = "sum";
  tabla[100] = "mul";
  tabla[101] = "sttimer";
  tabla[102] = "stoperr";
  tabla[103] = "cword";
  tabla[104] = "clrhalterr";
}

inicia2() {
  tabla[105] = "sethalterr";
  tabla[106] = "testhalterr";
  tabla[107] = "dup";
  tabla[108] = "move2dinit";
  tabla[109] = "move2dall";
  tabla[110] = "move2dnonzero";
  tabla[111] = "move2dzero";
  tabla[112] =
  tabla[113] =
  tabla[114] =
  tabla[115] = "?";
  tabla[116] = "unpacksn";
  tabla[117] =
  tabla[118] =
  tabla[119] =
  tabla[120] =
  tabla[121] =
  tabla[122] =
  tabla[123] =
  tabla[124] = "?";
  tabla[125] = "postnormsn";
  tabla[126] = "roundsn";
  tabla[127] =
  tabla[128] =
  tabla[129] = "?";
  tabla[130] = "ldinf";
  tabla[131] = "fmul";
  tabla[132] = "cflerr";
  tabla[133] = "crcword";
  tabla[134] = "crcbyte";
  tabla[135] = "bitcnt";
  tabla[136] = "bitrevword";
  tabla[137] = "bitrevnbits";
  tabla[138] = "pop";
  tabla[139] = "timerdisableh";
  tabla[140] = "timerdisablel";
  tabla[141] = "timerenableh";
  tabla[142] = "timerenablel";
  tabla[143] = "ldmemstartval";
  tabla[144] =
  tabla[145] = "?";
  tabla[146] = "wsubdb";
  tabla[147] = "fpldnldbi";
  tabla[148] = "fpchkerror";
  tabla[149] = "fpstnldb";
  tabla[150] = "?";
  tabla[151] = "fpldnlsni";
  tabla[152] = "fpadd";
  tabla[153] = "fpstnlsn";
  tabla[154] = "fpsub";
  tabla[155] = "fpldnldb";
  tabla[156] = "fpmul";
  tabla[157] = "fpdiv";
  tabla[158] = "?";
  tabla[159] = "fpldnlsn";
  tabla[160] = "fpremfirst";
  tabla[161] = "fpremstep";
  tabla[162] = "fpnan";
  tabla[163] = "fpordered";
  tabla[164] = "fpnotfinite";
  tabla[165] = "fpgt";
  tabla[166] = "fpeq";
  tabla[167] = "fpi32tor32";
  tabla[168] = "?";
  tabla[169] = "fpi32tor64";
  tabla[170] = "?";
  tabla[171] = "fpb32tor64";
  tabla[172] = "?";
  tabla[173] = "fptesterror";
  tabla[174] = "fprtoi32";
  tabla[175] = "fpstnli32";
  tabla[176] = "fpldzerosn";
}

inicia3() {
  tabla[177] = "fpldzerodb";
  tabla[178] = "fpint";
  tabla[179] = "?";
  tabla[180] = "fpdup";
  tabla[181] = "fprev";
  tabla[182] = "?";
  tabla[183] = "fpldnladddb";
  tabla[184] = "?";
  tabla[185] = "fpldnlmuldb";
  tabla[186] = "?";
  tabla[187] = "fpldnladdsn";
  tabla[188] = "fpentry";
  tabla[189] = "fpldnlmulsn";
  tabla[190] =
  tabla[191] =
  tabla[192] =
  tabla[193] = "?";
  tabla[194] = "break";
  tabla[195] = "clrj0break";
  tabla[196] = "setj0break";
  tabla[197] = "testj0break";
  tabla[198] =
  tabla[199] =
  tabla[200] =
  tabla[201] =
  tabla[202] =
  tabla[203] =
  tabla[204] =
  tabla[205] =
  tabla[206] =
  tabla[207] =
  tabla[208] =
  tabla[209] =
  tabla[210] =
  tabla[211] =
  tabla[212] =
  tabla[213] =
  tabla[214] =
  tabla[215] =
  tabla[216] =
  tabla[217] = "?";
  tabla[218] =
  tabla[219] =
  tabla[220] =
  tabla[221] =
  tabla[222] =
  tabla[223] =
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
  tabla[237] = "?";
  tabla[238] =
  tabla[239] =
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
  tabla[255] =
  tabla[256] =
  tabla[257] = "?";
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
  tabla[271] =
  tabla[272] =
  tabla[273] = "?";
  tabla[274] = "fpusqrtfirst";
  tabla[275] = "fpusqrtstep";
  tabla[276] = "fpusqrtlast";
  tabla[277] = "fpurp";
  tabla[278] = "fpurm";
  tabla[279] = "fpurz";
  tabla[280] = "fpur32tor64";
  tabla[281] = "fpur64tor32";
  tabla[282] = "fpuexpdec32";
  tabla[283] = "fpuexpinc32";
  tabla[284] = "fpuabs";
  tabla[285] = "?";
  tabla[286] = "fpunoround";
  tabla[287] = "fpuchki32";
  tabla[288] = "fpuchki64";
  tabla[289] = "?";
  tabla[290] = "fpudivby2";
  tabla[291] = "fpumulby2";
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
  tabla[303] =
  tabla[304] =
  tabla[305] =
  tabla[306] = "?";
  tabla[307] = "fpurn";
  tabla[308] = "fpuseterror";
  tabla[309] =
  tabla[310] =
  tabla[311] = "?";
  tabla[312] =
  tabla[313] =
  tabla[314] =
  tabla[315] =
  tabla[316] =
  tabla[317] =
  tabla[318] =
  tabla[319] =
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
  tabla[331] = "?";
  tabla[332] =
  tabla[333] =
  tabla[334] =
  tabla[335] =
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
  tabla[367] =
  tabla[368] =
  tabla[369] =
  tabla[370] =
  tabla[371] = "?";
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
  tabla[383] =
  tabla[384] =
  tabla[385] =
  tabla[386] =
  tabla[387] =
  tabla[388] =
  tabla[389] =
  tabla[390] =
  tabla[391] = "?";
  tabla[392] =
  tabla[393] =
  tabla[394] =
  tabla[395] =
  tabla[396] =
  tabla[397] =
  tabla[398] =
  tabla[399] =
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
  tabla[411] = "?";
  tabla[412] =
  tabla[413] =
  tabla[414] =
  tabla[415] =
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
  tabla[427] =
  tabla[428] = "?";
  tabla[429] = "fpuclearerror";
  tabla[430] =
  tabla[431] =
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
  tabla[447] =
  tabla[448] =
  tabla[449] = "?";
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
  tabla[463] =
  tabla[464] =
  tabla[465] =
  tabla[466] =
  tabla[467] =
  tabla[468] =
  tabla[469] = "?";
  tabla[470] =
  tabla[471] =
  tabla[472] =
  tabla[473] =
  tabla[474] =
  tabla[475] =
  tabla[476] =
  tabla[477] =
  tabla[478] =
  tabla[479] =
  tabla[480] =
  tabla[481] =
  tabla[482] =
  tabla[483] =
  tabla[484] =
  tabla[485] =
  tabla[486] =
  tabla[487] =
  tabla[488] =
  tabla[489] = "?";
  tabla[490] =
  tabla[491] =
  tabla[492] =
  tabla[493] =
  tabla[494] =
  tabla[495] =
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
  tabla[509] = "?";
  tabla[510] =
  tabla[511] =
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

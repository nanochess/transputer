/*
** Editor de texto para el G10.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** 15-jun-1995
*/

#define EOF  (-1)
#define NULL  0
#define ESC   0x1b

int *apunta;
char *video;

char *primero, *ultimo, *pos_cursor, *pos_ventana;
int cur_linea, cur_columna;
int ven_linea, ven_columna;
int linea_act, columna_act;
int insercion;
int archivo;
int fondo, barra;

char nombre[15];

#define TAM_BUF 65536
char buffer[TAM_BUF];

#define TAM_DIR  5120
char dir[TAM_DIR];

#define TAM_Y  23
#define TAM_X  80

main() {
  int a, b, c;
  apunta = 0x80000110;
  video = *apunta;
  fondo = 0x1f;
  barra = 0x3f;
  color(barra);
  putchar(12);
  puts(" EDITOR   F1=Ayuda   NUEVO.C        Linea:1      Columna:1       Modo:Inserci¢n ");
  color(fondo);
  strcpy(nombre, "NUEVO.C");
  buffer[0] = 0;
  pos_ventana =
  pos_cursor =
  primero =
  ultimo = buffer;
  cur_linea = cur_columna = 0;
  ven_linea = ven_columna = 0;
  linea_act = columna_act = 0;
  insercion = 1;
  actualiza_ventana(0, 22);
  posiciona_cursor();
  while(1) {
    estatus();
    c = getchar();
    if(c == 0x1b) break;
    if((c >= 0x80) & (c <= 0xff)) c = 0;
    if(c == 0x03) acentua(&c);
    if((c >= 0x20) & (c <= 0xff) | (c == '\r')) {
      if(c == '\r') c = '\n';
      if(insercion) inserta_caracter(c);
      else if((*pos_cursor == '\n') | (*pos_cursor == 0)) inserta_caracter(c);
      else {
        *pos_cursor = c;
        if(c != '\n') actualiza_ventana(cur_linea, cur_linea);
        else actualiza_ventana(cur_linea, 22);
        posiciona_cursor();
      }
      mueve_der();
    } else if(c == 0x16) mueve_der(1);
    else if(c == 0x14) mueve_izq(1);
    else if(c == 0x18) mueve_arriba(1);
    else if(c == 0x12) mueve_abajo(1);
    else if(c == 0x19) {
      a = ven_linea;
      b = ven_columna;
      c = 0;
      while(c++<23) mueve_arriba(0);
      if((a != ven_linea) | (b != ven_columna))
        actualiza_ventana(0, 22);
      posiciona_cursor();
    } else if(c == 0x13) {
      a = ven_linea;
      b = ven_columna;
      c = 0;
      while(c++<23) mueve_abajo(0);
      if((a != ven_linea) | (b != ven_columna))
        actualiza_ventana(0, 22);
      posiciona_cursor();
    } else if(c == 0x17) {
      pos_cursor = pos_ventana = primero;
      cur_linea = cur_columna = 0;
      ven_linea = ven_columna = 0;
      linea_act = columna_act = 0;
      actualiza_ventana(0, 22);
      posiciona_cursor();
    } else if(c == 0x11) {
      pos_cursor = pos_ventana = primero;
      cur_linea = cur_columna = 0;
      ven_linea = ven_columna = 0;
      linea_act = columna_act = 0;
      while(*pos_cursor != 0) mueve_der(0);
      actualiza_ventana(0, 22);
      posiciona_cursor();
    } else if(c == 0x10) {
      insercion = !insercion;
    } else if(c == 0x01) ayuda();
    else if(c == 0x02) graba();
    else if(c == 0x04) lee();
    else if(c == 0x05) busca();
    else if(c == 0x06) pega();
    else if(c == 0x07) substituye();
    else if(c == 0x08) {
      mueve_izq();
      borra();
    }
    else if(c == 0x09) marca_inicio();
    else if(c == 0x0a) marca_final();
    else if(c == 0x0b) copia();
    else if(c == 0x0c) corta();
    else if(c == 0x0e) borra();
  }
  color(0x0a);
  putchar(12);
  exit(1);
}

inserta_caracter(c) int c; {
  char *inicio, *destino, bytes;
  if(ultimo == buffer + TAM_BUF - 1) return;
  inicio = ultimo;
  destino = ultimo+1;
  while(inicio != pos_cursor - 1) {
    *destino = *inicio;
    --destino;
    --inicio;
  }
  ++ultimo;
  *pos_cursor = c;
  if(c != '\n') actualiza_ventana(cur_linea, cur_linea);
  else actualiza_ventana(cur_linea, 22);
  posiciona_cursor();
}

/*
** Esto es necesario para los teclados no espa¤oles.
*/
acentua(car) char *car; {
  int sig;
  sig = getchar();
  *car = 0;
  if(sig == 'o') *car = '¢';
  else if(sig == 'a') *car = ' ';
  else if(sig == 'e') *car = '‚';
  else if(sig == 'i') *car = '¡';
  else if(sig == 'u') *car = '£';
  else if(sig == 'v') *car = '';
  else if(sig == 'q') *car = '¨';
  else if(sig == 'w') *car = '­';
  else if(sig == 'n') *car = '¤';
  else if(sig == 'E') *car = '';
  else if(sig == 'Q') *car = '¨';
  else if(sig == 'W') *car = '­';
}

estatus() {
  color(barra);
  pos(42, 0);
  decimal(linea_act + 1);
  pos(57, 0);
  decimal(columna_act + 1);
  pos(70, 0);
  if(insercion) puts("Inserci¢n");
  else          puts("Normal   ");
  color(fondo);
  posiciona_cursor();
}

decimal(numero) int numero; {
  char matriz[6];
  matriz[5] = 0;
  matriz[0] = ((numero / 10000) % 10) + '0';
  matriz[1] = ((numero /  1000) % 10) + '0';
  matriz[2] = ((numero /   100) % 10) + '0';
  matriz[3] = ((numero /    10) % 10) + '0';
  matriz[4] = ( numero          % 10) + '0';
  while((matriz[0] == '0') & (matriz[1] != ' ')) {
    matriz[0] = matriz[1];
    matriz[1] = matriz[2];
    matriz[2] = matriz[3];
    matriz[3] = matriz[4];
    matriz[4] = ' ';
  }
  puts(matriz);
}

borra() {
  char *inicio, *destino, c;
  if(pos_cursor == ultimo) return;
  c = *pos_cursor;
  inicio = pos_cursor + 1;
  destino = pos_cursor;
  while(destino != ultimo) {
    *destino = *inicio;
    ++destino;
    ++inicio;
  }
  --ultimo;
  if(c != '\n') actualiza_ventana(cur_linea, cur_linea);
  else actualiza_ventana(cur_linea, 22);
  posiciona_cursor();
}

mueve_arriba(actual) int actual; {
  int aven_linea, aven_columna;
  if(pos_cursor == primero) return;
  aven_linea = ven_linea;
  aven_columna = ven_columna;
  if(*(pos_cursor-1) != '\n') {
    while(1) {
      if(pos_cursor == primero) break;
      if(*(pos_cursor-1) == '\n') break;
      mueve_izq(0);
    }
  } else {
    mueve_izq(0);
    while(1) {
      if(pos_cursor == primero) break;
      if(*(pos_cursor-1) == '\n') break;
      mueve_izq(0);
    }
  }
  if(actual) {
    if((aven_linea != ven_linea) |
       (aven_columna != ven_columna))
      actualiza_ventana(0, 22);
    posiciona_cursor();
  }
}

mueve_abajo(actual) int actual; {
  int aven_linea, aven_columna;
  aven_linea = ven_linea;
  aven_columna = ven_columna;
  if((pos_cursor != primero) & (*(pos_cursor-1) != '\n')) {
    while(1) {
      if(pos_cursor == primero) break;
      if(*(pos_cursor-1) == '\n') break;
      mueve_izq(0);
    }
  } else {
    while(1) {
      if(*pos_cursor == 0) break;
      if(*pos_cursor == '\n') {
        mueve_der(0);
        break;
      }
      mueve_der(0);
    }
  }
  if(actual) {
    if((aven_linea != ven_linea) |
       (aven_columna != ven_columna))
      actualiza_ventana(0, 22);
    posiciona_cursor();
  }
}

mueve_izq(actual) int actual; {
  char *chequeo;
  int aven_linea, aven_columna;
  if(pos_cursor == primero) return;
  if(*--pos_cursor != '\n') {
    --cur_columna;
    --columna_act;
    if(cur_columna == -1) {
      ++cur_columna;
      --ven_columna;
      if(actual) actualiza_ventana(0, 22);
    }
  } else {
    aven_linea = ven_linea;
    aven_columna = ven_columna;
    --cur_linea;
    --linea_act;
    if(cur_linea == -1) {
      ++cur_linea;
      --ven_linea;
      pos_ventana = linea_ant(pos_ventana);
    }
    cur_columna =
    ven_columna =
    columna_act = 0;
    chequeo = prin_lin(pos_cursor);
    while(*chequeo++ != '\n') {
      ++cur_columna;
      ++columna_act;
      if(cur_columna == TAM_X) {
        --cur_columna;
        ++ven_columna;
      }
    }
    if((aven_linea != ven_linea) |
       (aven_columna != ven_columna))
      if(actual) actualiza_ventana(0, 22);
  }
  if(actual) posiciona_cursor();
}

mueve_der(actual) int actual; {
  if(*pos_cursor == 0) return;
  if(*pos_cursor++ != '\n') {
    ++cur_columna;
    ++columna_act;
    if(cur_columna == TAM_X) {
      --cur_columna;
      ++ven_columna;
      if(actual) actualiza_ventana(0, 22);
    }
  } else {
    cur_columna =
    ven_columna =
    columna_act = 0;
    ++cur_linea;
    ++linea_act;
    if(cur_linea == TAM_Y) {
      --cur_linea;
      ++ven_linea;
      pos_ventana = sig_linea(pos_ventana);
      if(actual) actualiza_ventana(0, 22);
    }
  }
  if(actual) posiciona_cursor();
}

actualiza_ventana(inicio, final) int inicio, final; {
  char *ven, *lin, *vid;
  int conteo;
  ven = pos_ventana;
  conteo = inicio;
  while(conteo--) {
    ven = sig_linea(ven);
  }
  while(inicio <= final) {
    vid = video + (inicio + 1) * 160;
    lin = ven;
    conteo = ven_columna;
    while(conteo--) {
      if((*lin != '\n') & (*lin != 0)) ++lin;
      else break;
    }
    conteo = 0;
    while(conteo < 80) {
      if((*lin == '\n') | (*lin == 0)) break;
      *vid++ = *lin++;
      *vid++ = 0x1f;
      ++conteo;
    }
    while(conteo < 80) {
      *vid++ = ' ';
      *vid++ = 0x1f;
      ++conteo;
    }
    if(*lin == '\n') ++lin;
    ++inicio;
    ven = sig_linea(ven);
  }
}

prin_lin(pos) char *pos; {
  while(1) {
    if(pos == primero) return pos;
    if(*--pos == '\n') return pos+1;
  }
}

linea_ant(pos) char *pos; {
  while(1) {
    if(pos == primero) return pos;
    if(*--pos == '\n') break;
  }
  while(1) {
    if(pos == primero) return pos;
    if(*--pos == '\n') break;
  }
  return ++pos;
}

sig_linea(pos) char *pos; {
  while((*pos != '\n') & (*pos != 0)) ++pos;
  if(*pos == '\n') ++pos;
  return pos;
}

posiciona_cursor() {
  pos(cur_columna, cur_linea + 1);
}

pos(x, y) int x, y; {
  putchar(0x1b);
  putchar(2);
  putchar(x);
  putchar(0x1b);
  putchar(3);
  putchar(y);
}

strcmp(uno, dos) char *uno, *dos; {
  while(1) {
    if(*uno < *dos) return -1;
    if(*uno > *dos) return 1;
    if(*uno == 0) return 0;
    uno++;
    dos++;
  }
}

strcpy(destino, inicio) char *destino, *inicio; {
  while(*destino++ = *inicio++) ;
}

strcat(destino, inicio) char *destino, *inicio; {
  while(*destino) ++destino;
  strcpy(destino, inicio);
}

color(col) int col; {
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

toupper(car) int car; {
  if((car >= 'a') & (car <= 'z')) return car - 32;
  else return car;
}

ventana(x, y, tx, ty, borde, texto)
  int x, y, tx, ty, borde, texto;
{
  int a,b;
  int nx,ny;
  pos(nx=x, ny=y);
  color(borde);
  putchar(0xda);
  a=0;
  while(a++<tx) putchar(0xc4);
  putchar(0xbf);
  b=0;
  while(b++<ty) {
    ++ny;
    pos(nx, ny);
    color(borde);
    putchar(0xb3);
    color(texto);
    a=0;
    while(a++<tx) putchar(' ');
    color(borde);
    putchar(0xb3);
  }
  ++ny;
  pos(nx, ny);
  color(borde);
  putchar(0xc0);
  a=0;
  while(a++<tx) putchar(0xc4);
  putchar(0xd9);
  pos(x+1, y+1);
  color(texto);
}

marca_inicio() {
}

marca_final() {
}

copia() {
}

corta() {
}

pega() {
}

busca() {
}

substituye() {
}

ayuda() {
  int x, y;
  x = 15;
  y = 2;
  ventana(x,y,45,15,0x2e,0x2f);
  ++x;
  ++x;
  ++y;
  pos(x,y);
  puts("Ayuda");
  ++y;
  ++y;
  pos(x,y);
  puts("F2 = Graba el texto en el disco.");
  ++y;
  pos(x,y);
  puts("F3 = Acentua la sig. pulsacion.");
  ++y;
  pos(x,y);
  puts("F4 = Lee un texto del disco.");
  ++y;
  pos(x,y);
  puts("F5 = Busqueda.");
  ++y;
  pos(x,y);
  puts("F6 = Pega el texto cortado.");
  ++y;
  pos(x,y);
  puts("F7 = Busqueda y substituci¢n.");
  ++y;
  pos(x,y);
  puts("F9 = Marca inicio de bloque.");
  ++y;
  pos(x,y);
  puts("F10= Marca final de bloque.");
  ++y;
  pos(x,y);
  puts("F11= Copia el bloque en la pos. del cursor.");
  ++y;
  pos(x,y);
  puts("F12= Corta el bloque.");
  ++y;
  pos(x,y);
  puts("Del= Borra el caracter debajo del cursor.");
  ++y;
  pos(x,y);
  puts("BkSp= Borra el caracter detras del cursor.");
  ++y;
  pos(x,y);
  puts("Ins= Cambia el modo de insercion/normal.");
  getchar();
  color(fondo);
  actualiza_ventana(0,22);
  posiciona_cursor();
}

#define LEE_SECTOR       1

char mensaje[520];

lee_sector(pista, lado, sector) int pista, lado, sector; {
  mensaje[0] = LEE_SECTOR;
  mensaje[1] = pista;
  mensaje[2] = lado;
  mensaje[3] = sector;
  apunta = mensaje;
  mandar(0x80000108, &apunta, 4);
  recibir(0x8000010c, &apunta, 1);
  return mensaje[0];
}

traga_dir() {
  int conteo, car, d, sector, posdir;
  char nombre[32];
  pos(2, 4);
  sector = 2;
  posdir = -1;
  d = 0;
  while(1) {
    if(posdir == -1) {
      lee_sector(0, 0, ++sector);
      posdir = 1;
    }
    conteo = 0;
    while(conteo < 32)
      nombre[conteo++] = mensaje[posdir++];
    if(posdir == 513) posdir = -1;
    if(nombre[0] == 0x80) continue;
    if(nombre[0] == 0) break;
    conteo = 0;
    while(conteo < 8)
      dir[d++] = nombre[conteo++];
    if(nombre[conteo] == ' ') dir[d++] = ' ';
    else dir[d++] = '.';
    while(conteo < 11)
      dir[d++] = nombre[conteo++];
  }
  return (d/12);
}

visual_dir(cuantos) int cuantos; {
  int x, y, conteo, salida;
  x = 2;
  y = 4;
  salida = 0;
  while(cuantos--) {
    pos(x, y);
    conteo = 0;
    while(conteo++ < 12)
      putchar(dir[salida++]);
    if(++y == 20) {
      y = 4;
      x = x + 16;
    }
  }
}

sel(cuantos) int cuantos; {
  int x, y, actual, car, conteo;
  char *dat;
  x = 1;
  y = 4;
  actual = 0;
  while(1) {
    pos(x, y);
    color(0xcf);
    putchar(' ');
    dat = dir;
    dat = dat + 12 * actual;
    conteo = 0;
    while(conteo++ < 12) putchar(*dat++);
    putchar(' ');
    car = getchar();
    pos(x, y);
    color(0x2f);
    putchar(' ');
    dat = dir;
    dat = dat + 12 * actual;
    conteo = 0;
    while(conteo++ < 12) putchar(*dat++);
    putchar(' ');
    if(car == 0x0d) return actual;
    else if(car == 0x1b) return -1;
    else if(car == 0x18) {
      if(actual) {
        --actual;
        if(--y == 3) {
          x = x - 16;
          y = 19;
        }
      }
    } else if(car == 0x14) {
      if(x != 1) {
        x = x - 16;
        actual = actual - 16;
      }
    } else if(car == 0x16) {
      if(actual + 16 <= cuantos - 1) {
        x = x + 16;
        actual = actual + 16;
      }
    } else if(car == 0x12) {
      if(actual != cuantos - 1) {
        ++actual;
        if(++y == 20) {
          x = x + 16;
          y = 4;
        }
      }
    } else if(car == 0x17) {
      actual = 0;
      x = 1;
      y = 4;
    } else if(car == 0x11) {
      x = 1 + 16 * ((cuantos - 1) / 16);
      y = 4 + ((cuantos - 1) % 16);
      actual = cuantos - 1;
    }
  }
}

lee() {
  int cuantos;
  char *posi;
  int conteo, val;
  char nom[15];
  ventana(0,1,78,21,0x2e,0x2f);
  puts(" Lectura de archivo:");
  cuantos = traga_dir();
  visual_dir(cuantos);
  cuantos = sel(cuantos);
  if(cuantos != -1) {
    posi = dir;
    posi = posi + cuantos * 12;
    conteo = val = 0;
    while(conteo++ < 12) {
      if(*posi != ' ')
        nom[val++] = *posi;
      ++posi;
    }
    while(val < 12) nom[val++] = ' ';
    nom[val] = 0;
    strcpy(nombre, nom);
    pos(21, 0);
    color(barra);
    puts(nombre);
    buffer[0] = 0;
    pos_ventana =
    pos_cursor =
    primero =
    ultimo = buffer;
    cur_linea = cur_columna = 0;
    ven_linea = ven_columna = 0;
    linea_act = columna_act = 0;
    estatus();
    actualiza_ventana(0,22);
    posiciona_cursor();
    archivo = fopen(nombre, "r");
    while((val = fgetc(archivo)) != EOF) {
      if(val == '\r') continue;
      if(val == 26) continue;
      if(val == '\n') {
        ++linea_act;
        estatus();
      }
      if(ultimo == buffer + TAM_BUF - 1) {
        memoria_agotada();
        break;
      }
      *ultimo++ = val;
    }
    *ultimo = 0;
    linea_act = 0;
    fclose(archivo);
    estatus();
  }
  color(fondo);
  actualiza_ventana(0,22);
  posiciona_cursor();
}

memoria_agotada() {
  ventana(29,10,18,1,0x4e,0x4f);
  puts(" Memoria agotada. ");
  getchar();
  color(fondo);
  actualiza_ventana(0,22);
  posiciona_cursor();
}

error_crea() {
  ventana(19,10,37,1,0x4e,0x4f);
  puts(" Error: No se pudo crear el archivo. ");
  getchar();
}

error_esc() {
  ventana(24,10,29,1,0x4e,0x4f);
  puts(" Error: El disco esta lleno. ");
  getchar();
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
      return 1;
    } else if(car == ESC) {
      puts("\r\n");
      *pos = 0;
      return 0;
    } else {
      if(ahora == pos + tam - 1) continue;
      putchar(car);
      *ahora++ = car;
    }
  }
}

graba() {
  int cuantos, val;
  char nom[15];
  char *dat;

  ventana(0,1,78,21,0x2e,0x2f);
  puts(" Grabaci¢n de archivo:");
  cuantos = traga_dir();
  visual_dir(cuantos);
  pos(2,21);
  puts("¨ Nombre para el archivo ? ");
  if(lee_linea(nom, 15) & (nom[0] != 0)) {
    pos(2,22);
    puts("Escribiendo...");
    archivo = fopen(nom, "w");
    val = 0;
    while(nom[val] != 0) ++val;
    while(val < 12) nom[val++] = ' ';
    nom[val] = 0;
    strcpy(nombre, nom);
    pos(21, 0);
    color(barra);
    puts(nombre);
    pos(16,22);
    if(archivo == NULL) error_crea();
    else {
      dat = primero;
      while(dat != ultimo) {
        if(*dat == '\n')
          if(fputc('\r', archivo) == EOF) {
            error_esc();
            break;
          }
        if(fputc(*dat++, archivo) == EOF) {
          error_esc();
          break;
        }
      }
      fclose(archivo);
    }
  }
  color(fondo);
  actualiza_ventana(0,22);
  posiciona_cursor();
}

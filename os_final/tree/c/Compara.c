/*
** Comparador de archivos.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 5 de julio de 1995.
** Revisión: 7 de septiembre de 1995. Muestra un mensaje si no hay diffs.
** Revisión: 10 de septiembre de 1995. Recorta las líneas demasiado largas.
*/

#define EOF  (-1)

main() {
  int a,b;
  int c,d;
  int linea1[80];
  int linea2[80];
  int e,f;
  char *g,*h;
  int i;

  puts("\r\n");
  color(15);
  puts("Comparador de Archivos  (c) Copyright 1995 Oscar Toledo G.\r\n");
  puts("\r\n");
  color(10);
  puts("Archivo original > ");
  entra_linea(linea1, 80);
  if(*linea1 == 0) {
    exit(1);
  }
  puts("Archivo nuevo    > ");
  entra_linea(linea2, 80);
  if(*linea2 == 0) {
    exit(1);
  }
  a = fopen(linea1, "r");
  b = fopen(linea2, "r");
  while(1) {
    e = lee_linea(a, linea1);
    f = lee_linea(b, linea2);
    if(e == EOF && f == EOF) {
      color(14);
      ilustra_linea("");
      ilustra_linea("No hay diferencias.");
      break;
    }
    g = linea1;
    h = linea2;
    while(1) {
      if(*g != *h) {
        color(14);
        ilustra_linea("---Archivo Original---------------------");
        color(10);
        ilustra_linea(linea1);
        for(i=0; i<9; i++) {
          lee_linea(a, linea1);
          ilustra_linea(linea1);
        }
        color(14);
        ilustra_linea("---Archivo Nuevo------------------------");
        color(10);
        ilustra_linea(linea2);
        for(i=0; i<9; i++) {
          lee_linea(b, linea2);
          ilustra_linea(linea2);
        }
        puts("\r\n");
        goto error;
      }
      if(*g == 0) break;
      ++g;
      ++h;
    }
  }
  error:
  fclose(a);
  fclose(b);
}

lee_linea(archivo, ap)
  int archivo;
  char *ap;
{
  int car;
  int longitud;

  longitud = 0;
  while(1) {
    car = fgetc(archivo);
    if(car == EOF) {
      *ap++ = '<';
      *ap++ = 'F';
      *ap++ = 'I';
      *ap++ = 'N';
      *ap++ = '>';
      *ap++ = 0;
      return EOF;
    }
    if(car == '\r') continue;
    if(car == '\n') {
      *ap = 0;
      break;
    }
    *ap++ = car;
    ++longitud;
    if(longitud == 74) {
      *ap = 0;
      break;
    }
  }
  return 0;
}

ilustra_linea(linea)
  char *linea;
{
  puts(linea);
  puts("\r\n");
}

color(col) int col; {
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

entra_linea(pos, tam) char *pos; int tam; {
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

/*
** Filtro de caracteres. (PC a ISO 8859-1)
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 29 de septiembre de 1995.
*/

#define NULL     0
#define EOF      (-1)
#define TEMPORAL "B:/_FILTRO_.001"

int filtro[256];
int linea[64];

int archivo;
int temp;

main()
{
  int a;
  int car;

  for(a=0; a<256; ++a)
    filtro[a] = a;

  filtro[0xa2] = 0xf3;
  filtro[0xa0] = 0xe1;
  filtro[0x82] = 0xe9;
  filtro[0xa1] = 0xed;
  filtro[0xa3] = 0xfa;
  filtro[0x81] = 0xfc;
  filtro[0xa8] = 0xbf;
  filtro[0xad] = 0xa1;
  filtro[0xa4] = 0xf1;
  filtro[0xa5] = 0xd1;
  filtro[0x90] = 0xc9;

  filtro[0xda] = 0x87;
  filtro[0xc4] = 0x85;
  filtro[0xbf] = 0x89;
  filtro[0xb3] = 0x86;
  filtro[0xc0] = 0x8d;
  filtro[0xd9] = 0x8f;

  color(15);
  puts("\r\nConvertidor de archivos de texto   (c) Copyright 1995 Oscar Toledo G.\r\n\n");

  color(11);
  puts("Conversión de conjunto PC a ISO 8859-1\r\n\n");

  color(14);
  puts("Advertencia: Este programa re-escribe los archivos.\r\n\n");

  color(10);
  puts("Archivo de entrada > ");
  entra_linea(linea, 64);
  if(*linea) {
    puts("\r\n");
    color(11);
    puts("Trasladando...");
    archivo = fopen(linea, "r");
    if(archivo == 0) {
      color(15);
      puts("\rArchivo inexistente.\r\n");
      exit(1);
    }
    temp = fopen(TEMPORAL, "w");
    if(temp == 0) {
      fclose(archivo);
      color(15);
      puts("\rNo se pudo crear el archivo temporal.\r\n");
      exit(1);
    }
    while(1) {
      car = fgetc(archivo);
      if(car == EOF) break;
      if(fputc(filtro[car], temp) == EOF) {
        color(15);
        puts("\rError al escribir el archivo temporal.\r\n");
        fclose(temp);
        fclose(archivo);
        remove(TEMPORAL);
        exit(1);
      }
    }
    fclose(temp);
    fclose(archivo);
    archivo = fopen(linea, "w");
    if(archivo == 0) {
      color(15);
      puts("\rNo pudo re-escribirse el archivo anterior.\r\n");
      exit(1);
    }
    temp = fopen(TEMPORAL, "r");
    if(temp == 0) {
      fclose(archivo);
      remove(TEMPORAL);
      color(15);
      puts("\rError fatal, no pudo abrirse el archivo temporal.\r\n");
      exit(1);
    }
    while(1) {
      car = fgetc(temp);
      if(car == EOF) break;
      if(fputc(car, archivo) == EOF) {
        color(15);
        puts("\rError fatal al escribir el archivo original.\r\n");
        fclose(temp);
        fclose(archivo);
        remove(TEMPORAL);
        exit(1);
      }
    }
    fclose(temp);
    fclose(archivo);
    remove(TEMPORAL);
    color(15);
    puts("\rFin de la conversión.\r\n");
  } else {
    color(15);
    puts("\r\nFiltro cancelado.\r\n");
  }
}

color(col)
  int col;
{
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

/*
** Concatena archivos.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 31 de diciembre de 1995.
*/

int archivo1, archivo2, archivo3;

char nombre1[256];
char nombre2[256];
char nombre3[256];

char buffer[32768];

main()
{
  int bytes;

  color(15);
  puts("\r\nConcatenador de Archivos   (c) Copyright 1995 Oscar Toledo G.\r\n");
  color(10);
  puts("\r\nArchivo 1 > ");
  lee_linea(nombre1, 256);
  if(nombre1[0] == 0) {
    color(15);
    puts("\r\nConcatenador cancelado.\r\n");
    exit(1);
  }
  archivo1 = fopen(nombre1, "r");
  if(archivo1 == 0) {
    color(15);
    puts("\r\nEl archivo no existe.\r\n");
    exit(1);
  }
  puts("Archivo 2 > ");
  lee_linea(nombre2, 256);
  if(nombre2[0] == 0) {
    fclose(archivo1);
    color(15);
    puts("\r\nConcatenador cancelado.\r\n");
    exit(1);
  }
  archivo2 = fopen(nombre2, "r");
  if(archivo2 == 0) {
    fclose(archivo1);
    color(15);
    puts("\r\nEl archivo no existe.\r\n");
    exit(1);
  }
  puts("\r\nArchivo destino > ");
  lee_linea(nombre3, 256);
  if(nombre3[0] == 0) {
    fclose(archivo1);
    fclose(archivo2);
    color(15);
    puts("\r\nConcatenador cancelado.\r\n");
    exit(1);
  }
  archivo3 = fopen(nombre3, "w");
  if(archivo3 == 0) {
    fclose(archivo1);
    fclose(archivo2);
    color(15);
    puts("\r\nEl archivo no existe.\r\n");
    exit(1);
  }
  color(11);
  puts("\r\nConcatenando...");
  while(bytes = fread(archivo1, buffer, 32768))
    fwrite(archivo3, buffer, bytes);
  fclose(archivo1);
  while(bytes = fread(archivo2, buffer, 32768))
    fwrite(archivo3, buffer, bytes);
  fclose(archivo2);
  fclose(archivo3);
  color(15);
  puts("\rFin de concatenación.\r\n");
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

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

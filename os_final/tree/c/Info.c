/*
** Obtiene información acerca de un programa ejecutable .p
**
** (c) Copyright 1996 Oscar Toledo G.
**
** Creación: 4 de septiembre de 1996.
*/

#define TAM_LINEA    256
#define EOF          (-1)

int a, b;
int tam_programa;
int tam_datos;
int tam_pila;
int tam_extra;

int encabezado[6];

char programa[TAM_LINEA];

main()
{
  color(15);
  puts("\r\nInformación de un programa ejecutable   (c) Copyright 1996 Oscar Toledo G.\r\n");
  color(10);
  puts("\r\nNombre del archivo origen (con extensión .p) > ");
  lee_linea(programa, TAM_LINEA);
  a = fopen(programa, "r");
  if (a == 0) {
    color(15);
    puts("\r\nNo se pudo localizar el origen.\r\n");
    exit(1);
  }
  fread(a, encabezado, sizeof(int [6]));
  if (encabezado[0] != 0x4542544f) {
    color(15);
    puts("\r\nNo tiene el encabezado OTBE.\r\n");
    fclose(a);
    exit(1);
  }
  if (encabezado[1] != 0x96190308) {
    color(15);
    puts("\r\nEl número de versión es desconocido.\r\n");
    fclose(a);
    exit(1);
  }
  color(11);
  puts("\r\nTamaño del programa: ");
  decimal(encabezado[2], 1);
  puts("\r\nTamaño de datos extra: ");
  decimal(encabezado[3], 1);
  puts("\r\nTamaño de la pila: ");
  decimal(encabezado[4], 1);
  puts("\r\nTamaño de datos globales: ");
  decimal(encabezado[5], 1);
  puts("\r\n");
  fclose(a);
  exit(1);
}

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
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

decimal(numero, corta)
  int numero, corta;
{
  int divisor, digito, cual, es;

  divisor = 1000000000;
  cual = es = 0;
  while (divisor) {
    digito = numero / divisor;
    numero = numero % divisor;
    if (divisor == 1)
      es = 1;
    if (digito) {
      putchar(digito + '0');
      es = 1;
    } else if (es) {
      putchar('0');
    } else if (!corta)
      putchar(' ');
    ++cual;
    if (cual == 1)
      if (es)
        putchar(',');
      else if (!corta)
        putchar(' ');
    if (cual == 4)
      if (es)
        putchar(',');
      else if (!corta)
        putchar(' ');
    if (cual == 7)
      if (es)
        putchar(',');
      else if (!corta)
        putchar(' ');
    divisor /= 10;
  }
}

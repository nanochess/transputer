/*
** Divide un archivo en partes.
**
** (c) Copyright 1996 Oscar Toledo G.
**
** Creación: 17 de mayo de 1996.
** Revisión: 20 de mayo de 1996. Revisado para ser más amigable.
*/

#define TAM_LINEA     256
#define NULL          ((void *) 0)
#define FILE          int
#define EOF           (-1)

int num_lineas;
int parte, divisor;

FILE *a, *b;
char *pos, *pos_base;

char entrada[TAM_LINEA];
char salida[TAM_LINEA];
char lineas[TAM_LINEA];

main()
{
  int val;

  color(15);
  puts("\r\nSeparador de archivos de texto  (c) Copyright 1996 Oscar Toledo G.\r\n");
  color(10);
  puts("\r\nArchivo de entrada > ");
  lee_linea(entrada, TAM_LINEA);
  if (entrada[0] == 0) {
    color(15);
    puts("\r\nSeparador cancelado.\r\n");
    return;
  }
  puts("\r\nArchivo de salida > ");
  lee_linea(salida, TAM_LINEA);
  if (salida[0] == 0) {
    color(15);
    puts("\r\nSeparador cancelado.\r\n");
    return;
  }
  puts("\r\n¿ Cuantas líneas por bloque ? ");
  lee_linea(lineas, TAM_LINEA);
  if (lineas[0] == 0) {
    color(15);
    puts("\r\nSeparador cancelado.\r\n");
    return;
  }
  num_lineas = proc_dec(lineas);
  if (num_lineas < 100)
    num_lineas = 100;
  parte = 1;
  color(14);
  puts("\r\nTrabajando...");
  a = fopen(entrada, "r");
  if (a == NULL) {
    color(15);
    puts("\rNo se pudo abrir el archivo de entrada.\r\n");
    return;
  }
  pos_base = salida + strlen(salida);
  while (1) {
    pos = pos_base;
    *pos++ = '.';
    divisor = 100;
    while (divisor) {
      *pos++ = (parte / divisor) % 10 + '0';
      divisor /= 10;
    }
    *pos = 0;
    b = fopen(salida, "w");
    if (b == NULL) {
      color(15);
      puts("\rError, no se pudo abrir la salida\r\n");
      break;
    } else {
      val = divide();
      fclose(b);
      if (val == EOF)
        break;
    }
    parte++;
  }
  color(15);
  puts("\rFin de separación\r\n");
  fclose(a);
}

strlen(pos)
  char *pos;
{
  char *inicio = pos;

  while (*pos) ++pos;
  return pos - inicio;
}

divide()
{
  int car, d;

  d = 0;
  while ((car = fgetc(a)) != EOF) {
    fputc(car, b);
    if (car == '\n') {
      d++;
      if (d == num_lineas)
        return car;
    }
  }
  return car;
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

proc_dec(donde)
  char *donde;
{
  int val;

  val = 0;
  while (*donde >= '0' && *donde <= '9')
    val = val * 10 + *donde++ - '0';
  return val;
}

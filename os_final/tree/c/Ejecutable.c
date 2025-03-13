/*
** Prepara un programa ejecutable .p
**
** (c) Copyright 1996 Oscar Toledo Gutiérrez.
**
** Creación: 3 de febrero de 1996.
*/

#define TAM_LINEA    256
#define EOF          (-1)

int a, b;
int tam_programa;
int tam_datos;
int tam_pila;
int tam_extra;

char encabezado[64];

char programa[TAM_LINEA];
char t_pila[TAM_LINEA];
char t_extra[TAM_LINEA];
char destino[TAM_LINEA];

main()
{
  color(15);
  puts("\r\nPrepara un programa ejecutable   (c) Copyright 1996 Oscar Toledo G.\r\n");
  color(10);
  puts("\r\nNombre del archivo origen (con extensión .e) > ");
  lee_linea(programa, TAM_LINEA);
  a = fopen(programa, "r");
  if (a == 0) {
    color(15);
    puts("\r\nNo se pudo localizar el origen.\r\n");
    exit(1);
  }
  tam_datos = obtiene_pila();
  fseek(a, 0, 2);
  tam_programa = (ftell(a) + 15) & ~15;
  fseek(a, 0, 0);
  color(11);
  puts("\r\nTamaño del programa: ");
  decimal(tam_programa, 1);
  puts("\r\nTamaño de los datos: ");
  decimal(tam_datos, 1);
  puts("\r\n\n");
  color(10);
  puts("Tamaño para la pila > ");
  lee_linea(t_pila, TAM_LINEA);
  tam_pila = proc_dec(t_pila);
  puts("Tamaño para datos extras > ");
  lee_linea(t_extra, TAM_LINEA);
  tam_extra = proc_dec(t_extra);
  tam_pila = (tam_pila + 15) & ~15;
  tam_extra = (tam_extra + 15) & ~15;
  puts("\r\nArchivo para el ejecutable .p > ");
  lee_linea(destino, TAM_LINEA);
  b = fopen(destino, "w");
  if (b == 0) {
    color(15);
    puts("\r\nNo se pudo abrir el destino.\r\n");
    fclose(a);
    exit(1);
  }
  escribe_entero(encabezado, 0x4542544f);
  escribe_entero(encabezado + 4, 0x96190308);
  escribe_entero(encabezado + 8, tam_programa);
  escribe_entero(encabezado + 12, tam_extra);
  escribe_entero(encabezado + 16, tam_pila);
  escribe_entero(encabezado + 20, tam_datos);
  memset(encabezado + 24, 0, 40);
  fwrite(b, encabezado, 64);
  while (tam_programa--)
    fputc(fgetc(a), b);
  fclose(b);
  fclose(a);
  color(14);
  puts("\r\nEjecutable .p preparado.\r\n");
  exit(1);
}

obtiene_pila()
{
  int instruccion, operando;
  int espacio;

  espacio = 0;
  while (1) {
    instruccion = lee_instruccion(&operando);
    if (instruccion == 9) {          /* Posiblemente con paquete matemático */
      espacio = 96;
      fseek(a, operando, 1);
      instruccion = lee_instruccion(&operando);
      if (instruccion != 0) {
        espacio = -1;
        break;
      }
      fseek(a, operando, 1);
      instruccion = lee_instruccion(&operando);
      if (instruccion != 11) {
        espacio = -1;
        break;
      }
      espacio += -operando;
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      if (instruccion != 11)
        break;
      espacio += -operando;
      break;
    } else if (instruccion == 0) {   /* Sin paquete matemático */
      fseek(a, operando, 1);
      instruccion = lee_instruccion(&operando);
      if (instruccion != 11) {
        espacio = -1;
        break;
      }
      espacio += -operando;
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      instruccion = lee_instruccion(&operando);
      if (instruccion != 11)
        break;
      espacio += -operando;
      break;
    } else if (instruccion == -1) {
      espacio = -1;
      break;
    }
  }
  return (espacio * 4);
}

lee_instruccion(operando)
  int *operando;
{
  int byte;

  *operando = 0;
  while (1) {
    byte = fgetc(a);
    if (byte == EOF)
      return -1;
    if ((byte & 0xf0) == 0x20) {
      *operando = (*operando << 4) | (byte & 0x0f);
    } else if ((byte & 0xf0) == 0x60) {
      *operando = ~((*operando << 4) | (byte & 0x0f));
    } else {
      *operando = (*operando << 4) | (byte & 0x0f);
      return (byte >> 4);
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

proc_dec(donde)
  char *donde;
{
  int val;

  val = 0;
  while (*donde >= '0' && *donde <= '9')
    val = val * 10 + *donde++ - '0';
  return val;
}

escribe_entero(pos, valor)
  char *pos;
  int valor;
{
  *pos = valor;
  *(pos + 1) = valor >> 8;
  *(pos + 2) = valor >> 16;
  *(pos + 3) = valor >> 24;
}

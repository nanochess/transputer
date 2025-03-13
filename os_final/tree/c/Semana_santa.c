/*
** Cálculo de la semana santa.
**
** (c) Copyright 1996 Oscar Toledo G.
**
** Creación: 21 de febrero de 1996.
*/

#define TAM_LINEA    80

int a;
char an[TAM_LINEA];

main()
{
  char *pos;
  int m, d;

  color(15);
  puts("\r\nCálculo de la fecha de la semana santa  (c) Copyright 1996 Oscar Toledo G.");
  puts("\r\n");
  color(10);
  puts("\r\n¿ En que año esta interesado (1582-4200) ? ");
  lee_linea(an, TAM_LINEA);
  pos = an;
  while (*pos == ' ')
    ++pos;
  a = proc_dec(pos);
  if (a < 1582 || a > 4200) {
    color(15);
    puts("\r\nPrograma cancelado.\r\n");
    exit(1);
  }
  calcula_semana_santa(a, &m, &d);
  muestra_calendario(a, m, d);
  puts("\r\n");
}

calcula_semana_santa(an, m, d)
  int an, *m, *d;
{
  int u, siglo, x, z, y, v, w;

  u = (an % 19) + 1;
  siglo = 1 + an / 100;
  x = (siglo * 3 / 4) - 12;
  z = ((8 * siglo + 5) / 25) - 5;
  y = (5 * an / 4) - x - 10;
  v = ((11 * u) + 20 + z - x) % 30;
  if (v < 0)
    v += 30;
  if ((v == 25 && u > 11) || v == 24)
    v++;
  w = 44 - v;
  if (w < 21)
    w += 30;
  w = w + 7 - ((y + w) % 7);
  if (w <= 31) {
    *m = 3;
    *d = w;
  } else {
    *m = 4;
    *d = w - 31;
  }
}

muestra_calendario(a, m, d)
  int a, m, d;
{
  int c, i, nd, e;

  puts("\r\n");
  inicia_linea();
  c = ((8 - (d % 7)) % 7);
  if (c == 0)
    c = 7;
  color(15);
  if (m == 3) {
    nd = 31;
    puts("M A R Z O");
  } else if (m == 4) {
    nd = 30;
    puts("A B R I L");
  }
  puts(" ");
  decimal(a, 0);
  puts("\r\n\n");
  inicia_linea();
  color(14);
  puts("Lu Ma Mi Ju Vi Sa Do\r\n\n");
  inicia_linea();
  e = c - 1;
  while (e--)
    puts("   ");
  for (i = 1; i <= nd; ++i) {
    if (i == d - 2 || c == 7)
      color(12);
    else
      color(9);
    if (i < 10)
      putchar(' ');
    else
      putchar(i / 10 + '0');
    putchar(i % 10 + '0');
    putchar(' ');
    if (++c == 8) {
      c = 1;
      puts("\r\n");
      inicia_linea();
    }
  }
}

inicia_linea()
{
  puts("                        ");
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

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

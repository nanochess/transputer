/*
** Emulador CP/M
**
** por Oscar Toledo Gutiérrez.
**
** (c) Copyright Oscar Toledo Gutiérrez 1996.
**
** Creación: 16 de enero de 1996.
** Revisión: 26 de febrero de 1996. Se comenta un poco más.
*/

/*
** Active la siguiente definición para que se ejecute el archivo BASIC.HEX
*/
/*
#define BASICO
*/

#define BASE_MEMORIA   6784

char memoria[72704];

main()
{
  color(15);
  puts("\r\nEmulador CP/M Z80  (c) Copyright Oscar Toledo G. 1996\r\n");

#ifdef BASICO

  puts("\r\nPulse Ctrl-Z para salir del PALO ALTO 2K TINY-BASIC.\r\n");
  lee_archivo_intel_hex("C:/Em_Z80/BASIC.HEX");

#else

  lee_archivo("C:/Em_Z80/BIOS.CPM", 0xfa00);
  memoria[BASE_MEMORIA + 0] = 0xc3;
  memoria[BASE_MEMORIA + 1] = 0x00;
  memoria[BASE_MEMORIA + 2] = 0xfa;

#endif

  color(10);
  emulador(memoria, 0);
  color(11);
  puts("\r\nFin de emulación.\r\n");
}

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

#ifndef BASICO

lee_archivo(archivo, dir)
  char *archivo;
  int dir;
{
  int a, c;

  dir += BASE_MEMORIA;
  a = fopen(archivo, "r");
  while((c = fgetc(a)) != -1)
    memoria[dir++] = c;
  fclose(a);
}

#else

lee_archivo_intel_hex(archivo)
  char *archivo;
{
  int a;
  int dir;
  int c;
  int num;

  a = fopen(archivo, "r");
  while(1) {
    while((c = fgetc(a)) != ':' && c != -1) ;
    if(c == -1) break;
    num = lee_hex(a);
    if(num == 0) break;
    dir = lee_hex(a) << 8;
    dir |= lee_hex(a);
    dir += BASE_MEMORIA;
    lee_hex(a);
    while(num--)
      memoria[dir++] = lee_hex(a);
  }
  fclose(a);
}

lee_hex(a)
  int a;
{
  int b,c;

  b = fgetc(a) - '0';
  if(b > 9) b = b - 7;
  c = fgetc(a) - '0';
  if(c > 9) c = c - 7;
  b = (b << 4) | c;
  return b;
}

#endif

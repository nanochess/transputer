/*
** Vaciador hexadecimal de archivos.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 15 de septiembre de 1995.
*/

#define EOF  (-1)

main() {
  int a, b, c, car, pos, base;
  int linea1[80];
  int bytes[16];
  char *ap;

  pos = 0;
  puts("\r\n");
  color(15);
  puts("Vaciador hexadecimal de archivos  (c) Copyright 1995 Oscar Toledo G.\r\n");
  puts("\r\n");
  color(10);
  puts("Archivo que se va a vaciar > ");
  entra_linea(linea1, 80);
  if(*linea1 == 0) {
    exit(1);
  }
  puts("\r\n");
  color(11);
  puts("Pulse Espacio para avanzar en el listado, Esc para cancelar.\r\n\n");
  puts("La tecla D para cambiar la posición dentro del archivo.\r\n\n");
  puts("Las teclas del cursor también funcionan.\r\n\n");
  a = fopen(linea1, "r");
  while(1) {
    base = ftell(a);
    for(c=0; c<16; c++) {
      pos = ftell(a);
      for(b=0; b<16; b++)
        bytes[b] = fgetc(a);
      color(14);
      vaciado_hexadecimal(pos >> 24);
      vaciado_hexadecimal(pos >> 16);
      puts(":");
      vaciado_hexadecimal(pos >> 8);
      vaciado_hexadecimal(pos);
      puts("  ");
      color(10);
      for(b=0; b<16; b++) {
        vaciado_hexadecimal(bytes[b]);
        puts(" ");
        if((b & 3) == 3)
          puts(" ");
      }
      color(0x6f);
      for(b=0; b<16; b++) {
        car = bytes[b];
        if(car < 0x20) car = '.';
        putchar(car);
      }
      color(10);
      puts("\r\n");
    }
    car = getchar();
    if(car == 0x1b || car == 0x03)
      break;
    if(car == 0x18) {
      if(base > 255)
        fseek(a, base - 256, 0);
      else
        fseek(a, 0, 0);
    } else if(car == 0x17) {
      fseek(a, 0, 0);
    } else if(car == 0x11) {
      fseek(a, 0, 2);
      base = ftell(a) & (-256);
      fseek(a, base, 0);
    } else if(car == 0x19) {
      if(base > 65535)
        fseek(a, base - 65536, 0);
      else
        fseek(a, 0, 0);
    } else if(car == 0x13) {
      fseek(a, 65280, 1);
    } else if(car == 'D' || car == 'd') {
      color(10);
      puts("\r\nIntroduzca la nueva dirección: ");
      entra_linea(linea1, 10);
      ap = linea1;
      base = 0;
      while(*ap) {
        car = toupper(*ap++) - '0';
        if(car > 9)
          car -= 7;
        base = (base << 4) | car;
      }
      base = base & (-256);
      fseek(a, base, 0);
    }
    puts("\r\n");
  }
  fclose(a);
}

toupper(a)
  int a;
{
  if(a >= 'a' && a <= 'z')
    return a - 32;
  else
    return a;
}

vaciado_hexadecimal(a)
  int a;
{
  char *d;

  d = "0123456789ABCDEF";
  putchar(d[(a >> 4) & 15]);
  putchar(d[a & 15]);
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

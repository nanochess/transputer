/*
** Muestra el conjunto de caracteres.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 29 de septiembre de 1995.
*/

main() {
  int a;
  int b;
  color(15);
  puts("\r\nConjunto de caracteres actual   (c) Copyright 1995 Oscar Toledo G.\r\n\n");
  color(15);
  puts("      0 1 2 3 4 5 6 7 8 9 a b c d e f\r\n\n");
  for(a = 0x20; a < 0x100; a += 0x10) {
    color(15);
    puts("0x");
    if(a >= 0xa0)
      putchar(65 + ((a >> 4) - 10));
    else
      putchar(48 + (a >> 4));
    puts("0  ");
    color(10);
    for(b = a; b < a + 16; ++b) {
      putchar(b);
      putchar(' ');
    }
    puts("\r\n");
  }
}

color(col) int col; {
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

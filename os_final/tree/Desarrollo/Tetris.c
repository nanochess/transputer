/*
** Tetris.
**
** por John Tromp.  <tromp@piring.cwi.nl>
**
** Centro de Matemáticas y Ciencias Computacionales (CWI)
** Oetgensstraat 7
** 1701CK Heerhugowaard
** Holanda
**
** Ganador del International Obsfuscated C Code Competition de 1989
** en la categoría de juegos.
**
** Limpiado por Oscar Toledo G. y adaptado para el G10.
** 8 de marzo de 1996.
*/

#define CAIDAS_POR_SEGUNDO   (15625 / 2)
#define AZUL_OSCURO          0x01
#define NADA                 0x00

int c, i, I, j, posicion, col;
int *pieza, *m;
int w;                   /* Puntuación */
int contando;
int f[76];
int q[276];
int Q[276];

main(argc, argv)
  int argc;
  char **argv;
{
  char *a;

  f[ 0] =   7;  f[ 1] = -13;  f[ 2] = -12;  f[ 3] =   1;
  f[ 4] =   8;  f[ 5] = -11;  f[ 6] = -12;  f[ 7] =  -1;
  f[ 8] =   9;  f[ 9] =  -1;  f[10] =   1;  f[11] =  12;
  f[12] =   3;  f[13] = -13;  f[14] = -12;  f[15] =  -1;
  f[16] =  12;  f[17] =  -1;  f[18] =  11;  f[19] =   1;
  f[20] =  15;  f[21] =  -1;  f[22] =  13;  f[23] =   1;
  f[24] =  18;  f[25] =  -1;  f[26] =   1;  f[27] =   2;
  f[28] =   0;  f[29] = -12;  f[30] =  -1;  f[31] =  11;
  f[32] =   1;  f[33] = -12;  f[34] =   1;  f[35] =  13;
  f[36] =  10;  f[37] = -12;  f[38] =   1;  f[39] =  12;
  f[40] =  11;  f[41] = -12;  f[42] =  -1;  f[43] =   1;
  f[44] =   2;  f[45] = -12;  f[46] =  -1;  f[47] =  12;
  f[48] =  13;  f[49] = -12;  f[50] =  12;  f[51] =  13;
  f[52] =  14;  f[53] = -11;  f[54] =  -1;  f[55] =   1;
  f[56] =   4;  f[57] = -13;  f[58] = -12;  f[59] =  12;
  f[60] =  16;  f[61] = -11;  f[62] = -12;  f[63] =  12;
  f[64] =  17;  f[65] = -13;  f[66] =   1;  f[67] =  -1;
  f[68] =   5;  f[69] = -12;  f[70] =  12;  f[71] =  11;
  f[72] =   6;  f[73] = -12;  f[74] =  12;  f[75] =  24;

  a = "\x14\x18\x16\x20\x01\x1b";
  for (i = 0; i < 264; i++) {
    if (i >= 252 || (i + 1) % 12 < 2)
      q[i] = AZUL_OSCURO;
    else
      q[i] = NADA;
  }
  limpia_video();

  contando = 0;

  posicion = 17;
  pieza = f + rand() % 7 * 4;
  col = (rand() % 7) + 9;
  while (1) {
    if (c < 0) {
      if (valido(posicion + 12))
        posicion += 12;
      else {
        pone(col);
        ++w;
        for (j = 0; j < 252; j = 12 * (j / 12 + 1)) {
          while (q[++j]) {
            if (j % 12 == 10) {
              while (j % 12)
                q[j--] = 0;
              visualiza();
              while (--j)
                q[j + 12] = q[j];
              visualiza();
            }
          }
        }
        posicion = 17;
        pieza = f + rand() % 7 * 4;
        col = (rand() % 7) + 9;
        if (!valido(posicion))
          c = a[5];
      }
    }
    if (c == *a && valido(posicion - 1))
      --posicion;
    if (c == a[1]) {
      m = pieza;
      pieza = f + 4 * *pieza;
      if (!valido(posicion))
        pieza = m;
    }
    if (c == a[2] && valido(posicion + 1))
      ++posicion;
    if (c == a[3]) {
      while (valido(posicion + 12)) {
        ++w;
        posicion += 12;
      }
    }
    if (c == a[4] || c == a[5]) {
      limpia_video();
      color(10);
      decimal(w);
      if (c == a[5])
        break;
      for (j = 264; j--; Q[j] = 0) ;
      while (checa_tecla() != a[4]) ;
      limpia_video();
    }
    pone(col);
    visualiza();
    pone(0);
  }
/*
  d = popen("stty -cbreak echo stop \023;sort -mnr -o HI - HI;cat HI", "w");
  fprintf(d, "%4d from level %1d by %s\n", w, l, getlogin());
  pclose(d);
*/
}

/*
** Pone una pieza en el color b.
*/
pone(b)
  int b;
{
  for (i = 1; i <= 3; ++i)
    q[posicion + pieza[i]] = b;
}

/*
** Actualiza la visualización.
*/
visualiza()
{
  for (i = 12; i < 264; i++) {
    if (q[i] != Q[i]) {
      Q[i] = q[i];
      if (i - ++I || i % 12 == 0) {
        I = i;
        putchar(0x1b);
        putchar(3);
        putchar(i / 12);
        putchar(0x1b);
        putchar(2);
        putchar(i % 12 * 2 + 28);
      }
      color(q[i]);
      putchar(0x9f);
      putchar(0x9f);
    }
  }
  if (contando == 0) {
    c = Q[275];
    contando = clock();
  } else {
    Q[275] = c = checa_tecla();
    if (clock() - contando >= CAIDAS_POR_SEGUNDO) {
      contando = 0;
      c = -1;
    }
  }
}

/*
** Determina si se puede poner una pieza en la posición b.
*/
valido(b)
  int b;
{
  for (i = 1; i <= 3; ++i)
    if (q[b + pieza[i]])
      return 0;
  return 1;
}

limpia_video()
{
  putchar(12);
}

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

decimal(val)
  int val;
{
  if (val >= 10)
    decimal(val / 10);
  putchar(val % 10 + '0');
}

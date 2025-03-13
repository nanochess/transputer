/*
** Ajedrez.
**
** por Vern Paxson.  <tromp@piring.cwi.nl>
**
**     Lawrence Berkeley Laboratory
**     Computer Systems Engineering
**     Bldg. 46A, Room 1123
**     Lawrence Berkeley Laboratory
**     1 Cyclotron Rd.
**     Berkeley, CA 94720  USA
**
** Ganador del International Obsfuscated C Code Competition de 1989
** en la categoría de la mejor muestra.
**
** Limpiado por Oscar Toledo G. y adaptado para el G10.
** 19 de marzo de 1996.
*/

/*
** abs()
*/
#define a(x) (x < 0 ? -x : x)

/*
** sign()
*/
#define m(x) (x < 0 ? -1 : !!x)

#define h(x) ((x) <= 3 ? (x) : 7 - (x))

char *donde;
int v, w, Y, W, J, p, F, o, X, YY, _, P[64], s();

typedef int (*L)();

L q[64];

tj()
{
  int S = m(v) + (m(w) << 3);

  if (!S)
    return J; 
  for (v = W + S; v != J && !q[v]; v += S) ;
  return v; 
}

/*
** Movimiento del peon.
*/
k()
{
  _ = 'P';
  return v ? a(v) > 1 || w - Y || !q[J] :
             (w - Y && (w - Y * 2 || q[W + Y * 8] ||
              (J >> 3) - 3 + (Y - 1) / 2)) || q[J]; 
}

/*
** Movimiento del caballo
*/
bb()
{
  _ = 'C';
  return a(v * w) - 2;
}

/*
** Movimiento del alfil.
*/
c()
{
  _ = 'A';
  return a(v) - a(w) || tj() - J;
}

/*
** Movimiento de la torre.
*/
z()
{
  _ = 'T';
  return v * w || tj() - J;
}

/*
** Movimiento de la dama.
*/
l()
{
  _ = 'D';
  return (v * w && a(v) - a(w)) || tj() - J; 
}

/*
** Movimiento del rey.
*/
e()
{
  _ = 'R';
  return (v * v * v - v || w * w * w - w) &&
         (J - W - 2 || (W & 7) - 4 || (W >> 3 != (Y - 1 ? 7 : 0)) ||
         q[W + 1] || q[W + 2] || q[W + 3] != z || P[W + 3] * Y < 0); 
}

/*
** Checa si puede mover desde ur a n
**
** Si x no es 0, también checa si su rey queda en jaque.
*/
I(ur, n, x)
  int ur, n, x;
{
  /*
  ** Checa si es correcto el origen y el destino. 
  */
  if (P[ur] != Y || P[n] == Y)
    return 1;
  W = ur;
  J = n;
  /*
  ** Descompone en fila y columna.
  */
  v = (n & 7) - (ur & 7);
  w = (n >> 3) - (ur >> 3);
  /*
  ** Llama a la función de la pieza y opcionalmente al chequeo de jaque.
  */
  return (*q[ur])() || (x && QL(ur,n,s));
}

TT(W)
  int W;
{
  /*
  ** Indica que no evalue movimientos, sólo se require la letra de la pieza.
  */
  v = w = 0; 
  return (*q[W])() + 3; 
}

/*
** Detecta si se ataca al rey del bando actual.
*/
s()
{
  int j = -1, i;

  Y = -Y;
  for (i = 0; i < 64; ++i) {
    if (j < 0 && P[i] == -Y && TT(i) && _ == 'R') {
      j = i;
      i = -1;
    } else if (j >= 0 && !I(i, j, 0))
      return (Y = -Y);
  }
  return !(Y = -Y);
}

/*
** Inicializa el tablero.
*/
uv()
{
  for (v = 0; v < 64; ++v) {
    if (h(v >> 3) == 0) {
      int S = h(v & 7);

      q[v] = !S ? z : (S == 1 ? bb : (S == 2 ? c : (v & 7 > 3 ? l : e)));
    } else if (h(v >> 3) == 1)
      q[v] = k;
    else
      q[v] = 0;
    P[v] = !!q[v] * (28 - v);
  }
}

/*
** Muestra el tablero.
*/
y()
{
  int G = Y, i;

  J = 0; 
  for (i = 0; i < 64; ++i) {
    if ((i % 8) == 0) {
      puts("\r\n  ");
      putchar('0' + i / 8);
      putchar('0');
      putchar(' ');
    }
    if ((Y = P[i] = m(P[i])) && TT(i)) {
      putchar(Y < 0 ? _ : _ + 32);
      putchar(' ');
    } else
      puts("- ");
  }
  puts("\r\n     ");
  do {
    putchar('0' + (i++ & 7));
    putchar(' ');
  } while (i & 7) ;
  Y = G; 
  puts("\r\n");
}

/*
** Mueve una pieza (desde W a J).
*/
O(W, J)
  int W, J;
{
  /*
  ** Si es un peon, entonces checa si corona.
  */
  if ((q[J] = q[W]) == k && h(J >> 3) == 0)
    q[J] = l;
  /*
  ** Si es un rey, entonces checa si hace enroque.
  */
  if (q[W] == e)
    if (J - W == 2)
      O(J + 1, J - 1);
    else if (W - J == 2)
      O(W - 1, W + 1);
  /*
  ** Mueve la pieza en cuestión.
  */
  P[J] = P[W];
  q[W] = 0;
  P[W] = 0;
}

/*
** Ejecuta una jugada, ejecuta una función D, regresa la jugada.
** Retorna el resultado de la función D.
*/
QL(W, J, D)
  int W, J;
  L D;
{
  int HQ, YX; 
  L AJ, XY;

  /*
  ** Salva información del tablero.
  */
  HQ = P[J];
  XY = q[W];
  AJ = q[J];
  /*
  ** Efectua la jugada.
  */
  O(W, J);
  /*
  ** Ejecuta la función de evaluación.
  */
  YX = (*D)();
  /*
  ** Retrocede la jugada.
  */
  O(J, W);
  /*
  ** Restaura información del tablero.
  */
  q[J] = AJ;
  q[W] = XY;
  P[J] = HQ;
  return YX;
}

/*
** Evalua la posición.
*/
C()
{
  int i, j, BZ = 0;

  for (i = 0; i < 64; ++i) {
    L Z;

    if (Z = q[i]) {
      int r = h(i >> 3) + h(i & 7), G = Y,
          S, t1, t2;

      if (Z == z)               /* ¿ Es una torre ? */
        S = 88;                 /* 88 puntos por tener una torre */
      else if (Z == k)          /* ¿ Es un peón ? */
        S = 11 + r +            /* 11 puntos por tener un peón */
           (P[i] < 0 ?          /* más 1 punto por cada línea que */
            7 - (i >> 3) :      /* esté adelantado, más puntos */
            (i >> 3));          /* por estar en el centro. */
      else if (Z == l)          /* ¿ Es una dama ? */
        S = 124 - ((YY < 8 &&   /* 124 puntos por tener una dama */
            ((i && 7) != 3 ||   /* Menos 64 puntos por no moverla */
            (i >> 3) !=         /* después de 8 movimientos del usuario */
            (P[i] > 0 ? 0 :
            7))) ? 64 : 0);
      else if (Z == c)          /* ¿ Es un alfil ? */
        S = 41 + r;             /* 41 puntos por tener un alfil */
                                /* más puntos por estar en el centro */
      else if (Z == e)          /* ¿ Es un rey ? */
        S = 9999 - r - r;       /* 9999 puntos menos puntos por estar en */
                                /* el centro */
      else                      /* ¿ Es un caballo ? */
        S = 36 + r + r;         /* 36 puntos más puntos por estar en */
                                /* el centro */
      Y = P[i];
      /*
      ** Suma cinco puntos por cada pieza que pueda capturar.
      ** Suma un punto por cada cuadro al que pueda moverse.
      */
      for (j = 0; j < 64; ++j)
        if (!I(i, j, 0))
          S += (P[j] ? 5 : 1);
      BZ += G == Y ? S : -S;
      Y = G; 
    }
  }
  /*
  ** Muestra un punto por cada 64 posiciones evaluadas.
  */
  if (!(++X & 63))
    putchar('.');
  return BZ; 
}

/*
** La computadora intenta jugar.
*/
PX()
{
  int i, Q = 0, XP = 0, JZ = 4096, E = -9999, t, S = o; 

  if (!F--)
    return ++F + C();
  for (i = 0; i < JZ; ++i)
    if (!I(i >> 6, i & 63, 1)) {
      Y = -Y;
      o = -E;
      t = -QL(i >> 6, i & 63, PX); 
      Y = -Y; 
      if (t > E) {
        ++XP; 
        Q = i; 
        E = t; 
        if (E >= S)
          return ++F, E; 
      }
    }
  if (!XP)
    E = s() ? -9998 : 0;
  p = Q; 
  return ++F, E;
}

/*
** Muestra el tablero, pide el movimiento del usuario,
** checa si es legal, muestra el tablero y ejecuta el
** turno de la computadora.
*/
RZ()
{
  int i, j, T = 0; 
  char linea[80];

  for (; ;) {
    y(); 
    o = 9999; 
    do {
      puts("\r\n");
      decimal(X);
      putchar(' ');
      decimal(T);
      putchar(' ');
      decimal(C());
      puts(s() ? " !" : " >");
      lee_linea(linea, 80);
      if (linea[0] == 0)
        exit(1);
      donde = linea;
      i = proc_dec();
      i = ((i / 10) * 8) + i % 10;
      if (i == -1)
        continue;
      j = proc_dec();
      j = ((j / 10) * 8) + j % 10;
      if (j == -1)
        continue;
      if (!I(i, j, 1))
        break;
    } while (1);
    O(i, j);
    y();
    X = 0;
    ++YY;
    Y = -Y;
    T = PX();
    i = p >> 6;
    j = p & 63;
    if (I(i, j, 1)) {
      puts("Rats!\r\n"); 
      return; 
    }
    O(i, j);
    Y = -Y;
    if (T > 4096)
      puts("\r\nHar har.\r\n");
  }
}

main(ac,av)
  int ac;
  char **av;
{
  Y = -1;
  o = 9999;
  donde = av[1];
  F = ac > 1 ? proc_dec() : 2;
  uv();
  RZ(); 
}

decimal(val)
  int val;
{
  if (val < 0) {
    val = -val;
    putchar('-');
  }
  if (val > 10)
    decimal(val / 10);
  putchar(val % 10 + '0');
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

proc_dec()
{
  int val;

  while (*donde == ' ')
    ++donde;
  if (*donde < '0' || *donde > '9')
    return -1;
  val = 0;
  while (*donde >= '0' && *donde <= '9')
    val = val * 10 + *donde++ - '0';
  return val;
}

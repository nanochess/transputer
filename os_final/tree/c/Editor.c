/*
** Editor de texto para el G10.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 13 de junio de 1995.
** Revisión: 19 de junio de 1995. Mejora en la presentación total.
** Revisión: 21 de junio de 1995. Soporte para cortar y pegar.
** Revisión: 10 de julio de 1995. Soporte para busqueda y substitución.
** Revisión: 20 de julio de 1995. Mantiene los limites del bloque si el
**                                usuario se equivoca en busqueda y subs.
** Revisión: 9 de agosto de 1995. Substitución de & y | con && y || en
**                                algunas partes, esto acelera el editor.
** Revisión: 9 de agosto de 1995. Ligeras mejoras en el interfaz con el
**                                usuario.
** Revisión: 12 de agosto de 1995. Soporte para leer archivos en otras
**                                 unidades o subdirectorios.
** Revisión: 22 de septiembre de 1995. Indica en que línea esta la cadena
**                                     de letras encontrada.
** Revisión: 27 de septiembre de 1995. Cambio el color de las ventanas.
** Revision: 30 de septiembre de 1995. Soporte para el conjunto ISO 8859-1.
** Revisión: 9 de octubre de 1995. Ventana de "¿Esta seguro?" al pulsarse Esc.
** Revisión: 26 de octubre de 1995. Soporte para la secuencia F3 tecla en la
**                                  función lee_linea, la función toupper()
**                                  ahora funciona con el conjunto ISO8859-1.
** Revisión: 22 de noviembre de 1995. Navegación entre directorios más rápida.
** Revisión: 22 de noviembre de 1995. Expande automáticamente los tab's,
**                                    autodetecta que terminación de línea usa
**                                    el archivo.
** Revisión: 4 de diciembre de 1995. Soporte para directorios muy largos.
** Revisión: 22 de diciembre de 1995. Corrección en el acceso a subdirectorios.
** Revisión: 1o. de enero de 1996. Soporte para nombres de 15 letras.
** Revisión: 4 de enero de 1996. Corrección de un defecto en la ilustración
**                               de directorios.
** Revisión: 11 de enero de 1996. Soporte para insertar '\f' en un archivo
**                                de texto.
** Revisión: 15 de enero de 1996. Asume que la unidad A: siempre existe.
**                                Empieza en la unidad C:.
**                                Esto ahorra un posible desgaste del disco.
** Revisión: 31 de enero de 1996. Soporte para colorear la sintaxis del
**                                lenguaje C.
** Revisión: 2 de febrero de 1996. Soporte para C++.
** Revisión: 2 de febrero de 1996. Soporte para colorear los números.
** Revisión: 6 de febrero de 1996. Soporte para nombres de 32 letras.
** Revisión: 8 de febrero de 1996. Ahora hace su salida a través de la terminal.
** Revisión: 8 de febrero de 1996. Ahora puede variarse el tamaño de la
**                                 ventana.
** Revisión: 16 de marzo de 1996. Corrección de un defecto en la exploración
**                                de directorios.
** Revisión: 7 de abril de 1996. Se le hace un corte en la visualización si
**                               se detectan teclas, para acelerar la respuesta
** Revisión: 13 de octubre de 1996. Corrección de un defecto en visualización
**                                  de 0x0c.
*/

#define EOF  (-1)
#define NULL  0
#define ESC   0x1b

#define NO    0
#define SI    1

char *primero, *ultimo, *pos_cursor, *pos_ventana;
char *inicio_bloque, *final_bloque;
int cur_linea, cur_columna;
int ven_linea, ven_columna;
int linea_act, columna_act;
int insercion;
int archivo;
int fondo, barra;
int colorea_sintaxis;
int tecla_pulsada;

int archivos_totales;

#define NUM_PAL_RES        48

char *palabras_reservadas[NUM_PAL_RES];
int color_sintaxis[32];

char unidades[26];
char camino_actual[256];
char nombre_archivo[256];

#define TAM_BUF 65536
char buffer[TAM_BUF];

#define TAM_DIR  1120
char dir[TAM_DIR];

#define TAM_Y  23
#define TAM_X  80

#define TEMPORAL "B:Editor_texto.1"

main()
{
  int a, b, c;

  inicia_sintaxis();
  apaga_cursor();
  fondo = 0x0a;
  barra = 0x1f;
  color(barra);
  putchar(12);
  puts(" EDITOR  ");
  color(0x1e);
  puts("F1=Ayuda                   Linea:1      Columna:1       Modo:Inserción ");
  color(fondo);
  buffer[0] = 0;
  pos_ventana =
  pos_cursor =
  primero =
  ultimo = buffer;
  cur_linea = cur_columna = 0;
  ven_linea = ven_columna = 0;
  linea_act = columna_act = 0;
  insercion = 1;
  inicio_bloque = primero;
  final_bloque = primero;
  actualiza_ventana(0, TAM_Y - 1);
  checa_unidades();
  while(1) {
    estatus();
    posiciona_cursor();
    c = lee_teclado();
    if (c == 0x1b) {
      if (seguro())
        break;
      actualiza_ventana(7, 10);
      posiciona_cursor();
      c = 0;
    }
    if (c == 0x03)
      acentua(&c);
    if (c == 0xa0) {
      colorea_sintaxis = !colorea_sintaxis;
      actualiza_ventana(0, TAM_Y - 1);
      posiciona_cursor();
      c = 0;
    }
    if ((c >= 0x20 && c <= 0xff) || c == '\r' || c == 0x15) {
      if (c == 0x15)
        c = 0x0c;
      if (c == '\r')
        c = '\n';
      if (insercion)
        inserta_caracter(c, 1);
      else if (*pos_cursor == '\n' || *pos_cursor == 0)
        inserta_caracter(c, 1);
      else {
        *pos_cursor = c;
        if (c != '\n')
          actualiza_ventana(cur_linea, cur_linea);
        else
          actualiza_ventana(cur_linea, TAM_Y - 1);
        posiciona_cursor();
      }
      mueve_der(1);
    } else if (c == 0x16)
      mueve_der(1);
    else if (c == 0x14)
      mueve_izq(1);
    else if (c == 0x18)
      mueve_arriba(1);
    else if (c == 0x12)
      mueve_abajo(1);
    else if (c == 0x19) {
      a = ven_linea;
      b = ven_columna;
      c = 0;
      while (c++ < TAM_Y)
        mueve_arriba(0);
      if (a != ven_linea || b != ven_columna)
        actualiza_ventana(0, TAM_Y - 1);
      posiciona_cursor();
    } else if (c == 0x13) {
      a = ven_linea;
      b = ven_columna;
      c = 0;
      while (c++ < TAM_Y)
        mueve_abajo(0);
      if (a != ven_linea || b != ven_columna)
        actualiza_ventana(0, TAM_Y - 1);
      posiciona_cursor();
    } else if (c == 0x17) {
      pos_cursor = pos_ventana = primero;
      cur_linea = cur_columna = 0;
      ven_linea = ven_columna = 0;
      linea_act = columna_act = 0;
      actualiza_ventana(0, TAM_Y - 1);
      posiciona_cursor();
    } else if (c == 0x11) {
      pos_cursor = pos_ventana = primero;
      cur_linea = cur_columna = 0;
      ven_linea = ven_columna = 0;
      linea_act = columna_act = 0;
      while (*pos_cursor)
        mueve_der(0);
      actualiza_ventana(0, TAM_Y - 1);
      posiciona_cursor();
    } else if (c == 0x10)
      insercion = !insercion;
    else if (c == 0x01)
      ayuda();
    else if (c == 0x02)
      graba();
    else if (c == 0x04)
      lee();
    else if (c == 0x05)
      busca();
    else if (c == 0x06)
      pega();
    else if (c == 0x07)
      substituye();
    else if (c == 0x08) {
      mueve_izq(1);
      borra(1);
    } else if (c == 0x09)
      marca_inicio();
    else if (c == 0x0a)
      marca_final();
    else if (c == 0x0b)
      copia();
    else if (c == 0x0c)
      corta();
    else if (c == 0x0e)
      borra(1);
  }
  enciende_cursor();
  color(0x0a);
  putchar(12);
  remove(TEMPORAL);
  exit(1);
}

apaga_cursor()
{
  putchar(0x1b);
  putchar(4);
  putchar(0);
}

enciende_cursor()
{
  putchar(0x1b);
  putchar(4);
  putchar(1);
}

inserta_caracter(c, actual)
  int c, actual;
{
  char *inicio, *destino, bytes;

  if (ultimo == buffer + TAM_BUF - 1)
    return;
  inicio = ultimo;
  destino = ultimo+1;
  while (inicio != pos_cursor - 1)
    *destino-- = *inicio--;
  ++ultimo;
  *pos_cursor = c;
  if (pos_cursor < inicio_bloque)
    ++inicio_bloque;
  if (pos_cursor < final_bloque)
    ++final_bloque;
  if (actual) {
    if (c != '\n' && !colorea_sintaxis)
      actualiza_ventana(cur_linea, cur_linea);
    else
      actualiza_ventana(cur_linea, TAM_Y - 1);
    posiciona_cursor();
  }
}

/*
** Esto es necesario para los teclados no españoles.
*/
acentua(car)
  char *car;
{
  switch (*car = lee_teclado()) {
    case 'A': *car = 'Á';
              break;
    case 'a': *car = 'á';
              break;
    case 'E': *car = 'É';
              break;
    case 'e': *car = 'é';
              break;
    case 'I': *car = 'Í';
              break;
    case 'i': *car = 'í';
              break;
    case 'O': *car = 'Ó';
              break;
    case 'o': *car = 'ó';
              break;
    case 'U': *car = 'Ú';
              break;
    case 'u': *car = 'ú';
              break;
    case 'Y': *car = 'Ü';
              break;
    case 'y': *car = 'ü';
              break;
    case 'N': *car = 'Ñ';
              break;
    case 'n': *car = 'ñ';
              break;
    case 'V':
    case 'v': *car = '«';
              break;
    case 'B':
    case 'b': *car = '»';
              break;
    case 'Q':
    case 'q': *car = '¿';
              break;
    case 'W':
    case 'w': *car = '¡';
              break;
    case 'S':
    case 's': *car = '§';
              break;
    case 'R':
    case 'r': *car = '®';
              break;
    case 'T':
    case 't': *car = '©';
              break;
    case 'P':
    case 'p': *car = '¢';
              break;
    case 'D':
    case 'd': *car = 'º';
              break;
    case 'F':
    case 'f': *car = 'ª';
              break;
    case 'G':
    case 'g': *car = '£';
              break;
    case 'H':
    case 'h': *car = '¹';
              break;
    case 'J':
    case 'j': *car = '²';
              break;
    case 'K':
    case 'k': *car = '³';
              break;
    case 'L':
    case 'l': *car = 'µ';
              break;
    case 'Z':
    case 'z': *car = '¼';
              break;
    case 'X':
    case 'x': *car = '½';
              break;
    case 'C':
    case 'c': *car = '¾';
              break;
    case 'M':
    case 'm': *car = '°';
              break;
  }
}

estatus()
{
  apaga_cursor();
  color(0x1e);
  pos(42, 0);
  decimal(linea_act + 1);
  pos(57, 0);
  decimal(columna_act + 1);
  pos(70, 0);
  if (insercion)
    puts("Inserción");
  else
    puts("Normal   ");
}

decimal(numero)
  int numero;
{
  char matriz[6];

  matriz[5] = 0;
  matriz[0] = ((numero / 10000) % 10) + '0';
  matriz[1] = ((numero /  1000) % 10) + '0';
  matriz[2] = ((numero /   100) % 10) + '0';
  matriz[3] = ((numero /    10) % 10) + '0';
  matriz[4] = ( numero          % 10) + '0';
  while (matriz[0] == '0' && matriz[1] != ' ') {
    matriz[0] = matriz[1];
    matriz[1] = matriz[2];
    matriz[2] = matriz[3];
    matriz[3] = matriz[4];
    matriz[4] = ' ';
  }
  puts(matriz);
}

borra(actual)
  int actual;
{
  char *inicio, *destino, c;

  if (pos_cursor == ultimo)
    return;
  c = *pos_cursor;
  inicio = pos_cursor + 1;
  destino = pos_cursor;
  while (destino != ultimo)
    *destino++ = *inicio++;
  --ultimo;
  if (inicio_bloque > pos_cursor)
    --inicio_bloque;
  if (final_bloque > pos_cursor)
    --final_bloque;
  if (actual) {
    if (c != '\n' && !colorea_sintaxis)
      actualiza_ventana(cur_linea, cur_linea);
    else
      actualiza_ventana(cur_linea, TAM_Y - 1);
    posiciona_cursor();
  }
}

mueve_arriba(actual)
  int actual;
{
  int aven_linea, aven_columna;

  if (pos_cursor == primero)
    return;
  aven_linea = ven_linea;
  aven_columna = ven_columna;
  if (*(pos_cursor - 1) != '\n') {
    while (1) {
      if (pos_cursor == primero)
        break;
      if (*(pos_cursor - 1) == '\n')
        break;
      mueve_izq(0);
    }
  } else {
    mueve_izq(0);
    while (1) {
      if (pos_cursor == primero)
        break;
      if (*(pos_cursor - 1) == '\n')
        break;
      mueve_izq(0);
    }
  }
  if (actual) {
    if (aven_linea != ven_linea ||
        aven_columna != ven_columna)
      actualiza_ventana(0, TAM_Y - 1);
    posiciona_cursor();
  }
}

mueve_abajo(actual)
  int actual;
{
  int aven_linea, aven_columna;

  aven_linea = ven_linea;
  aven_columna = ven_columna;
  if (pos_cursor != primero && *(pos_cursor - 1) != '\n') {
    while (1) {
      if (pos_cursor == primero)
        break;
      if (*(pos_cursor - 1) == '\n')
        break;
      mueve_izq(0);
    }
  } else {
    while (1) {
      if (*pos_cursor == 0)
        break;
      if (*pos_cursor == '\n') {
        mueve_der(0);
        break;
      }
      mueve_der(0);
    }
  }
  if (actual) {
    if (aven_linea != ven_linea ||
        aven_columna != ven_columna)
      actualiza_ventana(0, TAM_Y - 1);
    posiciona_cursor();
  }
}

mueve_izq(actual)
  int actual;
{
  char *chequeo;
  int aven_linea, aven_columna;

  if (pos_cursor == primero)
    return;
  if (*--pos_cursor != '\n') {
    --columna_act;
    if (cur_columna-- == 0) {
      ++cur_columna;
      --ven_columna;
      if (actual)
        actualiza_ventana(0, TAM_Y - 1);
    }
  } else {
    aven_linea = ven_linea;
    aven_columna = ven_columna;
    --linea_act;
    if (cur_linea-- == 0) {
      ++cur_linea;
      --ven_linea;
      pos_ventana = linea_ant(pos_ventana);
    }
    cur_columna = 0;
    ven_columna = 0;
    columna_act = 0;
    chequeo = prin_lin(pos_cursor);
    while (*chequeo++ != '\n') {
      ++columna_act;
      if(++cur_columna == TAM_X) {
        --cur_columna;
        ++ven_columna;
      }
    }
    if (aven_linea != ven_linea ||
        aven_columna != ven_columna)
      if (actual)
        actualiza_ventana(0, TAM_Y - 1);
  }
  if (actual)
    posiciona_cursor();
}

mueve_der(actual)
  int actual;
{
  int aven_linea, aven_columna;

  if (*pos_cursor == 0)
    return;
  if (*pos_cursor++ != '\n') {
    ++columna_act;
    if(++cur_columna == TAM_X) {
      --cur_columna;
      ++ven_columna;
      if (actual)
        actualiza_ventana(0, TAM_Y - 1);
    }
  } else {
    aven_linea = ven_linea;
    aven_columna = ven_columna;
    cur_columna =
    ven_columna =
    columna_act = 0;
    ++linea_act;
    if (++cur_linea == TAM_Y) {
      --cur_linea;
      ++ven_linea;
      pos_ventana = sig_linea(pos_ventana);
    }
    if (aven_linea != ven_linea ||
        aven_columna != ven_columna)
      if (actual)
        actualiza_ventana(0, TAM_Y - 1);
  }
  if (actual)
    posiciona_cursor();
}

#define ES_NORMAL         0
#define ES_COMENTARIO     1
#define ES_COMILLAS       2
#define ES_APOSTROFE      3
#define ES_IDENTIFICADOR  4
#define ES_PALABRA_RES    5
#define ES_PALABRA_PRE    6
#define ES_PREPROCESADOR  7
#define ES_EXTENSION      8
#define ES_COMENTARIO_CPP 9
#define ES_NUMERO        10
#define ES_NUMERO_HEX1   11
#define ES_NUMERO_HEX2   12
#define ES_NUMERO_EXP1   13
#define ES_NUMERO_EXP2   14
#define ES_NUM_IGNORA    15

/*
** Las multi-líneas son:
** - Extensión con \ al final.
** - Comentarios.
*/
actualiza_ventana(inicio, final)
  int inicio, final;
{
  char *ven = pos_ventana, *lin, *proxima;
  int conteo, col = 0x0a;
  int estatus, fuera, col_anterior;
  int lineas_extras = 0, extension, espacios;
  int tecla = 0;

  apaga_cursor();

  col_anterior = 0xff;

  conteo = inicio;
  while (conteo--)
    ven = sig_linea(ven);
/*
** Ahora es necesario hacer una pequeña exploración para determinar si
** estamos dentro de un comentario o una extensión.
*/
  proxima = ven;
  estatus = ES_NORMAL;
  if (proxima > buffer + 1 && colorea_sintaxis) {
    proxima -= 2;
    while (proxima >= buffer) {
      if (*proxima == '*' &&
         *(proxima + 1) == '/')    /* Fuera de un comentario */
        break;
      if (*proxima == '/' &&
         *(proxima + 1) == '*') {  /* Dentro de un comentario */
        estatus = ES_COMENTARIO;
        break;
      }
      --proxima;
    }
  }

/*
** Checa si esta dentro de una línea extendida.
*/
  while(estatus == ES_NORMAL && ven > buffer + 1 && *(ven - 2) == '\\' &&
        *(ven - 1) == '\n' && colorea_sintaxis) {
    ven = linea_ant(ven);
    lineas_extras++;
  }

  extension = NO;
  espacios = SI;
  while (inicio <= final) {
    pos(0, inicio + 1);
    lin = ven;
    proxima = sig_linea(ven);
    if (!extension) {
      estatus = (estatus == ES_COMENTARIO) ? ES_COMENTARIO : ES_NORMAL;
      espacios = SI;
    }
    conteo = lineas_extras ? 1000000 : ven_columna;
    while (conteo--) {
      fuera = 0;
      if (colorea_sintaxis)
        estatus = procesa_estatus(lin, estatus, &espacios, &fuera);
      if (*lin && *lin != '\n')
        ++lin;
      else {
        if (*(lin - 1) == '\\' && *lin == '\n')
          extension = SI;
        else
          extension = NO;
        break;
      }
    }
    if (!lineas_extras) {
      conteo = 0;
      while (conteo < TAM_X) {
        fuera = 0;
        if (colorea_sintaxis)
          estatus = procesa_estatus(lin, estatus, &espacios, &fuera);
        if (lin >= inicio_bloque && lin < final_bloque) {
          if (colorea_sintaxis && *lin == '\\' &&
              (*(lin + 1) == '\n' || *(lin + 1) == 0))
            col = color_sintaxis[ES_EXTENSION + 16];
          else
            col = color_sintaxis[estatus + fuera + 16];
        } else {
          if (colorea_sintaxis && *lin == '\\' &&
              (*(lin + 1) == '\n' || *(lin + 1) == 0))
            col = color_sintaxis[ES_EXTENSION];
          else
            col = color_sintaxis[estatus + fuera];
        }
        if (*lin == 0 || *lin == '\n')
          break;
        if (col_anterior != col)
          color(col_anterior = col);
        if (*lin >= 32)
          putchar(*lin++);
        else {
          putchar('^');
          ++lin;
        }
        ++conteo;
      }
      if (*(lin - 1) == '\\' && *lin == '\n')
        extension = SI;
      else
        extension = NO;
      while (conteo < TAM_X) {
        if (col_anterior != col)
          color(col_anterior = col);
        putchar(' ');
        ++conteo;
      }
      ++inicio;
      if (tecla_pulsada = checa_tecla())
        return;
    } else
      lineas_extras--;
    if (colorea_sintaxis && estatus != ES_COMENTARIO_CPP) {
      if (estatus == ES_COMENTARIO ||
          (*(proxima - 2) == '\\' && *(proxima - 1) == '\n')) {
        while (1) {
          fuera = 0;
          estatus = procesa_estatus(lin, estatus, &espacios, &fuera);
          if (*lin && *lin != '\n')
            ++lin;
          else {
            if (*(lin - 1) == '\\' && *lin == '\n')
              extension = SI;
            else
              extension = NO;
            break;
          }
        }
      }
    }
    ven = proxima;
  }
}

procesa_estatus(lin, estatus, espacios, fuera)
  char *lin;
  int estatus;
  int *espacios;
  int *fuera;
{
  switch (estatus) {
    case ES_IDENTIFICADOR:
    case ES_PALABRA_RES:
    case ES_PALABRA_PRE:
      if (es_alfa(*lin) || isdigit(*lin))
        break;
      estatus = ES_NORMAL;
    procesa:
    case ES_NORMAL:
    case ES_PREPROCESADOR:
      if (*lin == '/') {
        if (*(lin + 1) == '*')
          estatus = ES_COMENTARIO;
        else if (*(lin + 1) == '/')
          estatus = ES_COMENTARIO_CPP;
      } else if (*lin == '#' && *espacios)
        estatus = ES_PREPROCESADOR;
      else if ((*lin == '"' || *lin == '\'') &&
               (*(lin - 1) != '\\' || *(lin - 1) == '\\')) {
        if (*lin == '"')
          estatus = ES_COMILLAS;
        else
          estatus = ES_APOSTROFE;
      } else if (*lin == 'L' && (*(lin + 1) == '"' || *(lin + 1) == '\'')) {
        estatus = ES_NUM_IGNORA;
      } else if (es_alfa(*lin)) {
        estatus = (estatus == ES_PREPROCESADOR) ? (ES_PALABRA_PRE) :
            (checa_reservada(lin) ? ES_PALABRA_RES : ES_IDENTIFICADOR);
      } else if (isdigit(*lin)) {
        if (toupper(*(lin + 1)) == 'X' && isxdigit(*(lin + 2)))
          estatus = ES_NUMERO_HEX1;
        else
          estatus = ES_NUMERO;
      } else if (*lin == '.' && isdigit(*(lin + 1)))
        estatus = ES_NUMERO;
      if (*lin != ' ')
        *espacios = NO;
      break;
    case ES_COMENTARIO:
      if (*(lin - 2) != '/' && *(lin - 1) == '*' && *lin == '/') {
        *fuera = ES_COMENTARIO;
        estatus = ES_NORMAL;
      }
    case ES_COMENTARIO_CPP:
      break;
    case ES_COMILLAS:
      if (*lin == '"' && (*(lin - 1) != '\\' || *(lin - 2) == '\\')) {
        *fuera = ES_COMILLAS;
        estatus = ES_NORMAL;
      }
      break;
    case ES_APOSTROFE:
      if (*lin == '\'' && (*(lin - 1) != '\\' || *(lin - 2) == '\\')) {
        *fuera = ES_COMILLAS;
        estatus = ES_NORMAL;
      }
      break;
    case ES_NUMERO_HEX1:
      estatus = ES_NUMERO_HEX2;
      break;
    case ES_NUMERO_HEX2:
      if (isxdigit(*lin))
        break;
      estatus = ES_NORMAL;
      goto procesa;
    case ES_NUMERO:
      if (isdigit(*lin) || *lin == '.')
        break;
      if (toupper(*lin) == 'L') {
        if (toupper(*(lin + 1)) == 'U') {
          estatus = ES_NUM_IGNORA;
        } else {
          *fuera = ES_NUM_IGNORA;
          estatus = ES_NORMAL;
        }
        break;
      }
      if (toupper(*lin) == 'U') {
        if (toupper(*(lin + 1)) == 'L') {
          estatus = ES_NUM_IGNORA;
        } else {
          *fuera = ES_NUM_IGNORA;
          estatus = ES_NORMAL;
        }
        break;
      }
      if (toupper(*lin) == 'F') {
        estatus = ES_NORMAL;
        *fuera = ES_NUM_IGNORA;
        break;
      }
      if (toupper(*lin) == 'E') {
        if (*(lin + 1) == '+' || *(lin + 1) == '-')
          estatus = ES_NUMERO_EXP1;
        else
          estatus = ES_NUMERO_EXP2;
        break;
      }
      estatus = ES_NORMAL;
      goto procesa;
    case ES_NUMERO_EXP1:
      break;
    case ES_NUMERO_EXP2:
      if (isdigit(*lin))
        break;
      estatus = ES_NORMAL;
      goto procesa;
    case ES_NUM_IGNORA:
      estatus = ES_NORMAL;
      *fuera = ES_NUM_IGNORA;
      break;
  }
  return estatus;
}

prin_lin(pos)
  char *pos;
{
  while (1) {
    if (pos == primero)
      return pos;
    if (*--pos == '\n')
      return pos + 1;
  }
}

linea_ant(pos)
  char *pos;
{
  while (1) {
    if (pos == primero)
      return pos;
    if (*--pos == '\n')
      break;
  }
  while (1) {
    if (pos == primero)
      return pos;
    if (*--pos == '\n')
      break;
  }
  return ++pos;
}

sig_linea(pos)
  char *pos;
{
  while (*pos != '\n' && *pos)
    ++pos;
  if (*pos == '\n')
    ++pos;
  return pos;
}

posiciona_cursor()
{
  pos(cur_columna, cur_linea + 1);
  enciende_cursor();
}

pos(x, y)
  int x, y;
{
  putchar(0x1b);
  putchar(2);
  putchar(x);
  putchar(0x1b);
  putchar(3);
  putchar(y);
}

strcmp(uno, dos)
  char *uno, *dos;
{
  while (1) {
    if (*uno < *dos)
      return -1;
    if (*uno > *dos++)
      return 1;
    if (*uno++ == 0)
      return 0;
  }
}

strcpy(destino, inicio)
  char *destino, *inicio;
{
  while (*destino++ = *inicio++) ;
}

strcat(destino, inicio)
  char *destino, *inicio;
{
  while (*destino)
    ++destino;
  strcpy(destino, inicio);
}

strlen(cadena)
  char *cadena;
{
  char *inicio;

  inicio = cadena;
  while (*cadena)
    ++cadena;
  return (cadena - inicio);
}

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

toupper(car)
  int car;
{
  if((car >= 'a' && car <= 'z')
  || (car >= 0xe0 && car <= 0xff))
    return car - 32;
  else
    return car;
}

ventana(x, y, tx, ty, borde, texto)
  int x, y, tx, ty, borde, texto;
{
  int a,b;
  int nx,ny;

  apaga_cursor();
  nx = x;
  ny = y;
  pos(nx, ny);
  color(borde);
  putchar(0x87);
  a = 0;
  while (a++ < tx)
    putchar(0x85);
  putchar(0x89);
  b = 0;
  while (b++ < ty) {
    ++ny;
    pos(nx, ny);
    color(borde);
    putchar(0x86);
    color(texto);
    a = 0;
    while (a++ < tx)
      putchar(' ');
    color(borde);
    putchar(0x86);
  }
  ++ny;
  pos(nx, ny);
  putchar(0x8d);
  a = 0;
  while (a++ < tx)
    putchar(0x85);
  putchar(0x8f);
  pos(x + 1, y + 1);
  color(texto);
}

marca_inicio()
{
  inicio_bloque = pos_cursor;
  actualiza_ventana(0, TAM_Y - 1);
}

marca_final()
{
  final_bloque = pos_cursor;
  actualiza_ventana(0, TAM_Y - 1);
}

copia()
{
  if (final_bloque <= inicio_bloque)
    return;
  pos(0, 24);
  corta2();
  actualiza_ventana(0, TAM_Y - 1);
  pega();
}

corta()
{
  int conteo;
  char *inicio, *destino;

  if (final_bloque <= inicio_bloque)
    return;
  busca_inicio_bloque();
  conteo = final_bloque - inicio_bloque;
  corta2();
  inicio = pos_cursor + conteo;
  destino = pos_cursor;
  while (inicio != ultimo + 1)
    *destino++ = *inicio++;
  ultimo = ultimo - conteo;
  actualiza_ventana(0, TAM_Y - 1);
  posiciona_cursor();
}

corta2()
{
  int archivo;

  archivo = fopen(TEMPORAL, "w");
  if (archivo == NULL)
    return;
  while (inicio_bloque < final_bloque)
    if (fputc(*inicio_bloque++, archivo) == EOF)
      break;
  fclose(archivo);
  inicio_bloque = primero;
  final_bloque = primero;
}

pega()
{
  int archivo;
  int byte;

  pos(0, 24);
  archivo = fopen(TEMPORAL, "r");
  if (archivo != NULL) {
    while ((byte = fgetc(archivo)) != EOF) {
      inserta_caracter(byte, 0);
      mueve_der(0);
    }
    fclose(archivo);
  }
  actualiza_ventana(0, TAM_Y - 1);
  posiciona_cursor();
}

busca_inicio_bloque()
{
  pos(0, 24);
  if (pos_cursor == inicio_bloque)
    return;
  if (pos_cursor < inicio_bloque) {
    while (pos_cursor < inicio_bloque)
      mueve_der(0);
  } else {
    while (pos_cursor > inicio_bloque)
      mueve_izq(0);
  }
}

checa(posicion, texto, opcion)
  char *posicion, *texto;
  int opcion;
{
  if (opcion == 1) {
    while (*texto)
      if (*texto++ != *posicion++)
        return 0;
    return 1;
  } else {
    while (*texto)
      if (toupper(*texto++) != toupper(*posicion++))
        return 0;
    return 1;
  }
}

seguro()
{
  char respuesta[2];
  int conteo;

  ventana(26, 8, 23, 2, 0x6e, 0x6f);
  pos(28, 9);
  puts("¿ Esta seguro (S/N) ?");
  pos(36, 10);
  color(0x0a);
  for (conteo = 0; conteo < 5; conteo++)
    putchar(' ');
  pos(37, 10);
  if (lee_linea(respuesta, 2) == 0)
    return 0;
  if (toupper(respuesta[0]) == 'S')
    return 1;
  return 0;
}

busca()
{
  char a[80], quepaso;
  char *inicio_orig, *final_orig;
  int linea, pulsacion;

  if (final_bloque <= inicio_bloque)
    return;
  inicio_orig = inicio_bloque;
  final_orig = final_bloque;
  if (pide_texto(6, "¿ Texto que se va a buscar ?", a)) {
    if (quepaso = opciones_b(11)) {
      busca_inicio_bloque();
      while (pos_cursor < final_bloque) {
        if (checa(pos_cursor, a, quepaso)) {
          inicio_bloque = pos_cursor;
          final_bloque = inicio_bloque + strlen(a);
          color(fondo);
          actualiza_ventana(0, TAM_Y - 1);
          estatus();
          if (cur_linea < 12)
            linea = 15;
          else
            linea = 8;
          ventana(6, linea, 63, 1, 0x6e, 0x6f);
          puts("Pulse 'Esc' para terminar la busqueda, otra tecla para seguir.");
          pulsacion = lee_teclado();
          if (pulsacion == ESC)
            break;
          final_bloque = inicio_bloque;
          pos(0, 24);
          color(fondo);
          actualiza_ventana(0, TAM_Y - 1);
          inicio_bloque = inicio_orig;
          final_bloque = final_orig;
        }
        mueve_der(0);
      }
      final_bloque = inicio_bloque;
    }
  }
  color(fondo);
  actualiza_ventana(0, TAM_Y - 1);
  posiciona_cursor();
}

substituye()
{
  char a[80], b[80], quepaso, opcion;
  char *inicio_orig, *final_orig;
  int linea, pulsacion, conteo;

  if (final_bloque <= inicio_bloque)
    return;
  inicio_orig = inicio_bloque;
  final_orig = final_bloque;
  if (pide_texto(5, "¿ Texto que se va a substituir ?", a)) {
    if (pide_texto(10, "¿ Substituir con ?", b)) {
      if (quepaso = opciones_b(15)) {
        busca_inicio_bloque();
        while (pos_cursor < final_bloque) {
          if (checa(pos_cursor, a, quepaso)) {
            inicio_bloque = pos_cursor;
            final_bloque = inicio_bloque + strlen(a);
            color(fondo);
            actualiza_ventana(0, TAM_Y - 1);
            estatus();
            if (cur_linea < 12)
              linea = 15;
            else
              linea = 8;
            ventana(2, linea, 74, 1, 0x6e, 0x6f);
            puts("Pulse 'S' para substituir, 'Esc' para terminar, otra tecla para seguir.");
            pulsacion = toupper(lee_teclado());
            if (pulsacion == ESC)
              break;
            inicio_bloque = inicio_orig;
            final_bloque = final_orig;
            if (pulsacion == 'S') {
              conteo = 0;
              while (conteo++ < strlen(a))
                borra(0);
              conteo = 0;
              while (conteo < strlen(b)) {
                inserta_caracter(b[conteo], 0);
                mueve_der(0);
                ++conteo;
              }
              final_orig = final_bloque;
            } else
              mueve_der(0);
            final_bloque = inicio_bloque;
            pos(0, 24);
            color(fondo);
            actualiza_ventana(0, TAM_Y - 1);
            inicio_bloque = inicio_orig;
            final_bloque = final_orig;
          } else
            mueve_der(0);
        }
        final_bloque = inicio_bloque;
      }
    }
  }
  color(fondo);
  actualiza_ventana(0, TAM_Y - 1);
  posiciona_cursor();
}

opciones_b(linea)
  int linea;
{
  int que_onda;
  char respuesta[2];

  ventana(11, linea, 53, 1, 0x6e, 0x6f);
  pos(12, linea+1);
  puts("¿ Diferenciar máyusculas de mínusculas (S/N) ? ");
  color(0x0a);
  puts("    ");
  pos(60, linea+1);
  if (lee_linea(respuesta, 2) == 0)
    return 0;
  if (toupper(respuesta[0]) == 'S')
    que_onda = 1;
  else
    que_onda = 2;
  return que_onda;
}

pide_texto(linea, texto, respuesta)
  int linea;
  char *texto, *respuesta;
{
  int conteo;

  ventana(1, linea, 76, 2, 0x6e, 0x6f);
  pos(39-((strlen(texto)+1)/2), linea+1);
  puts(texto);
  pos(3, linea+2);
  color(0x0a);
  for (conteo = 0; conteo < 74; conteo++)
    putchar(' ');
  pos(4, linea+2);
  if (lee_linea(respuesta, 72) == 0)
    return 0;
  return 1;
}

ayuda()
{
  int x, y;

  x = 15;
  y = 2;
  ventana(x,y,45,16,0x6e,0x6f);
  ++x;
  ++x;
  ++y;
  pos(x,y);
  puts("Ayuda");
  ++y;
  ++y;
  pos(x,y);
  puts("F2 = Graba el texto en el disco.");
  ++y;
  pos(x,y);
  puts("F3 = Acentua la sig. pulsación.");
  ++y;
  pos(x,y);
  puts("F4 = Lee un texto del disco.");
  ++y;
  pos(x,y);
  puts("F5 = Busqueda.");
  ++y;
  pos(x,y);
  puts("F6 = Pega el texto cortado.");
  ++y;
  pos(x,y);
  puts("F7 = Busqueda y substitución.");
  ++y;
  pos(x,y);
  puts("F9 = Marca inicio de bloque.");
  ++y;
  pos(x,y);
  puts("F10= Marca final de bloque.");
  ++y;
  pos(x,y);
  puts("F11= Copia el bloque en la pos. del cursor.");
  ++y;
  pos(x,y);
  puts("F12= Corta el bloque.");
  ++y;
  pos(x,y);
  puts("Del= Borra el caracter debajo del cursor.");
  ++y;
  pos(x,y);
  puts("BkSp= Borra el caracter detras del cursor.");
  ++y;
  pos(x,y);
  puts("Ins= Cambia el modo de inserción/normal.");
  ++y;
  pos(x,y);
  puts("Alt Espacio= Des/Activa color sintaxis C.");
  lee_teclado();
  color(fondo);
  actualiza_ventana(0, TAM_Y - 1);
  posiciona_cursor();
}

checa_unidades()
{
  int espacio;
  int cual;
  char unidad[5];

  strcpy(unidad, "A:/");
  for (cual = 1; cual < 26; ++cual)
    unidades[cual] = 0;
  unidades[0] = 1;
  for (cual = 'B'; cual <= 'Z'; ++cual) {
    unidad[0] = cual;
    if (espacio_libre(unidad, &espacio))
      break;
    unidades[cual - 'A'] = 1;
  }
  if (unidades[2])
    strcpy(camino_actual, "C:/");
  else
    strcpy(camino_actual, "A:/");
}

procesa_dir(cuantos)
  int cuantos;
{
  int conteo, car, d;
  int arc_dir;
  int cuenta;
  char nombre[64];

  cuenta = 0;
  while ((arc_dir = abre_dir(camino_actual)) == 0)
    camino_actual[3] = 0;
  d = 0;
  if (camino_actual[3]) {
    if (cuenta < cuantos && cuenta >= cuantos - 32) {
      strcpy(dir + d, "[volver atras]                    ");
      d += 35;
    }
    ++cuenta;
  }
  for (conteo = 0; conteo < 26; ++conteo) {
    if (unidades[conteo]) {
      if (cuenta < cuantos && cuenta >= cuantos - 32) {
        strcpy(dir + d, "Unidad A:                         ");
        dir[d+7] = conteo + 'A';
        d += 35;
      }
      ++cuenta;
    }
  }
  while (1) {
    conteo = 0;
    while (conteo < 64)
      nombre[conteo++] = fgetc(arc_dir);
    if (nombre[0] == 0x80)
      continue;
    if (nombre[0] == 0)
      break;
    if (nombre[48] & 1)
      continue;
    if (nombre[48] & 8) {
      if (cuenta < cuantos && cuenta >= cuantos - 32) {
        dir[d++] = '[';
        conteo = 0;
        while (conteo < 31 && nombre[conteo])
          dir[d++] = nombre[conteo++];
        dir[d++] = ']';
        while (conteo++ < 33)
          dir[d++] = ' ';
        dir[d-1] = 0;
      }
    } else {
      if (cuenta < cuantos && cuenta >= cuantos - 32) {
        conteo = 0;
        while (conteo < 31 && nombre[conteo])
          dir[d++] = nombre[conteo++];
        dir[d++] = 0;
        while (conteo++ < 34)
          dir[d++] = ' ';
      }
    }
    ++cuenta;
  }
  fclose(arc_dir);
  archivos_totales = cuenta;
  if (cuenta <= cuantos)
    return 32 - (cuantos - cuenta);
  else
    return 32;
}

visual_dir(cuantos)
  int cuantos;
{
  int x, y, conteo, salida, todos, col;

  pos(28, 2);
  color(0x1b);
  for (conteo = 0; conteo < 50; ++conteo)
    putchar(' ');
  pos(28, 2);
  puts(camino_actual);
  x = 2;
  y = 4;
  salida = 0;
  todos = 0;
  while (todos < 32) {
    pos(x, y);
    if (todos < cuantos) {
      if (dir[salida+34])
        col = 0x1e;
      else
        col = 0x1f;
      color(col);
      conteo = 0;
      while (conteo++ < 35)
        putchar(dir[salida++]);
    } else {
      conteo = 0;
      while (conteo++ < 35)
        putchar(' ');
    }
    if (++y == 20) {
      y = 4;
      x += 35;
    }
    ++todos;
  }
}

sel()
{
  int x, y, actual, car, conteo, conteo2, col, posicion, cuantos, actualizar;
  int exacto;
  char *dat;

  posicion = 0;
  x = 1;
  y = 4;
  actual = 0;
  while (1) {
    pos(x, y);
    exacto = (actual - posicion) * 35;
    if (dir[exacto + 34] == 0)
      col = 0x1f;
    else
      col = 0x1e;
    color(0xcf);
    putchar(' ');
    conteo = 0;
    while (conteo < 35)
      putchar(dir[exacto + conteo++]);
    car = lee_teclado();
    pos(x, y);
    color(col);
    putchar(' ');
    conteo = 0;
    while (conteo < 35)
      putchar(dir[exacto + conteo++]);
    if (car == 0x0d) {
      actual -= posicion;
      if (dir[actual * 35 + 34] == 0) {
        if (dir[actual * 35] == '[') {
          if (dir[actual * 35 + 7] == ' ' && dir[actual * 35 + 13] == ']') {
            conteo = 0;
            while (camino_actual[conteo])
              ++conteo;
            while (camino_actual[conteo] != '/')
              --conteo;
            if (conteo == 2)
              ++conteo;
            camino_actual[conteo] = 0;
          } else {
            conteo = 0;
            while (camino_actual[conteo])
              ++conteo;
            if (camino_actual[conteo-1] != '/')
              camino_actual[conteo++] = '/';
            conteo2 = actual * 35 + 1;
            while (dir[conteo2] != ']')
              camino_actual[conteo++] = dir[conteo2++];
            camino_actual[conteo] = 0;
          }
        } else {
          strcpy(camino_actual, "A:/");
          camino_actual[0] = dir[actual*35 + 7];
          bytes_libres();
        }
        posicion = 0;
        cuantos = procesa_dir(posicion + 32);
        visual_dir(cuantos);
        x = 1;
        y = 4;
        actual = 0;
      } else
        return actual;
    } else if (car == 0x1b)
      return -1;
    else if (car == 0x18) {
      if (actual) {
        --actual;
        if (--y == 3) {
          if (x == 1) {
            posicion -= 16;
            cuantos = procesa_dir(posicion + 32);
            visual_dir(cuantos);
          } else
            x = x - 35;
          y = 19;
        }
      }
    } else if (car == 0x14) {
      if (actual - 16 >= 0) {
        if (x != 1)
          x = x - 35;
        else {
          x = 1;
          posicion -= 16;
          cuantos = procesa_dir(posicion + 32);
          visual_dir(cuantos);
        }
        actual -= 16;
      }
    } else if (car == 0x16) {
      if (actual + 16 <= archivos_totales - 1) {
        if (x != 36)
          x = x + 35;
        else {
          x = 36;
          posicion += 16;
          cuantos = procesa_dir(posicion + 32);
          visual_dir(cuantos);
        }
        actual += 16;
      }
    } else if (car == 0x12) {
      if (actual != archivos_totales - 1) {
        ++actual;
        if (++y == 20) {
          if (x != 36)
            x = x + 35;
          else {
            posicion += 16;
            cuantos = procesa_dir(posicion + 32);
            visual_dir(cuantos);
          }
          y = 4;
        }
      }
    } else if (car == 0x17) {
      if (posicion) {
        posicion = 0;
        cuantos = procesa_dir(posicion + 32);
        visual_dir(cuantos);
      }
      actual = 0;
      x = 1;
      y = 4;
    } else if (car == 0x11) {
      actualizar = posicion;
      posicion = 0;
      cuantos = archivos_totales;
      while (cuantos > 32) {
        cuantos -= 16;
        posicion += 16;
      }
      x = 1 + 35 * ((cuantos - 1) / 16);
      y = 4 + ((cuantos - 1) % 16);
      if (posicion != actualizar) {
        cuantos = procesa_dir(posicion + 32);
        visual_dir(cuantos);
      }
      actual = archivos_totales - 1;
    }
  }
}

lee()
{
  int cuantos;
  char *posi;
  int cual, val;

  ventana(0, 1, 78, 21, 0x1e, 0x1f);
  puts(" Lectura de archivo:");
  pos(2, 21);
  bytes_libres();
  cuantos = procesa_dir(32);
  visual_dir(cuantos);
  cuantos = sel();
  if (cuantos != -1) {
    posi = dir + cuantos * 35;
    buffer[0] = 0;
    pos_ventana =
    pos_cursor =
    primero =
    ultimo = buffer;
    inicio_bloque = primero;
    final_bloque = primero;
    cur_linea = cur_columna = 0;
    ven_linea = ven_columna = 0;
    linea_act = columna_act = 0;
    actualiza_ventana(0, TAM_Y - 1);
    pos(0, 24);
    estatus();
    crea_nombre(posi);
    archivo = fopen(nombre_archivo, "r");
    cuantos = cual = 0;
    while ((val = fgetc(archivo)) != EOF) {
      if (val == '\r' || val == '\n') {
        if ((cual != 0 && val == cual) || (cual == 0)) {
          if((++linea_act % 64) == 0)
            estatus();
          cuantos = 0;
          if(cual == 0)
            cual = val;
          val = '\n';
        } else
          continue;
      }
      if (val == 0)
        val = ' ';
      if (val == 26)
        continue;
      if (val == '\t') {
        while (1) {
          if (ultimo == buffer + TAM_BUF - 1) {
            memoria_agotada();
            break;
          }
          *ultimo++ = ' ';
          ++cuantos;
          if (!(cuantos & 7))
            break;
        }
        continue;
      }
      if (ultimo == buffer + TAM_BUF - 1) {
        memoria_agotada();
        break;
      }
      *ultimo++ = val;
      ++cuantos;
    }
    *ultimo = 0;
    linea_act = 0;
    fclose(archivo);
    posiciona_cursor();
    estatus();
  }
  color(fondo);
  actualiza_ventana(0, TAM_Y - 1);
  posiciona_cursor();
}

memoria_agotada()
{
  ventana(29,10,18,1,0x4e,0x4f);
  puts(" Memoria agotada. ");
  lee_teclado();
  color(fondo);
  actualiza_ventana(0, TAM_Y - 1);
  posiciona_cursor();
}

error_crea()
{
  ventana(19,10,37,1,0x4e,0x4f);
  puts(" Error: No se pudo crear el archivo. ");
  lee_teclado();
}

error_esc()
{
  ventana(24,10,29,1,0x4e,0x4f);
  puts(" Error: El disco esta lleno. ");
  lee_teclado();
}

lee_linea(pos, tam)
  char *pos;
  int tam;
{
  char *ahora;
  int car;

  enciende_cursor();
  ahora = pos;
  while (1) {
    car = lee_teclado();
    if (car == 0x03)
      acentua(&car);
    if (car == 8) {
      if (ahora == pos)
        continue;
      putchar(8);
      --ahora;
      continue;
    } else if (car == 13) {
      puts("\r\n");
      *ahora = 0;
      return 1;
    } else if (car == ESC) {
      puts("\r\n");
      *pos = 0;
      return 0;
    } else if (car < 32) {
      continue;
    } else {
      if (ahora == pos + tam - 1)
        continue;
      putchar(car);
      *ahora++ = car;
    }
  }
}

graba()
{
  int cuantos, val;
  char nom[50];
  char *dat;

  ventana(0, 1, 78, 21, 0x1e, 0x1f);
  puts(" Grabación de archivo:");
  bytes_libres();
  cuantos = procesa_dir(32);
  visual_dir(cuantos);
  pos(2,21);
  puts("¿ Nombre para el archivo ? ");
  if (lee_linea(nom, 50) && nom[0]) {
    pos(2,22);
    puts("Escribiendo...");
    crea_nombre(nom);
    archivo = fopen(nombre_archivo, "w");
    val = 0;
    if (archivo == NULL)
      error_crea();
    else {
      dat = primero;
      while (dat != ultimo) {
        if (*dat == '\n')
          if (fputc('\r', archivo) == EOF) {
            error_esc();
            break;
          }
        if (fputc(*dat++, archivo) == EOF) {
          error_esc();
          break;
        }
      }
      fclose(archivo);
    }
  }
  color(fondo);
  actualiza_ventana(0, TAM_Y - 1);
  posiciona_cursor();
}

crea_nombre(nombre)
  char *nombre;
{
  char *apunta;

  if (toupper(nombre[0]) >= 'A' && toupper(nombre[0]) <= 'Z' &&
      toupper(nombre[0]) != camino_actual[0] && nombre[1] == ':')
    apunta = nombre_archivo;
  else {
    if (toupper(nombre[0]) == camino_actual[0] && nombre[1] == ':')
      nombre += 2;
    apunta = nombre_archivo;
    if (nombre[0] != '/') {
      strcpy(nombre_archivo, camino_actual);
      while (*apunta)
        ++apunta;
      if (*(apunta - 1) != '/')
        *apunta++ = '/';
    }
  }
  while (*apunta++ = *nombre++) ;
}

bytes_libres()
{
  int divisor, digito, cual, es, numero, conteo;

  color(0x1b);
  pos(51, 22);
  espacio_libre(camino_actual, &numero);
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
    } else if (es)
      putchar('0');
    else
      putchar(' ');
    ++cual;
    if (cual == 1 || cual == 4 || cual == 7)
      putchar(es ? ',' : ' ');
    divisor /= 10;
  }
  puts(" bytes libres.");
}

inicia_sintaxis()
{
  colorea_sintaxis = SI;

  palabras_reservadas[0] = "asm";
  palabras_reservadas[1] = "auto";
  palabras_reservadas[2] = "break";
  palabras_reservadas[3] = "case";
  palabras_reservadas[4] = "catch";
  palabras_reservadas[5] = "char";
  palabras_reservadas[6] = "class";
  palabras_reservadas[7] = "const";
  palabras_reservadas[8] = "continue";
  palabras_reservadas[9] = "default";
  palabras_reservadas[10] = "delete";
  palabras_reservadas[11] = "do";
  palabras_reservadas[12] = "double";
  palabras_reservadas[13] = "else";
  palabras_reservadas[14] = "enum";
  palabras_reservadas[15] = "extern";
  palabras_reservadas[16] = "float";
  palabras_reservadas[17] = "for";
  palabras_reservadas[18] = "friend";
  palabras_reservadas[19] = "goto";
  palabras_reservadas[20] = "if";
  palabras_reservadas[21] = "inline";
  palabras_reservadas[22] = "int";
  palabras_reservadas[23] = "long";
  palabras_reservadas[24] = "new";
  palabras_reservadas[25] = "operator";
  palabras_reservadas[26] = "private";
  palabras_reservadas[27] = "protected";
  palabras_reservadas[28] = "public";
  palabras_reservadas[29] = "register";
  palabras_reservadas[30] = "return";
  palabras_reservadas[31] = "short";
  palabras_reservadas[32] = "signed";
  palabras_reservadas[33] = "sizeof";
  palabras_reservadas[34] = "static";
  palabras_reservadas[35] = "struct";
  palabras_reservadas[36] = "switch";
  palabras_reservadas[37] = "template";
  palabras_reservadas[38] = "this";
  palabras_reservadas[39] = "throw";
  palabras_reservadas[40] = "try";
  palabras_reservadas[41] = "typedef";
  palabras_reservadas[42] = "union";
  palabras_reservadas[43] = "unsigned";
  palabras_reservadas[44] = "virtual";
  palabras_reservadas[45] = "void";
  palabras_reservadas[46] = "volatile";
  palabras_reservadas[47] = "while";

  color_sintaxis[0] = 0x0a;
  color_sintaxis[1] = 0x0f;
  color_sintaxis[2] = 0x0b;
  color_sintaxis[3] = 0x0b;
  color_sintaxis[4] = 0x0a;
  color_sintaxis[5] = 0x0e;
  color_sintaxis[6] = 0x0d;
  color_sintaxis[7] = 0x0d;
  color_sintaxis[8] = 0x09;
  color_sintaxis[9] = 0x0f;
  color_sintaxis[10] = 0x0b;
  color_sintaxis[11] = 0x0b;
  color_sintaxis[12] = 0x0b;
  color_sintaxis[13] = 0x0b;
  color_sintaxis[14] = 0x0b;
  color_sintaxis[15] = 0x0b;

  color_sintaxis[16] = 0xf0;
  color_sintaxis[17] = 0xf3;
  color_sintaxis[18] = 0xf1;
  color_sintaxis[19] = 0xf1;
  color_sintaxis[20] = 0xf0;
  color_sintaxis[21] = 0xf4;
  color_sintaxis[22] = 0xf5;
  color_sintaxis[23] = 0xf5;
  color_sintaxis[24] = 0xf2;
  color_sintaxis[25] = 0xf3;
  color_sintaxis[26] = 0xf1;
  color_sintaxis[27] = 0xf1;
  color_sintaxis[28] = 0xf1;
  color_sintaxis[29] = 0xf1;
  color_sintaxis[30] = 0xf1;
  color_sintaxis[31] = 0xf1;
}

checa_reservada(pos)
  char *pos;
{
  char **cual_palabra;
  char *pos1, *pos2;
  int min, max, centro, lado;

  cual_palabra = palabras_reservadas;
  min = 0;
  max = NUM_PAL_RES - 1;
  while (min <= max) {
    centro = (min + max) / 2;
    pos1 = pos;
    pos2 = palabras_reservadas[centro];
    while (1) {
      if (*pos1 < *pos2) {
        lado = -1;
        break;
      }
      if (*pos1++ > *pos2++) {
        lado = 1;
        break;
      }
      if (*pos1 == '\\' && *(pos1 + 1) == '\n')
        pos1 += 2;
      if (*pos2 == 0 && !isalnum(*pos1) && *pos1 != '_')
        return SI;
    }
    if (lado == -1)
      max = centro - 1;
    else if (lado == 1)
      min = centro + 1;
  }
  return NO;
}

es_alfa(byte)
  int byte;
{
  return (isalpha(byte) || byte == '_');
}

islower(byte)
  int byte;
{
  return (byte >= 'a' && byte <= 'z');
}

isupper(byte)
  int byte;
{
  return (byte >= 'A' && byte <= 'Z');
}

isalpha(byte)
  int byte;
{
  return (islower(byte) || isupper(byte));
}

isdigit(byte)
  int byte;
{
  return (byte >= '0' && byte <= '9');
}

isalnum(byte)
  int byte;
{
  return (isalpha(byte) || isdigit(byte));
}

isxdigit(byte)
  int byte;
{
  return (byte >= '0' && byte <= '9') ||
         (byte >= 'A' && byte <= 'F') ||
         (byte >= 'a' && byte <= 'f');
}

lee_teclado()
{
  int val;

  val = tecla_pulsada ? tecla_pulsada : getchar();
  tecla_pulsada = 0;
  return val;
}

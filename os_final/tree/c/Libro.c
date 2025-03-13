/*
** Imprime en formato libro en una impresora HP LaserJet IIP.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 10 de octubre de 1995.
** Revisión: 11 de octubre de 1995. Corrección de un defecto, contaba el \r
**                                  cómo un caracter extra.
** Revisión: 1o. de enero de 1996. Mejoras mínimas.
** Revisión: 11 de enero de 1996. Indica el número de páginas totales.
** Revisión: 28 de marzo de 1996. Soporte para enviar la salida a otro lado.
** Revisión: 28 de marzo de 1996. Corrección de un defecto que hacia que se
**                                perdiera un caracter en líneas muy largas
**                                al final de una hoja.
** Revisión: 31 de mayo de 1996. Soporte para el año 2000.
*/

#define EOF                 (-1)
#define NO                    0
#define SI                    1

#define LINEAS_POR_PAGINA    66
#define COLUMNAS_POR_PAGINA  80

#define SEEK_SET              0
#define SEEK_CUR              1
#define SEEK_END              2

struct paginas {
  int posicion_en_archivo;
  int longitud_en_bytes;
};

int numero_pagina_izquierda;
int numero_pagina_derecha;

int control, num_salida;

int numero_paginas;
int cuantas_paginas_faltan;

struct paginas paginas[1024];

char archivo[256];
char titulo[48];
char salida[256];

char encabezado_izquierdo[81];
char encabezado_derecho[81];

main()
{
  color(15);
  puts("\r\nImpresor en formato libro    (c) Copyright 1995 Oscar Toledo G.\r\n\n");
  color(10);
  puts("¿ Archivo para imprimir ? ");
  lee_linea(archivo, 256);
  if (*archivo == 0) {
    color(15);
    puts("\r\nImpresor cancelado.\r\n");
    exit(1);
  }
  puts("\r\n¿ Título para el archivo ? ");
  lee_linea(titulo, 48);
  if (*titulo == 0) {
    color(15);
    puts("\r\nImpresor cancelado.\r\n");
    exit(1);
  }
  puts("\r\n¿ Archivo de salida ? ");
  lee_linea(salida, 256);
  if (*salida == 0) {
    color(15);
    puts("\r\nImpresor cancelado.\r\n");
    exit(1);
  }
  color(11);
  puts("\r\nProcesando...");
  if(procesa_archivo()) {
    color(15);
    puts("\rError, no pudo leerse el archivo.\r\n");
    exit(1);
  }
  puts("\r");
  num_salida = fopen(salida, "w");
  if (num_salida == 0) {
    color(15);
    puts("\rNo se pudo abrir la salida.\r\n");
    exit(1);
  }
  color(14);
  decimal(numero_paginas, 0);
  puts(" páginas por imprimir, ");
  decimal(numero_paginas / 4, 0);
  puts(" hojas requeridas.\r\n");
  color(11);
  puts("Imprimiendo...");
  inicializa_impresora();
  prepara_encabezados();
  numero_pagina_izquierda = numero_paginas / 2;
  numero_pagina_derecha = numero_pagina_izquierda + 1;
  cuantas_paginas_faltan = numero_paginas;
  while(cuantas_paginas_faltan) {
    pagina_izquierda();
    prepara_izquierdo(numero_pagina_izquierda);
    imprime_pagina(numero_pagina_izquierda--);
    pagina_derecha();
    prepara_derecho(numero_pagina_derecha);
    imprime_pagina(numero_pagina_derecha++);
    expulsa_hoja();
    pagina_izquierda();
    prepara_izquierdo(numero_pagina_derecha);
    imprime_pagina(numero_pagina_derecha++);
    pagina_derecha();
    prepara_derecho(numero_pagina_izquierda);
    imprime_pagina(numero_pagina_izquierda--);
    expulsa_hoja();
    cuantas_paginas_faltan -= 4;
  }
  fclose(control);
  fin_impresion();
  color(11);
  puts("\rFin de impresión.\r\n");
  fclose(num_salida);
}

prepara_izquierdo(pagina)
  int pagina;
{
  numero(pagina, encabezado_izquierdo + 7);
  inicio_encabezado();
  imprime_texto(encabezado_izquierdo);
  fin_encabezado();
}

prepara_derecho(pagina)
  int pagina;
{
  numero(pagina, encabezado_derecho + 76);
  inicio_encabezado();
  imprime_texto(encabezado_derecho);
  fin_encabezado();
}

numero(numero, texto)
  int numero;
  char *texto;
{
  if(numero >= 1000)
    *texto++ = '0' + ((numero / 1000) % 10);
  else
    *texto++ = ' ';
  if(numero >= 100)
    *texto++ = '0' + ((numero / 100) % 10);
  else
    *texto++ = ' ';
  if(numero >= 10)
    *texto++ = '0' + ((numero / 10) % 10);
  else
    *texto++ = ' ';
  *texto++ = '0' + (numero % 10);
}

imprime_pagina(numero)
  int numero;
{
  int bytes;
  int caracter;
  int columna;

  fseek(control, paginas[numero - 1].posicion_en_archivo, SEEK_SET);
  bytes = paginas[numero - 1].longitud_en_bytes;
  columna = 0;
  while(bytes--) {
    caracter = fgetc(control);
    if(caracter == '\r' || caracter == '\f')
      continue;
    if(caracter == '\t') {
      do {
        imprime_caracter(' ');
        ++columna;
      } while(columna & 7);
    } else {
      if(caracter == '\n') {
        columna = 0;
        imprime_caracter('\r');
      }
      imprime_caracter(caracter);
    }
  }
}

prepara_encabezados()
{
  int conteo;
  char *titulo2;

  for(conteo = 0; conteo < 80; ++conteo) {
    encabezado_izquierdo[conteo] = ' ';
    encabezado_derecho[conteo] = ' ';
  }
  encabezado_izquierdo[conteo] = 0;
  encabezado_derecho[conteo] = 0;
  encabezado_izquierdo[0] = encabezado_derecho[69] = 'P';
  encabezado_izquierdo[1] = encabezado_derecho[70] = 'á';
  encabezado_izquierdo[2] = encabezado_derecho[71] = 'g';
  encabezado_izquierdo[3] = encabezado_derecho[72] = 'i';
  encabezado_izquierdo[4] = encabezado_derecho[73] = 'n';
  encabezado_izquierdo[5] = encabezado_derecho[74] = 'a';

  obtiene_hora(encabezado_derecho + 49);
  for(conteo = 49; conteo < 69; ++conteo) {
    encabezado_izquierdo[conteo - 36] = encabezado_derecho[conteo];
  }

  titulo2 = titulo;
  conteo = 0;
  while(*titulo2) {
    encabezado_derecho[conteo++] = *titulo2++;
  }

  titulo2 = titulo;
  conteo = 80 - conteo;
  while(*titulo2) {
    encabezado_izquierdo[conteo++] = *titulo2++;
  }
}

procesa_archivo()
{
  int caracter_antiguo;
  int posicion;
  int caracter;
  int lineas;
  int columna;
  int fin_de_archivo;

  control = fopen(archivo, "r");
  if(control == 0)
    return 1;
  numero_paginas = 0;
  fin_de_archivo = NO;
  posicion = 0;
  caracter_antiguo = NO;
  while(!fin_de_archivo) {
    paginas[numero_paginas].posicion_en_archivo = posicion - caracter_antiguo;
    lineas = 0;
    columna = 0;
    while(1) {
      if(!caracter_antiguo) {
        caracter = fgetc(control);
        if (caracter == EOF) {
          fin_de_archivo = SI;
          break;
        }
        ++posicion;
      } else
        caracter_antiguo = NO;
      if(caracter == '\r')
        continue;
      if(caracter == '\t') {
        columna = (columna + 8) & ~7;
      } else {
        if(caracter == '\f')
          break;
        if(caracter == '\n') {
          ++lineas;
          columna = 0;
          if(lineas == LINEAS_POR_PAGINA)
            break;
        } else if(columna++ == COLUMNAS_POR_PAGINA) {
          ++lineas;
          columna = 1;
          if(lineas == LINEAS_POR_PAGINA) {
            caracter_antiguo = SI;
            break;
          }
        }
      }
    }
    paginas[numero_paginas].longitud_en_bytes = posicion -
                               paginas[numero_paginas].posicion_en_archivo -
                               caracter_antiguo;
    ++numero_paginas;
  }
  while(numero_paginas & 3) {
    paginas[numero_paginas].posicion_en_archivo = 0;
    paginas[numero_paginas].longitud_en_bytes = 0;
    ++numero_paginas;
  }
  return 0;
}

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

inicializa_impresora()
{
  imprime_caracter(0x1b);
  imprime_texto("E");                    /* Inicializa la impresora */
  imprime_caracter(0x1b);
  imprime_texto("(0N");                  /* Selecciona conjunto ECMA - 94 */
  imprime_caracter(0x1b);
  imprime_texto("&l1O");                 /* Landscape */
  imprime_caracter(0x1b);
  imprime_texto("&l8d1e67f5.5C");        /* 8 lineas por pulgada, 1 línea */
                                         /* de margen, tamaño de 67 líneas, */
                                         /* VMI de 5.5 / 48 " */
  imprime_caracter(0x1b);
  imprime_texto("(s0p16.66h8.5v0s0b0T"); /* Selecciona Line Printer 8.5 p. */
  imprime_caracter(0x1b);
  imprime_texto("&s0C");                 /* Activa salto de fin de línea */
  imprime_caracter(0x1b);
  imprime_texto("&k6.7H");               /* Selecciona HMI de 6.7 / 120 " */
/*
  imprime_caracter(0x1b);
  imprime_texto("&a3825h5V");
  imprime_caracter(0x1b);
  imprime_texto("*c1h180v0P");
  imprime_caracter(0x1b);
  imprime_texto("&a3825h5670V");
  imprime_caracter(0x1b);
  imprime_texto("*c1h180v0P");
*/
}

pagina_izquierda()
{
  imprime_caracter(0x1b);
  imprime_texto("9");
  imprime_caracter(0x1b);
  imprime_texto("&a1r0l80M");
  imprime_caracter(0x0d);
}

pagina_derecha()
{
  imprime_caracter(0x1b);
  imprime_texto("9");
  imprime_caracter(0x1b);
  imprime_texto("&a1r109l189M");
  imprime_caracter(0x0d);
}

inicio_encabezado()
{
  imprime_caracter(0x0d);
  imprime_caracter(0x1b);
  imprime_texto("&dD");
}

fin_encabezado()
{
  imprime_caracter(0x1b);
  imprime_texto("&d@");
  imprime_texto("\r\n\r\n");
}

expulsa_hoja()
{
  imprime_caracter(0x0c);
}

fin_impresion()
{
  imprime_caracter(0x1b);
  imprime_texto("E");                    /* Inicializa la impresora */
}

lee_linea(pos, tam) char *pos; int tam; {
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

imprime_caracter(car)
  int car;
{
  fputc(car, num_salida);
}

imprime_texto(texto)
  char *texto;
{
  while(*texto)
    imprime_caracter(*texto++);
}

obtiene_hora(texto)
  char *texto;
{
  char oscar[6];
  int mes;
  int fecha;
  int an;
  int siglo;
  int dia;

  hora(oscar);
  an = oscar[0];
  siglo = (an > 99) ? 20 : 19;
  mes = oscar[1];
  fecha = oscar[2];
  if(fecha > 9) *texto = (fecha / 10) + '0';
  *(texto + 1) = (fecha % 10) + '0';
  *(texto + 2) = '-';
  switch(mes) {
    case 1: memcpy(texto + 3, "Ene", 3);
            break;
    case 2: memcpy(texto + 3, "Feb", 3);
            break;
    case 3: memcpy(texto + 3, "Mar", 3);
            break;
    case 4: memcpy(texto + 3, "Abr", 3);
            break;
    case 5: memcpy(texto + 3, "May", 3);
            break;
    case 6: memcpy(texto + 3, "Jun", 3);
            break;
    case 7: memcpy(texto + 3, "Jul", 3);
            break;
    case 8: memcpy(texto + 3, "Ago", 3);
            break;
    case 9: memcpy(texto + 3, "Sep", 3);
            break;
    case 10: memcpy(texto + 3, "Oct", 3);
            break;
    case 11: memcpy(texto + 3, "Nov", 3);
            break;
    case 12: memcpy(texto + 3, "Dic", 3);
            break;
  }
  memcpy(texto + 6, "-", 3);
  *(texto + 7) = (siglo / 10) + '0';
  *(texto + 8) = (siglo % 10) + '0';
  *(texto + 9) = ((an / 10) % 10) + '0';
  *(texto + 10) = (an % 10) + '0';
  texto += 11;
  if(oscar[3] > 9) *(texto + 2) = (oscar[3] / 10) + '0';
  *(texto + 3) = (oscar[3] % 10) + '0';
  *(texto + 4) = ':';
  *(texto + 5) = (oscar[4] / 10) + '0';
  *(texto + 6) = (oscar[4] % 10) + '0';
}

decimal(numero, digitos)
  int numero, digitos;
{
  int divisor, digito, cual, es;
  if(digitos == 0)
    divisor = 1000000;
  else {
    divisor = 1;
    digito = digitos;
    while(--digito)
      divisor *= 10;
  }
  cual = es = 0;
  while(divisor) {
    digito = numero / divisor;
    numero = numero % divisor;
    if(divisor == 1) es = 1;
    if(digito) {
      putchar(digito + '0');
      es = 1;
    } else if(es) {
      putchar('0');
    } else if(digitos) putchar(' ');
    ++cual;
    divisor = divisor / 10;
  }
}

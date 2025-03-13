/*
** Revisa y repara discos.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 24 de septiembre de 1995.
** Revisión: 16 de mayo de 1996. Soporte para limpiar la tabla de asignación.
** Revisión: 5 de octubre de 1996. Elimino información inutil.
*/

#define NULL              0
#define TAM_SECTOR      512

#define E_DIRECTORIO_INCORRECTO   1  /* Un directorio incorrecto ocurre cuando
                                        una entrada del directorio en los
                                        últimos 64 bytes del bloque no tiene
                                        un bloque a continuación. */
#define E_MAL_TERMINADO           2  /* Un archivo mal terminado ocurre cuando
                                        un archivo se esta escribiendo o
                                        creciendo y no se alcanza a escribir
                                        la tabla de asignación */
#define E_LONGITUD_INCORRECTA     3  /* Un archivo con longitud incorrecta
                                        ocurre cuando un archivo crece y no
                                        se cierra al final */

/*
** Los errores más comunes que ocurren en una unidad del Sistema Operativo
** Mexicano de 32 bits:
**
** Error:
**     La tabla de asignación reporta más bytes que todos los archivos y
**     directorios juntos.
** Causa:
**     Se estaba escribiendo un archivo y no se cerro.
** Reparación:
**     Eliminación de los bloques inútiles, cuidando de no eliminar otros
**     bloques ya que generalmente la secuencia no esta terminada.
**
** Error:
**     Directorio incorrecto.
** Causa:
**     Se estaba creando un archivo o directorio y se fue la luz o el disco
**     fue sacado de la unidad.
** Reparación:
**     Agregado de un bloque relleno con 0x00 al final del directorio.
**     Reparación de la tabla de asignación.
**
** Error:
**     Archivo mal terminado.
** Causa:
**     Se estaba agregando al final de un archivo y no se cerro, se fue la luz,
**     o se extrajo el disco de la unidad.
** Reparación:
**     Es necesario explorar el disco para averiguar donde continua el archivo.
**
** Error:
**     Archivo con longitud incorrecta.
** Causa:
**     Se estaba agregando al final de un archivo y no se cerro, se fue la luz,
**     o se extrajo el disco de la unidad.
** Reparación:
**     Contar el número de bloques en la tabla de asignación y poner como el
**     tamaño del archivo.
*/

int unidad_actual;
int ultimo_sector;
int ultimo_sector_tabla;

int tipo_de_medio;
int total_sectores;
int sectores_por_bloque;
int total_bloques;
int sector_tabla;
int tam_tabla;
int bytes_por_entrada;
int bloque_dir_raiz;

int num_archivos;
int num_directorios;
int num_bytes;
int num_bytes_dirs;
int num_bytes_logicos;

char *apunta;
char mensaje[520];
char mensaje2[520];

int mascara[8];
char bits[256];

int cuantos_casos_raros;
int casos_raros[256];

char bloques[8192];    /* Indica las secuencias iniciales */
char bloques2[8192];   /* Indica bloques que apuntan el mismo bloque */
char bloques3[8192];   /* Indica la asignación de bloques para archivos */
char bloques4[8192];   /* Indica los bloques ocupados en la tabla de espacio */

main() {
  int unidad;

  mascara[0] = 0x80;
  mascara[1] = 0x40;
  mascara[2] = 0x20;
  mascara[3] = 0x10;
  mascara[4] = 0x08;
  mascara[5] = 0x04;
  mascara[6] = 0x02;
  mascara[7] = 0x01;

  calcula_bits();

  ultimo_sector = -1;
  ultimo_sector_tabla = -1;

  color(15);
  puts("\r\nChequeo de discos  (c) Copyright 1995 Oscar Toledo G.\r\n\n");
  color(10);
  puts("Unidad de disco para checar (A-Z, o otra tecla para cancelar): ");
  unidad = toupper(getchar());
  if(isalpha(unidad)) {
    putchar(unidad);
    color(11);
    puts("\r\n\nRevisando la unidad de disco ");
    putchar(unidad);
    puts(": ...");
    unidad_actual = unidad - 'A';
    obtener_parametros();
    revisar_tabla_de_espacio();
    rastrear_directorios();
  } else {
    color(15);
    puts("\r\n\nPrograma cancelado.\r\n");
  }
}

calcula_bits()
{
  int a, b, c, d;

  for(a = 0; a < 256; ++a) {
    c = 0;
    d = a;
    for(b = 0; b < 8; ++b) {
      if(d & 0x80)
        ++c;
      d <<= 1;
    }
    bits[a] = c;
  }
}

isalpha(car) int car; {
  return (((car >= 'A') && (car <= 'Z')) ||
          ((car >= 'a') && (car <= 'z')));
}

toupper(car) int car; {
  if ((car >= 'a') && (car <= 'z')) return car - 32;
  else return car;
}

/*
** Obtiene los parametros de la unidad.
*/
obtener_parametros()
{
  lee_sector(0);
  tipo_de_medio = lee_entero(mensaje + 17);
  total_sectores = lee_entero(mensaje + 21);
  sectores_por_bloque = lee_entero(mensaje + 25);
  total_bloques = lee_entero(mensaje + 29);
  sector_tabla = lee_entero(mensaje + 33);
  tam_tabla = lee_entero(mensaje + 37);
  bytes_por_entrada = lee_entero(mensaje + 41);
  bloque_dir_raiz = lee_entero(mensaje + 45);
  if ((mensaje[511] != 0x12 || mensaje[512] != 0x34)
   || (sectores_por_bloque * total_bloques != total_sectores)
   || (total_sectores == 0)
   || (sector_tabla == 0)
   || (tam_tabla == 0)
   || (bytes_por_entrada != 1 && bytes_por_entrada != 2)
   || (bloque_dir_raiz == 0)
   || ((total_bloques * bytes_por_entrada + 511) / TAM_SECTOR != tam_tabla)) {
    color(15);
    puts("No es un disco del sistema operativo mexicano de 32 bits.\r\n");
    exit(1);
  }
}

/*
** Lee una entrada de la tabla de espacio.
*/
lee_entrada(x)
  int x;
{
  int sector, valor;

  x *= bytes_por_entrada;
  sector = sector_tabla + x / TAM_SECTOR;
  x = x % TAM_SECTOR + 1;
  lee_sector_tabla(sector);
  switch(bytes_por_entrada) {
    case 1: valor = mensaje2[x];
            if(valor == 0xff)
              valor = -1;
            break;
    case 2: valor = mensaje2[x] | (mensaje2[x+1] << 8);
            if(valor == 0xffff)
              valor = -1;
            break;
    case 4: valor = lee_entero(mensaje2 + x);
            break;
  }
  return valor;
}

/*
** Escribe en una entrada de la tabla de espacio.
*/
escribe_entrada(x, val)
  int x, val;
{
  int sector, cuenta;

  x *= bytes_por_entrada;
  sector = sector_tabla + x / TAM_SECTOR;
  x = x % TAM_SECTOR + 1;
  ultimo_sector = -1;
  lee_sector(sector);
  switch(bytes_por_entrada) {
    case 1: mensaje[x] = val;
            break;
    case 2: mensaje[x] = val;
            mensaje[x + 1] = val >> 8;
            break;
    case 4: mensaje[x] = val;
            mensaje[x + 1] = val >> 8;
            mensaje[x + 2] = val >> 16;
            mensaje[x + 3] = val >> 24;
            break;
  }
  for (cuenta = 512; cuenta >= 1; cuenta--)
    mensaje[cuenta + 5] = mensaje[cuenta];
  escribe_sector(sector);
}

/*
** Carga la tabla de espacio, descubre cuantas secuencias existen y secuencias
** que puedan referir dos veces el mismo bloque.
*/
revisar_tabla_de_espacio()
{
  int a, b, c;

  color(11);
  puts("\r                                                 ");
  puts("\rRevisando el mapa de espacio libre...");
  for(a = 0; a < 8192; ++a) {
    bloques4[a] = bloques3[a] = bloques2[a] = bloques[a] = 0x00;
  }
  for(a = bloque_dir_raiz; a < total_bloques; ++a) {
    b = lee_entrada(a);
    if (b == 0)
      continue;
    bloques4[a >> 3] |= mascara[a & 7];
  }
}

int cuenta(bloque)
  char *bloque;
{
  int a, b;

  b = 0;
  for(a = 0; a < 8192; ++a)
    b += bits[bloque[a]];
  return b;
}

/*
** Rastrea los directorios.
**
** Checa directorios que no tienen final (0xffff) o que están mal
** estructurados.
**
** Compagina cada archivo con su secuencia correspondiente, y elimina la
** secuencia de la tabla de espacio.
**
** Si un archivo existente, usa parte de una secuencia de otro archivo, el
** primer archivo encontrado queda para la secuencia, y el otro es eliminado.
**
** Si una secuencia sin usar, usa parte de una secuencia de otro archivo, el
** archivo queda en uso de la secuencia, y la secuencia sin usar es eliminada.
**
** Luego selecciona todos los archivos con tamaño 0 cómo candidatos a
** una secuencia no utilizada.
**
** Luego selecciona todos los archivos con el primer byte a 0x80 cómo
** candidatos a recuperación.
*/
rastrear_directorios()
{
  int temp, cuenta1, cuenta2, por_ciento;

  num_archivos = 0;
  num_directorios = 0;
  num_bytes = 0;
  num_bytes_dirs = 0;
  num_bytes_logicos = 0;
  cuantos_casos_raros = 0;
  color(11);
  puts("\r                                     ");
  puts("\rRevisando los directorios...");
  analisis(bloque_dir_raiz);
  puts("\r");
  color(14);
  decimal(num_bytes_logicos, 0);
  puts(" bytes ocupados por ");
  decimal(num_archivos, 1);
  if(num_archivos == 1)
    puts(" archivo.\r\n");
  else
    puts(" archivos.\r\n");
  decimal(num_bytes_dirs, 0);
  puts(" bytes ocupados por ");
  decimal(num_directorios, 1);
  if(num_directorios == 1)
    puts(" directorio.\r\n");
  else
    puts(" directorios.\r\n");
  temp = cuenta(bloques2);
  if (temp) {
    decimal(temp, 0);
    if(temp == 1)
      puts(" referencia cruzada.\r\n");
    else
      puts(" referencias cruzadas.\r\n");
  }
  temp = cuantos_casos_raros / 2;
  if (temp) {
    decimal(temp, 0);
    if(temp == 1)
      puts(" caso misterioso.\r\n");
    else
      puts(" casos misteriosos.\r\n");
  }
  cuenta1 = cuenta(bloques4) * sectores_por_bloque * TAM_SECTOR;
  cuenta2 = cuenta(bloques3) * sectores_por_bloque * TAM_SECTOR;
  if (cuenta1 != cuenta2) {
    decimal(cuenta1, 0);
    puts(" bytes ocupados, según la tabla de asignación\r\n");
    decimal(cuenta2, 0);
    puts(" bytes ocupados, según los directorios\r\n");
  } else {
    decimal(cuenta2, 0);
    puts(" bytes ocupados redondeado a bloques.\r\n");
  }
  decimal(cuenta2 - num_bytes_logicos, 0);
  puts(" bytes desperdiciados en disco.\r\n");
  if (cuenta1 != cuenta2) {
    color(10);
    puts("\r\n¿ Desea hacer una limpieza de la tabla de asignación (S/N) ? ");
    temp = toupper(getchar());
    if (isalpha(temp))
      putchar(temp);
    puts("\r\n");
    if (temp == 'S') {
      limpia_tabla();
      puts("\r\n");

      ultimo_sector = -1;
      ultimo_sector_tabla = -1;

      revisar_tabla_de_espacio();
      rastrear_directorios();
    }
  }
}

limpia_tabla()
{
  int a, b, bloque, liberar, cuenta;

  cuenta = 0;
  for (a = 0; a < bloque_dir_raiz; ++a) {
    bloques4[a >> 3] |= mascara[a & 7];
    bloques3[a >> 3] |= mascara[a & 7];
  }
  for (a = 0; a < 8192; ++a) {
    if (liberar = bloques4[a] & ~bloques3[a]) {
      bloque = a * 8;
      for (b = 0; b < 8; ++b) {
        if (liberar & 0x80) {
          color(15);
          decimal(bloque, 0);
          puts("   ");
          escribe_entrada(bloque, 0);
        }
        liberar <<= 1;
        ++bloque;
      }
    }
  }
}

analisis(bloque)
  int bloque;
{
  int sector, bloque_archivo;
  int longitud, longitud_bloques;
  int bloque_inicial;
  int cuenta_bloques, contenido;
  int desp;

  ++num_directorios;
  bloques[bloque >> 3] |= mascara[bloque & 7];
  if(sel_bloque(bloque))
    error(bloque, E_DIRECTORIO_INCORRECTO);
  num_bytes_dirs += sectores_por_bloque * TAM_SECTOR;
  num_bytes_logicos += sectores_por_bloque * TAM_SECTOR;
  bloque_inicial = bloque;
  sector = 0;
  while(1) {
    for(desp = 1; desp < 513; desp += 64) {
      lee_sector(bloque * sectores_por_bloque + sector);
      if(mensaje[desp] == 0)
        return;
      if(mensaje[desp] == 0x80)
        continue;
      bloque_archivo = lee_entero(mensaje + desp + 56);
      longitud = lee_entero(mensaje + desp + 60);
      longitud_bloques = (longitud + (sectores_por_bloque * TAM_SECTOR) - 1) /
                         (sectores_por_bloque * TAM_SECTOR);
      num_bytes_logicos += longitud;
      if(mensaje[desp + 48] & 8) {
        analisis(bloque_archivo);
      } else {
        num_archivos++;
        cuenta_bloques = 0;
        contenido = bloque_archivo;
        bloques[contenido >> 3] |= mascara[contenido & 7];
        while(contenido) {
          sel_bloque(contenido);
          contenido = lee_entrada(contenido);
          ++cuenta_bloques;
          if(contenido == -1)
            break;
          if(contenido == 0) {
            error(bloque_archivo, E_MAL_TERMINADO);
            break;
          }
        }
        num_bytes += longitud_bloques * TAM_SECTOR * sectores_por_bloque;
        if(cuenta_bloques != longitud_bloques)
          error(bloque_archivo, E_LONGITUD_INCORRECTA);
      }
    }
    ++sector;
    if(sector == sectores_por_bloque) {
      sector = 0;
      bloque = lee_entrada(bloque);
      if(bloque == -1 || bloque == 0) {
        error(bloque_inicial, E_DIRECTORIO_INCORRECTO);
        return;
      }
      sel_bloque(bloque);
      num_bytes_dirs += sectores_por_bloque * TAM_SECTOR;
      num_bytes_logicos += sectores_por_bloque * TAM_SECTOR;
    }
  }
}

error(bloque, err)
  int bloque, err;
{
  if(cuantos_casos_raros == 256)
    return;
  casos_raros[cuantos_casos_raros++] = bloque;
  casos_raros[cuantos_casos_raros++] = err;
}

int sel_bloque(cual)
  int cual;
{
  int doble;

  doble = 0;
  if(bloques3[cual >> 3] & mascara[cual & 7]) {
    doble = 1;
    bloques2[cual >> 3] |= mascara[cual & 7];
  }
  bloques3[cual >> 3] |= mascara[cual & 7];
  return doble;
}

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

#define LEE_SECTOR       1

lee_sector(sector)
  int sector;
{
  if(ultimo_sector == sector)
    return 0;
  mensaje[0] = LEE_SECTOR;
  mensaje[1] = unidad_actual;
  mensaje[2] = sector >> 24;
  mensaje[3] = sector >> 16;
  mensaje[4] = sector >> 8;
  mensaje[5] = sector;
  apunta = mensaje;
  mandar(0x80000108, &apunta, 4);
  recibir(0x8000010c, &apunta, 1);
  ultimo_sector = sector;
  return mensaje[0];
}

lee_sector_tabla(sector)
  int sector;
{
  if(ultimo_sector_tabla == sector)
    return 0;
  mensaje2[0] = LEE_SECTOR;
  mensaje2[1] = unidad_actual;
  mensaje2[2] = sector >> 24;
  mensaje2[3] = sector >> 16;
  mensaje2[4] = sector >> 8;
  mensaje2[5] = sector;
  apunta = mensaje2;
  mandar(0x80000108, &apunta, 4);
  recibir(0x8000010c, &apunta, 1);
  ultimo_sector_tabla = sector;
  return mensaje2[0];
}

#define ESCRIBE_SECTOR   2

escribe_sector(sector)
  int sector;
{
  mensaje[0] = ESCRIBE_SECTOR;
  mensaje[1] = unidad_actual;
  mensaje[2] = sector >> 24;
  mensaje[3] = sector >> 16;
  mensaje[4] = sector >> 8;
  mensaje[5] = sector;
  apunta = mensaje;
  mandar(0x80000108, &apunta, 4);
  recibir(0x8000010c, &apunta, 1);
  ultimo_sector = -1;
  return mensaje[0];
}

int lee_entero(pos)
  char *pos;
{
  return *pos | (*(pos + 1) << 8) | (*(pos + 2) << 16) | (*(pos + 3) << 24);
}

decimal(numero, corta)
  int numero, corta;
{
  int divisor, digito, cual, es;
  divisor = 1000000000;
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
    } else if(!corta) putchar(' ');
    ++cual;
    if(cual == 1) if(es) putchar(',');
    else if(!corta) putchar(' ');
    if(cual == 4) if(es) putchar(',');
    else if(!corta) putchar(' ');
    if(cual == 7) if(es) putchar(',');
    else if(!corta) putchar(' ');
    divisor = divisor / 10;
  }
}


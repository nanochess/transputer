/*
** Utilidad para preparar nuevos discos.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 9 de septiembre de 1995.
** Revisión: 11 de septiembre de 1995. Indica cuanto espacio esta disponible.
** Revisión: 6 de febrero de 1996. Soporte para nombres de 32 letras.
*/

char *apunta;
char linea[10];
char datos[512];
char tabla[160];
char mensaje[520];
char volumen[32];

main() {
  int tecla;
  int pista, lado, error, reintento;
  int espacio_total;

  puts("\r\n");
  color(15);
  puts("Preparador de discos   (c) Copyright 1995 Oscar Toledo G.\r\n");
  puts("\r\n");
  color(11);
  puts("¡Advertencia!, esta utilidad borrara el contenido\r\n");
  puts("de cualquier disco que se introduzca.\r\n");
  puts("\r\n");
  color(10);
  puts("¿Desea continuar con el preparador (S/N)? ");
  lee_linea(linea, 2);
  if(toupper(linea[0]) != 'S') {
    puts("\r\n");
    color(15);
    puts("El preparador ha sido cancelado.\r\n");
    exit(1);
  }
  color(11);
  puts("\r\n");
  puts("Para salir en cualquier momento, pulse Ctrl-C.\r\n\r\n");
  color(10);
  puts("Introduzca el disco que desea preparar para usar y pulse una tecla...");
  while(1) {
    if(checa_int()) break;
  }
  puts("\r\n\n");
  for(pista = 0; pista < 80; ++pista) {
    color(11);
    puts("Preparando pista ");
    color(15);
    decimal(pista);
    color(11);
    puts(", lado 0 ");
    putchar(8);
    for(lado = 0; lado < 2; ++lado) {
      color(15);
      putchar(8);
      putchar(lado + '0');
      error = formatea_pista(pista, lado);
      checa_int();
    }
    putchar(13);
  }
  color(10);
  puts("                                        \r");
  puts("Etiqueta de volumen: ");
  lee_linea(volumen, 32);
  color(11);
  puts("\r\nEscribiendo el sistema de archivo...");
  escribe_arranque();
  checa_int();
  escribe_tabla_de_espacio();
  checa_int();
  escribe_directorio();
  checa_int();
  espacio_libre("A:/", &espacio_total);
  puts("\r");
  decimal_largo(espacio_total, 1);
  puts(" bytes disponibles en el nuevo disco.      \r\n\n");
  color(15);
  puts("Disco preparado.\r\n");
  exit(1);
}

escribe_arranque()
{
  int azar;
  char *cadena;
  char *apunta;

  memset(datos, 0, 512);

  datos[  0] = 0x24;  datos[  1] = 0x0e;      /* Salto al arrancador */

  datos[  2] = 'S';   datos[  3] = 'O';   datos[  4] = 'M';
  datos[  5] = '3';   datos[  6] = '2';   datos[  7] = ' ';
  datos[  8] = 'v';   datos[  9] = '1';   datos[ 10] = '.';
  datos[ 11] = '0';

  azar = rand();
/*
** Número de serie.
*/
  datos[ 12] = azar;
  datos[ 13] = azar >> 8;
  datos[ 14] = azar >> 16;
  datos[ 15] = azar >> 24;
/*
** Tipo de medio. 1.44 MB.
*/
  datos[ 16] = 0x00; datos[ 17] = 0x00; datos[ 18] = 0x00; datos[ 19] = 0x00;
/*
** Total de sectores en el disco.
*/
  datos[ 20] = 0x40; datos[ 21] = 0x0b; datos[ 22] = 0x00; datos[ 23] = 0x00;
/*
** Sectores por bloque.
*/
  datos[ 24] = 0x02; datos[ 25] = 0x00; datos[ 26] = 0x00; datos[ 27] = 0x00;
/*
** Total de bloques en el disco.
*/
  datos[ 28] = 0xa0; datos[ 29] = 0x05; datos[ 30] = 0x00; datos[ 31] = 0x00;
/*
** Sector de inicio de la tabla de espacio.
*/
  datos[ 32] = 0x01; datos[ 33] = 0x00; datos[ 34] = 0x00; datos[ 35] = 0x00;
/*
** Tamaño en sectores de la tabla de espacio.
*/
  datos[ 36] = 0x06; datos[ 37] = 0x00; datos[ 38] = 0x00; datos[ 39] = 0x00;
/*
** Bytes ocupados por cada entrada en la tabla de espacio.
** (1, 2 o 4, el 3 no se acepta).
*/
  datos[ 40] = 0x02; datos[ 41] = 0x00; datos[ 42] = 0x00; datos[ 43] = 0x00;
/*
** Bloque de inicio del directorio raíz.
*/
  datos[ 44] = 0x04; datos[ 45] = 0x00; datos[ 46] = 0x00; datos[ 47] = 0x00;

/*
** Codigo para un arrancador, carga SOM32.BIN, que a su vez carga
** INTERFAZ.CMG
*/
  apunta = datos + 48;
  cadena = volumen;
  while(*apunta++ = *cadena++) ;
  hex(datos + 0x050, "68B0634221FB25FA30DF357FF8DE4021");
  hex(datos + 0x060, "102A9D7E21102A982110D021264E21FB");
  hex(datos + 0x070, "D14BD27221A470F171F1F4C024AC7081");
  hex(datos + 0x080, "D07181D172608FD26109217F212F8F22");
  hex(datos + 0x090, "204022FCD2217E7FF8D121264121FB8F");
  hex(datos + 0x0a0, "4F23F224F625FAD0D371211026922110");
  hex(datos + 0x0b0, "7022204024FA70222080D07181D17260");
  hex(datos + 0x0c0, "8FD272C061A37329B0F624F22434274C");
  hex(datos + 0x0d0, "21FB219024F224342124802B4021FB94");
  hex(datos + 0x0e0, "2194690E71F1AD71F17223FB7181D172");
  hex(datos + 0x0f0, "82D2600022F060BE45D010D11124F224");
  hex(datos + 0x100, "5244FB1124F2245341F77061ABB222F0");
  hex(datos + 0x110, "71608FD1417123FB40718123FB722148");
  hex(datos + 0x120, "24F0718223FB72214024F0718323FB72");
  hex(datos + 0x130, "4824F0718423FB72718523FB71D21224");
  hex(datos + 0x140, "F2245244FB1224F2245341F722F04573");
  hex(datos + 0x150, "746520646973636F206E6F20636F6E74");
  hex(datos + 0x160, "69656E6520656C2053697374656D6120");
  hex(datos + 0x170, "4F706572617469766F204D6578696361");
  hex(datos + 0x180, "6E6F20646520333220626974732E0049");
  hex(datos + 0x190, "6E736572746520756E20646973636F20");
  hex(datos + 0x1a0, "71756520636F6E74656E676120656C20");
  hex(datos + 0x1b0, "73697374656D61206F70657261746976");
  hex(datos + 0x1c0, "6F2C20792070756C7365206375616C71");
  hex(datos + 0x1d0, "75696572207465636C612E2E2E00534F");
  hex(datos + 0x1e0, "4D2E33322E62696E002863292046616D");
  hex(datos + 0x1f0, "2E20546F6C65646F2031393936001234");
  escribe_sector(0);
}

hex(direccion, datos)
  char *direccion, *datos;
{
  int valor;
  int valor2;

  while(*datos) {
    valor2 = *datos++ - '0';
    if(valor2 > 9) valor2 -= 7;
    valor = valor2 << 4;
    valor2 = *datos++ - '0';
    if(valor2 > 9) valor2 -= 7;
    valor |= valor2;
    *direccion++ = valor;
  }
}

escribe_tabla_de_espacio()
{
  memset(datos, 0, 512);
  escribe_sector(2);
  escribe_sector(3);
  escribe_sector(4);
  escribe_sector(5);
  escribe_sector(6);
  escribe_sector(7);
  datos[0] = 0xfd; datos[1] = 0xff;
  datos[2] = 0xfd; datos[3] = 0xff;
  datos[4] = 0xfd; datos[5] = 0xff;
  datos[6] = 0xfd; datos[7] = 0xff;
  datos[8] = 0xff; datos[9] = 0xff;
  escribe_sector(1);
}

escribe_directorio()
{
  memset(datos, 0, 512);
  escribe_sector(8);
  escribe_sector(9);
}

decimal_largo(numero, corta)
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

color(col) int col; {
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

toupper(car) int car; {
  if((car >= 'a') & (car <= 'z')) return car - 32;
  else return car;
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

decimal(numero)
  int numero;
{
  if (numero < 0) {
    putchar('-');
    if (numero < -9)
      decimal(-(numero / 10));
    putchar(-(numero % 10) + '0');
  } else {
    if (numero > 9)
      decimal(numero / 10);
    putchar((numero % 10) + '0');
  }
}

#define LEE_SECTOR       1

lee_sector(sector)
  int sector;
{
  mensaje[0] = LEE_SECTOR;
  mensaje[1] = 0;
  mensaje[2] = sector >> 24;
  mensaje[3] = sector >> 16;
  mensaje[4] = sector >> 8;
  mensaje[5] = sector;
  ctrl_maestro(mensaje);
  memcpy(datos, mensaje + 1, 512);
  return mensaje[0];
}

#define ESCRIBE_SECTOR   2

escribe_sector(sector)
  int sector;
{
  mensaje[0] = ESCRIBE_SECTOR;
  mensaje[1] = 0;
  mensaje[2] = sector >> 24;
  mensaje[3] = sector >> 16;
  mensaje[4] = sector >> 8;
  mensaje[5] = sector;
  memcpy(mensaje + 6, datos, 512);
  ctrl_maestro(mensaje);
  return mensaje[0];
}

#define FORMATEA_PISTA   3

formatea_pista(pista, lado)
  int pista, lado;
{
  mensaje[0] = FORMATEA_PISTA;
  mensaje[1] = 0;
  mensaje[2] = pista;
  mensaje[3] = lado;
  ctrl_maestro(mensaje);
  return mensaje[0];
}

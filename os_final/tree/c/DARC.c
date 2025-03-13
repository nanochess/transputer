/*
** Descompresor de archivos ARC.
**
** (c) Copyright 1995 Oscar Toledo Gutiérrez.
**
** Creación: 29 de diciembre de 1995.
** Revisión: 30 de diciembre de 1995. Corrección de defectos menores.
** Revisión: 1o. de enero de 1996. Mejoras mínimas.
*/

#define NULL          0
#define EOF         (-1)
#define TAM_LINEA   256

int firstc;
int firstch;
int ultimo_byte;
int rmask[9];

int archivo_arc;
int archivo_nuevo;

int tam_comprimido;
int tam_sin_comprimir;

int estado;

char encabezado[27];

char nombre[TAM_LINEA];
char dir[TAM_LINEA];
char archivo[TAM_LINEA];

int version;

int crc16_actual;
unsigned short crc[256];

main()
{
  int a;

  llena_tabla_crc();
  rmask[0] = 0;
  rmask[1] = 0x01;
  rmask[2] = 0x03;
  rmask[3] = 0x07;
  rmask[4] = 0x0f;
  rmask[5] = 0x1f;
  rmask[6] = 0x3f;
  rmask[7] = 0x7f;
  rmask[8] = 0xff;

  color(15);
  puts("\r\nDescompresor de archivos ARC  (c) Copyright 1995 Oscar Toledo G.\r\n");
  color(10);
  puts("\r\nNombre del archivo ARC para descomprimir > ");
  lee_linea(nombre, TAM_LINEA);
  if(nombre[0] == 0) {
    color(15);
    puts("\r\nPrograma cancelado.\r\n");
    exit(1);
  }
  puts("\r\nDirectorio destino. (Enter= No descomprime, solo lista)\r\n> ");
  lee_linea(dir, TAM_LINEA);
  if(dir[0] == 0)
    lista_los_archivos();
  else {
    if(dir[strlen(dir) - 1] == '/')
      dir[strlen(dir) - 1] = 0;
    descompresion_de_archivos();
  }
}

lista_los_archivos()
{
  archivo_arc = fopen(nombre, "r");
  if(archivo_arc == 0) {
    color(15);
    puts("\r\nEl archivo ARC no existe.\r\n");
    exit(1);
  }
  lista_archivos();
  fclose(archivo_arc);
}

descompresion_de_archivos()
{
  archivo_arc = fopen(nombre, "r");
  if(archivo_arc == 0) {
    color(15);
    puts("\r\nEl archivo ARC no existe.\r\n");
    exit(1);
  }
  descomprime_archivos();
  fclose(archivo_arc);
}

lee_entero(x)
  char *x;
{
  return *x | (*(x+1) << 8) | (*(x+2) << 16) | (*(x+3) << 24);
}

lee_entero_corto(x)
  char *x;
{
  return *x | (*(x+1) << 8);
}

lista_archivos(cuantos)
  int cuantos;
{
  int ac_tam1, ac_tam2, ac_total, relacion;

  ac_tam1 = ac_tam2 = ac_total = 0;
  puts("\r\n");
  color(14);
  puts("Longitud Metodo      Tamaño Total   Fecha   Tiempo CRC-16 Nombre\r\n");
  puts("-------- ------      ------ -----   -----   ------ ------ ------\r\n");
  color(11);
  while(lee_encabezado())
    descripcion(&ac_tam1, &ac_tam2, &ac_total);
  color(14);
  puts("--------             ------ -----                         ------\r\n");
  decimal(ac_tam2, 8);
  puts("            ");
  decimal(ac_tam1, 7);
  puts("  ");
  relacion = (ac_tam1 > 20000000) ?
             (ac_tam1 * 100 / ac_tam2) :
             (ac_tam1 / (ac_tam2 / 100));
  decimal(relacion, 3);
  puts("%                         ");
  decimal(ac_total, 6);
  puts("\r\n");
}

descomprime_archivos(cuantos)
  int cuantos;
{
  puts("\r\n");
  while(lee_encabezado()) {
    descompresion();
  }
}

lee_encabezado()
{
  int byte, cuenta;

  byte = fgetc(archivo_arc);
  if(byte == -1)
    return 0;
  cuenta = 10;
  while(byte != 0x1a) {
    --cuenta;
    if(cuenta == 0) {
      color(15);
      puts("Probablemente no es un archivo ARC.\r\n");
      return 0;
    }
    puts("Fuera de sincronia.\r\n");
    byte = fgetc(archivo_arc);
    if(byte == -1) {
      color(15);
      puts("Error en el archivo ARC.\r\n");
      return 0;
    }
  }
  byte = fgetc(archivo_arc);
  if(byte == 0)
    return 0;
  version = byte;
  if(version == 1) {
    fread(archivo_arc, encabezado, 23);
    version = 2;
    encabezado[23] = encabezado[13];
    encabezado[24] = encabezado[14];
    encabezado[25] = encabezado[15];
    encabezado[26] = encabezado[16];
  } else
    fread(archivo_arc, encabezado, 27);
  return 1;
}

descripcion(ac_tam1, ac_tam2, ac_total)
  int *ac_tam1, *ac_tam2, *ac_total;
{
  int temp, an, mes, dia, hora, minuto, relacion;
  char *apunta;
  int tam1, tam2;

  tam1 = lee_entero(encabezado + 13);
  tam2 = lee_entero(encabezado + 23);
  *ac_tam1 += tam1;
  *ac_tam2 += tam2;
  (*ac_total)++;
  relacion = (tam1 > 20000000) ?
             (tam1 * 100 / tam2) :
             (tam1 / (tam2 / 100));
  temp = lee_entero(encabezado + 23);
  decimal(temp, 8);
  puts(" ");
  switch(version) {
    case 1:
    case 2: puts("Metodo 1   ");
            break;
    case 3: puts("Metodo 3   ");
            break;
    case 4: puts("Metodo 4   ");
            break;
    case 5:
    case 6:
    case 7: puts("Metodo 5   ");
            break;
    case 8: puts("Metodo 8   ");
            break;
    case 9: puts("Metodo 9   ");
            break;
    default: puts("??????     ");
            break;
  }
  temp = lee_entero(encabezado + 13);
  decimal(temp, 7);
  puts("  ");
  decimal(relacion, 3);
  puts("% ");
  an = (((lee_entero_corto(encabezado + 17) >> 9) & 0x7f) + 80) % 100;
  mes = (lee_entero_corto(encabezado + 17) >> 5) & 0x0f;
  dia = lee_entero_corto(encabezado + 17) & 0x1f;
  hora = (lee_entero_corto(encabezado + 19) >> 11) & 0x1f;
  minuto = (lee_entero_corto(encabezado + 19) >> 5) & 0x3f;
  if(dia > 10)
    putchar(dia / 10 + '0');
  else
    putchar(' ');
  putchar(dia % 10 + '0');
  putchar('-');
  switch(mes) {
    case  1: puts("Ene");
             break;
    case  2: puts("Feb");
             break;
    case  3: puts("Mar");
             break;
    case  4: puts("Abr");
             break;
    case  5: puts("May");
             break;
    case  6: puts("Jun");
             break;
    case  7: puts("Jul");
             break;
    case  8: puts("Ago");
             break;
    case  9: puts("Sep");
             break;
    case 10: puts("Oct");
             break;
    case 11: puts("Nov");
             break;
    case 12: puts("Dic");
             break;
    default: puts("???");
             break;
  }
  putchar('-');
  putchar(an / 10 + '0');
  putchar(an % 10 + '0');
  putchar(' ');
  putchar(' ');
  if(hora > 10)
    putchar(hora / 10 + '0');
  else
    putchar(' ');
  putchar(hora % 10 + '0');
  putchar(':');
  putchar(minuto / 10 + '0');
  putchar(minuto % 10 + '0');
  putchar(' ');
  putchar(' ');
  hex(encabezado[22]);
  hex(encabezado[21]);
  putchar(' ');
  putchar(' ');
  puts(encabezado);
  if(checa_tecla()) {
    while(checa_tecla() == 0) ;
  }
  fseek(archivo_arc, lee_entero(encabezado + 13), 1);
  puts("\r\n");
}

descompresion()
{
  int donde, datos, temp;
  int longitud_original;
  int longitud_comprimida;
  int fecha;
  int tiempo;
  int crc16, byte;
  char *apunta;

  tam_sin_comprimir = longitud_original = lee_entero(encabezado + 23);
  color(11);
  switch(version) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9: puts("Extrayendo: ");
            break;
    default:puts(" Evadiendo: ");
            break;
  }
  color(14);
  tam_comprimido = longitud_comprimida = lee_entero(encabezado + 13);
  fecha = lee_entero_corto(encabezado + 17);
  tiempo = lee_entero_corto(encabezado + 19);
  crc16 = lee_entero_corto(encabezado + 21);
  puts(encabezado);
  donde = ftell(archivo_arc);
  inicia_calculo_crc();
  strcpy(archivo, dir);
  strcat(archivo, "/");
  strcat(archivo, encabezado);
  if(archivo[strlen(archivo) - 1] == '/') {
    archivo_nuevo = 0;
    crea_dir(archivo);
    atributo(archivo, 8, corrige_hora());
  } else {
    archivo_nuevo = fopen(archivo, "w");
    if(archivo_nuevo == 0) {
      mapea_directorios(archivo);
      archivo_nuevo = fopen(archivo, "w");
      if(archivo_nuevo == 0) {
        color(15);
        puts("\r\n            No se pudo crear el archivo.");
      }
    }
  }
  if(archivo_nuevo) {
    firstc = 1;
    firstch = 1;
    switch(version) {
      case 1:
      case 2: byte = lee_byte();
              while(byte != EOF) {
                escribe(byte);
                byte = lee_byte();
              }
              break;
      case 3: estado = 0;
              byte = lee_byte();
              while(byte != EOF) {
                escribe_ncr(byte);
                byte = lee_byte();
              }
              break;
      case 4: inicia_usq();
              byte = lee_usq();
              while(byte != EOF) {
                escribe_ncr(byte);
                byte = lee_usq();
              }
              break;
      case 5: inicia_ucr(0);
              byte = lee_ucr();
              while(byte != EOF) {
                escribe(byte);
                byte = lee_ucr();
              }
              break;
      case 6: inicia_ucr(0);
              byte = lee_ucr();
              while(byte != EOF) {
                escribe_ncr(byte);
                byte = lee_ucr();
              }
              break;
      case 7: inicia_ucr(1);
              byte = lee_ucr();
              while(byte != EOF) {
                escribe_ncr(byte);
                byte = lee_ucr();
              }
              break;
      case 8: decomp(0);
              break;
      case 9: decomp(1);
              break;
      default:
              color(15);
              puts("\r\n            Metodo de descompresión desconocido.");
              crc16_actual = ~lee_entero_corto(encabezado + 21);
              break;
    }
    fclose(archivo_nuevo);
    atributo(archivo, 0, corrige_hora());
    if(crc16_actual != lee_entero_corto(encabezado + 21)) {
      color(15);
      puts("\r\n            CRC-16 incorrecto en este archivo.");
    }
  }
  if(checa_tecla()) {
    while(checa_tecla() == 0) ;
  }
  puts("\r\n");
  fseek(archivo_arc, donde + lee_entero(encabezado + 13), 0);
}

hex(valor)
  int valor;
{
  hex1(valor >> 4);
  hex1(valor);
}

hex1(valor)
  int valor;
{
  valor &= 15;
  if(valor < 10)
    putchar(valor + '0');
  else
    putchar(valor + ('a' - 10));
}

tolower(c)
  int c;
{
  if(c >= 'A' && c <= 'Z')
    return c + 32;
  return c;
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

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

strcpy(a, b)
  char *a, *b;
{
  while(*a++ = *b++) ;
}

strcat(a, b)
  char *a, *b;
{
  while(*a) ++a;
  strcpy(a, b);
}

strlen(x)
  char *x;
{
  char *c;
  c = x;
  while(*x) ++x;
  return x - c;
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

llena_tabla_crc()
{
  crc[0] = 0x0000;
  crc[1] = 0xc0c1;
  crc[2] = 0xc181;
  crc[3] = 0x0140;
  crc[4] = 0xc301;
  crc[5] = 0x03c0;
  crc[6] = 0x0280;
  crc[7] = 0xc241;

  crc[8] = 0xc601;
  crc[9] = 0x06c0;
  crc[10] = 0x0780;
  crc[11] = 0xc741;
  crc[12] = 0x0500;
  crc[13] = 0xc5c1;
  crc[14] = 0xc481;
  crc[15] = 0x0440;

  crc[16] = 0xcc01;
  crc[17] = 0x0cc0;
  crc[18] = 0x0d80;
  crc[19] = 0xcd41;
  crc[20] = 0x0f00;
  crc[21] = 0xcfc1;
  crc[22] = 0xce81;
  crc[23] = 0x0e40;

  crc[24] = 0x0a00;
  crc[25] = 0xcac1;
  crc[26] = 0xcb81;
  crc[27] = 0x0b40;
  crc[28] = 0xc901;
  crc[29] = 0x09c0;
  crc[30] = 0x0880;
  crc[31] = 0xc841;

  crc[32] = 0xd801;
  crc[33] = 0x18c0;
  crc[34] = 0x1980;
  crc[35] = 0xd941;
  crc[36] = 0x1b00;
  crc[37] = 0xdbc1;
  crc[38] = 0xda81;
  crc[39] = 0x1a40;

  crc[40] = 0x1e00;
  crc[41] = 0xdec1;
  crc[42] = 0xdf81;
  crc[43] = 0x1f40;
  crc[44] = 0xdd01;
  crc[45] = 0x1dc0;
  crc[46] = 0x1c80;
  crc[47] = 0xdc41;

  crc[48] = 0x1400;
  crc[49] = 0xd4c1;
  crc[50] = 0xd581;
  crc[51] = 0x1540;
  crc[52] = 0xd701;
  crc[53] = 0x17c0;
  crc[54] = 0x1680;
  crc[55] = 0xd641;

  crc[56] = 0xd201;
  crc[57] = 0x12c0;
  crc[58] = 0x1380;
  crc[59] = 0xd341;
  crc[60] = 0x1100;
  crc[61] = 0xd1c1;
  crc[62] = 0xd081;
  crc[63] = 0x1040;

  crc[64] = 0xf001;
  crc[65] = 0x30c0;
  crc[66] = 0x3180;
  crc[67] = 0xf141;
  crc[68] = 0x3300;
  crc[69] = 0xf3c1;
  crc[70] = 0xf281;
  crc[71] = 0x3240;

  crc[72] = 0x3600;
  crc[73] = 0xf6c1;
  crc[74] = 0xf781;
  crc[75] = 0x3740;
  crc[76] = 0xf501;
  crc[77] = 0x35c0;
  crc[78] = 0x3480;
  crc[79] = 0xf441;

  crc[80] = 0x3c00;
  crc[81] = 0xfcc1;
  crc[82] = 0xfd81;
  crc[83] = 0x3d40;
  crc[84] = 0xff01;
  crc[85] = 0x3fc0;
  crc[86] = 0x3e80;
  crc[87] = 0xfe41;

  crc[88] = 0xfa01;
  crc[89] = 0x3ac0;
  crc[90] = 0x3b80;
  crc[91] = 0xfb41;
  crc[92] = 0x3900;
  crc[93] = 0xf9c1;
  crc[94] = 0xf881;
  crc[95] = 0x3840;

  crc[96] = 0x2800;
  crc[97] = 0xe8c1;
  crc[98] = 0xe981;
  crc[99] = 0x2940;
  crc[100] = 0xeb01;
  crc[101] = 0x2bc0;
  crc[102] = 0x2a80;
  crc[103] = 0xea41;

  crc[104] = 0xee01;
  crc[105] = 0x2ec0;
  crc[106] = 0x2f80;
  crc[107] = 0xef41;
  crc[108] = 0x2d00;
  crc[109] = 0xedc1;
  crc[110] = 0xec81;
  crc[111] = 0x2c40;

  crc[112] = 0xe401;
  crc[113] = 0x24c0;
  crc[114] = 0x2580;
  crc[115] = 0xe541;
  crc[116] = 0x2700;
  crc[117] = 0xe7c1;
  crc[118] = 0xe681;
  crc[119] = 0x2640;

  crc[120] = 0x2200;
  crc[121] = 0xe2c1;
  crc[122] = 0xe381;
  crc[123] = 0x2340;
  crc[124] = 0xe101;
  crc[125] = 0x21c0;
  crc[126] = 0x2080;
  crc[127] = 0xe041;

  crc[128] = 0xa001;
  crc[129] = 0x60c0;
  crc[130] = 0x6180;
  crc[131] = 0xa141;
  crc[132] = 0x6300;
  crc[133] = 0xa3c1;
  crc[134] = 0xa281;
  crc[135] = 0x6240;

  crc[136] = 0x6600;
  crc[137] = 0xa6c1;
  crc[138] = 0xa781;
  crc[139] = 0x6740;
  crc[140] = 0xa501;
  crc[141] = 0x65c0;
  crc[142] = 0x6480;
  crc[143] = 0xa441;

  crc[144] = 0x6c00;
  crc[145] = 0xacc1;
  crc[146] = 0xad81;
  crc[147] = 0x6d40;
  crc[148] = 0xaf01;
  crc[149] = 0x6fc0;
  crc[150] = 0x6e80;
  crc[151] = 0xae41;

  crc[152] = 0xaa01;
  crc[153] = 0x6ac0;
  crc[154] = 0x6b80;
  crc[155] = 0xab41;
  crc[156] = 0x6900;
  crc[157] = 0xa9c1;
  crc[158] = 0xa881;
  crc[159] = 0x6840;

  crc[160] = 0x7800;
  crc[161] = 0xb8c1;
  crc[162] = 0xb981;
  crc[163] = 0x7940;
  crc[164] = 0xbb01;
  crc[165] = 0x7bc0;
  crc[166] = 0x7a80;
  crc[167] = 0xba41;

  crc[168] = 0xbe01;
  crc[169] = 0x7ec0;
  crc[170] = 0x7f80;
  crc[171] = 0xbf41;
  crc[172] = 0x7d00;
  crc[173] = 0xbdc1;
  crc[174] = 0xbc81;
  crc[175] = 0x7c40;

  crc[176] = 0xb401;
  crc[177] = 0x74c0;
  crc[178] = 0x7580;
  crc[179] = 0xb541;
  crc[180] = 0x7700;
  crc[181] = 0xb7c1;
  crc[182] = 0xb681;
  crc[183] = 0x7640;

  crc[184] = 0x7200;
  crc[185] = 0xb2c1;
  crc[186] = 0xb381;
  crc[187] = 0x7340;
  crc[188] = 0xb101;
  crc[189] = 0x71c0;
  crc[190] = 0x7080;
  crc[191] = 0xb041;

  crc[192] = 0x5000;
  crc[193] = 0x90c1;
  crc[194] = 0x9181;
  crc[195] = 0x5140;
  crc[196] = 0x9301;
  crc[197] = 0x53c0;
  crc[198] = 0x5280;
  crc[199] = 0x9241;

  crc[200] = 0x9601;
  crc[201] = 0x56c0;
  crc[202] = 0x5780;
  crc[203] = 0x9741;
  crc[204] = 0x5500;
  crc[205] = 0x95c1;
  crc[206] = 0x9481;
  crc[207] = 0x5440;

  crc[208] = 0x9c01;
  crc[209] = 0x5cc0;
  crc[210] = 0x5d80;
  crc[211] = 0x9d41;
  crc[212] = 0x5f00;
  crc[213] = 0x9fc1;
  crc[214] = 0x9e81;
  crc[215] = 0x5e40;

  crc[216] = 0x5a00;
  crc[217] = 0x9ac1;
  crc[218] = 0x9b81;
  crc[219] = 0x5b40;
  crc[220] = 0x9901;
  crc[221] = 0x59c0;
  crc[222] = 0x5880;
  crc[223] = 0x9841;

  crc[224] = 0x8801;
  crc[225] = 0x48c0;
  crc[226] = 0x4980;
  crc[227] = 0x8941;
  crc[228] = 0x4b00;
  crc[229] = 0x8bc1;
  crc[230] = 0x8a81;
  crc[231] = 0x4a40;

  crc[232] = 0x4e00;
  crc[233] = 0x8ec1;
  crc[234] = 0x8f81;
  crc[235] = 0x4f40;
  crc[236] = 0x8d01;
  crc[237] = 0x4dc0;
  crc[238] = 0x4c80;
  crc[239] = 0x8c41;

  crc[240] = 0x4400;
  crc[241] = 0x84c1;
  crc[242] = 0x8581;
  crc[243] = 0x4540;
  crc[244] = 0x8701;
  crc[245] = 0x47c0;
  crc[246] = 0x4680;
  crc[247] = 0x8641;

  crc[248] = 0x8201;
  crc[249] = 0x42c0;
  crc[250] = 0x4380;
  crc[251] = 0x8341;
  crc[252] = 0x4100;
  crc[253] = 0x81c1;
  crc[254] = 0x8081;
  crc[255] = 0x4040;
}

inicia_calculo_crc()
{
  crc16_actual = 0;
}

escribe(byte)
  int byte;
{
  crc16_actual = ((crc16_actual >> 8) & 0xff)
                 ^ crc[(crc16_actual ^ byte) & 0xff];
  if(fputc(byte, archivo_nuevo) == -1) {
    fclose(archivo_nuevo);
    fclose(archivo_arc);
    color(15);
    puts("\r\nError de escritura.\r\n");
    exit(1);
  }
}

mapea_directorios(nombre)
  char *nombre;
{
  char *apunta;
  char dir[256];

  apunta = dir;
  strcpy(dir, nombre);
  if(*(apunta + 1) == ':' && *apunta)
    apunta += 2;
  if(*apunta == '/') ++apunta;
  while(1) {
    strcpy(dir, nombre);
    while(*apunta && *apunta != '/')
      ++apunta;
    if(*apunta == 0)
      return;
    *apunta++ = 0;
    crea_dir(dir);
  }
}

corrige_hora()
{
  int p;

  p = lee_entero_corto(encabezado + 19);
  p |= lee_entero_corto(encabezado + 17) << 16;
  p += 0x14000000;
  return p;
}

lee_byte()
{
  if(tam_comprimido == 0)
    return -1;
  --tam_comprimido;
  return fgetc(archivo_arc);
}

escribe_ncr(byte)
  int byte;
{
  switch(estado) {
    case 0:
      if(byte == 0x90)
        estado = 1;
      else {
        ultimo_byte = byte;
        escribe(byte);
      }
      break;
    case 1:
      if(byte == 0)
        escribe(0x90);
      else {
        while(--byte)
          escribe(ultimo_byte);
      }
      estado = 0;
      break;
  }
}

/*
** Algoritmo para el "Unsqueezing"
*/

#define ERROR         -1
#define SPEOF        256
#define NUMVALS      256

struct nd {
  int child[2];
};

struct nd node[257];
int bpos;
int curin;
int numnodes;

/*
** Prepara el Unsqueezing.
*/
inicia_usq()
{
  int i;

  bpos = 99;
  numnodes = lee_byte();
  numnodes |= lee_byte() << 8;

  if (numnodes < 0 || numnodes > NUMVALS) {
    color(15);
    puts("\r\nEl archivo tiene un árbol de decodificación incorrecto\r\n");
    fclose(archivo_nuevo);
    fclose(archivo_arc);
    exit(1);
  }
  node[0].child[0] = -(SPEOF + 1);
  node[0].child[1] = -(SPEOF + 1);

  for(i = 0; i <= numnodes - 1; ++i) {
    node[i].child[0] = lee_byte();
    node[i].child[0] |= lee_byte() << 8;
    node[i].child[1] = lee_byte();
    node[i].child[1] |= lee_byte() << 8;
  }
}

int lee_usq()
{
  int i;

  i = 0;
  while (i >= 0) {
    bpos++;
    if (bpos > 7) {
      if((curin = lee_byte()) == EOF)
        return ERROR;
      bpos = 0;
      i = node[i].child[1 & curin];
    } else {
      curin >>= 1;
      i = node[i].child[1 & curin];
    }
  }
  i = - (i + 1);

  if (i == SPEOF)
    return -1;
  else
    return i;
}

/*
** Rutinas para descompresión Lempel-Ziv-Welch.
** (unsquashing y uncrunching)
*/

#define TABSIZE    4096
#define TABSIZEM1  4095
#define NO_PRED     (-1)
#define EMPTY       (-1)

struct entry {
  short next;
  short predecessor;
  char follower;
  char used;
};

char stack[TABSIZE];
int sp;
struct entry string_tab[TABSIZE];

int code_count, code, oldcode, finchar;
int inbuf, outbuf, newhash;

#define Crunch_bits   12
#define Squash_bits   13
#define HSIZE       8192
#define INIT_bits      9
#define FIRST        257
#define CLEAR        256

int bits, n_bits, maxcode;
short prefix[HSIZE];
char suffix[HSIZE];
char buf[Squash_bits];
int clear_flg;
char stack1[HSIZE];
int free_ent;
int maxcodemax;
int offset, sizex;

int h(pred, foll)
  int pred, foll;
{
  long local;
  int V;

  if(!newhash)
    local = (pred + foll) | 0x0800;
  else
    local = (pred + foll) * 15073;
  return local & 0x0fff;
}

int eolist(index)
  int index;
{
  int temp;

  temp = string_tab[index].next;
  while (temp) {
    index = temp;
    temp = string_tab[index].next;
  }
  return index;
}

int hash(pred, foll)
  int pred, foll;
{
  int local, tempnext;

  local = h(pred, foll);
  if (!string_tab[local].used)
    return local;
  else {
    local = eolist(local);
    tempnext = (local + 101) & 0x0fff;
    while (string_tab[tempnext].used)
      if(++tempnext == TABSIZE)
        tempnext = 0;
    string_tab[local].next = tempnext;
    return tempnext;
  }
}

upd_tab(pred, foll)
  int pred, foll;
{
  int val;

  val = hash(pred, foll);
  string_tab[val].used = 1;
  string_tab[val].next = 0;
  string_tab[val].predecessor = pred;
  string_tab[val].follower = foll;
}

int gocode()
{
  int localbuf, returnval;
  if (inbuf == EMPTY) {
    localbuf = lee_byte();
    if (localbuf == EOF)
      return -1;
    localbuf &= 0xff;
    inbuf = lee_byte();
    if (inbuf == EOF)
      return -1;
    inbuf &= 0xff;
    returnval = ((localbuf << 4) & 0x0FF0) + ((inbuf >> 4) & 0x000F);
    inbuf = inbuf & 0x000f;
  } else {
    localbuf = lee_byte();
    if (localbuf == EOF)
      return -1;
    localbuf &= 0xff;
    returnval = localbuf + ((inbuf << 8) & 0x0F00);
    inbuf = EMPTY;
  }
  return returnval;
}

push(c)
  int c;
{
  stack[sp++] = c;
  if (sp >= TABSIZE) {
    fclose(archivo_arc);
    fclose(archivo_nuevo);
    color(15);
    puts("\r\nSobrepasamiento de la pila\r\n");
    exit(1);
  }
}

int pop()
{
  if (sp > 0)
    return stack[--sp];
  else
    return EMPTY;
}

init_tab()
{
  int i;

  memzero(string_tab, sizeof(string_tab));

  for(i = 0; i < 256; ++i)
    upd_tab(NO_PRED, i);

  inbuf = EMPTY;
}

inicia_ucr(i)
  int i;
{
  newhash = (i == 1);
  sp = 0;
  init_tab();
  code_count = TABSIZE - 256;
  firstc = 1;
}

int lee_ucr()
{
  int c, code, newcode;

  if(firstc) {
    firstc = 0;
    oldcode = gocode();
    finchar = string_tab[oldcode].follower;
    return finchar;
  }
  if(sp == 0) {
    newcode = gocode();
    code = newcode;
    if (code == -1)
      return -1;
    if (!string_tab[code].used) {
      code = oldcode;
      push(finchar);
    }
    while (string_tab[code].predecessor != NO_PRED) {
      push(string_tab[code].follower);
      code = string_tab[code].predecessor;
    }
    finchar = string_tab[code].follower;
    push(finchar);
    if (code_count) {
      upd_tab(oldcode, finchar);
      --code_count;
    }
    oldcode = newcode;
  }
  return pop();
}

int getcode()
{
  int code, r_off, bitsx, bp;

  if(firstch) {
    offset = 0;
    sizex = 0;
    firstch = 0;
  }
  bp = 0;
  if (clear_flg > 0 || offset >= sizex || free_ent > maxcode) {
    if(free_ent > maxcode) {
      ++n_bits;
      if(n_bits == bits)
        maxcode = maxcodemax;
      else
        maxcode = (1 << n_bits) - 1;
    }
    if(clear_flg > 0) {
      n_bits = INIT_bits;
      maxcode = (1 << n_bits) - 1;
      clear_flg = 0;
    }
    for(sizex = 0; sizex < n_bits; ++sizex) {
      code = lee_byte();
      if(code == -1)
        break;
      buf[sizex] = code;
    }
    if (sizex <= 0)
      return -1;
    offset = 0;
    sizex = (sizex << 3) - (n_bits - 1);
  }
  r_off = offset;
  bitsx = n_bits;

  bp += r_off >> 3;
  r_off &= 7;

  code = buf[bp++] >> r_off;
  bitsx -= 8 - r_off;
  r_off = 8 - r_off;

  if(bitsx >= 8) {
    code |= buf[bp++] << r_off;
    r_off += 8;
    bitsx -= 8;
  }

  code |= (buf[bp] & rmask[bitsx]) << r_off;
  offset += n_bits;

  return code;
}

decomp(SquashFlag)
  int SquashFlag;
{
  int stackp, finchar, code, oldcode, incode;

  if (SquashFlag == 0)
    bits = Crunch_bits;
  else
    bits = Squash_bits;

  if (firstch)
    maxcodemax = 1 << bits;

  if (SquashFlag == 0) {
    code = lee_byte();
    if (code != bits) {
      fclose(archivo_nuevo);
      fclose(archivo_arc);
      color(15);
      puts("\r\nEl archivo esta compactado con demasiados bits.\r\n");
      exit(1);
    }
  }

  clear_flg = 0;
  n_bits = INIT_bits;
  maxcode = (1 << n_bits ) - 1;

  for (code = 0; code < 256; ++code) {
    prefix[code] = 0;
    suffix[code] = code;
  }

  free_ent = FIRST;
  oldcode = getcode();
  finchar = oldcode;

  if (oldcode == -1)
    return;

  if (SquashFlag == 0)
    escribe_ncr(finchar);
  else
    escribe(finchar);

  stackp = 0;

  code = getcode();
  while (code > -1) {
    if(code == CLEAR) {
      for (code = 0; code < 256; ++code)
        prefix[code] = 0;
      clear_flg = 1;
      free_ent = FIRST - 1;
      code = getcode();
      if(code == -1)
        break;
    }

    incode = code;
    if (code >= free_ent) {
      stack1[stackp++] = finchar;
      code = oldcode;
    }

    while (code >= 256) {
      stack1[stackp++] = suffix[code];
      code = prefix[code];
    }

    finchar = suffix[code];
    stack1[stackp++] = finchar;
    do {
      if (SquashFlag == 0)
        escribe_ncr(stack1[--stackp]);
      else
        escribe(stack1[--stackp]);
    } while(stackp);

    code = free_ent;

    if (code < maxcodemax) {
      prefix[code] = oldcode;
      suffix[code] = finchar;
      free_ent = code + 1;
    }

    oldcode = incode;
    code = getcode();
  }
}

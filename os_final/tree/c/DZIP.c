/*
** Descompresor de archivos ZIP.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 26 de diciembre de 1995.
** Revisión: 1o. de enero de 1996. Mejoras mínimas.
** Revisión: 6 de febrero de 1996. Mantiene la hora de los directorios.
*/

#define NULL          0
#define TAM_LINEA   256
#define TAM_BUF    8192

char e_local[4];
char e_central[4];
char e_fin[4];
char e_ext[4];

char eb_actual[4];
char eb_local[26];
char eb_central[42];
char eb_fin[18];

int archivo_zip;
int tam_zip;
int archivo_nuevo;
int tam_comprimido;
int tam_sin_comprimir;

char nombre[TAM_LINEA];
char dir[TAM_LINEA];
char archivo[TAM_LINEA];

int crc32_actual;
int crc[256];

char buf1[TAM_BUF];

char ventana[32776];           /* Ventana de descompresión */

main()
{
  int a;

  inicia_datos();
  llena_tabla_crc();

  color(15);
  puts("\r\nDescompresor de archivos ZIP  (c) Copyright 1995 Oscar Toledo G.\r\n");
  color(10);
  puts("\r\nNombre del archivo ZIP para descomprimir > ");
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

inicia_datos()
{
  e_local[0] = e_central[0] = e_fin[0] = e_ext[0] = 'P';
  e_local[1] = e_central[1] = e_fin[1] = e_ext[1] = 'K';
  e_local[2] = 0x03;
  e_local[3] = 0x04;
  e_central[2] = 0x01;
  e_central[3] = 0x02;
  e_fin[2] = 0x05;
  e_fin[3] = 0x06;
  e_ext[2] = 0x07;
  e_ext[3] = 0x08;
}

lista_los_archivos()
{
  int comentario;

  archivo_zip = fopen(nombre, "r");
  if(archivo_zip == 0) {
    color(15);
    puts("\r\nEl archivo ZIP no existe.\r\n");
    exit(1);
  }
  busca_final();
  lee_encabezado_final();
  color(15);
  comentario = lee_entero_corto(eb_fin + 16);
  while(comentario--)
    putchar(fgetc(archivo_zip));
  fseek(archivo_zip, lee_entero(eb_fin + 12), 0);
  lista_archivos(lee_entero_corto(eb_fin + 6));
  fclose(archivo_zip);
}

descompresion_de_archivos()
{
  int comentario;

  archivo_zip = fopen(nombre, "r");
  if(archivo_zip == 0) {
    color(15);
    puts("\r\nEl archivo ZIP no existe.\r\n");
    exit(1);
  }
  busca_final();
  lee_encabezado_final();
  color(15);
  comentario = lee_entero_corto(eb_fin + 16);
  while(comentario--)
    putchar(fgetc(archivo_zip));
  color(11);
  fseek(archivo_zip, lee_entero(eb_fin + 12), 0);
  descomprime_archivos(lee_entero_corto(eb_fin + 6));
  fclose(archivo_zip);
}

lee_encabezado_final()
{
  fread(archivo_zip, eb_fin, 18);
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
  puts("Longitud Metodo      Tamaño Total   Fecha   Tiempo CRC-32   Nombre\r\n");
  puts("-------- ------      ------ -----   -----   ------ ------   ------\r\n");
  color(11);
  while(cuantos--) {
    fread(archivo_zip, eb_actual, 4);
    if(compara(eb_actual, e_central)) {
      descripcion(&ac_tam1, &ac_tam2, &ac_total);
    } else {
      puts("Error en el archivo ZIP.\r\n");
      break;
    }
  }
  color(14);
  puts("--------             ------ -----                           ------\r\n");
  decimal(ac_tam2, 8);
  puts("            ");
  decimal(ac_tam1, 7);
  puts("  ");
  relacion = (ac_tam1 > 20000000) ?
             (ac_tam1 * 100 / ac_tam2) :
             (ac_tam1 / (ac_tam2 / 100));
  decimal(relacion, 3);
  puts("%                           ");
  decimal(ac_total, 6);
  puts("\r\n");
}

descomprime_archivos(cuantos)
  int cuantos;
{
  puts("\r\n");
  while(cuantos--) {
    fread(archivo_zip, eb_actual, 4);
    if(compara(eb_actual, e_central)) {
      descompresion();
    } else {
      puts("Error en el archivo ZIP.\r\n");
      break;
    }
  }
}

busca_final()
{
  char *apunta, *origen;
  int donde, bytes, lo_tengo;

  fseek(archivo_zip, 0, 2);
  tam_zip = ftell(archivo_zip);
  fseek(archivo_zip, 0, 0);
  if(tam_zip <= TAM_BUF) {
    donde = 0;
    apunta = buf1 + fread(archivo_zip, buf1, TAM_BUF);
    while(1) {
      while(apunta != buf1 && *--apunta != 'P') ;
      if(compara(apunta, e_fin)) {
        apunta += 4;
        break;
      }
      if(apunta == buf1) {
        color(15);
        puts("\r\nEl archivo ZIP esta corrupto o está incompleto.\r\n");
        fclose(archivo_zip);
        exit(1);
      }
    }
  } else {
    donde = tam_zip;
    while(1) {
      if(donde - TAM_BUF < 0)
        donde = 0;
      else
        donde -= TAM_BUF;
      fseek(archivo_zip, donde, 0);
      bytes = fread(archivo_zip, buf1, TAM_BUF);
      if(bytes != TAM_BUF) {
        color(15);
        puts("\r\n¡Peligro!. Error en lectura.\r\n");
      }
      lo_tengo = 0;
      apunta = buf1 + TAM_BUF;
      while(apunta != buf1) {
        while(apunta != buf1 && *--apunta != 'P') ;
        origen = apunta;
        if(*apunta == 'P') {
          if(apunta == buf1 + (TAM_BUF - 1))
            *apunta = fgetc(archivo_zip);
          else
            ++apunta;
          if(*apunta == 'K') {
            if(apunta == buf1 + (TAM_BUF - 1))
              *apunta = fgetc(archivo_zip);
            else
              ++apunta;
            if(*apunta == 0x05) {
              if(apunta == buf1 + (TAM_BUF - 1))
                *apunta = fgetc(archivo_zip);
              else
                ++apunta;
              if(*apunta == 0x06) {
                if(apunta == buf1 + (TAM_BUF - 1))
                  *apunta = fgetc(archivo_zip);
                else
                  ++apunta;
                lo_tengo = 1;
                apunta = origen + 4;
                break;
              }
            }
          }
        }
        apunta = origen;
      }
      if(lo_tengo)
        break;
      if(donde == 0) {
        color(15);
        puts("\r\nEl archivo ZIP esta corrupto o está incompleto.\r\n");
        fclose(archivo_zip);
        exit(1);
      }
    }
  }
  fseek(archivo_zip, donde + (apunta - buf1), 0);
}

descripcion(ac_tam1, ac_tam2, ac_total)
  int *ac_tam1, *ac_tam2, *ac_total;
{
  int temp, an, mes, dia, hora, minuto, relacion;
  char *apunta;
  int tam1, tam2;

  fread(archivo_zip, buf1, 42);
  tam1 = lee_entero(buf1 + 16);
  tam2 = lee_entero(buf1 + 20);
  *ac_tam1 += tam1;
  *ac_tam2 += tam2;
  (*ac_total)++;
  relacion = (tam1 > 20000000) ?
             (tam1 * 100 / tam2) :
             (tam1 / (tam2 / 100));
  temp = lee_entero(buf1 + 20);
  decimal(temp, 8);
  puts(" ");
  switch(buf1[6]) {
    case 0: puts("Metodo 0   ");
            break;
    case 1: puts("Metodo 1   ");
            break;
    case 2:
    case 3:
    case 4:
    case 5: puts("Metodo 2   ");
            break;
    case 6: puts("Metodo 6   ");
            break;
    case 8: puts("Metodo 8   ");
            break;
    case 7: puts("?????");
            break;
  }
  temp = lee_entero(buf1 + 16);
  decimal(temp, 7);
  puts("  ");
  decimal(relacion, 3);
  puts("% ");
  an = (((lee_entero_corto(buf1 + 10) >> 9) & 0x7f) + 80) % 100;
  mes = (lee_entero_corto(buf1 + 10) >> 5) & 0x0f;
  dia = lee_entero_corto(buf1 + 10) & 0x1f;
  hora = (lee_entero_corto(buf1 + 8) >> 11) & 0x1f;
  minuto = (lee_entero_corto(buf1 + 8) >> 5) & 0x3f;
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
  hex(buf1[15]);
  hex(buf1[14]);
  hex(buf1[13]);
  hex(buf1[12]);
  putchar(' ');
  temp = buf1[24] | (buf1[25] << 8);
  while(temp--) {
    putchar(fgetc(archivo_zip));
  }
  temp = buf1[26] | (buf1[27] << 8);
  while(temp--) {
    fgetc(archivo_zip);
  }
  temp = buf1[28] | (buf1[29] << 8);
  while(temp--) {
    fgetc(archivo_zip);
  }
  if(checa_tecla()) {
    while(checa_tecla() == 0) ;
  }
  puts("\r\n");
}

descompresion()
{
  int donde, datos, temp;
  int longitud_original;
  int longitud_comprimida;
  int fecha;
  int tiempo;
  int crc32;
  char *apunta;

  fread(archivo_zip, eb_central, 42);
  tam_sin_comprimir = longitud_original = lee_entero(eb_central + 20);
  color(11);
  switch(eb_central[6]) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 8: puts("Extrayendo: ");
            break;
    default:puts(" Evadiendo: ");
            break;
  }
  color(14);
  tam_comprimido = longitud_comprimida = lee_entero(eb_central + 16);
  fecha = lee_entero_corto(eb_central + 10);
  tiempo = lee_entero_corto(eb_central + 8);
  crc32 = lee_entero(eb_central + 12);
  temp = eb_central[24] | (eb_central[25] << 8);
  apunta = nombre;
  while(temp--)
    putchar(*apunta++ = fgetc(archivo_zip));
  *apunta = 0;
  temp = eb_central[26] | (eb_central[27] << 8);
  while(temp--)
    fgetc(archivo_zip);
  temp = eb_central[28] | (eb_central[29] << 8);
  while(temp--)
    fgetc(archivo_zip);
  donde = ftell(archivo_zip);
  datos = lee_entero(eb_central + 38);
  fseek(archivo_zip, datos, 0);
  fread(archivo_zip, eb_actual, 4);
  if(compara(eb_actual, e_local)) {
    fread(archivo_zip, eb_local, 26);
    temp = eb_local[22] | (eb_local[23] << 8);
    while(temp--) {
      fgetc(archivo_zip);
    }
    temp = eb_local[24] | (eb_local[25] << 8);
    while(temp--) {
      fgetc(archivo_zip);
    }
    inicia_calculo_crc();
    strcpy(archivo, dir);
    strcat(archivo, "/");
    strcat(archivo, nombre);
    if(archivo[strlen(archivo) - 1] == '/') {
      archivo[strlen(archivo) - 1] = 0;
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
      switch(eb_central[6]) {
        case 0: extraccion();
                break;
        case 1: unShrink();
                break;
        case 2:
        case 3:
        case 4:
        case 5: unReduce();
                break;
        case 6: explode();
                break;
        case 8: inflate();
                break;
        default:
                color(15);
                puts("\r\n            Metodo de descompresión desconocido.");
                crc32_actual = ~lee_entero(eb_central + 12);
                break;
      }
      fclose(archivo_nuevo);
      atributo(archivo, 0, corrige_hora());
      if(~crc32_actual != lee_entero(eb_central + 12)) {
        color(15);
        puts("\r\n            CRC-32 incorrecto en este archivo.");
      }
    }
  } else {
    color(15);
    puts("\r\nEl archivo ZIP esta corrupto.\r\n");
    fclose(archivo_zip);
    exit(1);
  }
  if(checa_tecla()) {
    while(checa_tecla() == 0) ;
  }
  puts("\r\n");
  fseek(archivo_zip, donde, 0);
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

compara(x, y)
  char *x, *y;
{
  int bytes;

  for(bytes = 0; bytes < 4; ++bytes) {
    if(*x++ != *y++)
      return 0;
  }
  return 1;
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

llena_tabla_crc() {
  crc[0] = 0x00000000;
  crc[1] = 0x77073096;
  crc[2] = 0xee0e612c;
  crc[3] = 0x990951ba;
  crc[4] = 0x076dc419;
  crc[5] = 0x706af48f;
  crc[6] = 0xe963a535;
  crc[7] = 0x9e6495a3;
  crc[8] = 0x0edb8832;
  crc[9] = 0x79dcb8a4;
  crc[10] = 0xe0d5e91e;
  crc[11] = 0x97d2d988;
  crc[12] = 0x09b64c2b;
  crc[13] = 0x7eb17cbd;
  crc[14] = 0xe7b82d07;
  crc[15] = 0x90bf1d91;
  crc[16] = 0x1db71064;
  crc[17] = 0x6ab020f2;
  crc[18] = 0xf3b97148;
  crc[19] = 0x84be41de;
  crc[20] = 0x1adad47d;
  crc[21] = 0x6ddde4eb;
  crc[22] = 0xf4d4b551;
  crc[23] = 0x83d385c7;
  crc[24] = 0x136c9856;
  crc[25] = 0x646ba8c0;
  crc[26] = 0xfd62f97a;
  crc[27] = 0x8a65c9ec;
  crc[28] = 0x14015c4f;
  crc[29] = 0x63066cd9;
  crc[30] = 0xfa0f3d63;
  crc[31] = 0x8d080df5;
  crc[32] = 0x3b6e20c8;
  crc[33] = 0x4c69105e;
  crc[34] = 0xd56041e4;
  crc[35] = 0xa2677172;
  crc[36] = 0x3c03e4d1;
  crc[37] = 0x4b04d447;
  crc[38] = 0xd20d85fd;
  crc[39] = 0xa50ab56b;
  crc[40] = 0x35b5a8fa;
  crc[41] = 0x42b2986c;
  crc[42] = 0xdbbbc9d6;
  crc[43] = 0xacbcf940;
  crc[44] = 0x32d86ce3;
  crc[45] = 0x45df5c75;
  crc[46] = 0xdcd60dcf;
  crc[47] = 0xabd13d59;
  crc[48] = 0x26d930ac;
  crc[49] = 0x51de003a;
  crc[50] = 0xc8d75180;
  crc[51] = 0xbfd06116;
  crc[52] = 0x21b4f4b5;
  crc[53] = 0x56b3c423;
  crc[54] = 0xcfba9599;
  crc[55] = 0xb8bda50f;
  crc[56] = 0x2802b89e;
  crc[57] = 0x5f058808;
  crc[58] = 0xc60cd9b2;
  crc[59] = 0xb10be924;
  crc[60] = 0x2f6f7c87;
  crc[61] = 0x58684c11;
  crc[62] = 0xc1611dab;
  crc[63] = 0xb6662d3d;
  crc[64] = 0x76dc4190;
  crc[65] = 0x01db7106;
  crc[66] = 0x98d220bc;
  crc[67] = 0xefd5102a;
  crc[68] = 0x71b18589;
  crc[69] = 0x06b6b51f;
  crc[70] = 0x9fbfe4a5;
  crc[71] = 0xe8b8d433;
  crc[72] = 0x7807c9a2;
  crc[73] = 0x0f00f934;
  crc[74] = 0x9609a88e;
  crc[75] = 0xe10e9818;
  crc[76] = 0x7f6a0dbb;
  crc[77] = 0x086d3d2d;
  crc[78] = 0x91646c97;
  crc[79] = 0xe6635c01;
  crc[80] = 0x6b6b51f4;
  crc[81] = 0x1c6c6162;
  crc[82] = 0x856530d8;
  crc[83] = 0xf262004e;
  crc[84] = 0x6c0695ed;
  crc[85] = 0x1b01a57b;
  crc[86] = 0x8208f4c1;
  crc[87] = 0xf50fc457;
  crc[88] = 0x65b0d9c6;
  crc[89] = 0x12b7e950;
  crc[90] = 0x8bbeb8ea;
  crc[91] = 0xfcb9887c;
  crc[92] = 0x62dd1ddf;
  crc[93] = 0x15da2d49;
  crc[94] = 0x8cd37cf3;
  crc[95] = 0xfbd44c65;
  crc[96] = 0x4db26158;
  crc[97] = 0x3ab551ce;
  crc[98] = 0xa3bc0074;
  crc[99] = 0xd4bb30e2;
  crc[100] = 0x4adfa541;
  crc[101] = 0x3dd895d7;
  crc[102] = 0xa4d1c46d;
  crc[103] = 0xd3d6f4fb;
  crc[104] = 0x4369e96a;
  crc[105] = 0x346ed9fc;
  crc[106] = 0xad678846;
  crc[107] = 0xda60b8d0;
  crc[108] = 0x44042d73;
  crc[109] = 0x33031de5;
  crc[110] = 0xaa0a4c5f;
  crc[111] = 0xdd0d7cc9;
  crc[112] = 0x5005713c;
  crc[113] = 0x270241aa;
  crc[114] = 0xbe0b1010;
  crc[115] = 0xc90c2086;
  crc[116] = 0x5768b525;
  crc[117] = 0x206f85b3;
  crc[118] = 0xb966d409;
  crc[119] = 0xce61e49f;
  crc[120] = 0x5edef90e;
  crc[121] = 0x29d9c998;
  crc[122] = 0xb0d09822;
  crc[123] = 0xc7d7a8b4;
  crc[124] = 0x59b33d17;
  crc[125] = 0x2eb40d81;
  crc[126] = 0xb7bd5c3b;
  crc[127] = 0xc0ba6cad;
  crc[128] = 0xedb88320;
  crc[129] = 0x9abfb3b6;
  crc[130] = 0x03b6e20c;
  crc[131] = 0x74b1d29a;
  crc[132] = 0xead54739;
  crc[133] = 0x9dd277af;
  crc[134] = 0x04db2615;
  crc[135] = 0x73dc1683;
  crc[136] = 0xe3630b12;
  crc[137] = 0x94643b84;
  crc[138] = 0x0d6d6a3e;
  crc[139] = 0x7a6a5aa8;
  crc[140] = 0xe40ecf0b;
  crc[141] = 0x9309ff9d;
  crc[142] = 0x0a00ae27;
  crc[143] = 0x7d079eb1;
  crc[144] = 0xf00f9344;
  crc[145] = 0x8708a3d2;
  crc[146] = 0x1e01f268;
  crc[147] = 0x6906c2fe;
  crc[148] = 0xf762575d;
  crc[149] = 0x806567cb;
  crc[150] = 0x196c3671;
  crc[151] = 0x6e6b06e7;
  crc[152] = 0xfed41b76;
  crc[153] = 0x89d32be0;
  crc[154] = 0x10da7a5a;
  crc[155] = 0x67dd4acc;
  crc[156] = 0xf9b9df6f;
  crc[157] = 0x8ebeeff9;
  crc[158] = 0x17b7be43;
  crc[159] = 0x60b08ed5;
  crc[160] = 0xd6d6a3e8;
  crc[161] = 0xa1d1937e;
  crc[162] = 0x38d8c2c4;
  crc[163] = 0x4fdff252;
  crc[164] = 0xd1bb67f1;
  crc[165] = 0xa6bc5767;
  crc[166] = 0x3fb506dd;
  crc[167] = 0x48b2364b;
  crc[168] = 0xd80d2bda;
  crc[169] = 0xaf0a1b4c;
  crc[170] = 0x36034af6;
  crc[171] = 0x41047a60;
  crc[172] = 0xdf60efc3;
  crc[173] = 0xa867df55;
  crc[174] = 0x316e8eef;
  crc[175] = 0x4669be79;
  crc[176] = 0xcb61b38c;
  crc[177] = 0xbc66831a;
  crc[178] = 0x256fd2a0;
  crc[179] = 0x5268e236;
  crc[180] = 0xcc0c7795;
  crc[181] = 0xbb0b4703;
  crc[182] = 0x220216b9;
  crc[183] = 0x5505262f;
  crc[184] = 0xc5ba3bbe;
  crc[185] = 0xb2bd0b28;
  crc[186] = 0x2bb45a92;
  crc[187] = 0x5cb36a04;
  crc[188] = 0xc2d7ffa7;
  crc[189] = 0xb5d0cf31;
  crc[190] = 0x2cd99e8b;
  crc[191] = 0x5bdeae1d;
  crc[192] = 0x9b64c2b0;
  crc[193] = 0xec63f226;
  crc[194] = 0x756aa39c;
  crc[195] = 0x026d930a;
  crc[196] = 0x9c0906a9;
  crc[197] = 0xeb0e363f;
  crc[198] = 0x72076785;
  crc[199] = 0x05005713;
  crc[200] = 0x95bf4a82;
  crc[201] = 0xe2b87a14;
  crc[202] = 0x7bb12bae;
  crc[203] = 0x0cb61b38;
  crc[204] = 0x92d28e9b;
  crc[205] = 0xe5d5be0d;
  crc[206] = 0x7cdcefb7;
  crc[207] = 0x0bdbdf21;
  crc[208] = 0x86d3d2d4;
  crc[209] = 0xf1d4e242;
  crc[210] = 0x68ddb3f8;
  crc[211] = 0x1fda836e;
  crc[212] = 0x81be16cd;
  crc[213] = 0xf6b9265b;
  crc[214] = 0x6fb077e1;
  crc[215] = 0x18b74777;
  crc[216] = 0x88085ae6;
  crc[217] = 0xff0f6a70;
  crc[218] = 0x66063bca;
  crc[219] = 0x11010b5c;
  crc[220] = 0x8f659eff;
  crc[221] = 0xf862ae69;
  crc[222] = 0x616bffd3;
  crc[223] = 0x166ccf45;
  crc[224] = 0xa00ae278;
  crc[225] = 0xd70dd2ee;
  crc[226] = 0x4e048354;
  crc[227] = 0x3903b3c2;
  crc[228] = 0xa7672661;
  crc[229] = 0xd06016f7;
  crc[230] = 0x4969474d;
  crc[231] = 0x3e6e77db;
  crc[232] = 0xaed16a4a;
  crc[233] = 0xd9d65adc;
  crc[234] = 0x40df0b66;
  crc[235] = 0x37d83bf0;
  crc[236] = 0xa9bcae53;
  crc[237] = 0xdebb9ec5;
  crc[238] = 0x47b2cf7f;
  crc[239] = 0x30b5ffe9;
  crc[240] = 0xbdbdf21c;
  crc[241] = 0xcabac28a;
  crc[242] = 0x53b39330;
  crc[243] = 0x24b4a3a6;
  crc[244] = 0xbad03605;
  crc[245] = 0xcdd70693;
  crc[246] = 0x54de5729;
  crc[247] = 0x23d967bf;
  crc[248] = 0xb3667a2e;
  crc[249] = 0xc4614ab8;
  crc[250] = 0x5d681b02;
  crc[251] = 0x2a6f2b94;
  crc[252] = 0xb40bbe37;
  crc[253] = 0xc30c8ea1;
  crc[254] = 0x5a05df1b;
  crc[255] = 0x2d02ef8d;
}

inicia_calculo_crc()
{
  crc32_actual = 0xffffffff;
}

calcula_crc(buffer, tam)
  char *buffer;
  int tam;
{
  int crc32;

  crc32 = crc32_actual;
  while(tam--) {
    crc32 = crc[(crc32 ^ *buffer++) & 0xff] ^ (crc32 >> 8);
  }
  crc32_actual = crc32;
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

  p = lee_entero(eb_central + 8);
  p += 0x14000000;
  return p;
}

extraccion()
{
  int tam_global, bytes;

  tam_global = tam_sin_comprimir;
  while(tam_global) {
    if(tam_global > 32768) {
      bytes = fread(archivo_zip, ventana, 32768);
      escribe_y_calcula_crc(archivo_nuevo, ventana, bytes);
      tam_global -= bytes;
    } else {
      bytes = fread(archivo_zip, ventana, tam_global);
      escribe_y_calcula_crc(archivo_nuevo, ventana, bytes);
      tam_global -= bytes;
    }
  }
}

escribe_y_calcula_crc(archivo, buffer, bytes)
  int archivo, bytes;
  char *buffer;
{
  calcula_crc(buffer, bytes);
  return fwrite(archivo, buffer, bytes);
}

typedef unsigned long ulg;
typedef unsigned short ush;
typedef char uch;
typedef char byte;
typedef unsigned long ULONG;
typedef unsigned short UWORD;
typedef unsigned long longint;

#define OF(a) ()
#define __(a) ()

int zipeof;
int mask_bits[17];

#include "c:c/inflate.c"
#include "c:c/explode.c"
#include "c:c/unreduce.c"
#include "c:c/unshrink.c"

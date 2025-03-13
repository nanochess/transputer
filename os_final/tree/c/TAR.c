/*
** Manejador de archivos TAR.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 23 de noviembre de 1995.
** Revisión: 23 de diciembre de 1995. Corrección de un defecto en la ilustración
**                                    de los atributos.
** Revisión: 27 de diciembre de 1995. Ahora puede extraer los archivos TAR que
**                                    no indican los directorios explicitamente.
** Revisión: 1o. de enero de 1996. Mejoras mínimas.
** Revisión: 1o. de febrero de 1996. Modificado para soportar el siglo 21.
** Revisión: 6 de febrero de 1996. Soporte para nombres de 32 letras.
** Revisión: 10 de agosto de 1996. Simplificación del trabajo.
*/

#define NULL        0

char orden[80];
char nombre1[80];
char dir1[80];
char dir2[80];
int tam_global;
int archivo_tar;
int archivo_copia;
int nivel;
int dirs[64];
int dir;
char *apunta;
char buffer[512];
char enc[512];
char buf[32768];

main()
{
  color(15);
  puts("\r\nManejador de archivos TAR  (c) Copyright 1995 Oscar Toledo G.\r\n");
  color(11);
  puts("\r\nPara crear un archivo TAR:\r\n");
  color(14);
  puts("C archivo.tar C:/dir\r\n");
  color(11);
  puts("\r\nPara extraer un archivo TAR:\r\n");
  color(14);
  puts("X archivo.tar C:/dir\r\n");
  color(11);
  puts("\r\nPara listar un archivo TAR:\r\n");
  color(14);
  puts("T archivo.tar\r\n");
  color(10);
  puts("\r\nIntroduzca la orden > ");
  lee_linea(orden, 80);
  apunta = orden;
  while(*apunta == ' ')
    ++apunta;
  if(toupper(*apunta) == 'C')
    crea_archivo();
  else if(toupper(*apunta) == 'X')
    extrae_archivo();
  else if(toupper(*apunta) == 'T')
    lista_archivo();
  else {
    color(15);
    puts("\r\nPrograma cancelado.\r\n");
    exit(1);
  }
}

crea_archivo()
{
  char *apunta2;

  while(*++apunta == ' ') ;
  apunta2 = nombre1;
  while(*apunta != ' ' && *apunta)
    *apunta2++ = *apunta++;
  *apunta2 = 0;
  while(*apunta == ' ')
    ++apunta;
  apunta2 = dir1;
  while(*apunta)
    *apunta2++ = *apunta++;
  if(apunta2 != dir1 && *(apunta2-1) == '/')
    --apunta2;
  *apunta2 = 0;
  nivel = 0;
  dir = abre_dir(dir1);
  if(dir == 0) {
    color(15);
    puts("\r\nNo se pudo abrir el directorio.\r\n");
    exit(1);
  }
  archivo_tar = fopen(nombre1, "w");
  if(archivo_tar == 0) {
    fclose(dir);
    color(15);
    puts("\r\nNo se pudo crear el archivo TAR.\r\n");
    exit(1);
  }
  vacia_directorio(dir);
  agrega_fin_de_archivo();
  fclose(archivo_tar);
  fclose(dir);
}

vacia_directorio(dir)
  int dir;
{
  char nombre[64];
  char bnombre[256];
  char *apunta;
  int bytes, temp;

  while (1) {
    fread(dir, nombre, 64);
    if(checa_tecla()) {
      color(15);
      puts("\r\nInterrupción.\r\n");
      return;
    }
    if(nombre[0] == 0) return;
    if(nombre[0] == 0x80) continue;
    if(nombre[48] & 8) {
      dirs[nivel++] = dir;
      strcat(dir1, "/");
      strcat(dir1, nombre);
      strcat(dir2, nombre);
      strcat(dir2, "/");
      agrega_encabezado(dir2, "", nombre);
      descripcion(nombre, dir2);
      dir = abre_dir(dir1);
      if(dir == 0) {
        color(15);
        puts("\r\nNo se pudo abrir el directorio ");
        puts(dir1);
        puts("\r\n");
      } else {
        vacia_directorio(dir);
        fclose(dir);
      }
      apunta = dir1 + strlen(dir1);
      while(*--apunta != '/') ;
      *apunta = 0;
      apunta = dir2 + strlen(dir2) - 1;
      while(1) {
        if(apunta == dir2)
          break;
        if(*--apunta == '/') {
          ++apunta;
          break;
        }
      }
      *apunta = 0;
      dir = dirs[--nivel];
    } else {
      if(nombre[60] || nombre[61] || nombre[62] || nombre[63]) {
        agrega_encabezado(dir2, nombre, nombre);
        strcpy(bnombre, dir2);
        strcat(bnombre, nombre);
        descripcion(nombre, bnombre);
        strcpy(bnombre, dir1);
        strcat(bnombre, "/");
        strcat(bnombre, nombre);
        archivo_copia = fopen(bnombre, "r");
        if(archivo_copia == 0) {
          color(15);
          puts("\r\nNo se pudo abrir el archivo ");
          puts(dir1);
          puts("/");
          puts(nombre);
          puts("\r\n");
        } else {
          while(1) {
            bytes = fread(archivo_copia, buf, 32768);
            if(bytes == 0) break;
            if(bytes < 32768)
              bytes = ((bytes + 511) / 512) * 512;
            fwrite(archivo_tar, buf, bytes);
          }
          fclose(archivo_copia);
        }
      }
    }
  }
}

agrega_fin_de_archivo()
{
  memset(buffer, 0, 512);
  fwrite(archivo_tar, buffer, 512);
}

agrega_encabezado(nombre1, nombre2, estado)
  char *nombre1, *nombre2;
  char estado[64];
{
  char *apunta;
  int tam;
  int tiempo, checksum;

  memset(buffer, 0, 512);
  strcpy(buffer, nombre1);
  strcat(buffer, nombre2);
  if(estado[48] & 8)
    strcpy(buffer + 100, " 40666 ");
  else
    strcpy(buffer + 100, "100666 ");
  strcpy(buffer + 108, "     1 ");
  strcpy(buffer + 116, "     1 ");
  strcpy(buffer + 148, "        ");
  tam = obtiene_tam(estado + 60);
  llena_octal(buffer + 124, tam, 12);
  tiempo = convierte_tiempo(estado + 52);
  llena_octal(buffer + 136, tiempo, 12);
  buffer[156] = '0';
  checksum = calcula_checksum() & 65535;
  llena_octal(buffer + 148, checksum, 7);
  fwrite(archivo_tar, buffer, 512);
}

obtiene_tam(pos)
  char *pos;
{
  return (*pos) | (*(pos+1) << 8) | (*(pos+2) << 16) | (*(pos+3) << 24);
}

convierte_tiempo(tiempo)
  char tiempo[4];
{
  int segundos, minutos, hora, dia, mes, a;
  int t, c;
  int datos[12];

  datos[0] = 31; datos[1] = 28; datos[2] = 31; datos[3] = 30;
  datos[4] = 31; datos[5] = 30; datos[6] = 31; datos[7] = 31;
  datos[8] = 30; datos[9] = 31; datos[10] = 30; datos[11] = 31;

  segundos = (tiempo[0] & 0x1f) << 1;
  minutos = ((tiempo[0] | (tiempo[1] << 8)) & 0x07e0) >> 5;
  hora = (tiempo[1] & 0xf8) >> 3;
  dia = tiempo[2] & 0x1f;
  mes = ((tiempo[2] | (tiempo[3] << 8)) & 0x01e0) >> 5;
  a = ((tiempo[3] & 0xfe) >> 1);
  t = segundos + minutos * 60 + hora * 3600 + (dia - 1) * 86400;
  if(((a + 2) % 4) == 0)
    datos[1]++;
  c = 0;
  while(--mes) {
    t += datos[c++] * 86400;
  }
  c = 0;
  while(c < a) {
    if(((c + 2) % 4) == 0)
      t += 86400;
    t += 31536000;
    ++c;
  }
  return t;
}

llena_octal(destino, valor, tam)
  char *destino;
  int valor, tam;
{
  destino += tam;
  *--destino = ' ';
  --tam;
  while(1) {
    *--destino = (valor & 7) + '0';
    --tam;
    valor >>= 3;
    if(valor == 0) break;
    if(tam == 0) break;
  }
  while(tam) {
    *--destino = ' ';
    --tam;
  }
}

calcula_checksum()
{
  int valor;
  int cuantos;
  char *apunta;

  apunta = buffer;
  valor = 0;
  cuantos = 0;
  while(cuantos++ < 512) {
    valor += *apunta++;
  }
  return valor;
}

extrae_archivo()
{
  char bnombre[256];
  char *apunta2;
  int error, bytes;
  int nuevo_archivo;
  int tam;

  while(*++apunta == ' ') ;
  apunta2 = nombre1;
  while(*apunta != ' ' && *apunta)
    *apunta2++ = *apunta++;
  *apunta2 = 0;
  while(*apunta == ' ')
    ++apunta;
  apunta2 = dir1;
  while(*apunta)
    *apunta2++ = *apunta++;
  if(apunta2 != dir1 && *(apunta2-1) == '/')
    --apunta2;
  *apunta2 = 0;
  archivo_tar = fopen(nombre1, "r");
  if(archivo_tar == 0) {
    color(15);
    puts("\r\nNo se pudo leer el archivo TAR.\r\n");
    exit(1);
  }
  while(1) {
    if(checa_tecla()) {
      color(15);
      puts("\r\nInterrupción.\r\n");
      break;
    }
    error = lee_encabezado();
    if(error == 1) {
      color(15);
      puts("\r\nEncabezado erróneo.\r\n");
      break;
    } else if(error == 2)
      break;
    strcpy(bnombre, dir1);
    strcat(bnombre, "/");
    strcat(bnombre, buffer);
    if (*(bnombre + strlen(bnombre) - 1) == '/') {
      *(bnombre + strlen(bnombre) - 1) = 0;
      crea_dir(bnombre);
      atributo(bnombre, 8, corrige_hora());
    } else {
      nuevo_archivo = fopen(bnombre, "w");
      if(nuevo_archivo == 0) {
        mapea_directorios(bnombre);
        nuevo_archivo = fopen(bnombre, "w");
        if(nuevo_archivo == 0) {
          color(15);
          puts("\r\nNo se pudo crear el archivo.\r\n");
          break;
        }
      }
      tam = tam_global;
      tam_global = ((tam_global + 511) / 512) * 512;
      while(tam_global) {
        if(tam_global > 32768) {
          bytes = fread(archivo_tar, buf, 32768);
          fwrite(nuevo_archivo, buf, bytes);
          tam_global -= bytes;
          tam -= bytes;
        } else {
          bytes = fread(archivo_tar, buf, tam_global);
          if(bytes >= tam)
            fwrite(nuevo_archivo, buf, tam);
          else
            fwrite(nuevo_archivo, buf, bytes);
          tam_global -= bytes;
          tam -= bytes;
        }
      }
      fclose(nuevo_archivo);
      atributo(bnombre, 0, corrige_hora());
    }
  }
  fclose(archivo_tar);
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
  int dia, mes, an, hora, minutos, segundos, tiempo;
  int hora_bit;

  tiempo = extrae_octal(buffer + 136);
  recupera_tiempo(tiempo, &dia, &mes, &an, &hora, &minutos, &segundos);
  hora_bit = segundos / 2;
  hora_bit |= minutos << 5;
  hora_bit |= hora << 11;
  hora_bit |= dia << 16;
  hora_bit |= mes << 21;
  hora_bit |= an << 25;
  return hora_bit;
}

lista_archivo()
{
  char *apunta2;
  int error, bytes;

  while(*++apunta == ' ') ;
  apunta2 = nombre1;
  while(*apunta)
    *apunta2++ = *apunta++;
  *apunta2 = 0;
  archivo_tar = fopen(nombre1, "r");
  if(archivo_tar == 0) {
    color(15);
    puts("\r\nNo se pudo leer el archivo TAR.\r\n");
    exit(1);
  }
  while(1) {
    if(checa_tecla()) {
      color(15);
      puts("\r\nInterrupción.\r\n");
      break;
    }
    error = lee_encabezado();
    if(error == 1) {
      color(15);
      puts("\r\nEncabezado erróneo.\r\n");
      break;
    } else if(error == 2)
      break;
    tam_global = ((tam_global + 511) / 512) * 512;
    while(tam_global) {
      if(tam_global > 32768) {
        bytes = fread(archivo_tar, buf, 32768);
        tam_global -= bytes;
      } else {
        bytes = fread(archivo_tar, buf, tam_global);
        tam_global -= bytes;
      }
    }
  }
  fclose(archivo_tar);
}

lee_encabezado()
{
  int byte, checksum, valor, tam, tiempo, temp;
  int dia, mes, an, hora, minutos, segundos;

  fread(archivo_tar, buffer, 512);
  checksum = 0;
  for(byte = 0; byte < 512; ++byte)
    checksum += buffer[byte];
  if(checksum == 0)
    return 2;
  checksum = 0;
  for(byte = 0; byte < 512; ++byte) {
    if(byte >= 148 && byte < 156)
      checksum += ' ';
    else
      checksum += buffer[byte];
  }
  valor = extrae_octal(buffer + 148);
  if(valor != checksum)
    return 1;
  tam = extrae_octal(buffer + 124);
  tam_global = tam;
  tiempo = extrae_octal(buffer + 136);
  decimal(tam, 0);
  putchar(' ');
  recupera_tiempo(tiempo, &dia, &mes, &an, &hora, &minutos, &segundos);
  if(dia < 10)
    putchar(' ');
  else
    putchar((dia / 10) + '0');
  putchar((dia % 10) + '0');
  putchar('-');
  switch(mes) {
    case 1:  puts("Ene");
             break;
    case 2:  puts("Feb");
             break;
    case 3:  puts("Mar");
             break;
    case 4:  puts("Abr");
             break;
    case 5:  puts("May");
             break;
    case 6:  puts("Jun");
             break;
    case 7:  puts("Jul");
             break;
    case 8:  puts("Ago");
             break;
    case 9:  puts("Sep");
             break;
    case 10: puts("Oct");
             break;
    case 11: puts("Nov");
             break;
    case 12: puts("Dic");
             break;
  }
  puts("-");
  temp = an + 1970;
  putchar((temp / 1000) % 10 + '0');
  putchar((temp /  100) % 10 + '0');
  putchar((temp /   10) % 10 + '0');
  putchar( temp         % 10 + '0');
  puts(" ");
  temp = hora;
  if(temp < 10)
    putchar(' ');
  else
    putchar((temp / 10) + '0');
  putchar((temp % 10) + '0');
  putchar(':');
  temp = minutos;
  putchar((temp / 10) + '0');
  putchar((temp % 10) + '0');
  putchar(':');
  temp = segundos;
  putchar((temp / 10) + '0');
  putchar((temp % 10) + '0');
  puts(" ");
  puts(buffer);
  puts("\r\n");
  return 0;
}

recupera_tiempo(tiempo, dia, mes, an, hora, minutos, segundos)
  int tiempo, *dia, *mes, *an, *hora, *minutos, *segundos;
{
  int s_p_min, s_p_hora, s_p_dia, s_p_an;
  int datos[12];

  datos[0] = 31; datos[1] = 28; datos[2] = 31; datos[3] = 30;
  datos[4] = 31; datos[5] = 30; datos[6] = 31; datos[7] = 31;
  datos[8] = 30; datos[9] = 31; datos[10] = 30; datos[11] = 31;

  s_p_min = 60;
  s_p_hora = 3600;
  s_p_dia = 86400;
  s_p_an = 31536000;
  *an = 0;
  *mes = 0;
  *dia = 1;
  *hora = 0;
  *minutos = 0;
  *segundos = 0;
  while (tiempo >= s_p_an + (((*an + 2) % 4 == 0) ? s_p_dia : 0)) {
    if (((*an + 2) % 4) == 0)
      tiempo -= s_p_dia;
    tiempo -= s_p_an;
    (*an)++;
  }
  if (((*an + 2) % 4) == 0)
    datos[1]++;
  while (tiempo >= (datos[*mes] * s_p_dia))
    tiempo -= datos[(*mes)++] * s_p_dia;
  (*mes)++;
  while (tiempo >= s_p_dia) {
    tiempo -= s_p_dia;
    (*dia)++;
  }
  while (tiempo >= s_p_hora) {
    tiempo -= s_p_hora;
    (*hora)++;
  }
  while (tiempo >= s_p_min) {
    tiempo -= s_p_min;
    (*minutos)++;
  }
  *segundos = tiempo;
}

extrae_octal(donde)
  char *donde;
{
  int valor;

  while(*donde == ' ') ++donde;
  valor = 0;
  while(*donde >= '0' && *donde <= '9') {
    valor <<= 3;
    valor |= (*donde++ - '0');
  }
  return valor;
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

toupper(c)
  int c;
{
  if (c >= 'a' && c <= 'z')
    return c - 32;
  return c;
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

descripcion(quien, nombre)
  char quien[64], *nombre;
{
  int temp;

  decimal(obtiene_tam(quien + 60), 0);
  putchar(' ');
  temp = quien[54] & 31;
  if(temp < 10)
    putchar(' ');
  else
    putchar((temp / 10) + '0');
  putchar((temp % 10) + '0');
  putchar('-');
  switch(((quien[54] | quien[55] << 8) & 0x1e0) >> 5) {
    case 1:  puts("Ene");
             break;
    case 2:  puts("Feb");
             break;
    case 3:  puts("Mar");
             break;
    case 4:  puts("Abr");
             break;
    case 5:  puts("May");
             break;
    case 6:  puts("Jun");
             break;
    case 7:  puts("Jul");
             break;
    case 8:  puts("Ago");
             break;
    case 9:  puts("Sep");
             break;
    case 10: puts("Oct");
             break;
    case 11: puts("Nov");
             break;
    case 12: puts("Dic");
             break;
  }
  puts("-");
  temp = (quien[55] >> 1) + 1970;
  putchar((temp / 1000) % 10 + '0');
  putchar((temp /  100) % 10 + '0');
  putchar((temp /   10) % 10 + '0');
  putchar( temp         % 10 + '0');
  puts(" ");
  temp = (quien[53] & 0xf8) >> 3;
  if(temp < 10)
    putchar(' ');
  else
    putchar((temp / 10) + '0');
  putchar((temp % 10) + '0');
  putchar(':');
  temp = ((quien[52] | quien[53] << 8) & 0x7e0) >> 5;
  putchar((temp / 10) + '0');
  putchar((temp % 10) + '0');
  putchar(':');
  temp = (quien[52] & 0x1f) * 2;
  putchar((temp / 10) + '0');
  putchar((temp % 10) + '0');
  puts(" ");
  puts(nombre);
  puts("\r\n");
}

/*
** Convertidor de archivos PC.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 10 de octubre de 1995.
** Revisión: 20 de diciembre de 1995. Soporte para poner el archivo en
**                                    cualquier destino.
*/

#define EOF      (-1)

int archivo, longitud;
char *apunta;
char linea[80];
char nombre_real[11];
char mensaje[520];
char fat[4608];

main() {
  int tecla, error;
  int conteo, entrada, conteo2, archivos, no_es;
  int bloque;

  puts("\r\n");
  color(15);
  puts("Convertidor de archivos PC   (c) Copyright 1995 Oscar Toledo G.\r\n");
  puts("\r\n");
  color(14);
  puts("Inserte el disco PC (1.44 MB.) y pulse una tecla (Esc- cancela)...");
  tecla = getchar();
  if(tecla == 0x1b) {
    color(15);
    puts("\r\n\nConvertidor cancelado.\r\n");
    exit(1);
  }
  puts("\r\n\n");
  for(conteo = 1; conteo<10; conteo++) {
    lee_sector(conteo);
    memcpy(fat + ((conteo - 1) * 512), mensaje + 1, 512);
  }
  color(15);
  puts("Directorio:\r\n\n");
  color(15);
  archivos = 0;
  for(conteo = 19; conteo<26; conteo++) {
    lee_sector(conteo);
    for(entrada = 1; entrada<513; entrada += 32) {
      if(mensaje[entrada] != 0x80 && mensaje[entrada] != 0xe5
      && mensaje[entrada] != 0x00) {
        ++archivos;
        for(conteo2 = 0; conteo2<8; ++conteo2) {
          putchar(mensaje[entrada+conteo2]);
        }
        if(mensaje[entrada+conteo2] != ' ')
          putchar('.');
        else
          putchar(' ');
        for(; conteo2<11; ++conteo2) {
          putchar(mensaje[entrada+conteo2]);
        }
        puts("    ");
      }
    }
  }
  if(archivos % 5)
    puts("\r\n");
  puts("\r\n");
  color(10);
  puts("¿ Nombre del archivo para copiar ? ");
  lee_linea(linea, 14);
  if(linea[0] == 0) {
    color(15);
    puts("\r\nConvertidor cancelado.\r\n");
    exit(1);
  }
  for(conteo = 0; conteo<11; conteo++)
    nombre_real[conteo] = ' ';
  conteo = 0;
  apunta = linea;
  while(*apunta) {
    if(*apunta == '.') {
      conteo = 8;
    } else {
      if(conteo < 11)
        nombre_real[conteo++] = toupper(*apunta);
    }
    ++apunta;
  }
  color(10);
  puts("¿ Nombre del archivo destino ? ");
  lee_linea(linea, 80);
  if(linea[0] == 0) {
    color(15);
    puts("\r\nConvertidor cancelado.\r\n");
    exit(1);
  }
  if((archivo = fopen(linea, "w")) == 0) {
    color(15);
    puts("\r\nImposible abrir archivo temporal.\r\n");
    exit(1);
  }
  for(conteo = 19; conteo<26; conteo++) {
    lee_sector(conteo);
    for(entrada = 1; entrada<513; entrada += 32) {
      no_es = 0;
      for(conteo2 = 0; conteo2<11; ++conteo2) {
        if(mensaje[entrada+conteo2] != nombre_real[conteo2]) {
          no_es = 1;
          break;
        }
      }
      if(!no_es) {
        bloque = mensaje[entrada+26] | (mensaje[entrada+27] << 8);
        longitud = mensaje[entrada+28] | (mensaje[entrada+29] << 8)
                 | (mensaje[entrada+30] << 16) | (mensaje[entrada+31] << 24);
        while(longitud) {
          if(escribe_sector(bloque)) {
            color(15);
            puts("\r\nLa unidad destino esta llena.\r\n");
            break;
          }
          bloque = sig_bloque(bloque);
        }
        break;
      }
    }
    if(!no_es)
      break;
  }
  fclose(archivo);
  if(no_es) {
    color(15);
    puts("\r\nNo se encontró el archivo.\r\n");
    exit(1);
  }
  color(11);
  puts("\r\nFin de conversión.\r\n");
}

escribe_sector(bloque)
  int bloque;
{
  int conteo;
  int bytes;

  lee_sector(bloque + 31);
  if(longitud >= 512) {
    bytes = 512;
    longitud -= 512;
  } else {
    bytes = longitud;
    longitud = 0;
  }
  conteo=1;
  while(conteo <= bytes) {
    if(fputc(mensaje[conteo], archivo) == EOF) {
      return 1;
    }
    ++conteo;
  }
  return 0;
}

sig_bloque(bloque)
  int bloque;
{
  int pos;
  pos = (bloque / 2) * 3;
  if(bloque % 2) {
    return (fat[pos + 1] >> 4) | (fat[pos + 2] << 4);
  } else {
    return fat[pos] | ((fat[pos + 1] & 15) << 8);
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
  return mensaje[0];
}

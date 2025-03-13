/*
** Copiador binario de discos.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 11 de septiembre de 1995.
*/

#define EOF      (-1)

char *apunta;
char linea[10];
char datos[512];
char mensaje[520];

main() {
  int tecla;
  int cancelacion;

  puts("\r\n");
  color(15);
  puts("Copiador de discos   (c) Copyright 1995 Oscar Toledo G.\r\n");
  puts("\r\n");
  color(11);
  puts("¡Advertencia!, esta utilidad borrara el contenido\r\n");
  puts("del disco destino que se introduzca.\r\n");
  puts("\r\n");
  color(10);
  puts("¿Desea continuar con el copiador (S/N)? ");
  lee_linea(linea, 2);
  if(toupper(linea[0]) != 'S') {
    puts("\r\n");
    color(15);
    puts("El copiador ha sido cancelado.\r\n");
    exit(1);
  }
  color(11);
  puts("\r\n");
  puts("Para salir en cualquier momento, pulse Ctrl-C.\r\n\r\n");
  cancelacion = 0;
  if(!cancelacion)
    cancelacion |= copia_pista(0, 19);
  if(!cancelacion)
    cancelacion |= copia_pista(20, 39);
  if(!cancelacion)
    cancelacion |= copia_pista(40, 59);
  if(!cancelacion)
    cancelacion |= copia_pista(60, 79);
  if(cancelacion) {
    color(13);
    puts("\rSe ha cancelado la copia.                                   \r\n");
  } else {
    color(15);
    puts("\rFin de la copia.                                            \r\n");
  }
  exit(1);
}

copia_pista(inicio, final)
  int inicio, final;
{
  int temp;
  int conteo;
  int pista, lado, sector;
  int error;

  color(10);
  puts("\rInserte el disco origen, y pulse una tecla...");
  while(1) {
    if(checa_int()) break;
  }
  puts("\r\n\n");
  temp = fopen("B:/_COPIAD_.001", "w");
  if(temp == 0) {
    color(14);
    puts("No se pudo crear el archivo temporal.\r\n\n");
    return 1;
  }
  for(pista = inicio; pista <= final; ++pista) {
    for(lado = 0; lado <= 1; ++lado) {
      color(11);
      puts("\rLeyendo pista ");
      color(15);
      decimal(pista);
      color(11);
      puts(", lado ");
      color(15);
      decimal(lado);
      for(sector = 0; sector < 18; ++sector) {
        if(lee_sector(pista * 36 + lado * 18 + sector)) {
          color(14);
          puts("  Sección defectuosa.\r\n");
        }
        for(conteo = 0; conteo < 512; ++conteo) {
          if(fputc(datos[conteo], temp) == EOF) {
            color(14);
            puts("  No hay suficiente espacio en el archivo temporal.\r\n");
            fclose(temp);
            return 1;
          }
        }
      }
    }
  }
  fclose(temp);
  color(10);
  puts("\rInserte el disco destino, y pulse una tecla...");
  while(1) {
    if(checa_int()) break;
  }
  puts("\r\n\n");
  temp = fopen("B:/_COPIAD_.001", "r");
  if(temp == 0) {
    color(14);
    puts("Error fatal, no se puede leer el archivo temporal.\r\n\n");
    return 1;
  }
  for(pista = inicio; pista <= final; ++pista) {
    for(lado = 0; lado <= 1; ++lado) {
      color(11);
      puts("\rEscribiendo pista ");
      color(15);
      decimal(pista);
      color(11);
      puts(", lado ");
      color(15);
      decimal(lado);
      for(sector = 0; sector < 18; ++sector) {
        for(conteo = 0; conteo < 512; ++conteo) {
          if((datos[conteo] = fgetc(temp)) == EOF) {
            color(14);
            puts("  Error fatal, no hay suficiente datos en el archivo temporal.\r\n");
            fclose(temp);
            return 1;
          }
        }
        if(escribe_sector(pista * 36 + lado * 18 + sector)) {
          color(14);
          puts("  Sección defectuosa.\r\n");
        }
      }
    }
  }
  fclose(temp);
  remove("B:/_COPIAD_.001");
  return 0;
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

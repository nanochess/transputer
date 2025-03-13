/*
** Efemerides.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 10 de julio de 1995.
** Revisión: 9 de octubre de 1995. Soporte para recordatorio.
** Revisión: 22 de noviembre de 1995. Camino de EFEM.DAT a C:
** Revisión: 31 de mayo de 1996. Soporte para el año 2000.
*/

#define NULL   0
#define EOF  (-1)

#define SI  1
#define NO  0

int archivo;
char linea[100];

main() {
  char cadena[8];
  int byte;
  int conteo, ilustrar, primero, primero_2;

  putchar(0x1b);
  putchar(1);
  putchar(15);
  hora_actual(cadena);
  archivo = fopen("c:efem.dat", "r");
  if(archivo == NULL) puts("¡El archivo c:efem.dat no existe!\r\n");
  else {
    ilustrar = NO;
    primero = NO;
    *linea = 'A';
    while(*linea) {
      lee_linea();
      if(linea[0] == '-') {
        if(ilustrar)
          puts("\r\n");
        ilustrar = NO;
      }
      if(ilustrar) puts(linea);
      conteo = 0;
      while(conteo < 5) {
        if(linea[conteo] != cadena[conteo]) break;
        ++conteo;
      }
      if(conteo == 5) {
        ilustrar = SI;
        if(primero == NO && linea[5] != '?') {
          primero = SI;
          primero_2 = NO;
          puts("En un día cómo hoy, pero de ");
          putchar(linea[5]);
          putchar(linea[6]);
          putchar(linea[7]);
          putchar(linea[8]);
          puts(":\r\n");
        }
        if(primero_2 == NO && linea[5] == '?') {
          primero = NO;
          primero_2 = SI;
          puts("Recordatorio:\r\n");
        }
      }
    }
    fclose(archivo);
  }
}

lee_linea() {
  char *donde;
  int byte;
  donde = linea;
  while(1) {
    byte = fgetc(archivo);
    if(byte == EOF) break;
    *donde++ = byte;
    if(byte == '\n') break;
  }
  *donde = 0;
}

hora_actual(resultado) char *resultado; {
  char oscar[6];
  hora(oscar);
  *resultado++ = '-';
  *resultado++ = '0' + (oscar[2] / 10);
  *resultado++ = '0' + (oscar[2] % 10);
  *resultado++ = '0' + (oscar[1] / 10);
  *resultado++ = '0' + (oscar[1] % 10);
  *resultado++ = '0' + (oscar[0] / 10);
  *resultado++ = '0' + (oscar[0] % 10);
  *resultado = 0;
}

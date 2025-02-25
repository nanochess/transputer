/*
** Esto es lo que yo llamo un COMMAND.COM
**
** (c) Copyright 1995 Oscar Toledo Gutierrez.
**
** Creaci—n: 10 de junio de 1995.
*/

#define NULL  0
#define EOF  -1

char solicitud[64];
char orden[256];
int disponible, archivo, byte;
char *pos, *des;
char prog[256];
char mensaje[520];

main() {
  disponible = (_finprog() + 15) & ~15;

  while(1) {
    color(10);
    puts("A>");
    leelinea(orden, 256);
    pos = orden;
    while(*pos == ' ') ++pos;
    if(*pos == 0) continue;
    des = prog;
    while((*pos != ' ') & (*pos != 0))
      *des++ = toupper(*pos++);
    *des = 0;
    while(*pos == ' ') ++pos;
    if(strcmp(prog, "DIR") == 0) {
      dir();
      continue;
    }
    if(strcmp(prog, "VER") == 0) {
      version();
      continue;
    }
    if(strcmp(prog, "MEM") == 0) {
      memoria();
      continue;
    }
    if(strcmp(prog, "FIN") == 0) {
      color(10);
      exit(1);
    }
    des = prog;
    while(1) {
      if(*des == 0) {
        *des++ = '.';
        *des++ = 'C';
        *des++ = 'M';
        *des++ = 'G';
        *des++ = 0;
        break;
      }
      if(*des == '.') break;
      ++des;
    }
    des = disponible;
    archivo = fopen(prog, "r");
    if(archivo == NULL) {
      puts("Archivo inexistente.\r\n");
      continue;
    }
    while((byte = fgetc(archivo)) != EOF) {
      *des++ = byte;
    }
    fclose(archivo);
    disponible();
  }
}

memoria() {
  int a;
  a = &a;
  color(11);
  decimal(a - disponible, 1);
  puts(" bytes disponibles.\r\n");
}

version() {
  color(15);
  puts("\r\nInterfaz de texto. Versi—n 3.00.\r\n");
  puts("(c) Copyright 1995 Oscar Toledo G.\r\n\r\n");
}

dir() {
  char arc[32];
  char fat[512];
  int conteo;
  int num_arc;
  int temp;
  int sector;
  int posdir;

  sector = 2;
  posdir = -1;
  puts("\r\n");
  num_arc = 0;
  while(1) {
    if(posdir == -1) {
      lee_sector(0, 0, ++sector);
      posdir = 1;
    }
    conteo = 0;
    while(conteo < 32)
      arc[conteo++] = mensaje[posdir++];
    if(posdir == 513) posdir = -1;
    if(arc[0] == 0x80) continue;
    if(arc[0] == 0) break;
    color(15);
    conteo = 0;
    while(conteo < 8)
      putchar(arc[conteo++]);
    if(arc[conteo] == ' ') putchar(' ');
    else putchar('.');
    while(conteo < 11)
      putchar(arc[conteo++]);
    if(*pos == '/') {
      puts("    ");
    } else {
      puts(" ");
      color(10);
      decimal(arc[28] | arc[29] << 8 | arc[30] << 16 | arc[31] << 24, 0);
      puts("  ");
      color(14);
      temp = arc[22] & 31;
      if(temp < 10) putchar(' ');
      else putchar((temp / 10) + '0');
      putchar((temp % 10) + '0');
      temp = arc[22] | arc[23] << 8;
      puts("-");
      temp = (temp & 0x1e0) >> 5;
      if(temp == 1) puts("ene");
      if(temp == 2) puts("feb");
      if(temp == 3) puts("mar");
      if(temp == 4) puts("abr");
      if(temp == 5) puts("may");
      if(temp == 6) puts("jun");
      if(temp == 7) puts("jul");
      if(temp == 8) puts("ago");
      if(temp == 9) puts("sep");
      if(temp == 10) puts("oct");
      if(temp == 11) puts("nov");
      if(temp == 12) puts("dic");
      /* Patch for year 2000 Feb/23/2025 */
/*
      puts("-19");
      temp = (arc[23] >> 1) + 70;
      putchar((temp / 10) + '0');
      putchar((temp % 10) + '0');
 */
      puts("-");
      temp = (arc[23] >> 1) + 1970;
      putchar((temp / 1000) + '0');
      putchar((temp / 100) % 10 + '0');
      putchar((temp / 10) % 10 + '0');
      putchar((temp % 10) + '0');
      putchar(' ');
      temp = (arc[21] & 0xf8) >> 3;
      if(temp < 10) putchar(' ');
      else putchar((temp / 10) + '0');
      putchar((temp % 10) + '0');
      putchar(':');
      temp = ((arc[20] | arc[21] << 8) & 0x7e0) >> 5;
      putchar((temp / 10) + '0');
      putchar((temp % 10) + '0');
      putchar(':');
      temp = (arc[20] & 0x1f) << 1;
      putchar((temp / 10) + '0');
      putchar((temp % 10) + '0');
      puts("\r\n");
    }
    ++num_arc;
  }
  puts("\r\n");
  color(11);
  decimal(num_arc, 0);
  puts(" archivos.\r\n");
  lee_sector(0, 0, 2);
  memcpy(fat, mensaje + 1, 512);
  conteo = 0;
  temp = 0;
  while(temp < 160) {
    if(fat[temp++] == 0) conteo = conteo + 9216;
  }
  color(11);
  decimal(conteo, 0);
  puts(" bytes libres.\r\n\r\n");
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

strcmp(uno, dos) char *uno, *dos; {
  while(1) {
    if(*uno < *dos) return -1;
    if(*uno > *dos) return 1;
    if(*uno == 0) return 0;
    uno++;
    dos++;
  }
}

strcpy(destino, inicio) char *destino, *inicio; {
  while(*destino++ = *inicio++) ;
}

strcat(destino, inicio) char *destino, *inicio; {
  while(*destino) ++destino;
  strcpy(destino, inicio);
}

color(col) int col; {
  putchar(0x1b);
  putchar(0x01);
  putchar(col);
}

toupper(car) int car; {
  if((car >= 'a') & (car <= 'z')) return car - 32;
  else return car;
}

leelinea(pos, tam) char *pos; int tam; {
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

int *apunta;

lee_sector(pista, lado, sector) int pista, lado, sector; {
  mensaje[0] = LEE_SECTOR;
  mensaje[1] = pista;
  mensaje[2] = lado;
  mensaje[3] = sector;
  apunta = mensaje;
  mandar(0x80000108, &apunta, 4);
  recibir(0x8000010c, &apunta, 1);
  return mensaje[0];
}

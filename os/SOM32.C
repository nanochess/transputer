/*
** Sistema Operativo Mexicano de 32 bits (SOM32)
**
** (c) Copyright 1995 Oscar Toledo G.
**
** 17-jun-1995
*/

#define NULL            0

#define TAM_SECTOR    512
#define SECTORES       18
#define LADOS           2
#define PISTAS         80
#define SEC_DIR        10
#define BLOQUES       160
#define TAM_DIR      5120

/*
** 1. El sector de arranque se encuentra en la pista 0, lado 0, sector 1.
** 2. La tabla de asignaci—n de archivos se encuentra en la pista 0, lado 0,
**    sector 2.
** 3. El directorio ocupa la pista 0, lado 0, sector 3 - 12.
** 4. Los sectores 13-18 de la pista 0, lado 0, se encuentran desocupados.
** 5. Los datos del usuario ocupan el resto de las pistas.
*/

int arch_abiertos;
int respuesta;
int man1, man2;
int *apunta;
int video, pedido;
int *servicio1, *servicio2;
int *bajo_nivel1, *bajo_nivel2;
char *quien;

int pos_x_cursor, pos_y_cursor, color;
int cop;

char mensaje[520];

char fat[BLOQUES];
char dir[TAM_DIR];

#define MAX_ARCH        5

char buffers[2560];
int pdir[MAX_ARCH];  /* Posici—n del directorio para el nombre del archivo */
int pbyt[MAX_ARCH];  /* Posici—n en bytes dentro del archivo */
int tbyt[MAX_ARCH];  /* Tama–o en bytes del archivo */
int bini[MAX_ARCH];  /* Bloque inicial del archivo */
int bloq[MAX_ARCH];  /* Bloque en el que esta trabajando */
int sbuf[MAX_ARCH];  /* Sector del bloque que esta en el buffer (1-18) */
int psec[MAX_ARCH];  /* Posici—n en bytes dentro del sector (-1= no cargado) */
int imod[MAX_ARCH];  /* 1= Sector modificado */
int desa[MAX_ARCH];  /* 1= Descriptor usado */
int amod[MAX_ARCH];  /* 1= Archivo modificado */

/*
** Esto debe ser simple.
**
** El servidor proporciona los siguientes servicios:
**
** 1. Lee y escribe sectores de un disco flexible de 1.44 MB.
**    01 pista lado sector          Lee 512 bytes, y un byte de error.
**    02 pista lado sector bytes    Escribe 512 bytes, y da un byte de error.
** 2. Formatea pistas de un disco flexible de 1.44 MB.
**    03 pista lado                 Formatea la pista. y da un byte de error.
** 3. Proporciona acceso directo a un buffer de video de 80x25 con
**    atributos (tipo PC), s—lo que sin el parpadeo.
**    04 linea 80bytes.             Actualiza una linea del video real.
** 4. Bufferea las teclas pulsadas en el servidor.
**    05                            Lee una tecla del servidor o 0 si no hay.
** 5. Proporciona la fecha y hora actuales.
**    06 segundos minutos hora fecha mes a–o.    Lee la fecha.
** 6. Modificaci—n de la forma y posici—n del cursor.
**    07 posici—n (2 bytes)
**    08 inicio final (2 bytes)
**
** ARCH.C proporciona los siguientes servicios a los programas:
**
** 1. Abre archivos. fopen(archivo, modo).
**    Servicio: 01 archivo 00 modo 00 -> pal16. 0 = Error.
** 2. Cierra archivos. fclose(man).
**    Servicio: 02 pal8 -> pal16. 0 = Correcto.
** 3. Lee caracteres. fgetc(man).
**    Servicio: 03 pal8 -> pal16. -1 = Fin de archivo.
** 4. Escribe caracteres. fputc(byte, man).
**    Servicio: 04 pal8 pal8 -> pal16.  -1 = error.
** 5. Lee caracteres de la consola. getchar().
**    Servicio: 05 -> pal16.
** 6. Escribe caracteres en la consola. putchar().
**    Servicio: 06 pal8 -> pal16.
** 7. Borra archivos. remove(archivo).
**    Servicio: 07 archivo 00 -> pal16. 0 = Correcto.
** 8. Renombra archivos. rename(original, nuevo).
**    Servicio: 08 original 00 nuevo 00 -> pal16. 0 = Correcto.
** 9. Proporciona un apuntador para que los programas puedan accesar
**    un buffer de video libre. (C—mo si estuviera mapeado en memoria).
**    El apuntador estar‡ en 8000:0104.
** 10. Ejecuta INTERFAZ.CMG en el comienzo.
**
** Todos los servicios se accesaran pasando mensajes a ARCH.C, ARCH.C
** proporcionar‡ una direcci—n para el canal. (8000:0100)
*/

main() {
  char buf[32];
  char *destino;
  apunta = 0x80000110;
  video = *apunta;
  bajo_nivel1 = 0x80000108;
  bajo_nivel2 = 0x8000010C;
  servicio1 = 0x80000100;
  servicio2 = 0x80000104;
  arch_abiertos = 0;
  pos_x_cursor = 0;
  pos_y_cursor = 0;
  color = 0x07;
  forma_cursor(0, 15);
  situa_cursor();
  men("Sistema Operativo Multitarea de 32 bits. v1.00\r\n");
  men(">>>>> (c) Copyright 1995 Oscar Toledo G. <<<<<\r\n");
  men("\r\n");
/*
** codigo para probar
  quien = buf;
  *quien = 1;
  *(quien+1) = 'T';
  *(quien+2) = 'C';
  *(quien+3) = '.';
  *(quien+4) = 'C';
  *(quien+5) = 'M';
  *(quien+6) = 'G';
  *(quien+7) = 0;
  *(quien+8) = 'r';
  *(quien+9) = 0;
  man1 = abrir();
  *quien = 1;
  *(quien+1) = 'C';
  *(quien+2) = 0;
  *(quien+3) = 'w';
  *(quien+4) = 0;
  man2 = abrir();
  while(1) {
    *quien = 3;
    *(quien+1) = man1;
    respuesta = lee();
    if(respuesta == -1) break;
    *quien = 4;
    *(quien+1) = respuesta;
    *(quien+2) = man2;
    respuesta = escribe();
  }
  *quien = 2;
  *(quien+1) = man1;
  respuesta = cierra();
  *quien = 2;
  *(quien+1) = man2;
  respuesta = cierra();
  while(1) ;
*/

/*
** codigo final
*/
  quien = buf;
  *quien = 1;
  strcpy(quien + 1, "INTERFAZ.CMG");
  strcpy(quien + 14, "r");
  man1 = abrir();
  if(man1 == -1) {
    men("No se pudo cargar INTERFAZ.CMG\r\n\r\nSistema parado...");
    while(1) ;
  }
  destino = (_finprog() + 15) & ~15;
  while(1) {
    *quien = 3;
    *(quien+1) = man1;
    respuesta = lee();
    if(respuesta == -1) break;
    *destino++ = respuesta;
  }
  *quien = 2;
  *(quien+1) = man1;
  respuesta = cierra();
  inicia_tarea((_finprog() + 15) & ~15, buf - 512, 0, NULL);
  while(1) {
    recibir(servicio1, &quien, 4);
    pedido = *quien;
    if(pedido == 1) respuesta = abrir();
    else if(pedido == 2) respuesta = cierra();
    else if(pedido == 3) respuesta = lee();
    else if(pedido == 4) respuesta = escribe();
    else if(pedido == 5) {
      respuesta = checa_tecla();
    }
    else if(pedido == 6) {
      respuesta = *(quien + 1);
      escribe_car(respuesta);
    }
    else if(pedido == 7) respuesta = elimina();
    else if(pedido == 8) respuesta = renombra();
    else respuesta = -1;
    mandar(servicio2, &respuesta, 2);
  }
}

strcpy(destino, inicio) char *destino, *inicio; {
  while(*destino++ = *inicio++) ;
}

#define OCUPADO   1
#define SIN_USAR  0

carga_dir() {
  int conteo;
  int pos;
  if(arch_abiertos) return;
  lee_sector(0, 0, 2);
  memcpy(fat, mensaje + 1, 160);
  conteo = 3;
  pos = 0;
  while(conteo <= 12) {
    lee_sector(0, 0, conteo);
    memcpy(dir + pos, mensaje + 1, 512);
    ++conteo;
    pos = pos + 512;
  }
}

graba_dir() {
  int conteo;
  int pos;
  memcpy(mensaje + 4, fat, 160);
  memset(mensaje + 164, 0xfc, 352);
  escribe_sector(0, 0, 2);
  conteo = 3;
  pos = 0;
  while(conteo <= 12) {
    memcpy(mensaje + 4, dir + pos, 512);
    escribe_sector(0, 0, conteo);
    ++conteo;
    pos = pos + 512;
  }
}

toupper(car) int car; {
  if((car >= 'a') & (car <= 'z')) return car - 32;
  else return car;
}

forma(nombre, control) char *nombre, *control; {
  int conteo;
  char *pos;
  pos = control;
  conteo = 0;
  while(conteo++ < 11)
    *pos++ = ' ';
  pos = control;
  while(*nombre) {
    if(*nombre == '.') {
      pos = control + 8;
      ++nombre;
      continue;
    }
    if(pos == control + 11) {
      ++nombre;
      continue;
    }
    *pos++ = toupper(*nombre++);
  }
}

busca_dir(nombre) char *nombre; {
  char nom[11];
  int conteo;
  int pos;
  carga_dir();
  forma(nombre, nom);
  pos = 0;
  while(pos < TAM_DIR) {
    if(dir[pos] == 0) return NULL;
    if(dir[pos] != 0x80) {
      conteo = 0;
      while(conteo < 11) {
        if(nom[conteo] != dir[pos+conteo]) break;
        ++conteo;
      }
      if(conteo == 11) return dir+pos;
    }
    pos = pos + 32;
  }
  return NULL;
}

busca_esp() {
  int pos;
  pos = 0;
  while(pos < 5120) {
    if(dir[pos] == 0) return dir+pos;
    if(dir[pos] == 0x80) return dir+pos;
    pos = pos + 32;
  }
}

borra(pos_dir) char *pos_dir; {
  int bloque, sig;
  *pos_dir = 0x80;  /* borrado */
  bloque = (*(pos_dir+24)      ) |
           (*(pos_dir+25) <<  8) |
           (*(pos_dir+26) << 16) |
           (*(pos_dir+27) << 24);
  if(bloque) {
    while(1) {
      if(fat[bloque] == 0xff) {
        fat[bloque] = 0;
        break;
      }
      sig = fat[bloque];
      fat[bloque] = 0;
      bloque = sig;
    }
  }
  graba_dir();
}

crea(nombre) char *nombre; {
  char *pos_dir, *pos_fin;
  char nom[11];
  int conteo;
  int tiempo;
  int fecha;
  int seg, min, hor, fec, mes, an;
  if((pos_dir = busca_dir(nombre)) != NULL) return NULL;
  pos_fin = pos_dir = busca_esp();
  forma(nombre, nom);
  conteo = 0;
  while(conteo < 11)
    *pos_dir++ = nom[conteo++];
  conteo = 0;
  while(conteo++ < 9)
    *pos_dir++ = 0;
  obtener_hora(&seg, &min, &hor, &fec, &mes, &an);
  tiempo = seg / 2;
  tiempo = tiempo | (min << 5);
  tiempo = tiempo | (hor << 11);
  *pos_dir++ = tiempo & 255;
  *pos_dir++ = (tiempo >> 8) & 255;
  fecha = fec;
  fecha = fecha | (mes << 5);
  fecha = fecha | ((an - 70) << 9);
  *pos_dir++ = fecha & 255;
  *pos_dir++ = (fecha >> 8) & 255;
  conteo = 0;
  while(conteo++ < 8)
    *pos_dir++ = 0;
  graba_dir();
  return pos_fin;
}

busca_des() {
  int conteo;
  conteo = 0;
  while(conteo < MAX_ARCH) {
    if(desa[conteo] == SIN_USAR)
      return conteo;
    ++conteo;
  }
  return -1;
}

elimina() {
  char nombre[15];
  char *separa, *pos_dir;
  int longitud;
  separa = quien + 1;
  longitud = 0;
  while(*separa) {
    if(longitud != 14)
      nombre[longitud++] = *separa;
    ++separa;
  }
  nombre[longitud] = 0;
  if((pos_dir = busca_dir(nombre)) != NULL) {
    longitud = 0;
    while(longitud < MAX_ARCH) {
      if(desa[longitud] == OCUPADO)
        if(pdir[longitud] == pos_dir)
          return -1;
      ++longitud;
    }
    borra(pos_dir);
    return 0;
  }
  return -1;
}

renombra() {
  char nombre1[15];
  char nombre2[15];
  char nom[11];
  char *separa, *pos_dir;
  int longitud;
  separa = quien + 1;
  longitud = 0;
  while(*separa) {
    if(longitud != 14)
      nombre1[longitud++] = *separa;
    ++separa;
  }
  nombre1[longitud] = 0;
  ++separa;
  longitud = 0;
  while(*separa) {
    if(longitud != 14)
      nombre2[longitud++] = *separa;
    ++separa;
  }
  nombre2[longitud] = 0;
  if((pos_dir = busca_dir(nombre2)) != NULL) return -1;
  if((pos_dir = busca_dir(nombre1)) == NULL) return -1;
  longitud = 0;
  while(longitud < MAX_ARCH) {
    if(desa[longitud] == OCUPADO)
      if(pdir[longitud] == pos_dir)
        return -1;
    ++longitud;
  }
  forma(nombre2, nom);
  longitud = 0;
  while(longitud < 11)
    *pos_dir++ = nom[longitud++];
  graba_dir();
  return 0;
}

abrir() {
  char nombre[15];
  char modo[3];
  char *separa;
  int longitud;
  int desc;
  char *pos_dir;
  separa = quien + 1;
  longitud = 0;
  while(*separa) {
    if(longitud != 14)
      nombre[longitud++] = *separa;
    ++separa;
  }
  nombre[longitud] = 0;
  ++separa;
  longitud = 0;
  while(*separa) {
    if(longitud != 2)
      modo[longitud++] = *separa;
    ++separa;
  }
  modo[longitud] = 0;
  ++separa;
  if(modo[0] == 'r') {
    if((pos_dir = busca_dir(nombre)) == NULL) return 0;
    if((desc = busca_des()) == -1) return 0;
    ++arch_abiertos;
    desa[desc] = OCUPADO;
    pdir[desc] = pos_dir;
    tbyt[desc] = ((*(pos_dir+28)) | (*(pos_dir+29) << 8) |
                  (*(pos_dir+30) << 16) | (*(pos_dir+31) << 24));
    bloq[desc] =
    bini[desc] = ((*(pos_dir+24)) | (*(pos_dir+25) << 8) |
                  (*(pos_dir+26) << 16) | (*(pos_dir+27) << 24));
    pbyt[desc] =
    amod[desc] =
    imod[desc] = 0;
    sbuf[desc] = 1;
    psec[desc] = -1;
    return desc + 1;
  } else if(modo[0] == 'w') {
    if((desc = busca_des()) == -1) return 0;
    if((pos_dir = busca_dir(nombre)) != NULL)
      borra(pos_dir);
    ++arch_abiertos;
    desa[desc] = OCUPADO;
    pdir[desc] = crea(nombre);
    tbyt[desc] =
    bini[desc] =
    pbyt[desc] =
    bloq[desc] =
    amod[desc] =
    imod[desc] = 0;
    sbuf[desc] = 1;
    psec[desc] = -1;
    return desc + 1;
  } else return 0;
}

cierra() {
  int desc;
  char *pos_dir;
  desc = (*(quien+1)) - 1;
  if(desa[desc] != OCUPADO) return -1;
  if(desc < 0) return -1;
  if(desc >= MAX_ARCH) return -1;
  desa[desc] = SIN_USAR;
  if(amod[desc]) {
    if(imod[desc]) {
      if(pbyt[desc] == tbyt[desc]) {
        if(psec[desc] != 512) {
          memset(buffers + desc * TAM_SECTOR + psec[desc], 0xfc,
                 TAM_SECTOR - psec[desc]);
        }
      }
      vacia_sec(desc);
    }
    pos_dir = pdir[desc] + 24;
    *pos_dir++ = (bini[desc]      ) & 255;
    *pos_dir++ = (bini[desc] >>  8) & 255;
    *pos_dir++ = (bini[desc] >> 16) & 255;
    *pos_dir++ = (bini[desc] >> 24) & 255;
    *pos_dir++ = (tbyt[desc]      ) & 255;
    *pos_dir++ = (tbyt[desc] >>  8) & 255;
    *pos_dir++ = (tbyt[desc] >> 16) & 255;
    *pos_dir++ = (tbyt[desc] >> 24) & 255;
    graba_dir();
  }
  --arch_abiertos;
  return 0;
}

lee_sec(desc) int desc; {
  lee_sector(bloq[desc] >> 1, bloq[desc] & 1, sbuf[desc]);
  memcpy(buffers + desc * 512, mensaje + 1, 512);
}

vacia_sec(desc) int desc; {
  memcpy(mensaje + 4, buffers + desc * 512, 512);
  escribe_sector(bloq[desc] >> 1, bloq[desc] & 1, sbuf[desc]);
}

#define FINAL  0xFF

nuevo_bloque() {
  int conteo;
  conteo = 0;
  while(conteo < BLOQUES) {
    if(fat[conteo] == 0) return conteo;
    ++conteo;
  }
  return -1;
}

escribe() {
  int desc, bloque;
  char *buf;
  desc = (*(quien+2)) - 1;
  if(desa[desc] != OCUPADO) return -1;
  if(desc < 0) return -1;
  if(desc >= MAX_ARCH) return -1;
  if(psec[desc] == TAM_SECTOR) {
    if(imod[desc])
      vacia_sec(desc);
    psec[desc] = 0;
    if(sbuf[desc]++ == SECTORES) {
      sbuf[desc] = 1;
      imod[desc] = 0;
      if((bloque = nuevo_bloque()) == -1) return -1;
      fat[bloq[desc]] = bloque;
      bloq[desc] = bloque;
      fat[bloque] = FINAL;
    }
  }
  else if(bloq[desc] == 0) {
    psec[desc] = 0;
    sbuf[desc] = 1;
    imod[desc] = 0;
    if((bloque = nuevo_bloque()) == -1) return -1;
    bini[desc] = bloq[desc] = bloque;
    fat[bloque] = FINAL;
  }
  buf = buffers + desc * TAM_SECTOR;
  buf = buf + psec[desc]++;
  *buf = *(quien+1);
  tbyt[desc]++;
  pbyt[desc]++;
  imod[desc] = amod[desc] = 1;
  return *(quien+1);
}

lee() {
  int desc, bloque;
  char *buf;
  desc = (*(quien+1)) - 1;
  if(desa[desc] != OCUPADO) return -1;
  if(desc < 0) return -1;
  if(desc >= MAX_ARCH) return -1;
  if(pbyt[desc] == tbyt[desc]) return -1;
  if(psec[desc] == -1) {
    lee_sec(desc);
    psec[desc] = 0;
  }
  if(psec[desc] == TAM_SECTOR) {
    if(imod[desc]) {
      vacia_sec(desc);
      imod[desc] = 0;
    }
    if(sbuf[desc]++ == SECTORES) {
      sbuf[desc] = 1;
      bloq[desc] = fat[bloq[desc]];
    }
    lee_sec(desc);
    psec[desc] = 0;
  }
  buf = buffers + desc * TAM_SECTOR;
  buf = buf + psec[desc]++;
  pbyt[desc]++;
  return (*buf);
}

men(cad) char *cad; {
  while(*cad)
    escribe_car(*cad++);
}

escribe_car(c) int c; {
  char *vid, *vid1;
  int conteo;
  if(cop == -1) {
    cop = c;
  } else if(cop) {
    if(cop == 1) color = c;
    if(cop == 2) {
      pos_x_cursor = c;
      situa_cursor();
    }
    if(cop == 3) {
      pos_y_cursor = c;
      situa_cursor();
    }
    cop = 0;
  } else if(c == 0x0c) {
    vid = video;
    conteo = 2000;
    while(conteo--) {
      *vid++ = ' ';
      *vid++ = color;
    }
    pos_x_cursor = 0;
    pos_y_cursor = 0;
    situa_cursor();
  } else if(c == 0x0d) {
    pos_x_cursor = 0;
    situa_cursor();
  } else if(c == 0x08) {
    if(pos_x_cursor-- == 0) {
      pos_x_cursor = 79;
      if(pos_y_cursor-- == 0) {
        pos_y_cursor = 0;
      }
    }
    vid = video + (pos_x_cursor + pos_y_cursor * 80) * 2;
    *vid++ = ' ';
    *vid++ = color;
    situa_cursor();
  } else if(c == 0x0a) {
    if(++pos_y_cursor == 25) {
      --pos_y_cursor;
      rolado();
    }
    situa_cursor();
  } else if(c == 0x1b) {
    cop = -1;
  } else {
    vid = video + (pos_x_cursor + pos_y_cursor * 80) * 2;
    *vid++ = c;
    *vid++ = color;
    if(++pos_x_cursor == 80) {
      pos_x_cursor = 0;
      if(++pos_y_cursor == 25) {
        --pos_y_cursor;
        rolado();
      }
    }
    situa_cursor();
  }
}

rolado() {
  char *vid, *vid1;
  int conteo;
  vid = video;
  vid1 = video + 160;
  conteo = 24;
  while(conteo--) {
    memcpy(vid, vid1, 160);
    vid = vid + 160;
    vid1 = vid1 + 160;
  }
  conteo = 80;
  while(conteo--) {
    *vid++ = ' ';
    *vid++ = color;
  }
}

#define SITUA_CURSOR  7

situa_cursor() {
  int pos_final;
  pos_final = pos_x_cursor + pos_y_cursor * 80;
  mensaje[0] = SITUA_CURSOR;
  mensaje[1] = pos_final & 255;
  mensaje[2] = (pos_final >> 8) &255;
  apunta = mensaje;
  mandar(bajo_nivel1, &apunta, 4);
  recibir(bajo_nivel2, &apunta, 1);
}

#define FORMA_CURSOR  8

forma_cursor(inicio, final) int inicio, final; {
  mensaje[0] = FORMA_CURSOR;
  mensaje[1] = inicio;
  mensaje[2] = final;
  apunta = mensaje;
  mandar(bajo_nivel1, &apunta, 4);
  recibir(bajo_nivel2, &apunta, 1);
}

#define CHECA_TECLA   5

checa_tecla() {
  mensaje[0] = CHECA_TECLA;
  apunta = mensaje;
  mandar(bajo_nivel1, &apunta, 4);
  recibir(bajo_nivel2, &apunta, 1);
  return mensaje[0];
}

#define LEE_SECTOR       1

lee_sector(pista, lado, sector) int pista, lado, sector; {
  mensaje[0] = LEE_SECTOR;
  mensaje[1] = pista;
  mensaje[2] = lado;
  mensaje[3] = sector;
  apunta = mensaje;
  mandar(bajo_nivel1, &apunta, 4);
  recibir(bajo_nivel2, &apunta, 1);
  return mensaje[0];
}

#define ESCRIBE_SECTOR   2

escribe_sector(pista, lado, sector) int pista, lado, sector; {
  mensaje[0] = ESCRIBE_SECTOR;
  mensaje[1] = pista;
  mensaje[2] = lado;
  mensaje[3] = sector;
  apunta = mensaje;
  mandar(bajo_nivel1, &apunta, 4);
  recibir(bajo_nivel2, &apunta, 1);
  return mensaje[0];
}

#define FORMATEA_PISTA   3

formatea_pista(pista, lado) int pista, lado; {
  mensaje[0] = FORMATEA_PISTA;
  mensaje[1] = pista;
  mensaje[2] = lado;
  apunta = mensaje;
  mandar(bajo_nivel1, &apunta, 4);
  recibir(bajo_nivel2, &apunta, 1);
  return mensaje[0];
}

#define OBTENER_HORA     6

obtener_hora(segundos, minutos, hora, fecha, mes, logico)
int *segundos, *minutos, *hora, *fecha, *mes, *logico;
{
  mensaje[0] = OBTENER_HORA;
  apunta = mensaje;
  mandar(bajo_nivel1, &apunta, 4);
  recibir(bajo_nivel2, &apunta, 1);
  *segundos = mensaje[0];
  *minutos = mensaje[1];
  *hora = mensaje[2];
  *fecha = mensaje[3];
  *mes = mensaje[4];
  *logico = mensaje[5];
}

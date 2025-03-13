/*
** Interfaz de texto para el Sistema Operativo Mexicano de 32 bits.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 10 de junio de 1995.
** Revisión: 19 de junio de 1995. Soporte para multiordenes.
** Revisión: 10 de julio de 1995. Soporte para LINEA.
** Revisión: 11 de julio de 1995. Soporte para BLP.
** Revisión: 15 de julio de 1995. Soporte para DIR [dir].
** Revisión: 18 de julio de 1995. Indica <DIR> al listar el directorio.
** Revisión: 18 de julio de 1995. Indica errores de sintaxis en DIR.
** Revisión: 19 de julio de 1995. Soporte para subdirectorios.
** Revisión: 19 de julio de 1995. Corrección de un error que hacia
**                                que no cerrara un archivo, esto hacia
**                                que el sistema ya no pudiera abrir
**                                archivos y emitiera mensajes de error.
** Revisión: 19 de julio de 1995. Soporte para que INICIO.MAN conste
**                                de varias lineas.
** Revisión: 8 de septiembre de 1995. Soporte para la orden F3. (Repetición
**                                    de línea anterior)
** Revisión: 8 de septiembre de 1995. Los subdirectorios en listado ancho
**                                    son indicados con [SUBDIR].
** Revisión: 28 de septiembre de 1995. Hago que ocupe menos memoria.
** Revisión: 22 de noviembre de 1995. Ahora busca INICIO.MAN en C:
** Revisión: 7 de diciembre de 1995. Cuando llama a un programa, carga un
**                                   parametro con un apuntador a la línea
**                                   de ordenes introducida.
** Revisión: 1o. de enero de 1996. Mejoras mínimas.
** Revisión: 25 de enero de 1996. Soporte para crear y borrar directorios.
** Revisión: 25 de enero de 1996. Mejoras en el proceso de la tecla F3.
** Revisión: 29 de enero de 1996. Soporte para copiar, ver directorios y
**                                borrar archivos usando caracteres [] * ?
** Revisión: 6 de febrero de 1996. Soporte para nombres de 32 letras.
** Revisión: 6 de febrero de 1996. Embellecimientos menores.
** Revisión: 10 de marzo de 1996. A través de argc y argv (nuevas
**                                características del SOM32), averigua si
**                                es la primera ejecución.
** Revisión: 11 de marzo de 1996. Soporte para ejecución multitarea.
** Revisión: 28 de marzo de 1996. Soporte para borrar sin pedir confirmación.
** Revisión: 28 de marzo de 1996. Indica cuantos bytes ocupan los archivos.
** Revisión: 14 de mayo de 1996. Corrección de un defecto en COPIA.
*/

#define NULL         0
#define EOF       (-1)
#define ESC       0x1b

#define TAM_LINEA  256

int *disponible, archivo, byte;
int mandatos;
char *pos, *des;
char *orden, *prog;

char datos[256];
char buffer[320];

char ultima_multi[TAM_LINEA], multi[TAM_LINEA];

main(argc, argv)
  int argc;
  char **argv;
{
  char *pos_en_multiorden;
  int cuanto;

  mandatos = NULL;
  pos_en_multiorden = NULL;
  *ultima_multi = 0;

  while (1) {
    if (pos_en_multiorden == NULL) {
      color(10);
      while (1) {
        if (mandatos != NULL || argc == 1) {
          argc = 0;
          if (mandatos == NULL)
            mandatos = fopen(argv[0], "r");
          if (mandatos != NULL) {
            des = multi;
            while (1) {
              byte = fgetc(mandatos);
              if (byte == '\r')
                continue;
              if (byte == '\n')
                break;
              if (byte == EOF) {
                fclose(mandatos);
                mandatos = NULL;
                break;
              }
              *des++ = byte;
            }
            *des = 0;
          } else
            *multi = 0;
        } else {
          puts("\r\nA>");
          lee_linea(multi, 256);
          strcpy(ultima_multi, multi);
        }
        pos_en_multiorden = multi;
        while (*pos_en_multiorden == ' ')
          ++pos_en_multiorden;
        if (*pos_en_multiorden)
          break;
      }
    }
    orden = pos_en_multiorden;
    while (*pos_en_multiorden && *pos_en_multiorden != ';')
      pos_en_multiorden++;
    if (*pos_en_multiorden)
      *pos_en_multiorden++ = 0;
    else
      pos_en_multiorden = NULL;
    pos = orden;
    while (*pos == ' ')
      ++pos;
    if (*pos == 0)
      continue;
    prog = pos;
    while (*pos && *pos != ' ')
      *pos++;
    if (*pos)
      *pos++ = 0;
    while (*pos == ' ')
      ++pos;
    if (compara(prog, "AYUDA") == 0)
      ayuda();
    else if (compara(prog, "DIR") == 0)
      dir();
    else if (compara(prog, "ILUSTRA") == 0)
      ilustra();
    else if (compara(prog, "BD") == 0)
      bd();
    else if (compara(prog, "CD") == 0)
      cd();
    else if (compara(prog, "REN") == 0)
      renombra();
    else if (compara(prog, "BORRA") == 0)
      borra();
    else if (compara(prog, "COPIA") == 0)
      copia();
    else if (compara(prog, "VER") == 0) {
      if (!dargs())
        version();
    } else if (compara(prog, "MEM") == 0) {
      if (!dargs())
        memoria();
    } else if (compara(prog, "LINEA") == 0)
      ilustra_linea();
    else if (compara(prog, "BLP") == 0) {
      if (!dargs()) {
        color(10);
        putchar(12);
      }
    } else if (compara(prog, "FIN") == 0) {
      if (!dargs()) {
        color(10);
        exit(1);
      }
    } else {
      strcpy(buffer, prog);
      des = buffer;
      while (1) {
        if (*des == 0) {
          strcpy(des, ".p");
          break;
        }
        if (*des == '.')
          break;
        ++des;
      }
      if (buffer[0] == '@')
        disponible = execm(buffer + 1, 0, 0);
      else {
        disponible = exec(buffer, 0, 0);
        if (disponible != NULL) {
          execp(disponible);
          free(*(disponible + 1));
        }
      }
      if (disponible == NULL) {
        color(15);
        puts("Archivo inexistente.\r\n");
      }
    }
  }
}

ayuda()
{
  color(15);
  puts("\r\n");
  comp("AYUDA/19/Muestra esta ayuda.\r\n");
  comp("DIR [dir] [-A] [-P]/5/Ilustra el directorio.\r\n");
  comp("COPIA archivo1 archivo2 Copia archivo1 en archivo2.\r\n");
  comp("REN archivo1 archivo2/3/Renombra archivo1 cómo archivo2.\r\n");
  comp("ILUSTRA archivo/9/Ilustra el contenido de archivo.\r\n");
  comp("BORRA archivo/11/Borra el archivo.\r\n");
  comp("VER/21/Ilustra la versión del interfaz.\r\n");
  comp("MEM/21/Ilustra cuanta memoria disponible hay.\r\n");
  comp("LINEA [texto]/11/Ilustra el texto en la consola.\r\n");
  comp("BLP/21/Limpia la pantalla.\r\n");
  comp("FIN/21/Sale del interfaz.\r\n");
  comp("BD [dir]/16/Borra un directorio.\r\n");
  comp("CD [dir]/16/Crea un directorio.\r\n");
  puts("\r\n");
  puts("Multiordenes: EDITOR ; CC ; ENS   Multitarea: @RELOJ\r\n");
}

comp(dato)
  char *dato;
{
  int cuanto;

  while (*dato) {
    if (*dato == '/') {
      ++dato;
      cuanto = 0;
      while (*dato != '/')
        cuanto = cuanto * 10 + (*dato++ - '0');
      ++dato;
      while (cuanto--)
        putchar(' ');
    } else
      putchar(*dato++);
  }
}

memoria()
{
  color(15);
  decimal(memoria_disponible(), 1);
  puts(" bytes disponibles.\r\n");
}

version()
{
  color(15);
  puts("Interfaz de texto.\r\n");
  puts("(c) Copyright 1995-1996 Oscar Toledo G.\r\n");
}

dargs()
{
  while (*pos == ' ')
    ++pos;
  if(*pos) {
    color(15);
    puts("Demasiados parametros.\r\n");
    return 1;
  }
  return 0;
}

proc_nombre()
{
  char *nombre;

  while (*pos == ' ')
    ++pos;
  nombre = pos;
  while (*pos && *pos != ' ')
    ++pos;
  if (*pos == ' ') {
    *pos++ = 0;
    while (*pos == ' ')
      ++pos;
  } else
    *pos = 0;
  return nombre;
}

ilustra_linea()
{
  puts(pos);
  puts("\r\n");
}

bd()
{
  char *nombre;

  if (*pos == 0) {
    falta_nombre("directorio");
    return;
  }
  nombre = proc_nombre();
  if (dargs())
    return;
  if (borra_dir(nombre)) {
    color(15);
    puts("No se pudo borrar el directorio.\r\n");
  }
}

falta_nombre(cual)
  char *cual;
{
  color(15);
  puts("Falta el nombre del ");
  puts(cual);
  puts(".\r\n");
}

cd()
{
  char *nombre;

  if (*pos == 0) {
    falta_nombre("directorio");
    return;
  }
  nombre = proc_nombre();
  if (dargs())
    return;
  if (crea_dir(nombre)) {
    color(15);
    puts("No se pudo crear el directorio.\r\n");
  }
}

no_existe()
{
  color(15);
  puts("El archivo no existe.\r\n");
}

ilustra()
{
  char *nombre;
  int archivo, byte, cual;
  int lineas, tecla;

  if (*pos == 0) {
    falta_nombre("archivo");
    return;
  }
  nombre = proc_nombre();
  if (dargs())
    return;
  archivo = fopen(nombre, "r");
  if (archivo == NULL) {
    no_existe();
    return;
  }
  color(14);
  cual = 0;
  lineas = 0;
  while (1) {
    byte = fgetc(archivo);
    if (byte == EOF)
      break;
    if (cual == 0) {
      if (byte == '\n' || byte == '\r')
        cual = byte;
    }
    if (byte != '\n' && byte != '\r')
      putchar(byte);
    if (byte == cual) {
      puts("\r\n");
      ++lineas;
      if (lineas == 16) {
        lineas = 0;
        color(15);
        puts("Pulse una tecla para continuar, o 'Esc' para terminar...");
        tecla = getchar();
        comp("\r/56/\r");
        color(14);
        if (tecla == ESC)
          break;
      }
    }
  }
  fclose(archivo);
}

renombra()
{
  char *nombre1;
  char *nombre2;

  color(15);
  if (*pos == 0) {
    falta_nombre("archivo");
    return;
  }
  nombre1 = proc_nombre();
  if (*pos == 0) {
    falta_nombre("nuevo archivo");
    return;
  }
  nombre2 = proc_nombre();
  if (dargs())
    return;
  if (rename(nombre1, nombre2)) {
    puts("No se pudo renombrar el archivo.\r\n");
    return;
  }
}

borra()
{
  char *nombre;
  int primero, todos;
  char buf[3];

  color(15);
  if (*pos == 0) {
    falta_nombre("archivo");
    return;
  }
  nombre = proc_nombre();
  if (dargs())
    return;
  if (busca_prepara(nombre, datos) == 0) {
    puts("El directorio origen esta mal especificado.\r\n");
    return;
  }
  primero = 0;
  todos = 0;
  while (busca_sig(datos, buffer)) {
    if (buffer[48] & 8)
      continue;
    primero = 1;
    color(10);
    if (!todos) {
      puts("¿ Desea borrar ");
      color(14);
      puts(buffer + 64);
      color(10);
      puts(" (Si/No/Todos/Cancelar) ? ");
      color(10);
      lee_linea(buf, 2);
      buf[0] = toupper(buf[0]);
    }
    if (todos || buf[0] == 'T' || buf[0] == 'S') {
      if (buf[0] == 'T')
        todos = 1;
      puts("Borrando ");
      color(14);
      puts(buffer + 64);
      puts("\r\n");
      if (remove(buffer + 64)) {
        color(15);
        puts("Error al intentar borrarlo.\r\n");
      }
    } else if (buf[0] == 'C')
      break;
  }
  busca_fin(datos);
  if (!primero) {
    no_hay_especificacion();
    return;
  }
}

#define BUF_COPIA  16384

copia()
{
  char *nombre1;
  char *nombre2;
  char *temp;
  char *buf;
  int pos1, pos2;
  int arch1, arch2;
  int byte, pegar;
  int primero;

/*
** Este trozo de codigo evade un defecto que ocurre cuando se hace:
**
** COPIA C:/MASTER2/* C:MASTER3;ENS
**
** Ya que se agrega un / al final de MASTER3 y ENS es sobre-escrito.
*/
  temp = pos;
  pos--;
  buf = pos;
  while (*buf++ = *temp++) ;

  color(15);
  if (*pos == 0) {
    falta_nombre("archivo origen");
    return;
  }
  nombre1 = proc_nombre();
  if (*pos == 0) {
    falta_nombre("archivo destino");
    return;
  }
  nombre2 = proc_nombre();
  if (dargs())
    return;
  buf = malloc(BUF_COPIA);
  if (buf == NULL) {
    puts("Falta memoria.\r\n");
    return;
  }
  pegar = 0;
  if (arch2 = abre_dir(nombre2)) {
    fclose(arch2);
    pegar = 1;
    temp = nombre2 + strlen(nombre2);
    if (strlen(nombre2) && *(temp - 1) != '/') {
      *temp++ = '/';
      *temp = 0;
    }
  }
  if (busca_prepara(nombre1, datos) == 0) {
    color(15);
    puts("El directorio origen esta mal especificado.\r\n");
    free(buf);
    return;
  }
  if (!pegar) {
    arch2 = fopen(nombre2, "w");
    if(arch2 == NULL) {
      busca_fin(datos);
      fclose(arch1);
      color(15);
      puts("El disco esta lleno.\r\n");
      free(buf);
      return;
    }
  }
  primero = 0;
  while (busca_sig(datos, buffer)) {
    if (buffer[48] & 8)
      continue;
    arch1 = fopen(buffer + 64, "r");
    if (arch1 == NULL)
      continue;
    primero = 1;
    puts("Copiando ");
    puts(buffer + 64);
    puts(" en ");
    if (!pegar)
      puts(nombre2);
    else {
      strcpy(buffer + 64, nombre2);
      strcat(buffer + 64, buffer);
      puts(buffer + 64);
      arch2 = fopen(buffer + 64, "w");
      if (arch2 == NULL) {
        busca_fin(datos);
        fclose(arch1);
        color(15);
        puts("\r\nEl disco esta lleno.\r\n");
        free(buf);
        return;
      }
    }
    while (1) {
      pos1 = fread(arch1, buf, BUF_COPIA);
      pos2 = fwrite(arch2, buf, pos1);
      if (pos2 != pos1)
        break;
      if (pos1 != BUF_COPIA)
        break;
    }
    puts("\r\n");
    fclose(arch1);
    if (pos2 != pos1) {
      fclose(arch2);
      busca_fin(datos);
      color(15);
      puts("El disco esta lleno.\r\n");
      free(buf);
      return;
    }
    if (pegar)
      fclose(arch2);
  }
  free(buf);
  if (!pegar)
    fclose(arch2);
  busca_fin(datos);
  if (!primero) {
    no_hay_especificacion();
    return;
  }
}

no_hay_especificacion()
{
  color(15);
  puts("No hay ningún archivo con la especificación.\r\n");
}

dir()
{
  char arc[64];
  int conteo, valido;
  int num_arc;
  int temp;
  int sector;
  int posdir;
  int listado_ancho;
  int pausa_pagina;
  int cuantos, tecla;
  int arc_dir, es_esp;
  int espacio_total, bytes_usados;
  int es_subdir;
  char *directorio, *chequeo, *chequeo2;

  directorio = "A:/";
  listado_ancho = 0;
  pausa_pagina = cuantos = 0;
  bytes_usados = 0;
  while (*pos) {
    while (*pos == ' ')
      ++pos;
    if (*pos == 0)
      break;
    if (*pos == '-') {
      ++pos;
      if (toupper(*pos) == 'A')
        listado_ancho = 1;
      else if (toupper(*pos) == 'P')
        pausa_pagina = 1;
      else {
        color(15);
        puts("El modo de uso es: DIR [dir] [-A] [-P]\r\n");
        return;
      }
      ++pos;
    } else {
      directorio = pos;
      while (*pos && *pos != ' ')
        ++pos;
      if (*pos == 0)
        break;
      *pos++ = 0;
    }
  }

  sector = 0;
  posdir = -1;
  if (espacio_libre(directorio, &espacio_total)) {
    color(15);
    puts("La unidad no existe.\r\n");
    return;
  }
  es_esp = 0;
  if ((arc_dir = abre_dir(directorio)) == NULL) {
    if (busca_prepara(directorio, datos) == 0) {
      color(15);
      puts("El directorio esta mal especificado.\r\n");
      return;
    }
    es_esp = 1;
  }
  puts("\r\n");
  num_arc = 0;
  color(15);
  puts("  Volumen: ");
  valido = volumen(directorio, arc);
  puts(arc);
  puts("\r\n  Directorio: ");
  puts(directorio);
  puts("\r\n\n");
  while (1) {
    if (es_esp) {
      if (!busca_sig(datos, buffer))
        break;
      memcpy(arc, buffer, 64);
    } else {
      fread(arc_dir, arc, 64);
      if (arc[0] == 0x80)
        continue;
      if (arc[0] == 0)
        break;
    }
    es_subdir = arc[48] & 8;
    color(15);
    if (es_subdir) {
      temp = 2;
      putchar('[');
      puts(arc);
      putchar(']');
    } else {
      temp = 0;
      puts(arc);
    }
    if (listado_ancho) {
      temp = 40 - (strlen(arc) + temp);
      while (temp--)
        putchar(' ');
    } else {
      color(10);
      temp = 34 - (strlen(arc) + temp);
      while (temp--)
        putchar(' ');
      if (es_subdir)
        comp(" <Dir>/8/");
      else {
        puts(" ");
        bytes_usados += (temp = arc[60] | arc[61] << 8 |
                                arc[62] << 16 | arc[63] << 24);
        decimal(temp, 0);
      }
      puts("  ");
      color(14);
      temp = arc[54] & 31;
      if (temp < 10)
        putchar(' ');
      else
        putchar((temp / 10) + '0');
      putchar((temp % 10) + '0');
      putchar('-');
      switch (((arc[54] | arc[55] << 8) & 0x1e0) >> 5) {
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
        default: puts("err");
                 break;
      }
      puts("-");
      temp = 1970 + (arc[55] >> 1);
      putchar((temp / 1000) % 10 + '0');
      putchar((temp / 100) % 10 + '0');
      putchar((temp / 10) % 10 + '0');
      putchar((temp % 10) + '0');
      puts("  ");
      temp = (arc[53] & 0xf8) >> 3;
      if (temp < 10)
        putchar(' ');
      else
        putchar((temp / 10) + '0');
      putchar((temp % 10) + '0');
      putchar(':');
      temp = ((arc[52] | arc[53] << 8) & 0x7e0) >> 5;
      putchar((temp / 10) + '0');
      putchar((temp % 10) + '0');
      putchar(':');
      temp = (arc[52] & 0x1f) * 2;
      putchar((temp / 10) + '0');
      putchar((temp % 10) + '0');
      puts("\r\n");
      ++cuantos;
      if (cuantos == 16)
        if (pausa_pagina) {
          color(15);
          puts("Pulse una tecla para continuar, o 'Esc' para terminar...");
          tecla = getchar();
          comp("\r/56/\r");
          if (tecla == ESC)
            break;
          cuantos = 0;
          putchar(12);
        }
    }
    ++num_arc;
  }
  if (es_esp)
    busca_fin(datos);
  if (num_arc) {
    puts("\r\n");
    color(11);
    decimal(num_arc, 0);
    comp(" archivos./12/");
    decimal(bytes_usados, 0);
    puts(" bytes.\r\n");
  } else {
    color(15);
    puts("  No hay archivos.\r\n\r\n");
  }
  color(11);
  decimal(espacio_total, 0);
  puts(" bytes libres.\r\n");
  fclose(arc_dir);
}

decimal(numero, corta)
  int numero, corta;
{
  int divisor, digito, cual, es;

  divisor = 1000000000;
  cual = es = 0;
  while (divisor) {
    digito = numero / divisor;
    numero %= divisor;
    if (divisor == 1)
      es = 1;
    if (digito) {
      putchar(digito + '0');
      es = 1;
    } else if (es) {
      putchar('0');
    } else if (!corta)
      putchar(' ');
    ++cual;
    if (cual == 1)
      if (es)
        putchar(',');
      else if (!corta)
        putchar(' ');
    if (cual == 4)
      if (es)
        putchar(',');
      else if (!corta)
        putchar(' ');
    if (cual == 7)
      if (es)
        putchar(',');
      else if (!corta)
        putchar(' ');
    divisor /= 10;
  }
}

strcmp(uno, dos)
  char *uno, *dos;
{
  while (1) {
    if (*uno < *dos)
      return -1;
    if (*uno > *dos++)
      return 1;
    if (*uno++ == 0)
      return 0;
  }
}

compara(uno, dos)
  char *uno, *dos;
{
  while (1) {
    if (toupper(*uno) != *dos++)
      return 1;
    if (toupper(*uno++) == 0)
      return 0;
  }
}

color(col)
  int col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

lee_linea(pos, tam)
  char *pos;
  int tam;
{
  char *ahora, *ahora2, *ahora3;
  int car;

  ahora = pos;
  while (1) {
    car = getchar();
    if (car == 8) {
      if (ahora == pos)
        continue;
      putchar(8);
      --ahora;
      continue;
    } else if (car == 3) {
      ahora2 = ultima_multi;
      ahora3 = pos;
      while (ahora3 != ahora) {
        if (*ahora2)
          ++ahora2;
        ++ahora3;
      }
      for (; *ahora2; ++ahora2) {
        if (ahora == pos + tam - 1)
          continue;
        putchar(*ahora++ = *ahora2);
      }
    } else if (car == 13) {
      puts("\r\n");
      *ahora = 0;
      return;
    } else {
      if (ahora == pos + tam - 1)
        continue;
      putchar(*ahora++ = car);
    }
  }
}

strcpy(destino, inicio)
  char *destino, *inicio;
{
  while (*destino++ = *inicio++) ;
}

strlen(cadena)
  char *cadena;
{
  char *origen;

  origen = cadena;
  while (*cadena)
    ++cadena;
  return (cadena - origen);
}

toupper(car)
  int car;
{
  if ((car >= 'a' && car <= 'z')
   || (car >= 0xe0 && car <= 0xff))
    return car - 32;
  else
    return car;
}

/*
** Soporte para usar patrones para buscar varios archivos al mismo tiempo.
*/
busca_prepara(archivo, dato)
  char *archivo;
  char dato[256];
{
  char *inicio, *final;
  char dir[256];
  int arc_dir;

  strcpy(dir, archivo);
  if (isalpha(*dir) && *(dir + 1) == ':')
    inicio = dir + 2;
  else
    inicio = dir;
  final = inicio + strlen(inicio);
  while(final != inicio && *final != '/')
    --final;
  if (*final == '/')
    strcpy(dato + 8, final + 1);
  else
    strcpy(dato + 8, final);
  *final = 0;
  strcpy(dato + 9 + strlen(dato + 8), dir);
  dato[0] = 0;
  dato[1] = 0;
  dato[2] = 0;
  dato[3] = 0;
  if((arc_dir = abre_dir(dir)) == 0)
    return 0;
  dato[0] = arc_dir;
  dato[1] = arc_dir >> 8;
  dato[2] = arc_dir >> 16;
  dato[3] = arc_dir >> 24;
  return 1;
}

busca_sig(dato, buffer)
  char dato[256];
  char buffer[320];
{
  char entrada[64];
  char *patron;
  char *archivo;
  int arc_dir;

  arc_dir = dato[0] | (dato[1] << 8) | (dato[2] << 16) | (dato[3] << 24);
  while(1) {
    fread(arc_dir, entrada, 64);
    if(entrada[0] == 0)
      return 0;
    if(entrada[0] == 0x80)
      continue;
    if(concuerda(dato + 8, entrada)) {
      memcpy(buffer, entrada, 64);
      strcpy(buffer + 64, dato + 9 + strlen(dato + 8));
      strcat(buffer + 64, "/");
      strcat(buffer + 64, entrada);
      return 1;
    }
  }
}

int concuerda(patron, cadena)
  char *patron, *cadena;
{
  int car;

  while(1) {
    if ((car = *patron++) == 0)
      return (*cadena == 0);
    if (car == '?') {
      if (*cadena++ == 0)
        return 0;
      continue;
    }
    if (car == '*') {
      if (*patron == 0)
        return 1;
      while (*cadena) {
        if (car = concuerda(patron, cadena))
          return car;
        ++cadena;
      }
      return 0;
    }
    if (car == '[') {
      int e;
      char *q;
      int r;

      if (*cadena == 0)
        return 0;
      patron += (r = (*patron == '!' || *patron == '^'));
      q = patron;
      e = 0;
      while (*q) {
        if (e)
          e = 0;
        else if (*q == '\\')
          e = 1;
        else if (*q == ']')
          break;
        ++q;
      }
      if (*q != ']')
        return 0;
      car = 0;
      e = 0;
      while (patron < q) {
        if (e == 0 && *patron == '\\')
          e = 1;
        else if (e == 0 && *patron == '-')
          car = *(patron - 1);
        else {
          int cc = toupper(*cadena);

          if (*(patron + 1) != '-') {
            for (car = car ? car : *patron; car <= *patron; car++)
              if (toupper(car) == cc)
                return r ? 0 : concuerda(q + 1, cadena + 1);
          }
          car = e = 0;
        }
        ++patron;
      }
      return r ? concuerda(q + 1, cadena + 1) : 0;
    }
    if (car == '\\' && (car = *patron++) == 0)
      return 0;
    if (toupper(car) != toupper(*cadena++))
      return 0;
  }
}

busca_fin(dato)
  char dato[256];
{
  int arc_dir;

  arc_dir = dato[0] | (dato[1] << 8) | (dato[2] << 16) | (dato[3] << 24);
  if (arc_dir == 0)
    return;
  fclose(arc_dir);
}

isalpha(car)
  int car;
{
  return (toupper(car) >= 'A') && (toupper(car) <= 'Z');
}

strcat(des, ori)
  char *des, *ori;
{
  while (*des)
    ++des;
  strcpy(des, ori);
}

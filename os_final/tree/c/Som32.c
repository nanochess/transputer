/*
** Sistema Operativo Mexicano de 32 bits.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 14 de junio de 1995.
** Revisión: 15 de junio de 1995. Primera versión operativa.
** Revisión: 16 de junio de 1995. Corrección de varios errores.
** Revisión: 13 de julio de 1995. Soporte para varias unidades.
** Revisión: 13 de julio de 1995. Ahora el usuario sólo pasa apuntadores.
** Revisión: 13 de julio de 1995. Soporte para obtener el espacio libre.
** Revisión: 13 de julio de 1995. Soporte de servicio para leer el directorio.
** Revisión: 14 de julio de 1995. Corrijo un defecto en el renombrado de
**                                archivos.
** Revisión: 14 de julio de 1995. Corrijo un defecto en el acceso a muchos
**                                archivos en diferentes unidades.
** Revisión: 14 de julio de 1995. Soporte para obtener el nombre de volumen.
** Revisión: 15 de julio de 1995. Soporte para atributos.
** Revisión: 15 de julio de 1995. Soporte para cambiar el atributo de un
**                                archivo.
** Revisión: 15 de julio de 1995. Soporte para subdirectorios.
** Revisión: 18 de julio de 1995. Soporte para renombrar subdirectorios.
** Revisión: 18 de julio de 1995. Soporte para . y .. en subdirectorios.
** Revisión: 19 de julio de 1995. Corrección de un error al crear
**                                subdirectorios.
** Revisión: 20 de julio de 1995. Se acelera el tiempo de respuesta para
**                                la mayor parte de los servicios.
** Revisión: 24 de julio de 1995. Corrección de un error al accesar
**                                archivos en subdirectorios muy profundos.
** Revisión: 31 de julio de 1995. Soporte para codigos detallados de error.
** Revisión: 9 de agosto de 1995. Corrección de un error cuando el usuario
**                                intercambiaba discos.
** Revisión: 1o. de octubre de 1995. Corrección de el tipo de flec y fesc,
**                                   antes sólo usaba un int, ahora debe
**                                   ser una matriz de apuntadores a función.
** Revisión: 10 de octubre de 1995. Soporte de servicio de impresión, y
**                                  servicios fseek y ftell.
** Revisión: 29 de octubre de 1995. Mejoramiento de fseek. (más rápido)
** Revisión: 22 de noviembre de 1995. Soporte para CD-ROM's ISO-9660 y High
**                                    Sierra.
** Revisión: 23 de noviembre de 1995. Corrección de un defecto cuando se le
**                                    pedia la etiqueta de un CD-ROM.
** Revisión: 6 de diciembre de 1995. Corrección de un defecto en fseek, cuando
**                                   buscaba al final de un bloque.
** Revisión: 6 de diciembre de 1995. Modificado para que ocupe menos espacio.
** Revisión: 6 de diciembre de 1995. Soporte para los servicios fread y fwrite.
** Revisión: 20 de diciembre de 1995. Corrección de tremendo defecto en fwrite.
** Revisión: 21 de diciembre de 1995. Soporte para archivos especiales, CON:
**                                    IMP: COM: CIN: NUL:.
** Revisión: 21 de diciembre de 1995. Soporte para cambiar la hora de un
**                                    archivo.
** Revisión: 29 de diciembre de 1995. Corrección de un defecto en el manejo de
**                                    directorios ISO-9660.
** Revisión: 30 de diciembre de 1995. Ahora no se traba cuando se escribe en
**                                    una unidad llena.
** Revisión: 1o. de enero de 1996. Soporte para nombres de 15 letras.
** Revisión: 2 de enero de 1996. Corrección de un defecto en anda_camino() y
**                               otro en busca_dir()
** Revisión: 2 de enero de 1996. Detecta acceso a unidades que no existen.
** Revisión: 16 de enero de 1996. Detecta nombres ilegales.
** Revisión: 16 de enero de 1996. Exploración más rápida de directorios de
**                                CD-ROM's.
** Revisión: 20 de enero de 1996. Soporte para escribir y leer al mismo tiempo
**                                en un archivo.
** Revisión: 20 de enero de 1996. Soporte para agregar al final de un archivo.
** Revisión: 22 de enero de 1996. Ahora usa las estructuras de control Adán
**                                para acelerar el acceso.
** Revisión: 25 de enero de 1996. Ahora puede renombrar un mismo archivo para
**                                tener otra combinación de minúsculas y
**                                mayúsculas.
** Revisión: 30 de enero de 1996. Chequeo mejorado de errores en los discos
**                                insertados.
** Revisión: 3 de febrero de 1996. Soporte para extensiones Rock Ridge.
** Revisión: 6 de febrero de 1996. Soporte para nombres de 32 letras.
** Revisión: 8 de febrero de 1996. Se acelera la salida a la pantalla.
** Revisión: 16 de febrero de 1996. Detección de errores en el borrado de
**                                  archivos.
** Revisión: 8 de marzo de 1996. Rutinas para asignación dinámica de memoria.
** Revisión: 8 de marzo de 1996. Ejecución multi-tarea de programas.
** Revisión: 8 de marzo de 1996. Soporte indicación memoria libre.
** Revisión: 9 de marzo de 1996. Corrección de detalles.
** Revisión: 11 de marzo de 1996. Se hace más pequeño el manejo de memoria.
** Revisión: 11 de marzo de 1996. Se hace más óptimo el interfaz con los
**                                programas que usan los servicios.
** Revisión: 28 de marzo de 1996. Corrección de un defecto en abre_dir(),
**                                no checaba si el nombre era un dispositivo
**                                especial.
** Revisión: 2 de abril de 1996. Mejoras varias.
** Revisión: 7 de abril de 1996. Acceso más rápido a las unidades.
** Revisión: 8 de abril de 1996. Corrección de un defecto en carga_programa.
** Revisión: 10 de abril de 1996. Corrección de un defecto en trabaja_dir_cdrom
** Revisión: 14 de mayo de 1996. Corrección de un defecto en forma(), aceptaba
**                               nombres con strlen() == 0.
** Revisión: 31 de mayo de 1996. Corrección de un defecto en fseek.
** Revisión: 31 de mayo de 1996. Corrección de un defecto en busca_dir().
*/

#define SI              1
#define NO              0

#define NULL            0

#define TAM_SECTOR    512   /* Tamaño de un sector */

#define TAM_NOMBRE     32   /* Tamaño máximo de un nombre de archivo */

#define MAX_UNID        4   /* Número máximo de unidades de almacenamiento */
#define MAX_ARCH        5   /* Número máximo de archivos abiertos */

#define FINAL  0xffffffff   /* Final de una cadena enlazada de bloques */

/*
** Modos para busqueda en el directorio
*/
#define B_NORMAL   0x0001   /* Acepta archivos normales en la busqueda */
#define B_OCULTO   0x0002   /* Acepta archivos ocultos en la busqueda  */
#define B_SOLOLEC  0x0004   /* Acepta archivos de sólo lectura   ""    */
#define B_SISTEMA  0x0008   /* Acepta archivos del sistema       ""    */
#define B_SUBDIR   0x0010   /* Acepta subdirectorios             ""    */
#define B_TODOS    0x001f   /* Todos los archivos son aceptados        */

#define NO_EXISTE       0   /* Unidad no instalada */
#define UNIDAD_NORMAL   1   /* Unidad de escritura y lectura */
#define UNIDAD_CDROM    2   /* Unidad de sólo lectura */

#define ISO9660         1   /* Estandard ISO-9660 */
#define HIGHSIERRA      2   /* El ISO-9660 es solo una variación */
                            /* estandarizada del High Sierra */
/*
** Atributos posibles
*/
#define A_OCULTO   0x0001   /* No se puede borrar, renombrar ni truncar */
#define A_SISTEMA  0x0002   /* No se puede borrar, o renombrar          */
#define A_SOLOLEC  0x0004   /* Sólo puede ser leido, no puede borrarse  */
#define A_SUBDIR   0x0008   /* No puede tener puesto el bit A_SOLOLEC   */

/*
** Errores posibles.
*/
#define E_NORMAL      (0)   /* Sin novedad */
#define E_MANINV     (-1)   /* Número de manejador inválido */
#define E_SOLLEC     (-2)   /* El archivo es de sólo lectura */
#define E_ARCSIS     (-3)   /* El archivo es del sistema */
#define E_ARCDIR     (-4)   /* Es un directorio */
#define E_ARCNEX     (-5)   /* El archivo no existe */
#define E_NOMEXI     (-6)   /* Este nombre ya existe */
#define E_ARCUSO     (-7)   /* El archivo esta en uso */
#define E_NOMARC     (-8)   /* El nombre es de un archivo */
#define E_CAMINV     (-9)   /* El camino es inválido */
#define E_ATRINV    (-10)   /* El atributo es inválido */
#define E_DIROCU    (-11)   /* El directorio no esta vacio */
#define E_UNINEX    (-12)   /* La unidad no existe */
#define E_FALUNI    (-13)   /* Falla de la unidad o del medio */
#define E_DIREXI    (-14)   /* El directorio ya existia */
#define E_NOMINV    (-15)   /* Nombre inválido */
#define E_UNILLE    (-16)   /* Unidad llena */
#define E_ARCINV    (-17)   /* La operación es inválida con este archivo */
#define E_POSINV    (-18)   /* La posición es inválida */
#define E_UNIPRO    (-19)   /* Unidad protegida contra escritura */
#define E_DISMIS    (-20)   /* Disco misterioso introducido */

int respuesta;              /* Respuesta para el usuario */
int man1;                   /* Número de archivo en el acceso a INTERFAZ.CMG */
int *apunta;                /* Apuntador de trabajo */
int video, pedido;
int *servicio1, *servicio2; /* Canales de servicio del sistema operativo */
int *bajo_nivel1, *bajo_nivel2;  /* Canales de servicio de MAESTRO */
char *quien;                /* Apuntador al mensaje */

int un, *bl, *sb, *pb;      /* Para trabajo de conversión directorios CD-ROM */

/*
** Parametros de terminal.
*/
int pos_x_cursor, pos_y_cursor, color;
int cop;

/*
** parametros usados por anda_camino()
*/
int unidad_dir;      /* Unidad en la que esta el directorio */
int bloque_dir;      /* Bloque donde esta el directorio */
int sector_dir;      /* Sector del bloque */

int global;          /* Atributos de archivos encontrados por busca_dir() */
int dir_sector;      /* Sector del directorio */

int cursor_activado;

struct mem_dinamica {
  struct mem_dinamica *ap;
  int tam;
};

typedef struct mem_dinamica mem_d;

int mem_libre;
char *limite_mem;

mem_d *ap_libre;

int tabla_sector[MAX_UNID];  /* Indica cual sector tiene de la tabla */
int tabla_modif[MAX_UNID];   /* Indica si la tabla de bloques se ha modif. */
int unidad[MAX_UNID];        /* Indica que cosa es la unidad */
int arch_abiertos[MAX_UNID]; /* Archivos abiertos en cada unidad */
int removible[MAX_UNID];     /* Indica si es removible */
int info_cargada[MAX_UNID];  /* Indica si tiene su información cargada */

struct info {
  int tipo;          /* Tipo de medio */
  int sectores;      /* Número de sectores en la unidad */
  int secs_bloque;   /* Sectores por bloque */
  int bloques;       /* Número de bloques en la unidad */
  int sec_tabla;     /* Sector de la tabla de espacio */
  int tam_tabla;     /* Tamaño en sectores de la tabla de espacio */
  int tam_entradas;  /* Tamaño en bytes de cada entrada en la tabla */
  int dir_raiz;      /* Bloque del directorio raíz */
} info[MAX_UNID];

struct adan {
  int ocupado;       /* Indica si la estructura esta en uso */
  int (*f_lec)();    /* Función que se usa para efectuar la lectura */
  int (*f_esc)();    /* Función que se usa para efectuar la escritura */
  int unidad;        /* Unidad del archivo */
  int arch_expan;    /* Indica si el archivo ha crecido */
  int pos_actual;    /* Posición actual en bytes en el archivo */
  int tam_archivo;   /* Tamaño en bytes del archivo */
  int sec_dir;       /* Sector del directorio que contiene el nombre */
  char *pos_dir;     /* Posición en el sector del directorio */
  int bloq_inicial;  /* Primer bloque del archivo, 0= no asignado */
  int bloq_actual;   /* Bloque actual, 0= no asignado */
  int sec_bloque;    /* Sector que tenemos del bloque */
  int pos_sector;    /* Posición dentro del sector */
  int sec_modif;     /* Indica si se ha modificado algun dato */
  char sector[TAM_SECTOR];  /* Espejo del sector físico */
} adan[MAX_ARCH];

char mensaje[520];   /* Mensajes que se envian al controlador de bajo nivel */

char dir[512];       /* Almacenamiento para un sector de directorio */

char tabla[1536];    /* Almacenamiento para un sector de la tabla de espacio */
                     /* MAX_UNID * 512 */

main()
{
  char buf[32];
  char *destino;
  int *doble;
  char *programa;
  char *pila;
  int temp1, temp2;

  apunta = 0x80000110;
  video = *apunta;
  bajo_nivel1 = 0x80000108;
  bajo_nivel2 = 0x8000010C;
  servicio1 = 0x80000100;
  servicio2 = 0x80000104;
  color = 0x0F;
  cursor_activado = SI;
  forma_cursor(0, 15);
  situa_cursor();
  men("\fSistema Operativo Mexicano de 32 bits.\r\n");
  color = 0x0e;
  men("(c) Copyright 1995-1996 Oscar Toledo G.\r\n");

  unidad[0] = UNIDAD_NORMAL;  /* Disco flexible 1.44 mb. */
  removible[0] = SI;

  unidad[1] = UNIDAD_NORMAL;  /* Disco RAM de 512 kb. */
  removible[1] = NO;

  unidad[2] = UNIDAD_NORMAL;  /* Disco duro de 40 mb. */
  removible[2] = NO;

  unidad[3] = UNIDAD_CDROM;   /* Unidad de CD-ROM */
  removible[3] = SI;

  limite_mem = buf - 640;
  mem_libre = 0;
/*
** este trozo depende de que buf este alineado en una frontera de 32 bits
*/
  doble = buf + 4;
  quien = buf + 3;
  *(doble+3) = "C:/Inicio.man";
  *(doble+4) = NULL;
  temp1 = entero(quien + 5);
  temp2 = entero(quien + 9);
  carga_programa("C:/Interfaz.p", SI, &respuesta, &pila, 1, doble + 3);
  if (respuesta)
    inicia_tarea(respuesta, pila);
  else
    men("\r\nError al iniciar C:/Interfaz.p ");
  while (1) {
    recibir(servicio1, &quien, 4);
    pedido = *quien;
    if(pedido == 3) {           /* Lee un byte desde un archivo */
      respuesta = lee();
    } else if(pedido == 4) {    /* Escribe un byte en un archivo */
      respuesta = escribe();
    } else if(pedido == 1) {    /* Abre un archivo */
      respuesta = abrir();
    } else if(pedido == 2) {    /* Cierra un archivo */
      respuesta = cierra();
    } else if(pedido == 5) {    /* Lee un caracter del teclado */
      respuesta = checa_tecla();
    } else if(pedido == 6) {    /* Escribe un caracter en el video */
      escribe_car(respuesta = *(quien + 1));
    } else if(pedido == 7) {    /* Borra un archivo */
      respuesta = elimina();
    } else if(pedido == 8) {    /* Renombra un archivo */
      respuesta = renombra();
    } else if(pedido == 9) {    /* Proporciona la hora */
      respuesta = p_hora();
    } else if(pedido == 10) {   /* Lee un directorio */
      respuesta = abre_dir();
    } else if(pedido == 11) {   /* Obtiene espacio disponible en unidad */
      respuesta = o_espacio();
    } else if(pedido == 12) {   /* Obtiene el nombre del volumen en la unidad */
      respuesta = n_unidad();
    } else if(pedido == 13) {   /* Crea un directorio */
      respuesta = crea_dir();
    } else if(pedido == 14) {   /* Borra un directorio */
      respuesta = borra_dir();
    } else if(pedido == 15) {   /* Cambia el atributo de un archivo */
      respuesta = atributo();
    } else if(pedido == 16) {   /* Busca posición */
      respuesta = busca_posicion();
    } else if(pedido == 17) {   /* Obtiene posición */
      respuesta = posicion_actual();
    } else if(pedido == 18) {   /* Imprime un caracter */
      respuesta = imprime(*(quien + 1));
    } else if(pedido == 19) {   /* Lee un bloque de bytes */
      respuesta = lee_bloque_bytes();
    } else if(pedido == 20) {   /* Escribir un bloque de bytes */
      respuesta = escribe_bloque_bytes();
    } else if(pedido == 21) {   /* Pide un bloque de memoria */
      respuesta = pide_memoria(entero(quien + 1));
    } else if(pedido == 22) {   /* Libera un bloque de memoria */
      libera_memoria(entero(quien + 1));
      respuesta = 0;
    } else if(pedido == 23) {   /* Ejecución de un programa multi-tarea */
      temp1 = entero(quien + 5);
      temp2 = entero(quien + 9);
      carga_programa(entero(quien + 1), SI, &respuesta, &pila, temp1, temp2);
      if (respuesta)
        inicia_tarea(respuesta, pila);
    } else if(pedido == 24) {   /* Ejecución de un programa */
      temp1 = entero(quien + 5);
      temp2 = entero(quien + 9);
      carga_programa(entero(quien + 1), NO, &respuesta, &pila, temp1, temp2);
      if (respuesta)
        respuesta = pila;
    } else if(pedido == 25) {   /* Cuanta memoria libre */
      respuesta = mem_libre;
    } else {
      respuesta = -1;
    }
    mandar(servicio2, &respuesta, 4);
  }
}

carga_programa(nombre, multitarea, inicio, pila, argc, argv)
  char *nombre;
  int multitarea;
  char **inicio;
  int **pila;
  int argc;
  char **argv;
{
  int encabezado[16];
  int buf[8];
  char **arg;
  char *pos;
  int que_onda, bytes;
  int temp;

  quien = ((char *) buf) + 3;
  buf[1] = nombre;
  buf[2] = "r";
  if ((que_onda = abrir()) == 0) {      /* Abre el archivo para lectura */
    *inicio = NULL;
    return;
  }
  buf[1] = que_onda;
  buf[2] = encabezado;
  buf[3] = 64;
  if (lee_bloque_bytes() != 64          /* Intenta leer el encabezado */
   || encabezado[0] != 0x4542544f       /* Checa los números mágicos */
   || encabezado[1] != 0x96190308) {
    *inicio = NULL;
    return cierra();
  }
  bytes = 24 + 4 * argc;                /* Cuenta los bytes necesarios */
  arg = argv;
  for (temp = 0; temp < argc; temp++)
    bytes += strlen(*arg++) + 1;
  bytes = (bytes + 3) & ~3;
  temp = bytes;
  bytes += encabezado[2] + encabezado[3] +
           encabezado[4] + encabezado[5];
  bytes = (bytes + 3) & ~3;
  if ((*inicio = pide_memoria(bytes)) == NULL)  /* Pide el bloque de memoria */
    return cierra();
  *pila = (*inicio + bytes) - temp;
  buf[1] = que_onda;
  buf[2] = *inicio;
  buf[3] = encabezado[2];
  if (lee_bloque_bytes() != encabezado[2]) {    /* Lee el programa */
    libera_memoria(*inicio);
    *inicio = NULL;
    return cierra();
  }
  buf[1] = que_onda;
  bytes = cierra();                    /* Cierra el archivo */
  *(*pila + 0) = 0;                    /* Prepara la pila */
  *(*pila + 1) = *inicio;
  *(*pila + 2) = argc;
  *(*pila + 3) = *pila + 5;
  *(*pila + 4) = *inicio;
  arg = *pila + 5;                     /* Copia los argumentos */
  pos = arg + (argc + 1);
  while (argc--) {
    *arg++ = pos;
    pos = strcpy(pos, *argv++);
  }
  *arg = NULL;
}

strcpy(destino, origen)
  char *destino, *origen;
{
  while (*destino++ = *origen++) ;
  return destino;
}

strlen(cad)
  char *cad;
{
  char *origen;

  origen = cad;
  while (*cad)
    ++cad;
  return (cad - origen);
}

/*
** Prepara un bloque de control para usar.
*/
usar(ctrl)
  struct adan *ctrl;
{
  ctrl->ocupado = SI;
  ctrl->unidad = -1;
  ctrl->arch_expan = 0;
  ctrl->pos_actual = 0;
  ctrl->tam_archivo = 0;
  ctrl->sec_dir = 0;
  ctrl->pos_dir = 0;
  ctrl->bloq_inicial = 0;
  ctrl->bloq_actual = 0;
  ctrl->sec_bloque = 0;
  ctrl->pos_sector = 0;
  ctrl->sec_modif = 0;
}

/*
** Abre un directorio para leer.
*/
abre_dir()
{
  int cual_ctrl;
  struct adan *ctrl;

  if (dispositivo_especial(entero(quien + 1)))
    return 0;
  if (anda_camino(entero(quien + 1), SI))
    return 0;
  if ((cual_ctrl = busca_ctrl()) == -1)
    return 0;
  ctrl = &adan[cual_ctrl];
  ++arch_abiertos[unidad_dir];
  usar(ctrl);
  ctrl->unidad = unidad_dir;
  ctrl->bloq_inicial =
  ctrl->bloq_actual = bloque_dir;
  ctrl->pos_sector = -1;
  ctrl->f_lec = f_lec_dir;
  ctrl->f_esc = f_error;
  if (unidad[ctrl->unidad] == UNIDAD_CDROM)
    inicia_dir_cdrom(&ctrl->unidad, &ctrl->bloq_inicial, &ctrl->bloq_actual,
                     &ctrl->sec_bloque, &ctrl->pos_dir, &ctrl->tam_archivo);
  return (cual_ctrl + 1);
}

/*
** Determina si una unidad esta protegida.
*/
unidad_protegida(u)
  int u;
{
  return (unidad[u] == UNIDAD_CDROM);
}

/*
** Cambia la fecha, hora y atributos de un archivo.
*/
atributo()
{
  char nombre[TAM_NOMBRE];
  char *pos_dir;
  int longitud, atr;

  if (separa_nombre(entero(quien + 1), nombre))
    return E_NOMINV;
  if (anda_camino(entero(quien + 1), NO))
    return E_CAMINV;
  if (unidad_protegida(unidad_dir))
    return E_UNIPRO;
  if ((pos_dir = busca_dir(nombre, B_TODOS)) == NULL)
    return E_ARCNEX;
  memcpy(pos_dir + 48, quien + 5, 8);
  graba_dir(dir_sector);
  return E_NORMAL;
}

/*
** Crea un nuevo directorio.
*/
crea_dir()
{
  char nombre[TAM_NOMBRE];
  char *pos_dir;
  int bloque;

  if (separa_nombre(entero(quien + 1), nombre))
    return E_NOMINV;
  if (anda_camino(entero(quien + 1), NO))
    return E_CAMINV;
  if (unidad_protegida(unidad_dir))
    return E_UNIPRO;
  if (busca_dir(nombre, B_TODOS) != NULL)
    return (global & A_SUBDIR) ? E_DIREXI : E_NOMARC;
  if ((bloque = nuevo_bloque(unidad_dir, 0)) == -1)
    return E_UNILLE;
  escribe_tabla(bloque, unidad_dir, FINAL);
/*
** Nunca vaya a poner la sentencia de arriba (escribe_tabla)
** debajo de este if, crea() podria (a la hora de ampliar un dir.)
** enlazar ambos directorios en el mismo bloque.
*/
  if ((pos_dir = crea(nombre, A_SUBDIR)) == NULL) {
    escribe_tabla(bloque, unidad_dir, 0);
    vacia_tabla(unidad_dir);
    asegura_tabla(unidad_dir);
    return E_UNILLE;
  }
  limpia_bloque(bloque, unidad_dir);
  escribe_entero(pos_dir + 56, bloque);
  vacia_tabla(unidad_dir);
  asegura_tabla(unidad_dir);
  graba_dir(dir_sector);
  return E_NORMAL;
}

/*
** Borra un directorio.
*/
borra_dir()
{
  char nombre[TAM_NOMBRE];
  char *pos_dir;

  if (separa_nombre(entero(quien + 1), nombre))
    return E_NOMINV;
  if (anda_camino(entero(quien + 1), SI))
    return E_CAMINV;
  if (unidad_protegida(unidad_dir))
    return E_UNIPRO;
  if (checa_dir_vacio())
    return E_DIROCU;
  if (anda_camino(entero(quien + 1), NO))
    return E_CAMINV;
  if ((pos_dir = busca_dir(nombre, B_SUBDIR)) == NULL)
    return E_CAMINV;
  borra(pos_dir);
  asegura_tabla(unidad_dir);
  return E_NORMAL;
}

/*
** Pone el apuntador de archivo a una nueva posición
*/
busca_posicion()
{
  int bloque, tam_bloque;
  int posicion, nueva_posicion;
  int cual_ctrl;
  struct adan *ctrl;

  cual_ctrl = *(quien + 1) - 1;
  if (cual_ctrl < 0 || cual_ctrl >= MAX_ARCH)
    return E_MANINV;
  ctrl = &adan[cual_ctrl];
  if (!ctrl->ocupado)
    return E_MANINV;
  if (ctrl->f_lec != f_lec_disco
   && ctrl->f_esc != f_esc_disco)
    return E_ARCINV;
  nueva_posicion = entero(quien + 5);      /* Posición desde el comienzo */
  if (*(quien + 9) == 1)
    nueva_posicion += ctrl->pos_actual;    /* Posición desde aquí */
  else if (*(quien + 9) == 2)
    nueva_posicion += ctrl->tam_archivo;   /* Posición desde el final */
  if (nueva_posicion == ctrl->pos_actual)
    return E_NORMAL;
  if (nueva_posicion > ctrl->tam_archivo   /* Si no esta escribiendo o */
   && ctrl->f_esc != f_esc_disco)          /* actualizando el archivo, no */
    return E_POSINV;                       /* puede ampliarlo. */
  if (nueva_posicion < 0)                  /* Posición negativa */
    return E_POSINV;
  if (ctrl->sec_modif)
    vacia_sec(ctrl);
  tam_bloque = info[ctrl->unidad].secs_bloque * TAM_SECTOR;
  bloque = 0;
  posicion = 0;
  while (nueva_posicion != posicion) {
    if (bloque == 0) {
      bloque = ctrl->bloq_inicial;
      if (bloque == 0) {
        if ((bloque = nuevo_bloque(ctrl->unidad, 0)) == -1)
          return E_UNILLE;
        ctrl->bloq_inicial = bloque;
        escribe_tabla(bloque, ctrl->unidad, FINAL);
      }
    } else {
      ctrl->bloq_actual = bloque;
      if ((bloque = lee_tabla(bloque, ctrl->unidad)) == FINAL) {
        if ((bloque = nuevo_bloque(ctrl->unidad, ctrl->bloq_actual)) == -1)
          return E_UNILLE;
        escribe_tabla(ctrl->bloq_actual, ctrl->unidad, bloque);
        escribe_tabla(bloque, ctrl->unidad, FINAL);
      }
    }
    if (nueva_posicion <= posicion + tam_bloque)
      break;
    posicion += tam_bloque;
  }
  ctrl->bloq_actual = bloque;
  if (bloque) {
    ctrl->sec_bloque = (nueva_posicion - posicion) / TAM_SECTOR;
    ctrl->pos_sector = (nueva_posicion - posicion) % TAM_SECTOR;
    if(ctrl->sec_bloque == info[ctrl->unidad].secs_bloque) {
      --(ctrl->sec_bloque);
      ctrl->pos_sector = 512;
    } else
      lee_sec(ctrl);
  }
  ctrl->pos_actual = nueva_posicion;
  if (ctrl->pos_actual > ctrl->tam_archivo) {
    ctrl->tam_archivo = ctrl->pos_actual;
    ctrl->arch_expan = SI;
  }
  return E_NORMAL;
}

/*
** Obtiene la posición actual en el archivo.
*/
posicion_actual()
{
  int *donde;
  int cual_ctrl;
  struct adan *ctrl;

  cual_ctrl = *(quien + 1) - 1;
  if (cual_ctrl < 0 || cual_ctrl >= MAX_ARCH)
    return -1;
  ctrl = &adan[cual_ctrl];
  if (!ctrl->ocupado)
    return -1;
  return ctrl->pos_actual;
}

/*
** Obtiene el espacio disponible en la unidad.
*/
o_espacio()
{
  int bloques, tam_bloques;
  int cuantos;
  int contador;
  int unid;
  int *donde;

  separa_unidad(entero(quien + 1), &unid);
  if (existe(unid) == 0)
    return E_UNINEX;
  if (carga_info(unid) == 0)
    return E_DISMIS;
  donde = entero(quien + 5);
  if (unidad[unid] == UNIDAD_CDROM) { /* En un CD-ROM no hay nada disponible */
    *donde = 0;
    return E_NORMAL;
  }
  tam_bloques = info[unid].secs_bloque * TAM_SECTOR;
  bloques = info[unid].bloques;
  cuantos = 0;
  for (contador = 0; contador < bloques; contador++)
    if (lee_tabla(contador, unid) == 0)
      ++cuantos;
  asegura_tabla(unid);
  *donde = cuantos * tam_bloques;
  return E_NORMAL;
}

/*
** Obtiene la etiqueta de una unidad
*/
n_unidad()
{
  int unid;

  separa_unidad(entero(quien + 1), &unid);
  if (existe(unid) == 0)
    return E_UNINEX;
  if (carga_info(unid) == 0)
    return E_DISMIS;
  if (unidad[unid] == UNIDAD_CDROM) {
    lee_sector(unid, info[unid].tam_entradas);
    mensaje[72] = 0;
    memcpy(entero(quien + 5), mensaje + 41, 32);
  } else {
    lee_sector(unid, 0);
    memcpy(entero(quien + 5), mensaje + 49, 32);
  }
  return E_NORMAL;
}

/*
** Abre un archivo o dispositivo especial.
*/
abrir()
{
  char nombre[TAM_NOMBRE];
  char modo[3];
  char *separa;
  int atr, dispositivo;
  int cual_ctrl;
  struct adan *ctrl;
  char *pos_dir;

  separa = entero(quien + 5);
  if (dispositivo = dispositivo_especial(entero(quien + 1))) {
    if ((cual_ctrl = busca_ctrl()) == -1)
      return 0;
    ctrl = &adan[cual_ctrl];
    usar(ctrl);
    switch (dispositivo) {
      case 1: ctrl->f_lec = f_lec_consola;
              ctrl->f_esc = f_esc_consola;
              break;
      case 2: ctrl->f_lec = f_error;
              ctrl->f_esc = f_esc_impresora;
              break;
      case 3: ctrl->f_lec = f_lec_com;
              ctrl->f_esc = f_esc_com;
              break;
      case 4: regresa_cinta();
              ctrl->f_lec = f_lec_cinta;
              ctrl->f_esc = f_esc_cinta;
              break;
      case 5: ctrl->f_lec = f_error;
              ctrl->f_esc = f_esc_nulo;
              break;
    }
    return (cual_ctrl + 1);
  }
  if (separa_nombre(entero(quien + 1), nombre))
    return 0;
  if (anda_camino(entero(quien + 1), NO))
    return 0;
  if (*separa == 'r' || *separa == 'a') {
    if ((cual_ctrl = busca_ctrl()) == -1)
      return 0;
    if ((pos_dir = busca_dir(nombre,
                    B_NORMAL | B_OCULTO | B_SOLOLEC | B_SISTEMA)) == NULL) {
      if (*separa != 'a')
        return 0;
    } else {
      ++arch_abiertos[unidad_dir];
      ctrl = &adan[cual_ctrl];
      usar(ctrl);
      ctrl->unidad = unidad_dir;
      ctrl->sec_dir = dir_sector;
      ctrl->pos_dir = pos_dir;
      ctrl->tam_archivo = entero(pos_dir + 60);
      ctrl->bloq_inicial = entero(pos_dir + 56);
      if (*separa == 'r' || *(separa + 1) == '+')
        ctrl->f_lec = f_lec_disco;
      else
        ctrl->f_lec = f_error;
      if (*separa == 'a' || *(separa + 1) == '+')
        ctrl->f_esc = f_esc_disco;
      else
        ctrl->f_esc = f_error;
      if (*separa == 'a') {
        escribe_entero(nombre + 1, cual_ctrl + 1);
        escribe_entero(nombre + 5, 0);
        escribe_entero(nombre + 9, 2);
        quien = nombre;
        busca_posicion();
      }
      return (cual_ctrl + 1);
    }
  }
  if (*separa == 'w' || *separa == 'a') {
    if (unidad_protegida(unidad_dir))
      return 0;
    if ((cual_ctrl = busca_ctrl()) == -1)
      return 0;
    if ((pos_dir = busca_dir(nombre,
                    B_NORMAL | B_OCULTO | B_SISTEMA)) != NULL) {
      atr = entero(pos_dir + 48);
      borra(pos_dir);
    } else
      atr = 0;
    if ((pos_dir = crea(nombre, atr)) == NULL)
      return 0;
    ++arch_abiertos[unidad_dir];
    ctrl = &adan[cual_ctrl];
    usar(ctrl);
    ctrl->unidad = unidad_dir;
    ctrl->sec_dir = dir_sector;
    ctrl->pos_dir = pos_dir;
    if (*(separa + 1) == '+')
      ctrl->f_lec = f_lec_disco;
    else
      ctrl->f_lec = f_error;
    ctrl->f_esc = f_esc_disco;
    return (cual_ctrl + 1);
  }
  return 0;
}

/*
** Checa si la cadena es el nombre de un dispositivo especial.
*/
dispositivo_especial(entrada)
  char *entrada;
{
  if(entrada[3] != ':' || entrada[4])
    return 0;
  if(checa_cadena(entrada, "CON"))
    return 1;
  if(checa_cadena(entrada, "IMP"))
    return 2;
  if(checa_cadena(entrada, "COM"))
    return 3;
  if(checa_cadena(entrada, "CIN"))
    return 4;
  if(checa_cadena(entrada, "NUL"))
    return 5;
  return 0;
}

/*
** Cierra un archivo.
*/
cierra()
{
  char *pos_dir;
  int cual_ctrl;
  struct adan *ctrl;

  cual_ctrl = *(quien + 1) - 1;
  if (cual_ctrl < 0 || cual_ctrl >= MAX_ARCH)
    return E_MANINV;
  ctrl = &adan[cual_ctrl];
  if (!ctrl->ocupado)
    return E_MANINV;
  if (ctrl->sec_modif)                 /* Vacia sectores modificados */
    vacia_sec(ctrl);
  if (ctrl->arch_expan) {              /* Si el tamaño del archivo ha */
    unidad_dir = ctrl->unidad;         /* cambiado, actualiza el directorio */
    lee_dir(ctrl->sec_dir);
    escribe_entero(ctrl->pos_dir + 56, ctrl->bloq_inicial);
    escribe_entero(ctrl->pos_dir + 60, ctrl->tam_archivo);
    graba_dir(ctrl->sec_dir);
  }
  if (ctrl->unidad != -1) {
    vacia_tabla(ctrl->unidad);
    if(--arch_abiertos[ctrl->unidad] == 0)
      asegura_tabla(ctrl->unidad);
  }
  ctrl->ocupado = NO;
  return E_NORMAL;
}

/*
** Lee un byte de un archivo.
*/
lee()
{
  int cual_ctrl;
  struct adan *ctrl;

  cual_ctrl = *(quien + 1) - 1;
  if (cual_ctrl < 0 || cual_ctrl >= MAX_ARCH)
    return -1;
  ctrl = &adan[cual_ctrl];
  if (!ctrl->ocupado)
    return -1;
  return (*ctrl->f_lec)(ctrl);
}

/*
** Lee un bloque de bytes de un archivo.
*/
lee_bloque_bytes()
{
  int bytes, max_bytes, byte;
  char *donde;
  int cual_ctrl;
  struct adan *ctrl;

  cual_ctrl = *(quien + 1) - 1;
  if (cual_ctrl < 0 || cual_ctrl >= MAX_ARCH)
    return 0;
  ctrl = &adan[cual_ctrl];
  if (!ctrl->ocupado)
    return 0;
  max_bytes = entero(quien + 9);
  donde = entero(quien + 5);
  bytes = 0;
  while (bytes < max_bytes) {
    if ((byte = (*ctrl->f_lec)(ctrl)) == -1)
      break;
    *donde++ = byte;
    ++bytes;
  }
  return bytes;
}

/*
** Función de error.
*/
f_error()
{
  return -1;
}

/*
** Función para leer la consola con eco.
*/
f_lec_consola(ctrl)
  struct adan *ctrl;
{
  int car;

  while (1) {
    if (car = checa_tecla())
      break;
  }
  escribe_car(car);
  if (car == 0x1a)
    car = -1;
  return car;
}

/*
** Función para escribir en la consola.
*/
f_esc_consola(ctrl)
  struct adan *ctrl;
{
  escribe_car(*(quien + 1));
  return *(quien + 1);
}

/*
** Función para escribir en la impresora.
*/
f_esc_impresora(ctrl)
  struct adan *ctrl;
{
  imprime(*(quien + 1));
  return *(quien + 1);
}

/*
** Función para leer comunicaciones.
*/
f_lec_com(ctrl)
  struct adan *ctrl;
{
  return lee_com();
}

/*
** Función para escribir comunicaciones.
*/
f_esc_com(ctrl)
  struct adan *ctrl;
{
  escribe_com(*(quien + 1));
  return *(quien + 1);
}

/*
** Función para leer la cinta.
*/
f_lec_cinta(ctrl)
  struct adan *ctrl;
{
  return lee_cin();
}

/*
** Función para escribir la cinta.
*/
f_esc_cinta(ctrl)
  struct adan *ctrl;
{
  escribe_cin(*(quien + 1));
  return *(quien + 1);
}

/*
** Función para escribir en un dispositivo nulo.
*/
f_esc_nulo(ctrl)
  struct adan *ctrl;
{
  return *(quien + 1);
}

/*
** Lee un byte de un archivo en disco.
*/
f_lec_disco(ctrl)
  struct adan *ctrl;
{
  if (ctrl->pos_actual == ctrl->tam_archivo)  /* No hay más para leer */
    return -1;
  if (ctrl->bloq_actual == 0) {               /* Busca el primer bloque */
    ctrl->bloq_actual = ctrl->bloq_inicial;
    ctrl->sec_bloque = 0;
    ctrl->pos_sector = 0;
    lee_sec(ctrl);
  }
  if (ctrl->pos_sector == TAM_SECTOR) {       /* Pide más bloques */
    if (ctrl->sec_modif)
      vacia_sec(ctrl);
    if (++ctrl->sec_bloque == info[ctrl->unidad].secs_bloque) {
      ctrl->sec_bloque = 0;
      ctrl->bloq_actual = lee_tabla(ctrl->bloq_actual, ctrl->unidad);
    }
    ctrl->pos_sector = 0;
    lee_sec(ctrl);
  }
  ctrl->pos_actual++;
  return (ctrl->sector[ctrl->pos_sector++]);  /* Proporciona un byte */
}

/*
** Lee un byte de un directorio.
*/
f_lec_dir(ctrl)
  struct adan *ctrl;
{
  if (ctrl->pos_sector == -1) {
    if (unidad[ctrl->unidad] == UNIDAD_CDROM)
      trabaja_dir_cdrom(ctrl->unidad, ctrl->bloq_inicial, &ctrl->bloq_actual,
                        &ctrl->sec_bloque, &ctrl->pos_dir, ctrl->tam_archivo,
                        ctrl->sector);
    else
      lee_sec(ctrl);
    ctrl->pos_sector = 0;
  }
  if (ctrl->pos_sector == TAM_SECTOR) {
    ctrl->pos_sector = 0;
    if (unidad[ctrl->unidad] == UNIDAD_CDROM)
      trabaja_dir_cdrom(ctrl->unidad, ctrl->bloq_inicial, &ctrl->bloq_actual,
                        &ctrl->sec_bloque, &ctrl->pos_dir, ctrl->tam_archivo,
                        ctrl->sector);
    else {
      if (++ctrl->sec_bloque == info[ctrl->unidad].secs_bloque) {
        ctrl->sec_bloque = 0;
        if (lee_tabla(ctrl->bloq_actual, ctrl->unidad) == -1)
          return -1;
        ctrl->bloq_actual = lee_tabla(ctrl->bloq_actual, ctrl->unidad);
      }
      lee_sec(ctrl);
    }
  }
  ctrl->pos_actual++;
  return ctrl->sector[ctrl->pos_sector++];
}

/*
** Escribe un byte en un archivo.
*/
escribe()
{
  int cual_ctrl;
  struct adan *ctrl;

  cual_ctrl = *(quien + 5) - 1;
  if (cual_ctrl < 0 || cual_ctrl >= MAX_ARCH)
    return -1;
  ctrl = &adan[cual_ctrl];
  if (!ctrl->ocupado)
    return -1;
  return (*ctrl->f_esc)(ctrl);
}

/*
** Escribe un bloque en un archivo.
*/
escribe_bloque_bytes()
{
  int max_bytes, bytes;
  char *donde;
  int cual_ctrl;
  struct adan *ctrl;

  cual_ctrl = *(quien + 1) - 1;
  if (cual_ctrl < 0 || cual_ctrl >= MAX_ARCH)
    return 0;
  ctrl = &adan[cual_ctrl];
  if (!ctrl->ocupado)
    return 0;
  max_bytes = entero(quien + 9);
  donde = entero(quien + 5);
  for (bytes = 0; bytes < max_bytes; ++bytes) {
    *(quien + 1) = *donde++;
    if ((*ctrl->f_esc)(ctrl) == -1)
      break;
  }
  return bytes;
}

/*
** Escribe un byte en un archivo de disco.
*/
f_esc_disco(ctrl)
  struct adan *ctrl;
{
  int bloque;

  if (ctrl->pos_sector == TAM_SECTOR) {   /* ¿ Llenamos un sector ? */
    if (ctrl->sec_modif)                  /* Vacia el sector */
      vacia_sec(ctrl);
    ctrl->pos_sector = 0;
    if (++ctrl->sec_bloque == info[ctrl->unidad].secs_bloque) {
      if (lee_tabla(ctrl->bloq_actual, ctrl->unidad) == FINAL) {
        if ((bloque = nuevo_bloque(ctrl->unidad,
                                   ctrl->bloq_actual + 1)) == -1) {
          ctrl->pos_sector = TAM_SECTOR;
          --ctrl->sec_bloque;
          return -1;
        }
        escribe_tabla(ctrl->bloq_actual, ctrl->unidad, bloque);
        ctrl->bloq_actual = bloque;
        escribe_tabla(bloque, ctrl->unidad, FINAL);
        ctrl->sec_bloque = 0;
      } else {
        ctrl->bloq_actual = lee_tabla(ctrl->bloq_actual, ctrl->unidad);
        ctrl->sec_bloque = 0;
        lee_sec(ctrl);
      }
    }
  } else if (ctrl->bloq_actual == 0) {
    ctrl->pos_sector = 0;
    ctrl->sec_bloque = 0;
    ctrl->sec_modif = NO;
    if (ctrl->bloq_inicial) {
      ctrl->bloq_actual = ctrl->bloq_inicial;
      lee_sec(ctrl);
    } else {
      if ((bloque = nuevo_bloque(ctrl->unidad, 0)) == -1)
        return -1;
      ctrl->bloq_inicial = ctrl->bloq_actual = bloque;
      escribe_tabla(bloque, ctrl->unidad, FINAL);
    }
  }
  if (ctrl->pos_actual == ctrl->tam_archivo) {  /* ¿ Estaba al limite */
    ctrl->arch_expan = SI;                      /* Expande el archivo */
    ctrl->tam_archivo++;
  }
  ctrl->pos_actual++;
  ctrl->sec_modif = SI;
  return (ctrl->sector[ctrl->pos_sector++] = *(quien + 1));
}

/*
** Busca un bloque de control desocupado.
*/
busca_ctrl()
{
  int conteo;

  conteo = 0;
  while (conteo < MAX_ARCH) {
    if (adan[conteo].ocupado == NO)
      return conteo;
    ++conteo;
  }
  return -1;
}

/*
** Obtiene la hora.
*/
p_hora()
{
  char *apunta;
  int seg, min, hor, fec, mes, an;

  apunta = entero(quien + 1);
  obtener_hora(&seg, &min, &hor, &fec, &mes, &an);
  *apunta++ = an;
  *apunta++ = mes;
  *apunta++ = fec;
  *apunta++ = hor;
  *apunta++ = min;
  *apunta++ = seg;
  return 0;
}

/*
** Conversión minúsculas a mayúsculas.
*/
toupper(car)
  int car;
{
  if (((car >= 'a' ) && (car <= 'z' ))
   || ((car >= 0xe0) && (car <= 0xff)))
    return car - 32;
  return car;
}

/*
** Forma un nombre, se asegura de que no sea ilegal. (Un nombre de un
** dispositivo especial, un directorio, una unidad, etc.)
*/
forma(nombre, control)
  char *nombre, *control;
{
  int conteo;
  char *pos;

  if (*nombre == 0 || dispositivo_especial(nombre))
    return 1;
  if (isalpha(*nombre) && *(nombre + 1) == ':')
    return 1;
  memset(control, 0, TAM_NOMBRE);
  pos = control;
  while (*nombre) {
    if (pos != control + (TAM_NOMBRE - 1)) {
      if ((*pos++ = *nombre) == '/')
        return 1;
    }
    ++nombre;
  }
  if (control[0] == '.') {
    if (control[1] == '.' && control[2] == 0)
      return 1;
    if (control[1] == 0)
      return 1;
  }
  return 0;
}

/*
** Busca un nombre en un directorio.
*/
busca_dir(nombre, atributos)
  char *nombre; int atributos;
{
  char nom[TAM_NOMBRE];
  int conteo;
  int pos, vale;
  int bloque, sector, posicion, tam;
  int bloque_actual;

  global = 0;
  bloque = bloque_dir;
  sector = posicion = 0;
  if (forma(nombre, nom))
    return NULL;
  if (unidad[unidad_dir] == UNIDAD_CDROM)
    inicia_dir_cdrom(&unidad_dir, &bloque, &bloque_actual, &sector,
                     &posicion, &tam);
  while (1) {
    if (unidad[unidad_dir] == UNIDAD_CDROM)
      trabaja_dir_cdrom(unidad_dir, bloque, &bloque_actual, &sector,
                        &posicion, tam, dir);
    else
      lee_dir(info[unidad_dir].secs_bloque * bloque + sector);
    pos = 0;
    while (pos < 512) {
      if (dir[pos] == 0)
        return NULL;
      if (dir[pos] != 0x80) {
        conteo = 0;
        while (conteo < TAM_NOMBRE) {
          if (toupper(nom[conteo]) != toupper(dir[pos+conteo])) {
            conteo = TAM_NOMBRE;
            break;
          }
          if (nom[conteo] == 0)
            break;
          ++conteo;
        }
        if (conteo < TAM_NOMBRE) {
          vale = SI;
          global = dir[pos + 48];
          if (global & A_OCULTO)
            if ((atributos & B_OCULTO) == 0)
              vale = NO;
          if (global & A_SISTEMA)
            if ((atributos & B_SISTEMA) == 0)
              vale = NO;
          if (global & A_SOLOLEC)
            if ((atributos & B_SOLOLEC) == 0)
              vale = NO;
          if (atributos & B_NORMAL) {
            if (global & A_SUBDIR)
              if ((atributos & B_SUBDIR) == 0)
                vale = NO;
          } else {
            if (atributos & B_SUBDIR)
              if ((global & A_SUBDIR) == 0)
                vale = NO;
          }
          if (vale)
            return dir + pos;
          return NULL;
        }
      }
      pos += 64;
    }
    if (unidad[unidad_dir] == UNIDAD_CDROM)
      continue;
    if (++sector == info[unidad_dir].secs_bloque) {
      sector = 0;
      bloque = lee_tabla(bloque, unidad_dir);
    }
  }
}

/*
** Prepara un directorio de CD-ROM para lectura.
*/
inicia_dir_cdrom(unid, bini, bloq, sbuf, pblo, tbyt)
  int *unid, *bini, *bloq, *sbuf, *pblo, *tbyt;
{
  lee_sector(*unid, *bini * 4);
  *bloq = *bini;
  *sbuf = 0;
  *pblo = 0;
  *tbyt = entero(mensaje + 11);
}

/*
** Convierte un directorio de CD-ROM a un formato comprensible.
*/
trabaja_dir_cdrom(unid, bini, bloq, sbuf, pblo, tbyt, buff)
  int unid, bini, *bloq, *sbuf, *pblo, tbyt;
  char *buff;
{
  int cuantos, llena, bytes;
  int posicion, tam, l2;
  int fecha, tiempo, atributo, longitud, byte;
  char nombre[64];

  un = unid;
  bl = bloq;
  pb = pblo;
  sb = sbuf;
  lee_sector(unid, *bloq * 4 + *sbuf);
  for (cuantos = 0; cuantos < 512; ) {
    if (((*bloq - bini) * 2048) >= tbyt) {    /* ¿ Estamos al final ? */
      memset(buff + cuantos, 0, 64);
      cuantos += 64;
    } else if (*sbuf == 4 || (*(mensaje + 1 + *pblo) == 0)) { /* ¿ Más ? */
      ++*bloq;
      *sbuf = 0;
      *pblo = 0;
      lee_sector(unid, *bloq * 4);
    } else {
      tam = lee_byte() - 33;       /* Tamaño total de la entrada del dir. */
      memset(nombre, 0, 64);       /* Prepara una entrada de directorio */
      lee_byte();
      nombre[56] = lee_byte();     /* Obtiene bloque del archivo */
      nombre[57] = lee_byte();
      nombre[58] = lee_byte();
      nombre[59] = lee_byte();
      for (llena = 0; llena < 4; ++llena)
        lee_byte();
      nombre[60] = lee_byte();     /* Obtiene tamaño del archivo */
      nombre[61] = lee_byte();
      nombre[62] = lee_byte();
      nombre[63] = lee_byte();
      for (llena = 0; llena < 4; ++llena)
        lee_byte();
      fecha = lee_byte() - 70;     /* Transforma la fecha */
      if (fecha < 0) {             /* Detecta fechas inválidas */
        fecha = 0;
        tiempo = 0;
        for (llena = 0; llena < 5; ++llena)
          lee_byte();
      } else {
        fecha <<= 9;
        fecha |= lee_byte() << 5;
        fecha |= lee_byte();
        tiempo = lee_byte() << 11;
        tiempo |= lee_byte() << 5;
        tiempo |= lee_byte() >> 2;
      }
      nombre[52] = tiempo;
      nombre[53] = tiempo >> 8;
      nombre[54] = fecha;
      nombre[55] = fecha >> 8;
      if (info[unid].tam_tabla) {  /* Esto cambia de High-Sierra a ISO-9660 */
        lee_byte();
        atributo = lee_byte();
      } else {
        atributo = lee_byte();
        lee_byte();
      }
      for (llena = 0; llena < 6; ++llena)
        lee_byte();
      posicion = 0;
      tam -= (longitud = lee_byte());  /* Se prepara para leer el nombre */
      while (longitud--) {
        byte = lee_byte();
        if (byte == 0 || byte == 1)
          atributo |= 4;
        if (byte == ';')               /* Un ; indica versión, ignorar */
          posicion = TAM_NOMBRE - 1;
        if (posicion < TAM_NOMBRE - 1)
          nombre[posicion++] = byte;
      }
      if (tam && (*pb & 1)) {          /* Alineación a 16 bits */
        tam--;
        lee_byte();
      }
      while (tam >= 4) {               /* Si queda algo, puede ser Rockridge */
        byte = lee_byte();
        byte |= lee_byte() << 8;
        l2 = lee_byte() - 4;
        lee_byte();
        tam -= 4;
        if (l2 <= 0)
          break;
        tam -= l2;
        if (byte == 0x4d4e) {          /* Checa si es NM (name) */
          lee_byte();
          memset(nombre, 0, TAM_NOMBRE);
          posicion = 0;
          while (--l2) {               /* Procesa nombre Rockridge */
            byte = lee_byte();
            if (posicion < TAM_NOMBRE - 1)
              nombre[posicion++] = byte;
          }
        } else {                       /* Ignora otros datos */
          while (l2--)
            lee_byte();
        }
      }
      while (tam--)                    /* Se come los bytes extras */
        byte = lee_byte();
      if (!(atributo & 4)) {           /* Checa que no haya sido archivo */
        if (atributo & 2)              /* extendido */
          nombre[48] = 8;
        memcpy(buff + cuantos, nombre, 64);
        cuantos += 64;
      }
    }
  }
  unid = un;
  bloq = bl;
  pblo = pb;
  sbuf = sb;
}

/*
** Lee un byte de un directorio de CD-ROM.
*/
lee_byte()
{
  int byte;

  byte = *(mensaje + 1 + *pb);
  if (++*pb == TAM_SECTOR) {
    *pb = 0;
    lee_sector(un, *bl * 4 + ++*sb);
  }
  return byte;
}

/*
** Checa si un directorio está vacio.
** Usado por la función borra_dir.
*/
checa_dir_vacio()
{
  int pos;
  int bloque, sector;

  bloque = bloque_dir;
  sector = 0;
  while (1) {
    lee_dir(info[unidad_dir].secs_bloque * bloque + sector);
    pos = 0;
    while(pos < 512) {
      if (dir[pos] == 0)
        return 0;
      if (dir[pos] != 0x80)
        return 1;
      pos += 64;
    }
    if (++sector == info[unidad_dir].secs_bloque) {
      sector = 0;
      bloque = lee_tabla(bloque, unidad_dir);
    }
  }
}

/*
** Busca espacio en un directorio para agregar un nuevo nombre.
*/
busca_esp()
{
  int bloque, sector, sig;
  int pos;

  bloque = bloque_dir;
  sector = 0;
  while (1) {
    lee_dir(info[unidad_dir].secs_bloque * bloque + sector);
    pos = 0;
    while (pos < 512) {
      if (dir[pos] == 0) {
        if (pos == 448) {
          if (sector + 1 == info[unidad_dir].secs_bloque) {
            if ((sig = nuevo_bloque(unidad_dir, bloque + 1)) == -1)
              return NULL;
            escribe_tabla(bloque, unidad_dir, sig);
            escribe_tabla(sig, unidad_dir, FINAL);
            limpia_bloque(sig, unidad_dir);
            vacia_tabla(unidad_dir);
          }
        }
        return dir + pos;
      }
      if (dir[pos] == 0x80)
        return dir + pos;
      pos += 64;
    }
    ++sector;
    if (sector == info[unidad_dir].secs_bloque) {
      sector = 0;
      bloque = lee_tabla(bloque, unidad_dir);
    }
  }
}

/*
** Elimina una entrada del directorio, también borra la cadena en la tabla
** de bloques.
*/
borra(pos_dir)
  char *pos_dir;
{
  int bloque, sig;

  *pos_dir = 0x80;  /* borrado */
  bloque = entero(pos_dir + 56);
  if (bloque) {
    while(1) {
      sig = lee_tabla(bloque, unidad_dir);
      escribe_tabla(bloque, unidad_dir, 0);
      if (sig == FINAL || sig == 0)
        break;
      bloque = sig;
    }
  }
  graba_dir(dir_sector);
  vacia_tabla(unidad_dir);
}

/*
** Crea un nuevo archivo, con tamaño 0.
*/
crea(nombre, atr)
  char *nombre; int atr;
{
  char *pos_dir, *pos_fin;
  char nom[TAM_NOMBRE];
  int conteo;
  int tiempo;
  int fecha;
  int seg, min, hor, fec, mes, an;

  if ((pos_dir = busca_dir(nombre, B_TODOS)) != NULL)
    return NULL;
  pos_fin = pos_dir = busca_esp();
  if (pos_dir == NULL)
    return NULL;
  if (forma(nombre, nom))
    return NULL;
  conteo = 0;
  while (conteo < TAM_NOMBRE)
    *pos_dir++ = nom[conteo++];
  conteo = 0;
  while (conteo++ < 16)
    *pos_dir++ = 0;
  escribe_entero(pos_dir, atr);
  pos_dir += 4;
  obtener_hora(&seg, &min, &hor, &fec, &mes, &an);
  tiempo = seg / 2;
  tiempo = tiempo | (min << 5);
  tiempo = tiempo | (hor << 11);
  *pos_dir++ = tiempo;
  *pos_dir++ = tiempo >> 8;
  fecha = fec;
  fecha = fecha | (mes << 5);
  fecha = fecha | ((an - 70) << 9);
  *pos_dir++ = fecha;
  *pos_dir++ = fecha >> 8;
  conteo = 0;
  while (conteo++ < 8)
    *pos_dir++ = 0;
  graba_dir(dir_sector);
  return pos_fin;
}

/*
** Elimina un archivo.
*/
elimina()
{
  char nombre[TAM_NOMBRE];
  char *separa, *pos_dir;
  int longitud;

  if (separa_nombre(entero(quien + 1), nombre))
    return E_NOMINV;
  if (anda_camino(entero(quien + 1), NO))
    return E_CAMINV;
  if (unidad_protegida(unidad_dir))
    return E_UNIPRO;
  if ((pos_dir = busca_dir(nombre, B_NORMAL)) != NULL) {
    longitud = 0;
    while (longitud < MAX_ARCH) {
      if (adan[longitud].ocupado)
        if (adan[longitud].unidad == unidad_dir)
          if (adan[longitud].sec_dir == dir_sector)
            if (adan[longitud].pos_dir == pos_dir)
              return E_ARCUSO;
      ++longitud;
    }
    borra(pos_dir);
    asegura_tabla(unidad_dir);
    return E_NORMAL;
  }
  if (global & A_SOLOLEC)
    return E_SOLLEC;
  if (global & A_SISTEMA)
    return E_ARCSIS;
  if (global & A_SUBDIR)
    return E_ARCDIR;
  return E_ARCNEX;
}

/*
** Renombra un archivo.
*/
renombra()
{
  char nombre1[TAM_NOMBRE];
  char nom[TAM_NOMBRE];
  char *pos_dir, *pos_dir2;
  int dir_sector2;
  int longitud;

  if (separa_nombre(entero(quien + 1), nombre1))
    return E_NOMINV;
  if (anda_camino(entero(quien + 1), NO))
    return E_CAMINV;
  if (unidad_protegida(unidad_dir))
    return E_UNIPRO;
  if (forma(entero(quien + 5), nom))
    return E_NOMINV;
  pos_dir2 = busca_dir(entero(quien + 5), B_TODOS);
  dir_sector2 = dir_sector;
  if ((pos_dir = busca_dir(nombre1, B_NORMAL | B_SUBDIR)) == NULL) {
    if (global & A_SOLOLEC)
      return E_SOLLEC;
    if (global & A_SISTEMA)
      return E_ARCSIS;
    return E_ARCNEX;
  }
  if (pos_dir2 != NULL && (pos_dir2 != pos_dir || dir_sector2 != dir_sector))
    return E_NOMEXI;
  memcpy(pos_dir, nom, TAM_NOMBRE);
  graba_dir(dir_sector);
  return E_NORMAL;
}

/*
** Lee un sector de un archivo.
*/
lee_sec(ctrl)
  struct adan *ctrl;
{
  lee_sector(ctrl->unidad, ctrl->bloq_actual * info[ctrl->unidad].secs_bloque +
                           ctrl->sec_bloque);
  memcpy(ctrl->sector, mensaje + 1, TAM_SECTOR);
}

/*
** Vacia un sector de un archivo.
*/
vacia_sec(ctrl)
  struct adan *ctrl;
{
  memcpy(mensaje + 6, ctrl->sector, TAM_SECTOR);
  escribe_sector(ctrl->unidad, ctrl->bloq_actual *
                               info[ctrl->unidad].secs_bloque +
                               ctrl->sec_bloque);
  ctrl->sec_modif = NO;
}

/*
** Lee un sector de directorio.
*/
lee_dir(sector)
  int sector;
{
  lee_sector(unidad_dir, sector);
  memcpy(dir, mensaje + 1, TAM_SECTOR);
  dir_sector = sector;
}

/*
** Graba un sector de directorio.
*/
graba_dir(sector)
  int sector;
{
  memcpy(mensaje + 6, dir, TAM_SECTOR);
  escribe_sector(unidad_dir, sector);
  dir_sector = sector;
}

/*
** Limpia un bloque de directorio, usado por crea_dir().
*/
limpia_bloque(bloque, unidad)
  int bloque, unidad;
{
  int sector;
  int cuenta;

  sector = bloque * (cuenta = info[unidad].secs_bloque);
  while (cuenta--) {
    memset(mensaje + 6, 0x00, TAM_SECTOR);
    escribe_sector(unidad, sector++);
  }
}

/*
** Lee una entrada de la tabla de bloques
*/
lee_tabla(bloque, u)
  int bloque, u;
{
  int inicio_tabla, bytes_por_entrada, contenido;
  char *desplazamiento;

  if (unidad[u] == UNIDAD_CDROM)
    return bloque + 1;
  if (bloque >= info[u].bloques)
    return 0;
  inicio_tabla = info[u].sec_tabla;
  bytes_por_entrada = info[u].tam_entradas;
  lee_sector_tabla(inicio_tabla + (bloque * bytes_por_entrada / TAM_SECTOR),
                   u);
  desplazamiento = tabla + u * TAM_SECTOR +
                   (bloque * bytes_por_entrada % TAM_SECTOR);
  if (bytes_por_entrada == 1) {
    contenido = *desplazamiento;
    if (contenido == 0xff)
      contenido = FINAL;
  } else if(bytes_por_entrada == 2) {
    contenido = *desplazamiento | (*(desplazamiento + 1) << 8);
    if (contenido == 0xffff)
      contenido = FINAL;
  } else if(bytes_por_entrada == 4)
    contenido = entero(desplazamiento);
  return contenido;
}

/*
** Escribe un valor en la tabla de bloques.
*/
escribe_tabla(bloque, unidad, valor)
  int bloque, unidad, valor;
{
  int inicio_tabla, bytes_por_entrada;
  char *desplazamiento;

  if (bloque >= info[unidad].bloques)
    return;
  inicio_tabla = info[unidad].sec_tabla;
  bytes_por_entrada = info[unidad].tam_entradas;
  lee_sector_tabla(inicio_tabla + (bloque * bytes_por_entrada / TAM_SECTOR),
                   unidad);
  desplazamiento = tabla + unidad * TAM_SECTOR +
                   (bloque * bytes_por_entrada % TAM_SECTOR);
  if (bytes_por_entrada == 1)
    *desplazamiento = valor;
  else if (bytes_por_entrada == 2) {
    *desplazamiento++ = valor;
    *desplazamiento = valor >> 8;
  } else if (bytes_por_entrada == 4)
    escribe_entero(desplazamiento, valor);
  tabla_modif[unidad] = SI;
}

/*
** Lee un sector de la tabla de bloques
*/
lee_sector_tabla(sector, unidad)
  int sector, unidad;
{
  if (tabla_sector[unidad] != sector) {
    vacia_tabla(unidad);
    lee_sector(unidad, tabla_sector[unidad] = sector);
    memcpy(tabla + unidad * TAM_SECTOR, mensaje + 1, TAM_SECTOR);
  }
}

/*
** Vacia un sector de la tabla de bloques.
*/
vacia_tabla(unidad)
  int unidad;
{
  if (tabla_modif[unidad]) {
    memcpy(mensaje + 6, tabla + unidad * TAM_SECTOR, TAM_SECTOR);
    escribe_sector(unidad, tabla_sector[unidad]);
    tabla_modif[unidad] = NO;
  }
}

/*
** Asegura la tabla de bloques. (absolutamente necesario en removibles)
*/
asegura_tabla(unidad)
  int unidad;
{
  vacia_tabla(unidad);
  if (removible[unidad])
    tabla_sector[unidad] = 0;
}

/*
** Separa el nombre de archivo de un camino.
*/
separa_nombre(texto, nombre)
  char *texto, *nombre;
{
  int extra, longitud;
  char *texto2;

  texto = separa_unidad(texto, &extra);
  while (1) {
    texto2 = texto;
    while (*texto2 && *texto2 != '/')
      ++texto2;
    if (*texto2 == '/')
      texto = texto2 + 1;
    else
      break;
  }
  if (*texto == 0)
    return 1;
  return forma(texto, nombre);
}

/*
** Checa si es una letra.
*/
isalpha(car)
  int car;
{
  return((car >= 'A') && (car <= 'Z') ||
         (car >= 'a') && (car <= 'z'));
}

/*
** Separa la unidad de un camino.
*/
separa_unidad(texto, unidad)
  char *texto;
  int *unidad;
{
  *unidad = 0;
  if (isalpha(*texto)) {
    if (*(texto+1) == ':') {
      *unidad = toupper(*texto) - 'A';
      return texto + 2;
    }
  }
  return texto;
}

/*
** Checa si una unidad existe.
*/
existe(u)
  int u;
{
  if(u < 0 || u >= MAX_UNID)
    return 0;
  else
    return unidad[u];
}

/*
** Anda un camino, detecta si es válido.
*/
anda_camino(texto, todo)
  char *texto; int todo;
{
  int bl[32];
  int raya;
  int cbl;
  char nombre[TAM_NOMBRE];
  char *pos1;
  char *pos_dir;

  texto = separa_unidad(texto, &unidad_dir);
  if (existe(unidad_dir) == 0)
    return 1;
  if (carga_info(unidad_dir) == 0)
    return 1;
  bloque_dir = info[unidad_dir].dir_raiz;
  sector_dir = 0;
  pos1 = texto;
  cbl = 0;
  while (1) {
    if (cbl == 32)
      return 1;
    bl[cbl++] = bloque_dir;
    if (*pos1 == '/')
      ++pos1;
    if (*pos1 == 0)
      return !todo;
    pos1 = separa_componente(pos1, nombre);
    if (raya = (*pos1 == '/'))
      ++pos1;
    if (*pos1 == 0)
      if (!todo)
        return raya;
    if (nombre[0] == '.' && nombre[1] == 0)
      --cbl;
    else if (nombre[0] == '.' && nombre[1] == '.' && nombre[2] == 0) {
      if (cbl != 1) {
        cbl -= 2;
        bloque_dir = bl[cbl];
      }
    } else {
      if ((pos_dir = busca_dir(nombre, B_SUBDIR)) == NULL)
        return 1;
      else
        bloque_dir = entero(pos_dir + 56);
    }
  }
  return 0;
}

/*
** Separa un componente de un camino. (un nombre)
*/
separa_componente(texto, nombre)
  char *texto, *nombre;
{
  int longitud;

  longitud = 0;
  while (1) {
    if (*texto == 0)
      break;
    if (*texto == '/')
      break;
    if (longitud != (TAM_NOMBRE - 1)) {
      *nombre++ = *texto;
      ++longitud;
    }
    ++texto;
  }
  *nombre = 0;
  return texto;
}

/*
** Carga información de una unidad.
*/
carga_info(u)
  int u;
{
  int pos, cuenta;
  int con_particion = NO;
  int sector, num_sector, extendido;

  if (arch_abiertos[u])
    return 1;
  if (!removible[u] && info_cargada[u])
    return 1;
  if (unidad[u] == UNIDAD_CDROM) {
    info[u].tipo = UNIDAD_CDROM;
    info[u].sectores = 0x2a800000;  /* 680 megabytes */
    info[u].secs_bloque = 4;        /* 4 sectores por bloque (2K) */
    info[u].bloques = 0xaa00000;
    return checa_iso9660(u, &info[u]);
  } else {
    sector = 0;
    while (1) {
      lee_sector(u, sector);
      if (mensaje[511] == 0x55 && mensaje[512] == 0xaa) {        /* Disco PC */
        return 0;
      } else if (mensaje[511] == 0x12 && mensaje[512] == 0x34) { /* Disco G10 */
        memcpy(&info[u], mensaje + 17, sizeof(struct info));
        info_cargada[u] = SI;
        return 1;
      } else
        return 0;
    }
  }
}

/*
** Detecta los CD-ROM's High-Sierra y ISO-9660.
*/
checa_iso9660(u, info)
  int u;
  struct info *info;
{
  int bloque;
  int id;

  bloque = 64;
  id = NO;
  while (1) {
    lee_sector(u, bloque);
    if (!id) {
      if (checa_cadena(mensaje + 2, "CD001"))
        id = ISO9660;
      else if (checa_cadena(mensaje + 10, "CDROM"))
        id = HIGHSIERRA;
      else
        return 0;
    }
    if (id && (id == ISO9660 ? mensaje[1] : mensaje[9]) == 1) {
      info->sec_tabla = 0;                  /* No vale para un CD-ROM */
      info->tam_tabla = id == ISO9660;      /* 1 si ISO-9660 */
      info->tam_entradas = bloque;          /* Bloque maestro */
      info->dir_raiz = entero(id == ISO9660 ? mensaje + 159 : mensaje + 183);
      info_cargada[u] = SI;
      return 1;
    }
    bloque += 4;
  }
}

checa_cadena(origen, destino)
  char *origen, *destino;
{
  while(*destino)
    if (toupper(*origen++) != *destino++)
      return 0;
  return 1;
}

/*
** Busca un nuevo bloque para un archivo.
*/
nuevo_bloque(unidad, inicio)
  int unidad, inicio;
{
  int conteo, total;

  total = info[unidad].bloques;
  conteo = inicio;
  while (conteo < total) {
    if (lee_tabla(conteo, unidad) == 0)
      return conteo;
    ++conteo;
  }
  conteo = 0;
  while (conteo < inicio) {
    if (lee_tabla(conteo, unidad) == 0)
      return conteo;
    ++conteo;
  }
  return -1;
}

/*
** Manda un mensaje al video.
*/
men(cad)
  char *cad;
{
  while (*cad)
    escribe_car(*cad++);
}

/*
** Simulador de terminal.
*/
escribe_car(c)
  int c;
{
  char *vid;
  int conteo;

  if (cop) {
    if (cop == -1)
      cop = c;
    else {
      if (cop == 1)
        color = c;
      else if (cop == 2) {
        pos_x_cursor = c;
        situa_cursor();
      } else if (cop == 3) {
        pos_y_cursor = c;
        situa_cursor();
      } else if (cop == 4) {
        cursor_activado = c;
        if (c == 0)
          forma_cursor(16, 0);
        else
          forma_cursor(0, 15);
        situa_cursor();
      }
      cop = 0;
    }
  } else if (c == 0x0c) {
    vid = video;
    conteo = 2000;
    while (conteo--) {
      *vid++ = ' ';
      *vid++ = color;
    }
    pos_x_cursor = 0;
    pos_y_cursor = 0;
    situa_cursor();
  } else if (c == 0x0d) {
    pos_x_cursor = 0;
    situa_cursor();
  } else if (c == 0x08) {
    if (pos_x_cursor-- == 0) {
      pos_x_cursor = 79;
      if (pos_y_cursor-- == 0) {
        pos_y_cursor = 0;
      }
    }
    vid = video + (pos_x_cursor + pos_y_cursor * 80) * 2;
    *vid++ = ' ';
    *vid++ = color;
    situa_cursor();
  } else if (c == 0x0a) {
    if (++pos_y_cursor == 25) {
      --pos_y_cursor;
      rolado();
    }
    situa_cursor();
  } else if (c == 0x1b)
    cop = -1;
  else {
    vid = video + (pos_x_cursor + pos_y_cursor * 80) * 2;
    *vid = c;
    *(vid + 1) = color;
    if (++pos_x_cursor == 80) {
      pos_x_cursor = 0;
      if (++pos_y_cursor == 25) {
        --pos_y_cursor;
        rolado();
      }
    }
    situa_cursor();
  }
}

/*
** Rola la pantalla una línea hacia arriba.
*/
rolado()
{
  char *vid, *vid1;
  int conteo;

  vid = video;
  vid1 = video + 160;
  conteo = 24;
  while (conteo--) {
    memcpy(vid, vid1, 160);
    vid += 160;
    vid1 += 160;
  }
  conteo = 80;
  while (conteo--) {
    *vid++ = ' ';
    *vid++ = color;
  }
}

#define SITUA_CURSOR  7

situa_cursor()
{
  int pos_final;

  if (!cursor_activado)
    return;
  pos_final = pos_x_cursor + pos_y_cursor * 80;
  mensaje[0] = SITUA_CURSOR;
  mensaje[1] = pos_final;
  mensaje[2] = pos_final >> 8;
  ctrl_maestro(mensaje);
}

#define FORMA_CURSOR  8

forma_cursor(inicio, final)
  int inicio, final;
{
  mensaje[0] = FORMA_CURSOR;
  mensaje[1] = inicio;
  mensaje[2] = final;
  ctrl_maestro(mensaje);
}

#define CHECA_TECLA   5

checa_tecla()
{
  mensaje[0] = CHECA_TECLA;
  ctrl_maestro(mensaje);
  return mensaje[0];
}

#define LEE_SECTOR       1

lee_sector(unidad, sector)
  int unidad, sector;
{
  mensaje[0] = LEE_SECTOR;
  mensaje[1] = unidad;
  mensaje[2] = sector >> 24;
  mensaje[3] = sector >> 16;
  mensaje[4] = sector >> 8;
  mensaje[5] = sector;
  ctrl_maestro(mensaje);
  return mensaje[0];
}

#define ESCRIBE_SECTOR   2

escribe_sector(unidad, sector)
  int unidad, sector;
{
  mensaje[0] = ESCRIBE_SECTOR;
  mensaje[1] = unidad;
  mensaje[2] = sector >> 24;
  mensaje[3] = sector >> 16;
  mensaje[4] = sector >> 8;
  mensaje[5] = sector;
  ctrl_maestro(mensaje);
  return mensaje[0];
}

#define FORMATEA_PISTA   3

formatea_pista(pista, lado)
  int pista, lado;
{
  mensaje[0] = FORMATEA_PISTA;
  mensaje[1] = pista;
  mensaje[2] = lado;
  ctrl_maestro(mensaje);
  return mensaje[0];
}

#define OBTENER_HORA     6

obtener_hora(segundos, minutos, hora, fecha, mes, logico)
  int *segundos, *minutos, *hora, *fecha, *mes, *logico;
{
  mensaje[0] = OBTENER_HORA;
  ctrl_maestro(mensaje);
  *segundos = mensaje[0];
  *minutos = mensaje[1];
  *hora = mensaje[2];
  *fecha = mensaje[3];
  *mes = mensaje[4];
  *logico = mensaje[5];
}

#define IMPRIME         9

imprime(caracter)
  int caracter;
{
  mensaje[0] = IMPRIME;
  mensaje[1] = caracter;
  ctrl_maestro(mensaje);
  return E_NORMAL;
}

#define LEE_COM        24

lee_com()
{
  mensaje[0] = LEE_COM;
  ctrl_maestro(mensaje);
  return mensaje[0];
}

#define ESCRIBE_COM    25

escribe_com(c)
  int c;
{
  mensaje[0] = ESCRIBE_COM;
  mensaje[1] = c;
  ctrl_maestro(mensaje);
  return E_NORMAL;
}

#define LEE_CINTA      26

lee_cin()
{
  mensaje[0] = LEE_CINTA;
  ctrl_maestro(mensaje);
  if(mensaje[1])
    return -1;
  return mensaje[0];
}

#define ESCRIBE_CINTA  27

escribe_cin(c)
  int c;
{
  mensaje[0] = ESCRIBE_CINTA;
  mensaje[1] = c;
  ctrl_maestro(mensaje);
  return E_NORMAL;
}

#define REGRESA_CINTA  28

regresa_cinta()
{
  mensaje[0] = REGRESA_CINTA;
  ctrl_maestro(mensaje);
  return E_NORMAL;
}

entero(direccion)
  char *direccion;
{
  return *direccion | (*(direccion + 1) << 8) |
       (*(direccion + 2) << 16) | (*(direccion + 3) << 24);
}

escribe_entero(dir, valor)
  char *dir;
  int valor;
{
  *dir++ = valor;
  *dir++ = valor >> 8;
  *dir++ = valor >> 16;
  *dir = valor >> 24;
}

/*
** Rutinas para asignación dinámica de memoria.
*/
char *pide_memoria(num_bytes)
  int num_bytes;
{
  mem_d *ap, *ap_prev;
  int num_unidades;

  if ((ap_prev = ap_libre) == NULL) {
    char *final;

    final = _finprog();
    ap_libre = ap_prev = final;
    num_unidades = (limite_mem - final) / sizeof(mem_d);
    ap_prev->ap = ap_prev;
    ap_prev->tam = num_unidades;
    mem_libre += num_unidades * sizeof(mem_d);
  }
  num_unidades = (num_bytes + sizeof(mem_d) - 1) / sizeof(mem_d) + 1;
  for (ap = ap_prev->ap; ; ap_prev = ap, ap = ap->ap) {
    if (ap->tam >= num_unidades) {
      if (ap->tam == num_unidades)
        ap_prev->ap = ap->ap;
      else {
        ap->tam -= num_unidades;
        ap += ap->tam;
        ap->tam = num_unidades;
      }
      mem_libre -= num_unidades * sizeof(mem_d);
      ap_libre = ap_prev;
      return (ap + 1);
    }
    if (ap == ap_libre)
      return NULL;
  }
}

libera_memoria(bloque)
  char *bloque;
{
  mem_d *ap_b, *ap;

  ap_b = (mem_d *) bloque - 1;
  mem_libre += ap_b->tam * sizeof(mem_d);
  for (ap = ap_libre; !(ap_b > ap && ap_b < ap->ap); ap = ap->ap)
    if (ap >= ap->ap && (ap_b > ap || ap_b < ap->ap))
      break;
  if(ap_b + ap_b->tam == ap->ap) {
    ap_b->tam += ap->ap->tam;
    ap_b->ap = ap->ap->ap;
  } else
    ap_b->ap = ap->ap;
  if(ap + ap->tam == ap_b) {
    ap->tam += ap_b->tam;
    ap->ap = ap_b->ap;
  } else
    ap->ap = ap_b;
  ap_libre = ap;
}

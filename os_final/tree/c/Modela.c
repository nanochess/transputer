/*
** Programa para modelado en 3 dimensiones.
**
** Con distintos tipos de sombreado.
**
** (c) Copyright 1996 Oscar Toledo G.
**
** Creación: 18 de abril de 1996.
** Creación: 14 de mayo de 1996. Soporte para archivos tipo 2.
*/

#define TAM_LINEA      256

#define SI               1
#define NO               0

#define NULL           ((void *) 0)

/*
#define DEPURA
*/

typedef int FILE;

typedef char BYTE;        /* Caracter sin signo de 8 bits */
typedef short INT16;      /* Entero de 16 bits */
typedef int INT;          /* Entero básico de la máquina */
typedef long INT32;       /* Entero de 32 bits */
typedef float REAL;       /* Número de punto flotante básico */

FILE *archivo,            /* Archivo de entrada actual*/
     *entrada,            /* Archivo de escena */
     *salida;             /* Archivo de salida */

INT num_cuadros,          /* Número de cuadros */
    centro_x, centro_y,   /* Centro de la salida */
    res_x, res_y,         /* Resolución de la salida */
    nivel_subdivision;    /* Nivel de subdivisión para superficies de Bézier */

BYTE escena[TAM_LINEA];   /* Nombre de la escena */
BYTE opcion[TAM_LINEA];   /* Opciones pedidas por el usuario */
BYTE cadena[TAM_LINEA];

typedef struct {          /* Vector de tres dimensiones */
  REAL x, y, z;
} VECTOR;

enum EFECTOS {
  EFF_NINGUNO,            /* Ningún efecto */
  EFF_RETORCIDO,          /* Efecto de retorcido */
};

typedef struct objeto {   /* DESCRIPTOR DE OBJETO */
  INT es_bezier;          /* ¿ Objeto con superficies de Bézier ? */
  char objeto[TAM_LINEA]; /* Archivo del objeto */
  enum EFECTOS efecto;    /* ¿ Lleva algún efecto ? */
  VECTOR ambiente,        /* Iluminación ambiente */
         difusa,          /* Iluminación difusa */
         especular;       /* Iluminación especular */
  REAL lustre;            /* Lustre de la superficie */
  VECTOR escala,          /* Escala del objeto */
         rotacion,        /* Rotación del objeto */
         traslacion;      /* Traslación del objeto */
  struct objeto *sig;     /* Siguiente objeto */
} OBJETO;

REAL lee_double();

typedef struct {     /* RETAZO DE BÉZIER (PATCH) */
  INT vertices[16];  /* Números de los vertices */
} RETAZO;

typedef struct {     /* OBJETO CON SUPERFICIES DE BÉZIER */
  INT num_vertices;  /* Número de vertices en el objeto */
  INT num_retazos;   /* Número de retazos */
  VECTOR *vertices;  /* Apuntador a una matriz de vertices */
  RETAZO *retazos;   /* Apuntador a una matriz de retazos */
} OBJETO_BEZIER;

typedef struct lista_poligonos {  /* POLÍGONOS QUE COMPARTEN UN VERTICE */
  struct poligono *poli;          /* Apuntador al polígono */
  struct lista_poligonos *sig;    /* Siguiente polígono en la lista */
} LISTA_POLIGONOS;

typedef struct vector_ext {       /* VERTICE */
  VECTOR vector;                  /* Posición en 3-D */
  struct vector_ext *siguiente;   /* Siguiente vertice */
  LISTA_POLIGONOS lista;          /* Poligonos que usan este vertice */
} VECTOR_EXT;

typedef struct poligono {         /* POLÍGONO */
  VECTOR_EXT *lados[4];           /* Apuntador a cada vertice */
  REAL prof;                      /* Profundidad en la pantalla */
  OBJETO *objeto;                 /* Para parametros de visualización */
  struct poligono *siguiente;     /* Siguiente polígono */
} POLIGONO;

typedef struct {                  /* PUNTO BIDIMENSIONAL */
  INT x, y;                       /* x, y */
} PUNTO;

typedef REAL MAT4X4[4][4];

POLIGONO *primer_poligono, *ultimo_poligono;  /* Lista de polígonos */
VECTOR_EXT *primer_vertice, *ultimo_vertice;  /* Lista de vertices */
OBJETO *primer_objeto, *ultimo_objeto;        /* Lista de objetos */

MAT4X4 matriz_actual;

OBJETO_BEZIER *objeto_bezier;

#ifdef DEPURA
int total_poligonos;         /* Cuenta los polígonos usados en la escena */
#endif

#define MALLA_DE_PUNTOS      0x41   /* Resalta las esquinas de los polígonos */
#define MALLA_DE_LINEAS      0x42   /* Delinea los bordes de los polígonos */
#define SOMBREADO_CONSTANTE  0x43   /* Sombrea cada polígono como un plano */
#define SOMBREADO_GOURAUD    0x44   /* Interpola las intensidades */
#define SOMBREADO_PHONG      0x45   /* Interpola las normales */

/*
** Aquí empieza a rodar la bola.
*/
main()
{
  color(15);
  printf("\nModelado en 3 dimensiones  (c) Copyright 1996 Oscar Toledo G.\n");
  color(10);
  printf("\nArchivo de escena > ");
  lee_linea(escena, TAM_LINEA);
  printf("\nOpciones de ilustración:\n\n");
  printf("A - Malla de puntos.\n");
  printf("B - Malla de líneas.\n");
  printf("C - Sombreado constante.\n");
  printf("D - Sombreado de Gouraud.\n");
  printf("E - Sombreado de Phong. (el mejor)\n\n");
  printf("Opción > ");
  lee_linea(opcion, TAM_LINEA);
  opcion[0] = toupper(opcion[0]);
  printf("\nProcesando la escena...");
  archivo = fopen(escena, "r");
  if (archivo == NULL) {
    color(15);
    printf("\n\nNo se pudo abrir la escena.\n");
    return;
  }
  entrada = archivo;
  prepara_animacion();
  while (num_cuadros--) {
    inicia_todo();
    procesa_escena();
    genera_escena();
    almacena_escena();
    fin_escena();
  }
  fin_animacion();
  fclose(archivo);
#ifdef DEPURA
  printf("Se generaron %d polígonos\n", total_poligonos);
#endif
}

/*
** Inicia las variables requeridas.
*/
inicia_todo()
{
  primer_poligono = ultimo_poligono = NULL;
  primer_vertice = ultimo_vertice = NULL;
  primer_objeto = ultimo_objeto = NULL;
}

/*
** Se encarga de preparar la animación.
*/
prepara_animacion()
{
  num_cuadros = lee_entero();
  res_x = lee_entero();
  res_y = lee_entero();
  nivel_subdivision = lee_entero();
#ifdef DEPURA
  printf("Número de cuadros : %d\n", num_cuadros);
  printf("Resolución x, y : %d,%d\n", res_x, res_y);
  printf("Nivel de subdivisión : %d\n", nivel_subdivision);
#endif
}

/*
** Se encarga de cargar los objetos y subdividir las curvas de Bézier.
** Prepara las listas de polígonos.
*/
procesa_escena()
{
  INT objetos_en_escena;

  inicia_observador();
  objetos_en_escena = lee_entero();
  while (objetos_en_escena--) {
#ifdef DEPURA
    printf("Leyendo datos de objeto\n");
#endif
    switch (lee_datos_objeto()) {
      case 1:
#ifdef DEPURA
        printf("Cargando objeto de Bézier\n");
#endif
        carga_objeto_bezier();
#ifdef DEPURA
        printf("Subdividiendo\n");
        total_poligonos = 0;
#endif
        subdivide_bezier();
        break;
      case 0:
#ifdef DEPURA
        printf("Cargando polígonos\n");
#endif
        carga_objeto_normal();
        break;
      case 2:
#ifdef DEPURA
        printf("Cargando polígonos tipo 2\n");
#endif
        carga_objeto_especial();
        break;
    }
  }
}

/*
** Checa la perspectiva, observador y luz.
*/
inicia_observador()
{
  INT en_perspectiva;
  REAL x, y, z, ds, ang_obs, rot_obs, ang_luz, rot_luz;

  en_perspectiva = lee_entero();
  x = lee_double();
  y = lee_double();
  z = lee_double();
  ds = lee_double();
#ifdef DEPURA
  printf("Parametros perspectiva: %d, %f, %f, %f, %f\n", en_perspectiva,
         x, y, z, ds);
#endif
  inicia_perspectiva(en_perspectiva, x, y, z, ds);
  ang_obs = lee_double();
  rot_obs = lee_double();
  ang_luz = lee_double();
  rot_luz = lee_double();
#ifdef DEPURA
  printf("Parametros observador: %f, %f, %f, %f\n", ang_obs, rot_obs,
         ang_luz, rot_luz);
#endif
  inicia_dibujo(ang_obs, rot_obs, ang_luz, rot_luz);
}

/*
** Lee los datos del objeto.
*/
INT lee_datos_objeto()
{
  OBJETO *nuevo_objeto;

  nuevo_objeto = malloc(sizeof(OBJETO));
  if (nuevo_objeto == NULL)
    error_fatal(1);
  nuevo_objeto->es_bezier = lee_entero();
  lee_cadena();
  strcpy(nuevo_objeto->objeto, cadena);
  nuevo_objeto->efecto = lee_entero();
  nuevo_objeto->ambiente.x = lee_double();
  nuevo_objeto->ambiente.y = lee_double();
  nuevo_objeto->ambiente.z = lee_double();
  nuevo_objeto->difusa.x = lee_double();
  nuevo_objeto->difusa.y = lee_double();
  nuevo_objeto->difusa.z = lee_double();
  nuevo_objeto->especular.x = lee_double();
  nuevo_objeto->especular.y = lee_double();
  nuevo_objeto->especular.z = lee_double();
  nuevo_objeto->lustre = lee_double();
  nuevo_objeto->escala.x = lee_double();
  nuevo_objeto->escala.y = lee_double();
  nuevo_objeto->escala.z = lee_double();
  nuevo_objeto->rotacion.x = lee_double();
  nuevo_objeto->rotacion.y = lee_double();
  nuevo_objeto->rotacion.z = lee_double();
  nuevo_objeto->traslacion.x = lee_double();
  nuevo_objeto->traslacion.y = lee_double();
  nuevo_objeto->traslacion.z = lee_double();
  nuevo_objeto->sig = NULL;
#ifdef DEPURA
  printf("Preparando matriz con parametros:\n");
  printf("Trasladar : %f, %f, %f\n", nuevo_objeto->traslacion.x,
         nuevo_objeto->traslacion.y, nuevo_objeto->traslacion.z);
  printf("Escala : %f, %f, %f\n", nuevo_objeto->escala.x,
         nuevo_objeto->escala.y, nuevo_objeto->escala.z);
  printf("Rotación : %f, %f, %f\n", nuevo_objeto->rotacion.x,
         nuevo_objeto->rotacion.y, nuevo_objeto->rotacion.z);
#endif
  prepara_matriz(nuevo_objeto->traslacion.x, nuevo_objeto->traslacion.y,
                 nuevo_objeto->traslacion.z,
                 nuevo_objeto->escala.x, nuevo_objeto->escala.y,
                 nuevo_objeto->escala.z,
                 nuevo_objeto->rotacion.x, nuevo_objeto->rotacion.y,
                 nuevo_objeto->rotacion.z, matriz_actual);
#ifdef DEPURA
  printf("La matriz es:\n");
  printf("%f %f %f %f\n", matriz_actual[0][0], matriz_actual[0][1],
                          matriz_actual[0][2], matriz_actual[0][3]);
  printf("%f %f %f %f\n", matriz_actual[1][0], matriz_actual[1][1],
                          matriz_actual[1][2], matriz_actual[1][3]);
  printf("%f %f %f %f\n", matriz_actual[2][0], matriz_actual[2][1],
                          matriz_actual[2][2], matriz_actual[2][3]);
  printf("%f %f %f %f\n", matriz_actual[3][0], matriz_actual[3][1],
                          matriz_actual[3][2], matriz_actual[3][3]);
#endif
  if (primer_objeto == NULL)
    primer_objeto = ultimo_objeto = nuevo_objeto;
  else {
    ultimo_objeto->sig = nuevo_objeto;
    ultimo_objeto = nuevo_objeto;
  }
  return nuevo_objeto->es_bezier;
}

/*
** Carga un objeto con superficies de Bézier.
*/
carga_objeto_bezier()
{
  INT num_vertices, num_retazos, a, b;

  archivo = fopen(ultimo_objeto->objeto, "r");
  if (archivo == NULL) {
    archivo = entrada;
    error_fatal(2);
  }
  objeto_bezier = malloc(sizeof(OBJETO));
  if (objeto_bezier == NULL)
    error_fatal(1);
  lee_cadena();
  num_vertices = lee_entero();
  num_retazos = lee_entero();
  objeto_bezier->num_vertices = num_vertices;
  objeto_bezier->num_retazos = num_retazos;
  objeto_bezier->vertices = malloc(sizeof(VECTOR) * num_vertices);
  if (objeto_bezier->vertices == NULL) {
    fclose(archivo);
    archivo = entrada;
    free(objeto_bezier);
    error_fatal(1);
  }
  objeto_bezier->retazos = malloc(sizeof(RETAZO) * num_retazos);
  if (objeto_bezier->retazos == NULL) {
    fclose(archivo);
    archivo = entrada;
    free(objeto_bezier->vertices);
    free(objeto_bezier);
    error_fatal(1);
  }
  for (a = 0; a < num_vertices; a++) {
    b = lee_entero();
    objeto_bezier->vertices[a].x = lee_double();
    objeto_bezier->vertices[a].y = lee_double();
    objeto_bezier->vertices[a].z = lee_double();
  }
  for (a = 0; a < num_retazos; a++) {
    b = lee_entero();
    for (b = 0; b < 16; b++)
      objeto_bezier->retazos[a].vertices[b] = lee_entero() - 1;
  }
  fclose(archivo);
  archivo = entrada;
}

/*
** Subdivide las superficies de Bézier.
*/
subdivide_bezier()
{
  VECTOR definicion[16];
  INT a, b;

  for (a = 0; a < objeto_bezier->num_retazos; a++) {
    for (b = 0; b < 16; b++)
      transforma_vector(&objeto_bezier->vertices[
                           objeto_bezier->retazos[a].vertices[b]
                        ], matriz_actual, &definicion[b]);
    divide_cara(definicion, nivel_subdivision);
  }
  free(objeto_bezier->vertices);
  free(objeto_bezier->retazos);
  free(objeto_bezier);
}

/*
** Subdivisión recursiva de una cara.
*/
divide_cara(definicion, nivel)
  VECTOR definicion[16];
  INT nivel;
{
  VECTOR def_sup_izq[16];
  VECTOR def_sup_der[16];
  VECTOR def_inf_izq[16];
  VECTOR def_inf_der[16];

  if (nivel == 0) {   /* ¿ Máximo nivel de subdivisión ? */
    agrega_poligono(&definicion[0], &definicion[3],
                    &definicion[15], &definicion[12], ultimo_objeto);
    return;
  }
  divide_izq_der(definicion, def_inf_izq, def_inf_der);
  divide_sup_inf(def_inf_izq, def_inf_izq, def_sup_izq);
  divide_sup_inf(def_inf_der, def_inf_der, def_sup_der);
  divide_cara(def_sup_izq, nivel - 1);
  divide_cara(def_sup_der, nivel - 1);
  divide_cara(def_inf_izq, nivel - 1);
  divide_cara(def_inf_der, nivel - 1);
}

/*
** Divide por un lado. (u constante)
*/
divide_izq_der(def, izq, der)
  VECTOR def[16], izq[16], der[16];
{
  VECTOR temp1[4], temp2[4], medio;
  INT i, j;

  for (i = 0; i < 4; i++) {
    temp1[0] = def[i];
    temp1[1].x = (def[i].x + def[i + 4].x) * 0.5;
    temp1[1].y = (def[i].y + def[i + 4].y) * 0.5;
    temp1[1].z = (def[i].z + def[i + 4].z) * 0.5;
    medio.x = (def[i + 4].x + def[i + 8].x) * 0.5;
    medio.y = (def[i + 4].y + def[i + 8].y) * 0.5;
    medio.z = (def[i + 4].z + def[i + 8].z) * 0.5;
    temp1[2].x = (temp1[1].x + medio.x) * 0.5;
    temp1[2].y = (temp1[1].y + medio.y) * 0.5;
    temp1[2].z = (temp1[1].z + medio.z) * 0.5;
    temp2[2].x = (def[i + 8].x + def[i + 12].x) * 0.5;
    temp2[2].y = (def[i + 8].y + def[i + 12].y) * 0.5;
    temp2[2].z = (def[i + 8].z + def[i + 12].z) * 0.5;
    temp2[1].x = (medio.x + temp2[2].x) * 0.5;
    temp2[1].y = (medio.y + temp2[2].y) * 0.5;
    temp2[1].z = (medio.z + temp2[2].z) * 0.5;
    temp1[3].x = (temp1[2].x + temp2[1].x) * 0.5;
    temp1[3].y = (temp1[2].y + temp2[1].y) * 0.5;
    temp1[3].z = (temp1[2].z + temp2[1].z) * 0.5;
    temp2[0] = temp1[3];
    temp2[3] = def[i + 12];
    for (j = 0; j < 4; j++) {
      izq[j * 4 + i] = temp1[j];
      der[j * 4 + i] = temp2[j];
    }
  }
}

/*
** Divide por el otro lado. (v constante)
*/
divide_sup_inf(def, sup, inf)
  VECTOR def[16], sup[16], inf[16];
{
  VECTOR temp1[4], temp2[4], medio;
  INT i, j;

  for (i = 0; i < 16; i += 4) {
    temp1[0] = def[i];
    temp1[1].x = (def[i].x + def[i + 1].x) * 0.5;
    temp1[1].y = (def[i].y + def[i + 1].y) * 0.5;
    temp1[1].z = (def[i].z + def[i + 1].z) * 0.5;
    medio.x = (def[i + 1].x + def[i + 2].x) * 0.5;
    medio.y = (def[i + 1].y + def[i + 2].y) * 0.5;
    medio.z = (def[i + 1].z + def[i + 2].z) * 0.5;
    temp1[2].x = (temp1[1].x + medio.x) * 0.5;
    temp1[2].y = (temp1[1].y + medio.y) * 0.5;
    temp1[2].z = (temp1[1].z + medio.z) * 0.5;
    temp2[2].x = (def[i + 2].x + def[i + 3].x) * 0.5;
    temp2[2].y = (def[i + 2].y + def[i + 3].y) * 0.5;
    temp2[2].z = (def[i + 2].z + def[i + 3].z) * 0.5;
    temp2[1].x = (medio.x + temp2[2].x) * 0.5;
    temp2[1].y = (medio.y + temp2[2].y) * 0.5;
    temp2[1].z = (medio.z + temp2[2].z) * 0.5;
    temp1[3].x = (temp1[2].x + temp2[1].x) * 0.5;
    temp1[3].y = (temp1[2].y + temp2[1].y) * 0.5;
    temp1[3].z = (temp1[2].z + temp2[1].z) * 0.5;
    temp2[0] = temp1[3];
    temp2[3] = def[i + 3];
    for (j = 0; j < 4; j++) {
      sup[i + j] = temp1[j];
      inf[i + j] = temp2[j];
    }
  }
}

/*
** Carga un objeto polígonal.
*/
carga_objeto_normal()
{
  INT num_poligonos, a, b;
  VECTOR definicion[4], temp;

  archivo = fopen(ultimo_objeto->objeto, "r");
  if (archivo == NULL) {
    archivo = entrada;
    error_fatal(2);
  }
  lee_cadena();
  num_poligonos = lee_entero();
  for (a = 0; a < num_poligonos; a++) {
    for (b = 0; b < 4; b++) {
      temp.x = lee_double();
      temp.y = lee_double();
      temp.z = lee_double();
      transforma_vector(&temp, matriz_actual, &definicion[b]);
    }
    agrega_poligono(&definicion[0], &definicion[1], &definicion[2],
                    &definicion[3], ultimo_objeto);
  }
  fclose(archivo);
  archivo = entrada;
}

/*
** Carga un objeto polígonal de tipo especial.
*/
carga_objeto_especial()
{
  INT num_poligonos, num_vertices, a, b, lados;
  VECTOR *vertices, definicion[4], temp;

  archivo = fopen(ultimo_objeto->objeto, "r");
  if (archivo == NULL) {
    archivo = entrada;
    error_fatal(2);
  }
  lee_cadena();
  num_vertices = lee_entero();
  num_poligonos = lee_entero();
  vertices = malloc(sizeof(VECTOR) * num_vertices);
  for (a = 0; a < num_vertices; a++) {
    temp.x = lee_double();
    temp.y = lee_double();
    temp.z = lee_double();
    transforma_vector(&temp, matriz_actual, &vertices[a]);
  }
  for (a = 0; a < num_poligonos; a++) {
    lados = lee_entero();
    for (b = 0; b < lados; b++)
      definicion[b] = vertices[lee_entero() - 1];
    if (lados == 3)
      definicion[3] = definicion[2];
    agrega_poligono(&definicion[0], &definicion[1], &definicion[2],
                    &definicion[3], ultimo_objeto);
  }
  free(vertices);
  fclose(archivo);
  archivo = entrada;
}

/*
** Genera la escena.
*/
genera_escena()
{
  ilumina_poligonos();
}

/*
** Almacena la escena.
*/
almacena_escena()
{
}

/*
** Fin de escena.
** Libera la memoria.
*/
fin_escena()
{
  OBJETO *limpia3, *sig3;
  VECTOR_EXT *limpia, *sig;
  LISTA_POLIGONOS *limpia2, *sig2;

  limpia = primer_vertice;
  while (limpia != NULL) {
    limpia2 = limpia->lista.sig;
    while (limpia2 != NULL) {
      sig2 = limpia2->sig;
      free(limpia2);
      limpia2 = sig2;
    }
    sig = limpia->siguiente;
    free(limpia);
    limpia = sig;
  }
  limpia3 = primer_objeto;
  while (limpia3 != NULL) {
    sig3 = limpia3->sig;
    free(limpia3);
    limpia3 = sig3;
  }
}

/*
** Fin de animación.
** Cierra los archivos.
*/
fin_animacion()
{
}

color(col)
  INT col;
{
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

lee_linea(pos, tam)
  BYTE *pos;
  INT tam;
{
  BYTE *ahora;
  INT car;

  ahora = pos;
  while (1) {
    car = getchar();
    if (car == 8) {
      if (ahora == pos)
        continue;
      putchar(8);
      --ahora;
      continue;
    } else if(car == 13) {
      puts("\r\n");
      *ahora = 0;
      return;
    } else {
      if (ahora == pos + tam - 1)
        continue;
      putchar(car);
      *ahora++ = car;
    }
  }
}

printf(cad, v)
  BYTE *cad;
  INT v;
{
  INT *args;
  BYTE *pos2;
  double *pos;
  INT regs, val;

  regs = 1;
  args = &v;
  while (*cad) {
    if (*cad == '\n') {
      puts("\r\n");
      ++cad;
    } else if (*cad == '%') {
      while (*cad < 'a' || *cad > 'z')
        ++cad;
      if (*cad == 'l')
        ++cad;
      if (*cad == 'd') {
        decimal(*args++);
        regs = 0;
      } else if (*cad == 's') {
        pos2 = *args++;
        while (*pos2)
          putchar(*pos2++);
        regs = 0;
      } else if (*cad == 'x') {
        hex(*args++);
        regs = 0;
      } else if (*cad == 'f') {
        if (regs)
          ++args;
        pos = args;
        decimal((int) *pos);
        putchar('.');
        val = (int) (*pos * 100);
        if (val < 0)
          val = -val;
        rango(val);
        args += 2;
        regs = 0;
      }
      ++cad;
    } else
      putchar(*cad++);
  }
}

decimal(val)
  INT val;
{
  if (val < 0) {
    putchar('-');
    val = -val;
  }
  if (val >= 10)
    decimal(val / 10);
  putchar(val % 10 + '0');
}

hex(val)
  INT val;
{
  char *a = "0123456789abcdef";

  if (val < 0 || val >= 0x10)
    hex(val >> 4);
  putchar(a[val & 0x0f]);
}

rango(val)
  INT val;
{
  putchar((val / 10) % 10 + '0');
  putchar( val       % 10 + '0');
}

lee_cadena()
{
  INT car;
  BYTE *pos;
  INT primero;

  primero = SI;
  pos = cadena;
  while (1) {
    car = fgetc(archivo);
    if (car == 0x0d) continue;
    if (car == 0x0a && primero) continue;
    if (car == 0x0a || car == 0xff) break;
    *pos++ = car;
    primero = NO;
  }
  *pos = 0;
}

lee_entero()
{
  INT car;
  INT val;
  INT inicio;

  val = 0;
  inicio = SI;
  while (1) {
    car = fgetc(archivo);
    if (inicio && (car == 0x0d || car == 0x0a || car == 0x20))
      continue;
    inicio = NO;
    if (car < '0' || car > '9')
      break;
    val = val * 10 + car - '0';
  }
  return val;
}

REAL lee_double()
{
  INT car;
  REAL val;
  REAL escala;
  INT inicio;
  INT negar;
  INT fraccion;

  val = 0;
  inicio = SI;
  negar = NO;
  fraccion = NO;
  escala = 10.;
  while (1) {
    car = fgetc(archivo);
    if (inicio && (car == 0x0d || car == 0x0a || car == 0x20))
      continue;
    inicio = NO;
    if (car == '-') {
      negar = SI;
      continue;
    }
    if (car == '.') {
      fraccion = SI;
      continue;
    }
    if (car < '0' || car > '9')
      break;
    if (fraccion) {
      val = val + ((car - '0') / escala);
      escala *= 10.0;
    } else
      val = val * 10.0 + (car - '0');
  }
  if (negar)
    val = -val;
  return val;
}

REAL cos(), sin();

/*
** Convierte grados a radianes.
*/
REAL gr(grados)
  REAL grados;
{
  return (grados * 3.14159) / 180;
}

/*
** Saca el coseno de x grados.
*/
REAL cosg(grados)
  REAL grados;
{
  return cos(gr(grados));
}

/*
** Saca el seno de x grados.
*/
REAL sing(grados)
  REAL grados;
{
  return sin(gr(grados));
}

/*
** Parametros cálculados por la computadora.
*/
VECTOR luz, vista;
REAL cosa, sina, cosb, sinb, cosacosb, sinasinb, cosasinb, sinacosb;

inicia_dibujo(ang_obs, rot_obs, ang_luz, rot_luz)
  REAL ang_obs, rot_obs, ang_luz, rot_luz;
{
  REAL pi, theta;

  centro_x = res_x / 2;
  centro_y = res_y / 2;
  cosa = cosg(ang_obs);
  sina = sing(ang_obs);
  cosb = cosg(rot_obs);
  sinb = sing(rot_obs);
  cosacosb = cosa * cosb;
  sinasinb = sina * sinb;
  cosasinb = cosa * sinb;
  sinacosb = sina * cosb;
  pi = gr(ang_luz);
  theta = gr(rot_luz);
  luz.x = sin(theta) * cos(pi);      /* Ángulo de la luz */
  luz.y = sin(theta) * sin(pi);
  luz.z = cos(theta);
  pi = gr(270.0 - ang_obs);
  theta = gr(90.0 - rot_obs);
  vista.x = sin(theta) * cos(pi);    /* Ángulo de la vista */
  vista.y = sin(theta) * sin(pi);
  vista.z = cos(theta);
}

/*
** Parametros de la perspectiva.
*/
INT ps;
REAL mx, my, mz, ds;

inicia_perspectiva(en_perspectiva, x, y, z, m)
  INT en_perspectiva;
  REAL x, y, z, m;
{
  ps = en_perspectiva;
  mx = x;
  my = y;
  mz = z;
  ds = m;
}

/*
** Mapea las coordenadas del objeto a las coordenadas de visualización.
*/
mapea_coordenadas(vector_origen, vector_nuevo)
  VECTOR *vector_origen;
  VECTOR *vector_nuevo;
{
  vector_nuevo->x = mx + vector_origen->x * cosa - vector_origen->y * sina;
  vector_nuevo->y = my + vector_origen->x * sinasinb +
                         vector_origen->y * cosasinb + vector_origen->z * cosb;
  vector_nuevo->z = mz + vector_origen->x * sinacosb +
                         vector_origen->y * cosacosb - vector_origen->z * sinb;
}

strcpy(destino, origen)
  BYTE *destino, *origen;
{
  while (*destino++ = *origen++) ;
}

/*
** Prepara un matriz para transformar un objeto.
*/
prepara_matriz(tx, ty, tz, ex, ey, ez, rx, ry, rz, matriz)
  REAL tx, ty, tz, ex, ey, ez, rx, ry, rz;
  MAT4X4 matriz;
{
  MAT4X4 M1, M2, M3, M4, M5, M6;

  escala_3d(ex, ey, ez, M1);
  rota_3d(1, rx, M2);
  rota_3d(2, ry, M3);
  rota_3d(3, rz, M4);
  traslada_3d(tx, ty, tz, M5);
  multiplica_matrices(M2, M1, M6);
  multiplica_matrices(M3, M6, M1);
  multiplica_matrices(M4, M1, M2);
  multiplica_matrices(M5, M2, matriz);
}

/*
** Inicializa una matriz.
*/
matriz_cero(mat)
  MAT4X4 mat;
{
  INT a, b;

  for (a = 0; a < 4; a++)
    for (b = 0; b < 4; b++)
      mat[a][b] = 0.0;
}

/*
** Prepara una matriz de traslación.
*/
traslada_3d(x, y, z, mat)
  REAL x, y, z;
  MAT4X4 mat;
{
  INT a;

  matriz_cero(mat);
  for (a = 0; a < 4; a++)
    mat[a][a] = 1.0;
  mat[0][3] = -x;
  mat[1][3] = -y;
  mat[2][3] = -z;
}

/*
** Prepara una matriz de escala.
*/
escala_3d(x, y, z, mat)
  REAL x, y, z;
  MAT4X4 mat;
{
  matriz_cero(mat);
  mat[0][0] = x;
  mat[1][1] = y;
  mat[2][2] = z;
  mat[3][3] = 1.0;
}

/*
** Prepara una matriz para rotación en 3-D.
*/
rota_3d(m, angulo, mat)
  INT m;
  REAL angulo;
  MAT4X4 mat;
{
  INT m1, m2;
  REAL c, s;

  matriz_cero(mat);
  mat[m-1][m-1] = 1.0;
  mat[3][3] = 1.0;
  m1 = (m % 3) + 1;
  m2 = m1 % 3;
  m1 -= 1;
  c = cosg(angulo);
  s = sing(angulo);
  mat[m1][m1] = c;
  mat[m1][m2] = s;
  mat[m2][m2] = c;
  mat[m2][m1] = -s;
}

/*
** Multiplica dos matrices, guarda el resultado en una tercer matriz.
*/
multiplica_matrices(m1, m2, m3)
  MAT4X4 m1, m2, m3;
{
  INT a, b, c;
  REAL coeficiente;

  for (a = 0; a < 4; a++) {
    for (b = 0; b < 4; b++) {
      coeficiente = 0.0;
      for (c = 0; c < 4; c++)
        coeficiente += m1[a][c] * m2[c][b];
      m3[a][b] = coeficiente;
    }
  }
}

/*
** Transforma un vector de acuerdo a una matriz.
*/
transforma_vector(vector, matriz, resultado)
  VECTOR *vector, *resultado;
  MAT4X4 matriz;
{
  resultado->x = matriz[0][0] * vector->x + matriz[0][1] * vector->y +
                 matriz[0][2] * vector->z + matriz[0][3];
  resultado->y = matriz[1][0] * vector->x + matriz[1][1] * vector->y +
                 matriz[1][2] * vector->z + matriz[1][3];
  resultado->z = matriz[2][0] * vector->x + matriz[2][1] * vector->y +
                 matriz[2][2] * vector->z + matriz[2][3];
}

/*
** Errores fatales.
*/
error_fatal(num)
  INT num;
{
  switch (num) {
    case 1:
      printf("No hay memoria.\n");
      break;
    case 2:
      printf("No hay archivo.\n");
      break;
  }
  fin_escena();
  fin_animacion();
  fclose(archivo);
  exit(1);
}

/*
** Define un nuevo vertice.
*/
agrega_vertice(vec, poli)
  VECTOR *vec;
  POLIGONO *poli;
{
  VECTOR_EXT *nuevo_vec, *explora;
  LISTA_POLIGONOS *nuevo;

  nuevo_vec = malloc(sizeof(VECTOR_EXT));
  if (nuevo_vec == NULL)
    error_fatal(1);
  nuevo_vec->vector = *vec;
  nuevo_vec->siguiente = NULL;
  nuevo_vec->lista.poli = NULL;
  nuevo_vec->lista.sig = NULL;
  if (primer_vertice == NULL)
    explora = primer_vertice = ultimo_vertice = nuevo_vec;
  else {
    explora = primer_vertice;
    while (explora != NULL) {
      if (explora->vector.x == vec->x &&
          explora->vector.y == vec->y &&
          explora->vector.z == vec->z) {
        free(nuevo_vec);
        break;
      }
      explora = explora->siguiente;
    }
    if (explora == NULL) {
      ultimo_vertice->siguiente = nuevo_vec;
      explora = ultimo_vertice = nuevo_vec;
    }
  }
  if (explora->lista.poli == NULL)
    explora->lista.poli = poli;
  else {
    nuevo = malloc(sizeof(LISTA_POLIGONOS));
    if (nuevo == NULL)
      error_fatal(1);
    nuevo->poli = poli;
    nuevo->sig = explora->lista.sig;
    explora->lista.sig = nuevo;
  }
  return explora;
}

/*
** Define un nuevo polígono.
** Agrega vertices nuevos si es necesario.
*/
agrega_poligono(lado1, lado2, lado3, lado4, objeto)
  VECTOR *lado1, *lado2, *lado3, *lado4;
  OBJETO *objeto;
{
  POLIGONO *nuevo_poligono, *explora, *anterior;
  VECTOR lado5, lado6, lado7, lado8;
  REAL prof;

  mapea_coordenadas(lado1, &lado5);
  mapea_coordenadas(lado2, &lado6);
  mapea_coordenadas(lado3, &lado7);
  mapea_coordenadas(lado4, &lado8);
  prof = lado5.z;
  if (lado6.z > prof)
    prof = lado6.z;
  if (lado7.z > prof)
    prof = lado7.z;
  if (lado8.z > prof)
    prof = lado8.z;
  nuevo_poligono = malloc(sizeof(POLIGONO));
  if (nuevo_poligono == NULL)
    error_fatal(1);
#ifdef DEPURA
  ++total_poligonos;
#endif
  nuevo_poligono->lados[0] = agrega_vertice(&lado5, nuevo_poligono);
  nuevo_poligono->lados[1] = agrega_vertice(&lado6, nuevo_poligono);
  nuevo_poligono->lados[2] = agrega_vertice(&lado7, nuevo_poligono);
  nuevo_poligono->lados[3] = agrega_vertice(&lado8, nuevo_poligono);
  nuevo_poligono->siguiente = NULL;
  nuevo_poligono->prof = prof;
  nuevo_poligono->objeto = objeto;
  if (primer_poligono == NULL)
    primer_poligono = ultimo_poligono = nuevo_poligono;
  else {
    anterior = NULL;
    explora = primer_poligono;
    while (1) {
      if (prof > explora->prof) {
        nuevo_poligono->siguiente = explora;
        if (anterior != NULL)
          anterior->siguiente = nuevo_poligono;
        if (explora == primer_poligono)
          primer_poligono = nuevo_poligono;
        return;
      }
      if (explora->siguiente == NULL) {
        explora->siguiente = nuevo_poligono;
        ultimo_poligono = nuevo_poligono;
        return;
      }
      anterior = explora;
      explora = explora->siguiente;
    }
  }
}

/*
** Ilumina todos los polígonos en el sombreado pedido.
*/
ilumina_poligonos()
{
  PUNTO poli[4];
  VECTOR normales[4];
  POLIGONO *explora, *sig;
  INT r, g, b;

  putchar(12);
  modo_grafico();
  getchar();
  explora = primer_poligono;
  while (explora != NULL) {
    if (ps) {
      poli[0].x = centro_x + (int) (ds * explora->lados[0]->vector.x /
                                         explora->lados[0]->vector.z);
      poli[0].y = centro_y - (int) (ds * explora->lados[0]->vector.y /
                                         explora->lados[0]->vector.z);
      poli[1].x = centro_x + (int) (ds * explora->lados[1]->vector.x /
                                         explora->lados[1]->vector.z);
      poli[1].y = centro_y - (int) (ds * explora->lados[1]->vector.y /
                                         explora->lados[1]->vector.z);
      poli[2].x = centro_x + (int) (ds * explora->lados[2]->vector.x /
                                         explora->lados[2]->vector.z);
      poli[2].y = centro_y - (int) (ds * explora->lados[2]->vector.y /
                                         explora->lados[2]->vector.z);
      poli[3].x = centro_x + (int) (ds * explora->lados[3]->vector.x /
                                         explora->lados[3]->vector.z);
      poli[3].y = centro_y - (int) (ds * explora->lados[3]->vector.y /
                                         explora->lados[3]->vector.z);
    } else {
      poli[0].x = centro_x + (int) explora->lados[0]->vector.x;
      poli[0].y = centro_y - (int) explora->lados[0]->vector.y;
      poli[1].x = centro_x + (int) explora->lados[1]->vector.x;
      poli[1].y = centro_y - (int) explora->lados[1]->vector.y;
      poli[2].x = centro_x + (int) explora->lados[2]->vector.x;
      poli[2].y = centro_y - (int) explora->lados[2]->vector.y;
      poli[3].x = centro_x + (int) explora->lados[3]->vector.x;
      poli[3].y = centro_y - (int) explora->lados[3]->vector.y;
    }
    switch (opcion[0]) {
      case MALLA_DE_PUNTOS:
        puntos_poligono(poli);
        break;
      case MALLA_DE_LINEAS:
        lineas_poligono(poli);
        break;
      case SOMBREADO_CONSTANTE:
        obtiene_normal(explora);
        intensidad(explora->objeto, &r, &g, &b);
        sombreado_constante(poli, explora->objeto, r, g, b);
        break;
      case SOMBREADO_GOURAUD:
        calcula_vertices(explora, normales);
        sombreado_gouraud(poli, normales, explora->objeto);
        break;
      case SOMBREADO_PHONG:
        calcula_vertices(explora, normales);
        sombreado_phong(poli, normales, explora->objeto);
        break;
    }
    sig = explora->siguiente;
    free(explora);
    explora = sig;
  }
  getchar();
  modo_texto();
}

INT round();

VECTOR normal;

REAL sqrt();

#define tam_vector(v1)         (sqrt(v1.x*v1.x+v1.y*v1.y+v1.z*v1.z))
#define normaliza(v1)          {double v;v=1.0/tam_vector(v1);\
                                v1.x*=v;v1.y*=v;v1.z*=v;}
#define resta_vector(v1,v2,v3) {v1.x=v2.x-v3.x;v1.y=v2.y-v3.y;v1.z=v2.z-v3.z;}
#define suma_vector(v1,v2,v3) {v1.x=v2.x+v3.x;v1.y=v2.y+v3.y;v1.z=v2.z+v3.z;}
#define escala(v1,v2,e) {v1.x=v2.x*(e);v1.y=v2.y*(e);v1.z=v2.z*(e);}
#define cruce_vector(v1,v2,v3) {v1.x=v2.y*v3.z-v2.z*v3.y;\
                                v1.y=v2.z*v3.x-v2.x*v3.z;\
                                v1.z=v2.x*v3.y-v2.y*v3.x;}

/*
** Obtiene la normal de un polígono.
*/
obtiene_normal(poli)
  POLIGONO *poli;
{
  VECTOR temp1, temp2, temp3, dir1, dir2, normal1, normal2;
  REAL tam1, tam2;

  temp1 = poli->lados[1]->vector;
  temp2 = poli->lados[0]->vector;
  temp3 = poli->lados[3]->vector;
  resta_vector(dir1, temp1, temp2);
  resta_vector(dir2, temp3, temp2);
  cruce_vector(normal1, dir1, dir2);
  tam1 = tam_vector(normal1);
  temp1 = poli->lados[3]->vector;
  temp2 = poli->lados[2]->vector;
  temp3 = poli->lados[1]->vector;
  resta_vector(dir1, temp1, temp2);
  resta_vector(dir2, temp3, temp2);
  cruce_vector(normal2, dir1, dir2);
  tam2 = tam_vector(normal2);
  if (tam1 == 0.0) {
    escala(normal, normal2, 1.0 / tam2);
  } else if (tam2 == 0.0) {
    escala(normal, normal1, 1.0 / tam1);
  } else {
    escala(normal1, normal1, 1.0 / tam1);
    escala(normal2, normal2, 1.0 / tam2);
    suma_vector(normal, normal1, normal2);
    escala(normal, normal, 0.5);
  }
}

/*
** Cálcula la normal de un vertice, saca la media de todos los polígonos
** que usan ese vertice.
*/
VECTOR calculo(vertice)
  VECTOR_EXT *vertice;
{
  LISTA_POLIGONOS *lista;
  VECTOR acumula;
  INT num_polis = 0;

  acumula.x = 0.0;
  acumula.y = 0.0;
  acumula.z = 0.0;
  lista = &vertice->lista;
  while (lista != NULL) {
    obtiene_normal(lista->poli);
    suma_vector(acumula, acumula, normal);
    lista = lista->sig;
    ++num_polis;
  }
  escala(acumula, acumula, 1.0 / num_polis);
  normaliza(acumula);
  return acumula;
}

/*
** Cálcula la normal de todos los vertices de un polígono.
*/
calcula_vertices(poli, normales)
  POLIGONO *poli;
  VECTOR normales[4];
{
  normales[0] = calculo(poli->lados[0]);
  normales[1] = calculo(poli->lados[1]);
  normales[2] = calculo(poli->lados[2]);
  normales[3] = calculo(poli->lados[3]);
}

REAL pow(x, y)
  REAL x;
  INT y;
{
  REAL BPower;
  INT t;

  if (y == 0)
    return 1.0;
  else {
    BPower = 1.0;
    for (t = 0; t < y; t++)
      BPower *= x;
    return BPower;
  }
}

/*
** Cálcula la intensidad de acuerdo a la normal.
*/
intensidad(objeto, r, g, b)
  OBJETO *objeto;
  INT *r, *g, *b;
{
  REAL coseno, coseno2, ami;
  VECTOR temp, ref, color, difuso;

  coseno = normal.x * luz.x + normal.y * luz.y + normal.z * luz.z;
  color.x = objeto->ambiente.x;
  color.y = objeto->ambiente.y;
  color.z = objeto->ambiente.z;
  if (coseno > 0.0) {
    escala(temp, normal, coseno * 2.0);
    normaliza(temp);
    resta_vector(ref, temp, luz);
    normaliza(ref);
    coseno2 = vista.x * ref.x + vista.y * ref.y + vista.z * ref.z;
    color.x += objeto->difusa.x * coseno;
    color.y += objeto->difusa.y * coseno;
    color.z += objeto->difusa.z * coseno;
    ami = pow(coseno2, (INT) objeto->lustre);
    color.x += objeto->especular.x * ami;
    color.y += objeto->especular.y * ami;
    color.z += objeto->especular.z * ami;
  }
  *r = (INT) (255 * color.x);
  *g = (INT) (255 * color.y);
  *b = (INT) (255 * color.z);
  if (*r < 0) *r = 0;
  if (*g < 0) *g = 0;
  if (*b < 0) *b = 0;
  if (*r > 255) *r = 255;
  if (*g > 255) *g = 255;
  if (*b > 255) *b = 255;
}

/*
** Sombrea un polígono con sombreado de Phong.
*/
sombreado_phong(poli, normales, objeto)
  PUNTO poli[4];
  VECTOR normales[4];
  OBJETO *objeto;
{
  INT r, g, b;
  INT min_y, max_y, min_x, max_x, x;
  INT a;
  INT linea;
  INT vertice_anterior, vertice;
  REAL angulo, escal;
  VECTOR vmin, vmax, vcal;
  VECTOR paso, temp, temp2;

  max_y = min_y = poli[0].y;
  for (a = 1; a < 4; a++) {
    if (poli[a].y > max_y)
      max_y = poli[a].y;
    if (poli[a].y < min_y)
      min_y = poli[a].y;
  }
  if (min_y < 0)
    min_y = 0;
  if (max_y >= res_y)
    max_y = res_y - 1;
  for (linea = min_y; linea <= max_y; linea++) {
    min_x = res_x + 1;
    max_x = -1;
    vertice_anterior = 3;
    for (vertice = 0; vertice < 4; vertice++) {
      if (poli[vertice_anterior].y >= linea || poli[vertice].y >= linea) {
        if (poli[vertice_anterior].y <= linea || poli[vertice].y <= linea) {
          if (poli[vertice_anterior].y != poli[vertice].y) {
            angulo = (REAL) (linea - poli[vertice_anterior].y) /
                            (poli[vertice].y - poli[vertice_anterior].y);
            if (angulo < 0.0)
              angulo = 0.0;
            if (angulo > 1.0)
              angulo = 1.0;
            x = round((1.0 - angulo) * poli[vertice_anterior].x +
                      (      angulo) * poli[vertice].x);
            escala(temp, normales[vertice_anterior], 1.0 - angulo);
            escala(temp2, normales[vertice], angulo);
            suma_vector(vcal, temp, temp2);
            if (x < min_x) {
              min_x = x;
              vmin = vcal;
            }
            if (x > max_x) {
              max_x = x;
              vmax = vcal;
            }
          }
        }
      }
      vertice_anterior = vertice;
    }
    if (min_x <= max_x) {
      resta_vector(temp, vmax, vmin);
      escal = 1.0 / (max_x - min_x + 1);
      escala(paso, temp, escal);
      normal = vmin;
      for (x = min_x; x <= max_x; x++) {
        if (x >= 0 && x < res_x) {
          intensidad(objeto, &r, &g, &b);
          dibuja_pixel(x, linea, r, g, b);
        }
        suma_vector(normal, normal, paso);
      }
    }
  }
}

/*
** Cálcula un polígono con sombreado de Gouraud.
*/
sombreado_gouraud(poli, normales, objeto)
  PUNTO poli[4];
  VECTOR normales[4];
  OBJETO *objeto;
{
  INT r, g, b;
  INT min_y, max_y, min_x, max_x, x;
  INT a;
  INT linea;
  INT vertice_anterior, vertice;
  REAL angulo, escal;
  VECTOR vmin, vmax, vcal;
  VECTOR paso, temp, temp2;

  for (a = 0; a < 4; a++) {
    normal = normales[a];
    intensidad(objeto, &r, &g, &b);
    normales[a].x = r;
    normales[a].y = g;
    normales[a].z = b;
  }
  max_y = min_y = poli[0].y;
  for (a = 1; a < 4; a++) {
    if (poli[a].y > max_y)
      max_y = poli[a].y;
    if (poli[a].y < min_y)
      min_y = poli[a].y;
  }
  if (min_y < 0)
    min_y = 0;
  if (max_y >= res_y)
    max_y = res_y - 1;
  for (linea = min_y; linea <= max_y; linea++) {
    min_x = res_x + 1;
    max_x = -1;
    vertice_anterior = 3;
    for (vertice = 0; vertice < 4; vertice++) {
      if (poli[vertice_anterior].y >= linea || poli[vertice].y >= linea) {
        if (poli[vertice_anterior].y <= linea || poli[vertice].y <= linea) {
          if (poli[vertice_anterior].y != poli[vertice].y) {
            angulo = (REAL) (linea - poli[vertice_anterior].y) /
                            (poli[vertice].y - poli[vertice_anterior].y);
            if (angulo < 0.0)
              angulo = 0.0;
            if (angulo > 1.0)
              angulo = 1.0;
            x = round((1.0 - angulo) * poli[vertice_anterior].x +
                      (      angulo) * poli[vertice].x);
            escala(temp, normales[vertice_anterior], 1.0 - angulo);
            escala(temp2, normales[vertice], angulo);
            suma_vector(vcal, temp, temp2);
            if (x < min_x) {
              min_x = x;
              vmin = vcal;
            }
            if (x > max_x) {
              max_x = x;
              vmax = vcal;
            }
          }
        }
      }
      vertice_anterior = vertice;
    }
    if (min_x <= max_x) {
      resta_vector(temp, vmax, vmin);
      escal = 1.0 / (max_x - min_x + 1);
      escala(paso, temp, escal);
      normal = vmin;
      for (x = min_x; x <= max_x; x++) {
        if (x >= 0 && x < res_x)
          dibuja_pixel(x, linea, (int) normal.x, (int) normal.y,
                                 (int) normal.z);
        suma_vector(normal, normal, paso);
      }
    }
  }
}

/*
** Sombrea un polígono con sombreado constante.
*/
sombreado_constante(poli, objeto, r, g, b)
  PUNTO poli[4];
  OBJETO *objeto;
  INT r, g, b;
{
  INT min_y, max_y, min_x, max_x, x;
  INT a;
  INT linea;
  INT vertice_anterior, vertice;
  REAL angulo, escal;

  max_y = min_y = poli[0].y;
  for (a = 1; a < 4; a++) {
    if (poli[a].y > max_y)
      max_y = poli[a].y;
    if (poli[a].y < min_y)
      min_y = poli[a].y;
  }
  if (min_y < 0)
    min_y = 0;
  if (max_y >= res_y)
    max_y = res_y - 1;
  for (linea = min_y; linea <= max_y; linea++) {
    min_x = res_x + 1;
    max_x = -1;
    vertice_anterior = 3;
    for (vertice = 0; vertice < 4; vertice++) {
      if (poli[vertice_anterior].y >= linea || poli[vertice].y >= linea) {
        if (poli[vertice_anterior].y <= linea || poli[vertice].y <= linea) {
          if (poli[vertice_anterior].y != poli[vertice].y) {
            angulo = (REAL) (linea - poli[vertice_anterior].y) /
                            (poli[vertice].y - poli[vertice_anterior].y);
            if (angulo < 0.0)
              angulo = 0.0;
            if (angulo > 1.0)
              angulo = 1.0;
            x = round((1.0 - angulo) * poli[vertice_anterior].x +
                      (      angulo) * poli[vertice].x);
            if (x < min_x)
              min_x = x;
            if (x > max_x)
              max_x = x;
          }
        }
      }
      vertice_anterior = vertice;
    }
    if (min_x <= max_x)
      for (x = min_x; x <= max_x; x++)
        if (x >= 0 && x < res_x)
          dibuja_pixel(x, linea, r, g, b);
  }
}

toupper(car)
  int car;
{
  if (car >= 0x61 && car <= 0x7a)
    return car - 32;
  return car;
}

/*
** Marca los bordes de un polígono.
*/
puntos_poligono(poli)
  PUNTO poli[4];
{
  dibuja_pix(poli[0].x, poli[0].y);
  dibuja_pix(poli[1].x, poli[1].y);
  dibuja_pix(poli[2].x, poli[2].y);
  dibuja_pix(poli[3].x, poli[3].y);
}

/*
** Dibuja los bordes de un polígono con líneas.
*/
lineas_poligono(poli)
  PUNTO poli[4];
{
  INT a, b;

  for (a = 0; a < 4; a++) {
    b = (a + 1) % 4;
    linea_video(poli[a].x, poli[a].y, poli[b].x, poli[b].y);
  }
}

/*
** Dibuja una línea en el video.
*/
linea_video(x1, y1, x2, y2)
  INT x1, y1, x2, y2;
{
  INT diffx, diffy, dirx, diry;
  INT ciclo;

  diffx = x2 - x1;
  diffy = y2 - y1;
  if (diffx < 0) {
    diffx = -diffx;
    dirx = -1;
  } else
    dirx = 1;
  if (diffy < 0) {
    diffy = -diffy;
    diry = -1;
  } else
    diry = 1;
  if (diffy < diffx) {
    ciclo = diffx >> 1;
    while (x1 != x2) {
      dibuja_pix(x1, y1);
      ciclo += diffy;
      if (ciclo > diffx) {
        ciclo -= diffx;
        y1 += diry;
      }
      x1 += dirx;
    }
  } else {
    ciclo = diffy >> 1;
    while (y1 != y2) {
      dibuja_pix(x1, y1);
      ciclo += diffx;
      if (ciclo > diffy) {
        ciclo -= diffy;
        x1 += dirx;
      }
      y1 += diry;
    }
  }
  dibuja_pix(x1, y1);
}

dibuja_pix(x, y)
  INT x, y;
{
  if (x >= 0 && x < res_x && y >= 0 && y < res_y)
    dibuja_pixel_256(x, y, 255);
}

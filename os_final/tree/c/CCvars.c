/*
** Compilador de C para transputer.
** Definiciones de variables.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 2 de junio de 1995.
** Revisión: 25 de julio de 1995. Se agregan N_ANDB y N_ORB, se comentan
**                                las operaciones.
** Revisión: 25 de julio de 1995. Se agrega N_TRI.
** Revisión: 25 de julio de 1995. Se elimina la variable modo_c.
** Revisión: 25 de julio de 1995. Se agrega las variables dentro_pp, nivel_if
**                                y evadir_nivel.
** Revisión: 25 de julio de 1995. Se agregan las macros de tipos para futura
**                                expansión del compilador.
** Revisión: 26 de julio de 1995. Se agregan las macros N_A??? para operadores
**                                de asignación, desaparecen N_GBYTE y N_GPAL.
** Revisión: 26 de julio de 1995. Se agrega la macro N_AIXP.
** Revisión: 26 de julio de 1995. Se elimina la macro E_ASM.
** Revisión: 27 de julio de 1995. Se agrega la macro N_COMA.
** Revisión: 10 de agosto de 1995. Se agregan las macros CHAR, SHORT, USHORT,
**                                 UINT, VOID, FLOAT, y DOUBLE.
** Revisión: 10 de agosto de 1995. Se agregan las variables sig_tipo,
**                                 tipo_basico, tipo_proc, t_char, t_short,
**                                 t_int, t_ushort, t_uint, t_float, t_double,
**                                 t_achar, t_void y la matriz tipos.
** Revisión: 11 de agosto de 1995. Se agrega la variable t_func.
** Revisión: 11 de agosto de 1995. Se agrega la variable multi.
** Revisión: 12 de agosto de 1995. Nueva variable. nivel_include. Desaparecen
**                                 las variables c_funcion_actual,
**                                 c_comienzo_funcion, c_linea_actual, y
**                                 c_dentro_funcion. Nueva matriz. incl.
** Revisión: 22 de agosto de 1995. Nueva macro STRUCT, nueva variable
**                                 lista_struct.
** Revisión: 23 de agosto de 1995. Se agrega la variable ultima_estruct,
**                                 las macros EST_QUE_ES, EST_ES_UNION,
**                                 EST_TAM, EST_LISTA, EST_SIG, EST_NOMBRE,
**                                 MIE_TIPO, MIE_POSICION, MIE_SIG, y
**                                 MIE_NOMBRE.
** Revisión: 24 de agosto de 1995. Se agregan las variables lista_enum y
**                                 ultimo_enum, las macros ENUM_VALOR,
**                                 ENUM_SIG y ENUM_NOMBRE.
** Revisión: 5 de septiembre de 1995. Se agrega N_COPIA y N_RESULTA.
** Revisión: 6 de septiembre de 1995. Se agrega la macro MAX_INIC, y las
**                                    variables nodo_inic y vars_inicializadas.
** Revisión: 12 de septiembre de 1995. Ahora MAX_INIC es por 3.
** Revisión: 12 de septiembre de 1995. Se añade N_CFLOAT, N_CDOUBLE, N_IGUALPF,
**                                     N_MAYORPF, N_SUMAPF, N_RESTAPF, N_MULPF,
**                                     N_DIVPF, N_CEROPF, N_NUMPF, N_ENTPF,
**                                     N_PFENT, N_PARF y N_IXF.
** Revisión: 12 de septiembre de 1995. Nueva matriz regsf[].
** Revisión: 22 de noviembre de 1995. Se añade MAX_CONST, TAM_DOUBLE, la matriz
**                                    constantes y la variable const_definidas.
** Revisión: 29 de noviembre de 1995. Se añaden las macros car_act() y
**                                    nueva_etiq(), también N_CONVFD.
** Revisión: 30 de noviembre de 1995. Se añaden las macros N_CDI y N_CFI para
**                                    optimación interna.
** Revisión: 1o. de diciembre de 1995. Se añaden las macros N_ENTF y N_CONVDF.
** Revisión: 1o. de enero de 1996. Mejoras mínimas.
** Revisión: 20 de junio de 1996. Se añade la macro NIVEL.
*/
 
#define PROGRAMA     "Compilador de C para transputer  (c) Copyright 1995-1996 Oscar Toledo G."

#define NO           0
#define SI           1

#define NULL         0

/* Define parametros de la tabla de nombres */

#define TAM_SIM      28
#define TAM_TABLA    17920
#define NUM_GLBS     608
#define INICIO_GLB   tabla
#define FIN_GLB      (INICIO_GLB+NUM_GLBS*TAM_SIM)
#define INICIO_LOC   (FIN_GLB+TAM_SIM)
#define FIN_LOC      tabla+(TAM_TABLA-TAM_SIM)

/* Define formato de los nombres */

#define NOMBRE       0
#define IDENT        17
#define CLASE        18
#define NIVEL        19
#define TIPO         20
#define POSICION     24

/* Tamaño máximo de los nombres */

#define TAM_NOMBRE   17
#define MAX_NOMBRE   16

/* Valores posibles para "IDENT" */

#define VARIABLE     1
#define ETIQUETA     2
#define FUNCION      3
#define TYPEDEF      4

/* Valores posibles para "CLASE" */

#define STATIC       1
#define AUTO         2
#define EXTERN       3

/* Valores posibles para "TIPO" */

#define CHAR         0
#define SHORT        1
#define INT          2
#define USHORT       3
#define UINT         4
#define FLOAT        5
#define DOUBLE       6
#define VOID         7  /* Esta es una caracteristica del C K&R añadida */
                        /* con el *NIX versión 7, así cómo la asignación */
                        /* y paso cómo parametros de estructuras, y enum. */
#define STRUCT       8  /* Se considera cómo un tipo básico */
#define ENUM         9

#define APUNTADOR   10
#define MATRIZ      11
#define FUNCION     12

#define FUNC_REF     0
#define FUNC_TIPO    1
#define FUNC_DEF     2

/* Define los desplazamientos en la cola de while's */

#define B_ANTERIOR   0
#define B_PILA       1
#define B_BUCLE      2
#define B_FIN        3

/* Define el almacenamiento de cadenas */

#define TAM_LITS     4096
#define MAX_LITS     (TAM_LITS-1)

/* Define la linea de entrada */

#define TAM_LINEA    512
#define MAX_LINEA    (TAM_LINEA-1)

/* Define el almacenamiento de macros */

#define TAM_MAC      16384
#define MAX_MAC      (TAM_MAC-1)

/* Define el espacio disponible para substitución de argumentos de macros */

#define TAM_AMAC     1024
#define MAX_AMAC     (TAM_AMAC-1)

/* Define el espacio disponible para definiciones de tipos */

#define TAM_TIPOS    10240
#define MAX_TIPOS    (tipos+TAM_TIPOS)

/* Define los tipos de sentencias */

#define E_IF         1
#define E_WHILE      2
#define E_RETURN     3
#define E_BREAK      4
#define E_CONT       5
#define E_EXPR       6

/* Número máximo de cases * 2 */

#define MAX_CASOS    200

/* Número máximo de #include * 5 */

#define MAX_INCL     50

/* Definiciones de estructura */

#define EST_QUE_ES    0   /* char, indica si es un rótulo de struct o enum */
#define EST_ES_UNION  1   /* char, indica si es una unión o una estructura */
#define EST_TAM       2   /* int, tamaño total de la estructura/unión */
#define EST_LISTA     6   /* char*, lista de miembros */
#define EST_SIG      10   /* char*, siguiente rótulo */
#define EST_NOMBRE   14   /* char[], rótulo */

/* Definiciones de miembros */

#define MIE_TIPO      0   /* char*, tipo del miembro */
#define MIE_POSICION  4   /* int, posición dentro de la estructura */
#define MIE_SIG       8   /* char*, siguiente miembro */
#define MIE_NOMBRE   12   /* nombre del miembro */

/* Definiciones de enumeradores */

#define ENUM_VALOR    0   /* int, valor del enumerador */
#define ENUM_SIG      4   /* char*, siguiente enumerador */
#define ENUM_NOMBRE   8   /* char[], nombre del enumerador */

/* Reserva espacio para las variables */

char *ap_glb, *ap_loc;  /* Apuntadores a las sigs. entradas libres en */
                        /* la tabla de nombres */

int *ultimo_bucle;      /* Apuntador al último bucle abierto */

int ap_lit;             /* Apuntador a la sig. entrada para las cadenas */

int ap_mac;             /* Indice en el buffer de macros */

int pos_linea,          /* Apuntadores a las lineas de análisis */
    pos_linea_m;

/* Almacenamiento miscelaneo */

int sig_etiq,           /* Siguiente etiqueta disponible */
    etiq_lit,           /* Etiqueta para el buffer de cadenas */
    pila,               /* Apuntador de pila del compilador */
    pila_args,          /* Pila de argumentos */
    nivel,              /* No. de bloques abiertos */
    errores,            /* No. de errores detectados */
    pausa,              /* Indica si se detiene en caso de error */
    eof,                /* Indica el final del archivo de entrada */
    entrada,            /* Archivo de entrada */
    salida,             /* Archivo de salida */
    entrada2,           /* Archivo #include */
    intercala_fuente,   /* Indica si incluye el prog. en la salida */
    ultima_sentencia,   /* Ultima sentencia ejecutada */
    desvio_salida,      /* Indica desvio de la salida a la consola */
    comienzo_funcion,   /* Linea de comienzo de la funcion actual */
    linea_actual,       /* Linea en el archivo actual */
    dentro_funcion,     /* Indica si esta dentro de una funcion */
    dentro_pp,          /* Indica si esta dentro del preprocesador */
    nivel_if,           /* Nivel de anidamiento de #if... */
    nivel_incl,         /* Nivel de anidamiento de #include */
    evadir_nivel,       /* Nivel que esta evadiendo en el preprocesador */
    dentro_switch,      /* Indica si esta dentro de una sentencia switch */
    etiqueta_default,   /* Etiqueta para el default */
   *inicio_lista,       /* Inicio de la lista de cases */
   *sig_case,           /* Siguiente posición disponible para un case */
    casos[MAX_CASOS],   /* Hasta 100 cases */
    incl[MAX_INCL];     /* Almacenamiento de #include */

char *funcion_actual,   /* Apuntador a la definicion de la función actual */
     *sig_tipo,         /* Sig. posición disponible en la tabla de tipos */
     *tipo_basico,      /* Tipo básico de la declaración actual */
     *tipo_proc,        /* Tipo procesado */
     *t_char,           /* Tipo char o unsigned char */
     *t_short,          /* Tipo short */
     *t_int,            /* Tipo int o long */
     *t_ushort,         /* Tipo unsigned short */
     *t_uint,           /* Tipo unsigned int o unsigned long */
     *t_float,          /* Tipo float */
     *t_double,         /* Tipo double */
     *t_void,           /* Tipo void */
     *t_achar,          /* Tipo apuntador a char */
     *t_func;           /* Función que retorna int */

char *lista_estruct,    /* Lista de nombres de estructuras */
     *ultima_estruct,   /* Ultima estructura definida */
     *lista_enum,       /* Lista de constantes de enumeradores */
     *ultimo_enum;      /* Ultimo enumerador definido */

char *ap_c;             /* Apuntador de trabajo */
int *ap_e;              /* Apuntador de trabajo */
int pos_global;         /* Posición para variables estáticas */
int usa_expr;           /* Indica si se usa el resultado de la expr. */

char args[2];           /* Indica si las dos palabras están ocupadas */
                        /* Un call asigna 4 palabras, una es el retorno y */
                        /* otra el enlace estático, las otras dos pueden */
                        /* ser argumentos o estar desocupadas */
char linea[TAM_LINEA];  /* Buffer de analisis */
char linea_m[TAM_LINEA];/* Buffer para el preproceso */
char lits[TAM_LITS];    /* Almacenamiento de cadenas literales */
char macs[TAM_MAC];     /* Buffer de macros */
char amacs[TAM_AMAC];   /* Buffer para argumentos de macros */
char tipos[TAM_TIPOS];  /* Tabla de tipos */
char tabla[TAM_TABLA];  /* Tabla de nombres */

#define MAX_INIC   48   /* Máximo número de inicializaciones de variables */
                        /* automáticas locales * 3. */

int nodo_inic[MAX_INIC]; /* Nodo correspondiente a la inicialización */
int vars_inicializadas;  /* Variables inicializadas */

#define TAM_ARBOL 128    /* Tamaño máximo de una expresión */

int nodo_izq[TAM_ARBOL]; /* Descendiente izquierdo del nodo */
int nodo_der[TAM_ARBOL]; /* Descendiente derecho del nodo */
int oper[TAM_ARBOL];     /* Operador correspondiente al nodo */
int esp[TAM_ARBOL];      /* Valor correspondiente al nodo */
int regs[TAM_ARBOL];     /* Registros requeridos para evaluar el nodo */
int regsf[TAM_ARBOL];    /* Registros requeridos para evaluar el nodo */
int ultimo_nodo;         /* Ultimo nodo definido */
int raiz_arbol;          /* Raíz del arbol actual */
int es_control;          /* Indica si la expresión es para una sentencia */
                         /* de control */
int etiq_and, etiq_or;   /* Etiquetas de salida para && y || */
int multi;               /* Multiplicación para suma y resta con apuntadores */

#define MAX_CONST 200    /* Número máximo de constantes de punto flotante */
#define TAM_DOUBLE  8    /* Tamaño en bytes del tipo double */

union {
  double valor;          /* Valor de la constante */
  char byte[TAM_DOUBLE]; /* Esto es dependiente de la máquina */
} constantes[MAX_CONST];

int const_definidas;     /* Constantes definidas */

/* Tipos de operadores */

#define N_OR      1      /* OR binario */
#define N_XOR     2      /* XOR binario */
#define N_AND     3      /* AND binario */
#define N_IGUAL   4      /* Compara si es igual */
#define N_CIGUAL  5      /* Comparación con constante */
#define N_MAYOR   6      /* Compara si es mayor */
#define N_CSUMA   7      /* Suma de constante */
#define N_NULO    8      /* Para un nodo sin operación */
#define N_STNL    9      /* Almacena una palabra indirecta */
#define N_SMAYOR  10     /* Compara si es mayor sin signo */
#define N_FUNC    11     /* Llamada a función */
#define N_FUNCI   12     /* Llamada a función indirecta */
#define N_PAR     13     /* Parametro de una función */
#define N_CD      14     /* Corrimiento a la derecha */
#define N_CI      15     /* Corrimiento a la izquierda */
#define N_SUMA    16     /* Suma */
#define N_RESTA   17     /* Resta */
#define N_MUL     18     /* Multiplicación */
#define N_DIV     19     /* División */
#define N_MOD     20     /* Resto */
#define N_NEG     21     /* Negación */
#define N_COM     22     /* Complemento binario */
#define N_INC     23     /* Preincremento */
#define N_STL     24     /* Almacena una variable local */
#define N_PINC    25     /* Posincremento */
#define N_LDNL    26     /* Obtiene una palabra indirecta */
#define N_NOT     27     /* NOT boleano */
#define N_IXP     28     /* Indexa una dirección por palabras */
#define N_APFUNC  29     /* Obtiene un apuntador a una función */
#define N_CONST   30     /* Carga de una constante */
#define N_LIT     31     /* Carga la dirección de una cadena */
#define N_CBYTE   32     /* Lee un byte de la memoria */
#define N_CPAL    33     /* Lee una palabra de la memoria */
#define N_LDLP    34     /* Obtiene un apuntador local */
#define N_LDL     35     /* Obtiene una variable local */
#define N_LDNLP   36     /* Suma un desplazamiento en palabras */
#define N_ANDB    37     /* AND boleano */
#define N_ORB     38     /* OR boleano */
#define N_TRI     39     /* Operador trinario ?: */
#define N_ASIGNA  40     /* Asignación normal */
#define N_AOR     41     /* |= */
#define N_AXOR    42     /* ^= */
#define N_AAND    43     /* &= */
#define N_ACI     44     /* <<= */
#define N_ACD     45     /* >>= */
#define N_ASUMA   46     /* += */
#define N_ARESTA  47     /* -= */
#define N_AMUL    48     /* *= */
#define N_ADIV    49     /* /= */
#define N_AMOD    50     /* %= */
#define N_AIXP    51     /* += 4* */
#define N_CUENTA  52     /* Cuenta número de palabras */
#define N_COMA    53     /* Operador coma */
#define N_CSHORT  54     /* Obtiene una palabra corta indirecta */
#define N_CUSHORT 55     /* Obtiene una palabra corta sin signo indirecta */
#define N_COPIA   56     /* Copia un bloque de memoria */
#define N_RESULTA 57     /* Reserva espacio para recibir una estructura */
#define N_CFLOAT  58     /* Lee un float de la memoria, convierte a double */
#define N_CDOUBLE 59     /* Lee un double de la memoria */
#define N_IGUALPF 60     /* Compara si es igual (punto flotante) */
#define N_MAYORPF 61     /* Compara si es mayor (punto flotante) */
#define N_SUMAPF  62     /* Suma (punto flotante) */
#define N_RESTAPF 63     /* Resta (punto flotante) */
#define N_MULPF   64     /* Multiplicación (punto flotante) */
#define N_DIVPF   65     /* División (punto flotante) */
#define N_CEROPF  66     /* Carga cero en precisión doble */
#define N_NUMPF   67     /* Carga un número en la pila de punto flotante */
#define N_ENTPF   68     /* Convierte un entero a double */
#define N_PFENT   69     /* Convierte punto flotante a entero */
#define N_IXF     70     /* Indexa una dirección por palabras dobles */
#define N_PARF    71     /* Parametro de una función (punto flotante) */
#define N_CONVFD  72     /* Convierte float en double */
#define N_CFI     73     /* Lee un float indexado de la memoria */
#define N_CDI     74     /* Lee un double indexado de la memoria */
#define N_CONVDF  75     /* Convierte un double a float */
#define N_ENTF    76     /* Convierte un entero a float */
#define N_CEROF   77     /* Carga cero de precisión simple */

/*
** Macros para acelerar el compilador.
*/

/*
** Caracter en la posición actual.
*/
#define car_act           linea[pos_linea]

/*
** Siguiente etiqueta interna disponible.
*/
#define nueva_etiq        (sig_etiq++)

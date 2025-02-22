/*
 ** Transputer assembler.
 **
 ** (c) Copyright 1995-2025 Oscar Toledo G.
 ** https://nanochess.org/
 **
 ** Creation date: Feb/01/2025. Ported from old code from Jun/14/1995.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define NO       0
#define SI       1

#define TAM_LIN  128
#define TAM_BUF  2048

/*
 ** Etiqueta del ensamblador
 */
struct etiqueta {
    struct etiqueta *siguiente;   /* Siguiente etiqueta en la tabla */
    struct etiqueta *secuencia;   /* Secuencia lineal */
    int tipo;                     /* 0= Es una dirección, 1= Es un dato */
    int dato;                     /* Dato/dirección */
    char nombre[1];               /* Nombre de la etiqueta, terminado en 0 */
};

/*
 ** Indefinido para ensanchamiento
 */
struct indefinido {
    struct indefinido *siguiente; /* Siguiente indefinido */
    int codigo;                   /* Código de instrucción (0 a 15), 16 indica */
    /* db y 17 indica dw */
    int direccion;                /* Dirección */
    char expresion[1];            /* Expresión */
};

/*
 ** Tabla de instrucciones
 */
char *tabla[] = {
    
    /*
     ** Instrucciones básicas
     */
    "j", "ldlp", "pfix", "ldnl", "ldc", "ldnlp", "nfix", "ldl",
    "adc", "call", "cj", "ajw", "eqc", "stl", "stnl", "opr",
    
    /*
     ** Operaciones básicas
     */
    "rev", "lb", "bsub", "endp", "diff", "add", "gcall", "in",
    "prod", "gt", "wsub", "out", "sub", "startp", "outbyte", "outword",
    
    /*
     ** Instrucciones
     */
    "seterr", "?", "resetch", "csub0", "?", "stopp", "ladd", "stlb",
    "sthf", "norm", "ldiv", "ldpi", "stlf", "xdble", "ldpri", "rem",
    "ret", "lend", "ldtimer", "?", "?", "?", "?", "?",
    "?", "testerr", "testpranal", "tin", "div", "?", "dist", "disc",
    "diss", "lmul", "not", "xor", "bcnt", "lshr", "lshl", "lsum",
    "lsub", "runp", "xword", "sb", "gajw", "savel", "saveh", "wcnt",
    "shr", "shl", "mint", "alt", "altwt", "altend", "and", "enbt",
    "enbc", "enbs", "move", "or", "csngl", "ccnt1", "talt", "ldiff",
    "sthb", "taltwt", "sum", "mul", "sttimer", "stoperr", "cword", "clrhalterr",
    "sethalterr", "testhalterr", "dup", "move2dinit",
    "move2dall", "move2dnonzero", "move2dzero", "?",
    "?", "?", "?", "unpacksn", "?", "?", "?", "?",
    "?", "?", "?", "?", "postnormsn", "roundsn", "?", "?",
    "?", "ldinf", "fmul", "cflerr", "crcword", "crcbyte", "bitcnt", "bitrevword",
    "bitrevnbits", "pop", "timerdisableh", "timerdisablel",
    "timerenableh", "timerenablel", "ldmemstartval", "?",
    "?", "wsubdb", "fpldnldbi", "fpchkerror",
    "fpstnldb", "?", "fpldnlsni", "fpadd",
    "fpstnlsn", "fpsub", "fpldnldb", "fpmul",
    "fpdiv", "?", "fpldnlsn", "fpremfirst",
    "fpremstep", "fpnan", "fpordered", "fpnotfinite",
    "fpgt", "fpeq", "fpi32tor32", "?",
    "fpi32tor64", "?", "fpb32tor64", "?",
    "fptesterror", "fprtoi32", "fpstnli32", "fpldzerosn",
    "fpldzerodb", "fpint", "?", "fpdup", "fprev", "?", "fpldnladddb", "?",
    "fpldnlmuldb", "?", "fpldnladdsn", "fpentry", "fpldnlmulsn", "?", "?", "?",
    "?", "break", "clrj0break", "setj0break", "testj0break", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    
    /*
     ** Instrucciones de microcódigo de la FPU
     */
    "?", "fpusqrtfirst", "fpusqrtstep", "fpusqrtlast",
    "fpurp", "fpurm", "fpurz", "fpur32tor64",
    "fpur64tor32", "fpuexpdec32", "fpuexpinc32", "fpuabs",
    "?", "fpunoround", "fpuchki32", "fpuchki64",
    "?", "fpudivby2", "fpumulby2", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "fpurn", "fpuseterror", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "fpuclearerror", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
};

/*
 ** Tabla de dispersión
 */
struct etiqueta *dispersion[256];  /* Tabla de dispersión para etiquetas */
struct etiqueta *ultima_definida;  /* Última etiqueta definida */

char *input_file;
char *output_file;
char *temporary1_file;
char *temporary2_file;
char *library_file;

int paso;                          /* Paso del ensamblaje */
int err;
FILE *archivo_entrada;
char *ap_proceso;                  /* Apuntador de proceso (en línea) */

char preins[8];
char oriins[8];
int acumula;

FILE *temp1;
FILE *temp2;
int linea_actual;                  /* Línea actual de proceso */
int errores_detectados;            /* Total de errores detectados */
int disponible;
int pos_ens;
char *pos_global;
struct etiqueta *primer_etiq;
struct indefinido *primer_indef;
struct indefinido *ultimo_indef;
int num_indef;
int ptemp1;
int ptemp2;
char nom[15];
char linea[TAM_LIN];
char separa[TAM_LIN];
char separa2[TAM_LIN];
char etiq_indef[TAM_LIN];
char btemp1[TAM_BUF];
char btemp2[TAM_BUF];

void ensambla(void);
int calcula_dispersion(char *);
struct etiqueta *define_etiqueta(char *);
struct etiqueta *busca_etiqueta(char *);
void libera_memoria(void);
char *separa_componente(void);
void procesa(void);
void verifica_final(void);
void error_extras(void);
void ins_op(int);
void agrega_indefinido(int);
int evalua_expresion(void);
int eval1(void);
int eval2(void);
int eval3(void);
int eval_hex(void);
int eval_dec(void);
void gen_ins(int, int);
void ins_sim(int);
void ins_ext(int);
void def_byte(void);
void def_pal32(void);
void def_espacio(void);
void def_equiv(void);
int compara(char *, char *);
int lee_linea(void);
void enlaza(void);
void repaso(void);
void copia(int, int);
void etemp1(int);
void vtemp1(void);
void etemp2(int);
void vtemp2(void);
int ltemp1(void);
void error(char *, char *);

/*
 ** Main program
 */
int main(int argc, char *argv[])
{
    fprintf(stderr, "\n");
    fprintf(stderr, "Transputer assembler v0.1. Feb/01/2025\n");
    fprintf(stderr, "by Oscar Toledo G. https://nanochess.org/\n\n");
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage: tasm input.len output.cmg library.len\n\n");
        exit(1);
    }
    input_file = argv[1];
    output_file = argv[2];
    if (argc == 4)
        library_file = argv[3];
    else
        library_file = NULL;
    ensambla();
    exit(0);
}

/*
 ** Realiza el ensamblaje
 */
void ensambla(void)
{
    temporary1_file = "_tasm1.tmp";
    temporary2_file = "_tasm2.tmp";
    disponible = 1;
    primer_etiq = NULL;
    ultima_definida = NULL;
    primer_indef = NULL;
    ultimo_indef = NULL;
    num_indef = 0;
    pos_ens = 0;
    archivo_entrada = fopen(input_file, "r");
    if (archivo_entrada == NULL) {
        fprintf(stderr, "Unable to open '%s'\n", input_file);
        return;
    }
    temp1 = fopen(temporary1_file, "wb");
    if (temp1 == NULL) {
        fprintf(stderr, "Couldn't open temporary write file\n");
        fclose(archivo_entrada);
        return;
    }
    ptemp1 = 0;
    paso = 0;
    linea_actual = 0;
    procesa();
    fclose(archivo_entrada);
    if (library_file != NULL) {
        archivo_entrada = fopen(library_file, "r");
        if (archivo_entrada == NULL) {
            fprintf(stderr, "Unable to open '%s'\n", library_file);
        } else {
            procesa();
            fclose(archivo_entrada);
        }
    }
    vtemp1();
    paso = 1;
    enlaza();
    libera_memoria();
    fprintf(stderr, "%d error(s) detected.\n", errores_detectados);
    fprintf(stderr, "%d line(s) assembled.\n\n", linea_actual);
}

/*
 ** Cálcula el valor de dispersión para un nombre
 */
int calcula_dispersion(char *nombre)
{
    int valor;
    
    valor = 0;
    while (*nombre)
        valor = (valor << 1) + (*nombre++ & 0xff);
    return valor & 0xff;
}

/*
 ** Define una nueva etiqueta
 */
struct etiqueta *define_etiqueta(char *nombre)
{
    struct etiqueta *nueva;
    int c;
    
    nueva = malloc(sizeof(struct etiqueta) + strlen(nombre) + 1);
    if (nueva == NULL)
        return NULL;
    c = calcula_dispersion(nombre);
    nueva->siguiente = dispersion[c];
    dispersion[c] = nueva;
    nueva->secuencia = NULL;
    nueva->tipo = 0;
    nueva->dato = pos_ens;
    strcpy(nueva->nombre, nombre);
    if (primer_etiq == NULL)
        primer_etiq = nueva;
    if (ultima_definida != NULL)
        ultima_definida->secuencia = nueva;
    ultima_definida = nueva;
    return nueva;
}

/*
 ** Busca una etiqueta
 */
struct etiqueta *busca_etiqueta(char *nombre)
{
    struct etiqueta *lista;
    int c;
    
    c = calcula_dispersion(nombre);
    lista = dispersion[c];
    while (lista != NULL) {
        if (strcmp(lista->nombre, nombre) == 0)
            return lista;
        lista = lista->siguiente;
    }
    return NULL;
}

/*
 ** Libera la memoria asignada
 */
void libera_memoria(void)
{
    struct etiqueta *lista;
    struct etiqueta *siguiente;
    struct indefinido *lista2;
    struct indefinido *siguiente2;
    int c;
    
    for (c = 0; c < 256; c++) {
        lista = dispersion[c];
        while (lista != NULL) {
            siguiente = lista->siguiente;
            free(lista);
            lista = siguiente;
        }
        dispersion[c] = NULL;
    }
    primer_etiq = NULL;
    ultima_definida = NULL;
    lista2 = primer_indef;
    while (lista2 != NULL) {
        siguiente2 = lista2->siguiente;
        free(lista2);
        lista2 = siguiente2;
    }
    primer_indef = NULL;
    ultimo_indef = NULL;
}

/*
 ** Separa un componente de la línea de entrada
 */
char *separa_componente(void)
{
    char *ap1 = ap_proceso;
    char *ap2 = separa;
    int c;
    
    while (isspace(*ap1))
        ap1++;
    if (*ap1 == ';') {  /* Los comentarios son anulados */
        *ap2 = '\0';
        return ap2;
    }
    c = 0;
    while (*ap1) {
        if (c == 0 && isspace(*ap1))
            break;
        if (*ap1 == '"') {
            if (c & 1) {
                if (ap1[1] == '"') {
                    *ap2++ = *ap1++;
                    *ap2++ = *ap1++;
                    continue;
                }
            }
            c ^= 1;
        }
        if (*ap1 =='\'') {
            if (c & 2) {
                if (ap1[1] == '\'') {
                    *ap2++ = *ap1++;
                    *ap2++ = *ap1++;
                    continue;
                }
            }
            c ^= 2;
        }
        if (c)
            *ap2++ = *ap1;
        else
            *ap2++ = tolower(*ap1);
        ap1++;
    }
    *ap2 = '\0';
    while (isspace(*ap1))
        ap1++;
    ap_proceso = ap1;
    if (ap2 != separa)
        return ap2 - 1;
    return ap2;
}

/*
 ** Procesa el archivo de entrada
 */
void procesa(void)
{
    int instruccion;
    int etiq;
    char *ap;
    char *final;
    
    while (!lee_linea()) {
        etiq = NO;
        while (1) {
            ap = separa_componente();
            if (*ap == ':') {
                *ap = '\0';
                if (isalpha(*separa) || *separa == '_') {
                    if (busca_etiqueta(separa) != NULL) {
                        error("Etiqueta redefinida", separa);
                    } else if (define_etiqueta(separa) == NULL) {
                        error("Sin memoria", NULL);
                        return;
                    }
                } else {
                    error("Etiqueta inválida", separa);
                }
                ap = separa_componente();
            }
            if (separa[0] == '\0')
                break;
            if (compara("db", separa)) {
                def_byte();
                verifica_final();
                break;
            } else if (compara("dw", separa)) {
                def_pal32();
                verifica_final();
                break;
            } else if (compara("ds", separa)) {
                def_espacio();
                verifica_final();
                break;
            } else if (compara("equ", separa)) {
                def_equiv();
                verifica_final();
                break;
            } else {
                instruccion = 0;
                while (instruccion < 528) {
                    ap = tabla[instruccion++];
                    if (*ap == '?')
                        continue;
                    if (compara(ap, separa)) {
                        --instruccion;
                        if (instruccion < 16)
                            ins_op(instruccion);
                        else if (instruccion < 272)
                            ins_sim(instruccion - 16);
                        else if (instruccion < 528)
                            ins_ext(instruccion - 272);
                        verifica_final();
                        break;
                    }
                }
                if (instruccion == 528)
                    error("Instrucción indefinida", separa);
                else
                    break;
            }
        }
    }
    return;
}

/*
 ** Verifica el final correcto de una instrucción
 */
void verifica_final(void)
{
    if (*ap_proceso && *ap_proceso != ';')
        error_extras();
}

/*
 ** Indica un error de caracteres extras en la línea
 */
void error_extras(void)
{
    error("Caracteres extras", NULL);
}

/*
 ** Procesa una instrucción básica
 */
void ins_op(int ins)
{
    int valor;
    int arregla;
    int salto;
    
    separa_componente();
    err = 0;
    pos_global = separa;
    valor = evalua_expresion();
    if (*pos_global)
        error_extras();
    if (err) {
        agrega_indefinido(ins);
        etemp1(ins << 4);
        ++pos_ens;
        return;
    }
    if (ins == 0 || ins == 9 || ins == 10) {
        agrega_indefinido(ins);
        arregla = 1;
        while (1) {
            acumula = 0;
            gen_ins(ins, salto = (valor - (pos_ens + arregla)));
            if (pos_ens + acumula + salto == valor)
                break;
            ++arregla;
        }
    } else {
        acumula = 0;
        gen_ins(ins, valor);
    }
    valor = 0;
    while (valor < acumula) {
        etemp1(preins[valor++]);
        ++pos_ens;
    }
}

/*
 ** Agrega un indefinido a la lista
 */
void agrega_indefinido(int clave)
{
    struct indefinido *nuevo;
    char *pos;
    char *pos1;
    int u;
    
    nuevo = malloc(sizeof(struct indefinido) + strlen(separa) + 1);
    if (nuevo == NULL) {
        error("Sin memoria", NULL);
        return;
    }
    nuevo->siguiente = NULL;
    nuevo->codigo = clave;
    nuevo->direccion = pos_ens;
    strcpy(nuevo->expresion, separa);
    if (primer_indef == NULL)
        primer_indef = nuevo;
    if (ultimo_indef != NULL)
        ultimo_indef->siguiente = nuevo;
    ultimo_indef = nuevo;
    ++num_indef;
}

/*
 ** Evalua una expresión
 */
int evalua_expresion(void)
{
    int valor1;
    
    valor1 = eval1();
    while (*pos_global == '+' || *pos_global == '-') {
        if (*pos_global == '+') {
            ++pos_global;
            valor1 += eval1();
        } else {
            ++pos_global;
            valor1 -= eval1();
        }
    }
    return valor1;
}

/*
 ** Nivel 1 de evaluación
 */
int eval1(void)
{
    if (*pos_global == '+')
        ++pos_global;
    else if (*pos_global == '-') {
        ++pos_global;
        return -eval2();
    }
    return eval2();
}

/*
 ** Nivel 2 de evaluación
 */
int eval2(void)
{
    int valor1;
    
    valor1 = eval3();
    while (*pos_global == '*' || *pos_global == '/' || *pos_global == '%') {
        if (*pos_global == '*') {
            ++pos_global;
            valor1 *= eval3();
        } else if (*pos_global == '/') {
            ++pos_global;
            valor1 /= eval3();
        } else {
            ++pos_global;
            valor1 %= eval3();
        }
    }
    return valor1;
}

/*
 ** Nivel 3 de evaluación
 */
int eval3(void)
{
    struct etiqueta *etiqueta;
    char *ap;
    int valor;
    
    if (*pos_global == '(') {
        ++pos_global;
        valor = evalua_expresion();
        if (*pos_global != ')')
            error("Falta parentesis derecho", NULL);
        else
            ++pos_global;
        return valor;
    } else if (isdigit(*pos_global)) {
        if (*(pos_global + 1) == 'X' || *(pos_global + 1) == 'x')
            return eval_hex();
        else
            return eval_dec();
    } else if (*pos_global == 39) {
        pos_global++;
        valor = *pos_global++;
        if (valor == 39) {
            if (*pos_global == 39 && pos_global[1] == 39)
                pos_global += 2;
            else
                error("Falta apóstrofe final", NULL);
        } else {
            if (*pos_global == 39)
                pos_global++;
            else
                error("Falta apóstrofe final", NULL);
        }
        return valor;
    } else if (isalpha(*pos_global) || *pos_global == '_') {
        ap = separa2;
        while (*pos_global == '_'
               || isalpha(*pos_global)
               || isdigit(*pos_global))
            *ap++ = *pos_global++;
        *ap = '\0';
        if ((etiqueta = busca_etiqueta(separa2)) != NULL) {
            valor = etiqueta->dato;
            return valor;
        } else {
            strcpy(etiq_indef, separa2);
            err = 1;
            return 0;
        }
    } else {
        error("Error de sintaxis", NULL);
        return 0;
    }
}

/*
 ** Procesa un número hexadecimal
 */
int eval_hex(void)
{
    int c;
    int valor;
    
    valor = 0;
    pos_global += 2;
    while (isxdigit(*pos_global)) {
        c = toupper(*pos_global++) - '0';
        if (c > 9)
            c -= 7;
        valor = (valor << 4) | c;
    }
    return valor;
}

/*
 ** Procesa un número decimal
 */
int eval_dec(void)
{
    int c;
    int valor;
    
    valor = 0;
    while (isdigit(*pos_global)) {
        c = *pos_global++ - '0';
        valor = valor * 10 + c;
    }
    return valor;
}

/*
 ** Genera las instrucciones adecuadas para cargar un valor
 */
void gen_ins(int oper, int valor)
{
    if (valor < 0)
        gen_ins(6, ~valor >> 4);
    else if (valor >= 16)
        gen_ins(2, valor >> 4);
    preins[acumula++] = (oper << 4) | (valor & 15);
}

/*
 ** Genera una instrucción
 */
void ins_sim(int ins)
{
    if (ins > 15) {
        etemp1(0x20 + (ins >> 4));
        ++pos_ens;
    }
    etemp1(0xf0 + (ins & 15));
    ++pos_ens;
}

/*
 ** Genera una instrucción extendida
 */
void ins_ext(int ins)
{
    if (ins > 15) {
        etemp1(0x20 + (ins >> 4));
        ++pos_ens;
    }
    etemp1(0x40 + (ins & 15));
    ++pos_ens;
    etemp1(0x2a);
    ++pos_ens;
    etemp1(0xfb);
    ++pos_ens;
}

/*
 ** Definición de bytes
 */
void def_byte(void)
{
    int valor;
    
    separa_componente();
    pos_global = separa;
    while (1) {
        err = 0;
        if (*pos_global == '"') {
            pos_global++;
            while (*pos_global && (*pos_global != '"' || pos_global[1] == '"')) {
                etemp1(*pos_global);
                ++pos_ens;
                if (pos_global[0] == '"' && pos_global[1] == '"')
                    pos_global++;
                pos_global++;
            }
            if (*pos_global != '"') {
                error("Faltan comillas", NULL);
            } else {
                ++pos_global;
            }
        } else if (*pos_global == '\'') {
            pos_global++;
            while (*pos_global && (*pos_global != '\'' || pos_global[1] == '\'')) {
                etemp1(*pos_global);
                ++pos_ens;
                if (pos_global[0] == '\'' && pos_global[1] == '\'')
                    pos_global++;
                pos_global++;
            }
            if (*pos_global != '\'') {
                error("Faltan apóstrofe", NULL);
            } else {
                ++pos_global;
            }
        } else {
            valor = evalua_expresion();
            if (err)
                agrega_indefinido(16);
            etemp1(valor & 255);
            ++pos_ens;
        }
        if (*pos_global != ',') {
            if (*pos_global)
                error("Falta una coma", NULL);
            return;
        }
        ++pos_global;
    }
}

/*
 ** Definición de palabras de 32 bits
 */
void def_pal32(void)
{
    int valor;
    
    separa_componente();
    pos_global = separa;
    while (1) {
        err = 0;
        valor = evalua_expresion();
        if (err)
            agrega_indefinido(17);
        etemp1(valor & 255);
        ++pos_ens;
        etemp1((valor >> 8) & 255);
        ++pos_ens;
        etemp1((valor >> 16) & 255);
        ++pos_ens;
        etemp1((valor >> 24) & 255);
        ++pos_ens;
        if (*pos_global != ',') {
            if (*pos_global)
                error("Falta una coma", NULL);
            return;
        }
        ++pos_global;
    }
}

/*
 ** Definición de espacio libre
 */
void def_espacio(void)
{
    int valor;
    
    separa_componente();
    pos_global = separa;
    err = 0;
    valor = evalua_expresion();
    if (*pos_global)
        error_extras();
    if (err) {
        error("Se requiere un valor definido", NULL);
        return;
    }
    while (valor--) {
        etemp1(0);
        ++pos_ens;
    }
}

/*
 ** Definición de equivalencia de etiqueta
 */
void def_equiv(void)
{
    int valor;
    
    separa_componente();
    pos_global = separa;
    err = 0;
    valor = evalua_expresion();
    if (*pos_global)
        error_extras();
    if (err) {
        error("Se requiere un valor definido", NULL);
        return;
    }
    if (ultima_definida == NULL) {
        error("equ sin etiqueta", NULL);
        return;
    }
    ultima_definida->tipo = 1;
    ultima_definida->dato = valor;
}

/*
 ** Comparación con instrucción
 */
int compara(char *ins, char *separa)
{
    while (*ins == *separa++)
        if (*ins++ == 0)
            return 1;
    return 0;
}

/*
 ** Lee una línea del archivo de entrada
 */
int lee_linea(void)
{
    char *ap;
    int c;
    
    ap = linea;
    while (1) {
        c = fgetc(archivo_entrada);
        if (c == EOF) {
            if (ap != linea)
                break;
            return 1;  /* Fin de archivo */
        }
        *ap = c;
        if (*ap == '\r')
            continue;
        if (*ap == '\n')
            break;
        if (ap - linea < sizeof(linea) - 1)
            ap++;
    }
    *ap = '\0';
    linea_actual++;
    ap_proceso = linea;
    /*    fprintf(stderr, "[%s]\n", linea);*/
    return 0;
}

int algo;

/*
 ** Agranda las instrucciones del archivo final
 */
void enlaza(void)
{
    int c;
    char *p;
    
    while (1) {
        temp1 = fopen(temporary1_file, "rb");
        ptemp1 = TAM_BUF;
        temp2 = fopen(temporary2_file, "wb");
        ptemp2 = 0;
        algo = NO;
        repaso();
        vtemp2();
        fclose(temp1);
        temp1 = NULL;
        remove(temporary1_file);
        p = temporary1_file;
        temporary1_file = temporary2_file;
        temporary2_file = p;
        if (!algo) {
            temp1 = fopen(temporary1_file, "rb");
            temp2 = fopen(output_file, "wb");
            while (1) {
                c = fread(btemp1, 1, TAM_BUF, temp1);
                if (c == 0)
                    break;
                fwrite(btemp1, 1, c, temp2);
            }
            fclose(temp2);
            fclose(temp1);
            remove(temporary1_file);
            break;
        }
    }
}

/*
 ** Realiza un paso de ensanchamiento
 */
void repaso(void)
{
    struct indefinido *lista;
    struct etiqueta *lista2;
    int inicio;
    int ultimo;
    int pos;
    int valor;
    int byte;
    int acu;
    int ins;
    int arregla;
    int salto;
    int cuantos;
    int a;
    int c;
    
    ultimo = pos_ens;
    pos_ens = 0;
    cuantos = 0;
    inicio = 0;
    lista = primer_indef;
    a = 0;
    while (lista != NULL) {
        pos = lista->direccion;
        copia(inicio, pos);
        ins = lista->codigo;
        if (ins == 16) {
            byte = ltemp1();
            inicio = pos + 1;
        } else if (ins == 17) {
            byte = ltemp1();
            byte = ltemp1();
            byte = ltemp1();
            byte = ltemp1();
            inicio = pos + 4;
        } else {
            acu = 0;
            while (1) {
                oriins[acu++] = byte = ltemp1();
                if ((byte & 0xf0) == 0x20)
                    continue;
                if ((byte & 0xf0) == 0x60)
                    continue;
                break;
            }
            inicio = pos + acu;
        }
        lista->direccion = pos_ens;
        err = 0;
        pos_global = lista->expresion;
        valor = evalua_expresion();
        if (err) {
            error("Etiqueta indefinida", etiq_indef);
            valor = 0;
        }
        if (ins == 16) {
            etemp2(valor);
            ++pos_ens;
        } else if (ins == 17) {
            etemp2(valor);
            ++pos_ens;
            etemp2(valor >> 8);
            ++pos_ens;
            etemp2(valor >> 16);
            ++pos_ens;
            etemp2(valor >> 24);
            ++pos_ens;
        } else if (ins == 0 || ins == 9 || ins == 10) {
            acumula = 0;
            gen_ins(ins, valor - (pos_ens + acu));
        } else {
            acumula = 0;
            gen_ins(ins, valor);
        }
        if (ins != 16 && ins != 17) {
            valor = 0;
            while (valor < acumula) {
                etemp2(preins[valor++]);
                ++pos_ens;
            }
            if (acumula != acu) {
                algo = SI;
                lista2 = primer_etiq;
                while (lista2 != NULL) {
                    if (lista2->tipo == 0) {
                        valor = lista2->dato;
                        if (valor >= inicio + cuantos)
                            valor += acumula - acu;
                        lista2->dato = valor;
                    }
                    lista2 = lista2->secuencia;
                }
                cuantos += acumula - acu;
            }
        }
        lista = lista->siguiente;
#if 0
        putchar('\r');
        decimal(++a * 100 / num_indef);
        putchar('%');
#endif
    }
    copia(inicio, ultimo);
}

/*
 ** Copia datos del archivo temporal 1 en el archivo temporal 2
 */
void copia(int inicio, int final)
{
    while (inicio++ < final) {
        etemp2(ltemp1());
        ++pos_ens;
    }
}

/*
 ** Escribe datos al archivo temporal 1
 */
void etemp1(int dato)
{
    btemp1[ptemp1++] = dato;
    if (ptemp1 == TAM_BUF) {
        if (fwrite(btemp1, 1, ptemp1, temp1) != ptemp1)
            error("Disco lleno", NULL);
        ptemp1 = 0;
    }
}

/*
 ** Vacia los datos finales en el archivo temporal 1
 */
void vtemp1(void)
{
    if (ptemp1 > 0)
        if (fwrite(btemp1, 1, ptemp1, temp1) != ptemp1)
            error("Disco lleno", NULL);
    fclose(temp1);
    temp1 = NULL;
}

/*
 ** Escribe datos en el archivo temporal 2
 */
void etemp2(int dato)
{
    btemp2[ptemp2++] = dato;
    if (ptemp2 == TAM_BUF) {
        if (fwrite(btemp2, 1, ptemp2, temp2) != ptemp2)
            error("Disco lleno", NULL);
        ptemp2 = 0;
    }
}

/*
 ** Vacia datos en el archivo temporal 2
 */
void vtemp2(void)
{
    if (ptemp2 > 0)
        if (fwrite(btemp2, 1, ptemp2, temp2) != ptemp2)
            error("Disco lleno", NULL);
    fclose(temp2);
    temp2 = NULL;
}

/*
 ** Lee datos del archivo temporal 1
 */
int ltemp1(void)
{
    if (ptemp1 == TAM_BUF) {
        fread(btemp1, 1, TAM_BUF, temp1);
        ptemp1 = 0;
    }
    return btemp1[ptemp1++];
}

/*
 ** Indica un error
 */
void error(char *mensaje, char *dato)
{
    if (dato != NULL)
        fprintf(stderr, "%s '%s' at line %d\n", mensaje, dato, linea_actual);
    else
        fprintf(stderr, "%s at line %d\n", mensaje, linea_actual);
}





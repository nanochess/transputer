/*
** Compilador de C para transputer.
** Análisis sintáctico de alto nivel.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 3 de junio de 1995.
** Revisión: 24 de julio de 1995. Soporte para switch, case y default.
** Revisión: 24 de julio de 1995. Soporte para variables locales anidadas.
** Revisión: 25 de julio de 1995. Soporte para goto.
** Revisión: 26 de julio de 1995. Ahora se aceptan declaraciones de
**                                matrices con expresiones constantes cómo
**                                limites.
** Revisión: 26 de julio de 1995. Ahora los case pueden usar expresiones
**                                constantes.
** Revisión: 26 de julio de 1995. Traslado #include y #define al preprocesador.
** Revisión: 26 de julio de 1995. Traslado #asm al preprocesador.
** Revisión: 10 de agosto de 1995. Soporte para tipos complejos. Se agregan
**                                 las funciones p_tipo_1, p_tipo_2, p_tipo_3,
**                                 copia_tipo, guarda_tipo, tam_tipo.
** Revisión: 12 de agosto de 1995. Se corrige la sintaxis que reconoce
**                                 p_tipo_3.
** Revisión: 12 de agosto de 1995. Soporte para static, register y auto.
** Revisión: 12 de agosto de 1995. Soporte para typedef.
** Revisión: 12 de agosto de 1995. Corrección de un defecto en tipos_args().
** Revisión: 22 de agosto de 1995. Soporte para struct y union.
** Revisión: 22 de agosto de 1995. Nueva función. p_estructura().
** Revisión: 24 de agosto de 1995. Soporte para enum.
** Revisión: 5 de septiembre de 1995. Soporte para paso de estructuras cómo
**                                    parametros y resultados.
** Revisión: 5 de septiembre de 1995. Nueva función. ordena_args().
** Revisión: 6 de septiembre de 1995. Soporte para inicialización de
**                                    variables automáticas, nueva función.
**                                    inic_loc().
** Revisión: 6 de septiembre de 1995. Corrección de un defecto en la
**                                    compilación de la sentencia for.
** Revisión: 12 de septiembre de 1995. Soporte para float y double.
** Revisión: 23 de septiembre de 1995. Corrección de un defecto en s_return().
** Revisión: 27 de septiembre de 1995. Corrección de una llamada mal hecha a
**                                     checa_entero en s_switch, corrección
**                                     de un problema de tipos en p_estructura.
** Revisión: 24 de noviembre de 1995. Corrección de varios defectos en
**                                    tipos_args().
** Revisión: 24 de noviembre de 1995. Optimación de la salida de s_if().
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en el análisis
**                                    de la sintaxis de do-while.
** Revisión: 28 de diciembre de 1995. Ahora soporta declaraciones dentro de un
**                                    switch. (siempre que estén dentro de un
**                                    bloque)
** Revisión: 28 de diciembre de 1995. Corrección de errores en tam_tipo().
** Revisión: 25 de enero de 1996. Corrección de un defecto en la inicialización
**                                de variables, tronaba el sistema.
** Revisión: 3 de febrero de 1996. Aprovecha las hasta 2 palabras libres que
**                                 ocurren cuando se llama una función.
** Revisión: 30 de marzo de 1996. Corrección de un defecto tremendo en los
**                                switch cuando salta a default.
** Revisión: 9 de abril de 1996. Simplificación de ordena_args(), corrección
**                               de defectos.
** Revisión: 19 de abril de 1996. Corrección de un error en la declaración
**                                de funciones, no convertia float a double en
**                                algunos casos.
** Revisión: 20 de junio de 1996. Detección de variables locales redefinidas.
** Revisión: 12 de octubre de 1996. Optimiza estructuras que usan matrices
**                                  de caracteres.
*/

/*
** Procesa todo el texto de entrada.
**
** En este nivel, sólo declaraciones estáticas,
** #define, #include, y definiciones de función
** son legales.
*/
analiza()
{
  while (eof == 0) {    /* Trabaja hasta que no haya más entrada */
    if(amatch("typedef", 7))
      decl_typedef(NO);
    else {
      if(amatch("static", 6)) ;
      else if(amatch("register", 8)
           || amatch("auto", 4))
        error("Sólo se acepta static");
      decl_glb();
    }
    espacios();         /* Rastrea fin de archivo */
  }
}

/*
** Declara un tipo.
*/
decl_typedef(local)
  int local;
{
  int p;
  char nombre[TAM_NOMBRE];
  char *chequeo;
  p_tipo_1(SI);
  while (1) {
    if (fin_sentencia())
      break;
    if(p_tipo_2(nombre))
      pide(")");
    if(*tipo_proc == FUNCION)
      error("No se puede definir un tipo de función");
    if(local)
      chequeo = busca_loc(nombre);
    else
      chequeo = busca_glb(nombre);
    if(chequeo)
      redefinido(nombre);
    if(local)
      nueva_loc(nombre, TYPEDEF, AUTO, tipo_proc, 0);
    else
      nueva_glb(nombre, TYPEDEF, STATIC, tipo_proc, 0);
    if(match(",") == 0)
      break;
  }
  punto_y_coma();
}

/*
** Declara una variable global.
**
** Crea una entrada en la tabla, para que las
** referencias subsiguientes la llamen por nombre.
*/
decl_glb()
{
  int p;
  char nombre[TAM_NOMBRE];
  char *chequeo;

  p_tipo_1(SI);
  while (1) {
    if (fin_sentencia())
      break;
    p = p_tipo_2(nombre);
    if (*tipo_proc == FUNCION) {
      espacios();
      if (*(tipo_proc + 1) == FLOAT)
        *(tipo_proc + 1) = DOUBLE;
      if(p == 0 && (car_act == ';' || car_act == ',')) {
        if(chequeo = busca_glb(nombre)) {
          if(chequeo[IDENT] != FUNCION)
            redefinido(nombre);
          else if(chequeo[POSICION] != FUNC_REF)
            redefinido(nombre);
          else {
            chequeo[POSICION] = FUNC_TIPO;
            escribe_entero(chequeo + TIPO, tipo_proc);
          }
        } else
          nueva_glb(nombre, FUNCION, STATIC, tipo_proc, FUNC_TIPO);
      } else {
        nueva_func(nombre, p);
        return;
      }
    } else {
      if (busca_glb(nombre))    /* ¿ Ya estaba en la tabla ? */
        redefinido(nombre);
                                /* Agrega la nueva variable */
      nueva_glb(nombre, VARIABLE, STATIC, tipo_proc, pos_global);
      pos_global += (tam_tipo(tipo_proc) + 3) / 4;
    }
    if (match(",") == 0)
      break;
  }
  punto_y_coma();
}

/*
** Procesa declaraciones de variables locales.
*/
decl_loc()
{
  char nombre[TAM_NOMBRE];
  char *salva_tipo_basico, *salva_tipo_proc;
  char *tipo_expr, *chequeo;
  int pila2, p, sin_int, estaticas, nodo_expr;
  int tam_total, donde;

  pila2 = pila;
  while (1) {
    sin_int = estaticas = NO;
    if(amatch("register", 8)
    || amatch("auto", 4)) sin_int = SI;
    else if(amatch("static", 6))
      sin_int = estaticas = SI;
    if (p_tipo_1(sin_int) == 0)
      break;
    if (dentro_switch)
      error("No se pueden hacer declaraciones dentro de un switch");
    while (1) {
      if (fin_sentencia())
        break;
      p = p_tipo_2(nombre);
      if ((chequeo = busca_loc(nombre)) && nivel == chequeo[NIVEL])
        redefinido(nombre);
      if (*tipo_proc == FUNCION) {
        if (p)
          pide(")");
        nueva_loc(nombre, FUNCION, AUTO, tipo_proc, FUNC_TIPO);
      } else if (estaticas) {
        nueva_loc(nombre, VARIABLE, STATIC, tipo_proc, pos_global);
        pos_global += (tam_tipo(tipo_proc) + 3) / 4;
      } else {
                   /* Modifica la pila */
        tam_total = (tam_tipo(tipo_proc) + 3) / 4;
        if (tam_total == 1 && (args[0] == NO || args[1] == NO)) {
          if (args[0] == NO) {
            args[0] = SI;
            donde = 2;
          } else if (args[1] == NO) {
            args[1] = SI;
            donde = 3;
          }
        } else if (tam_total == 2 && args[0] == NO && args[1] == NO) {
          args[0] = SI;
          args[1] = SI;
          donde = 2;
        } else {
          pila2 -= (tam_tipo(tipo_proc) + 3) / 4;
          donde = pila2;
        }
        if(*tipo_proc != STRUCT && *tipo_proc != MATRIZ && match("=")) {
          salva_tipo_basico = tipo_basico;
          salva_tipo_proc = tipo_proc;
          usa_expr = SI;
          tipo_expr = almacena_expresion(NO);
          nodo_expr = ultimo_nodo;
          tipo_proc = salva_tipo_proc;
          tipo_basico = salva_tipo_basico;
          convierte_tipo(&nodo_expr, tipo_expr, tipo_proc);
          if(vars_inicializadas == MAX_INIC) {
            error("Demasiadas variables inicializadas");
          } else {
            nodo_inic[vars_inicializadas++] = nodo_expr;
            nodo_inic[vars_inicializadas++] = donde;
            nodo_inic[vars_inicializadas++] = tipo_proc;
          }
        }
        nueva_loc(nombre, VARIABLE, AUTO, tipo_proc, donde);
      }
      if (match(",") == 0)
        break;
    }
    punto_y_coma();
  }
  pila = desp_pila(pila2);
}

/*
** Inicializa las variables locales requeridas.
*/
inic_loc()
{
  int cuenta;
  cuenta = 0;
  while(cuenta < vars_inicializadas) {
    asigna(nodo_inic[cuenta], nodo_inic[cuenta+1], nodo_inic[cuenta+2]);
    cuenta += 3;
  }
}

/*
** Procesa la primera parte de un tipo, la variable tipo_basico
** contiene el tipo base del tipo completo.
*/
p_tipo_1(sin_int)
  int sin_int;
{
  int salva_posicion;
  char nombre[TAM_NOMBRE];
  char *chequeo;

  if (amatch("char", 4)) {
    tipo_basico = t_char;
    return 1;
  }
  if (amatch("int", 3)) {
    tipo_basico = t_int;
    return 1;
  }
  if (amatch("long", 4)) {
    if (amatch("int", 3)) ;
    else if (amatch("float", 5)) {
      tipo_basico = t_double;
      return 1;
    }
    tipo_basico = t_int;
    return 1;
  }
  if (amatch("short", 5)) {
    if (amatch("int", 3)) ;
    tipo_basico = t_short;
    return 1;
  }
  if (amatch("void", 4)) {
    tipo_basico = t_void;
    return 1;
  }
  if (amatch("float", 5)) {
    tipo_basico = t_float;
    return 1;
  }
  if (amatch("double", 6)) {
    tipo_basico = t_double;
    return 1;
  }
  if (amatch("struct", 6)) {
    p_estructura(NO);
    return 1;
  }
  if (amatch("union", 5)) {
    p_estructura(SI);
    return 1;
  }
  if (amatch("enum", 4)) {
    p_enumerador();
    return 1;
  }
  if (amatch("unsigned", 8)) {
    if (amatch("char", 4)) {
      tipo_basico = t_char;
      return 1;
    }
    if (amatch("short", 5)) {
      if (amatch("int", 3)) ;
      tipo_basico = t_ushort;
      return 1;
    }
    if (amatch("int", 3)) ;
    else if (amatch("long", 4)) {
      if (amatch("int", 3)) ;
    }
    tipo_basico = t_uint;
    return 1;
  }
  espacios();
  salva_posicion = pos_linea;
  if(nombre_legal(nombre)) {
    if((chequeo = busca_loc(nombre))
    || (chequeo = busca_glb(nombre))) {
      if(chequeo[IDENT] == TYPEDEF) {
        tipo_basico = lee_entero(chequeo + TIPO);
        return 1;
      }
    }
    pos_linea = salva_posicion;
  }
  if (sin_int) {
    tipo_basico = t_int;
    return 1;
  }
  return 0;
}

/*
** Genera un tipo procesado.
*/
p_tipo_2(nombre)
  char *nombre;
{
  int p;
  int decoracion;
  tipo_proc = sig_tipo;
  decoracion = 0;
  p = p_tipo_3(nombre, &decoracion, 0);
  if (decoracion == 0) {
    tipo_proc = tipo_basico;
    return p;
  }
  copia_tipo(tipo_basico);
  return p;
}

/*
** Copia un tipo en la siguiente posición disponible.
*/
copia_tipo(tipo)
  char *tipo;
{
  int a;
  while(*tipo >= APUNTADOR) {
    if(*tipo == MATRIZ) {
      guarda_tipo(*tipo++);
      guarda_tipo(*tipo++);
      guarda_tipo(*tipo++);
      guarda_tipo(*tipo++);
      guarda_tipo(*tipo++);
    } else
      guarda_tipo(*tipo++);
  }
  if(*tipo == STRUCT) {
    guarda_tipo(*tipo++);
    guarda_tipo(*tipo++);
    guarda_tipo(*tipo++);
    guarda_tipo(*tipo++);
  }
  guarda_tipo(*tipo++);
}

/*
** Almacena un byte de tipo.
*/
guarda_tipo(byte)
  int byte;
{
  if(sig_tipo >= MAX_TIPOS) {
    error("Tabla de tipos llena");
    cancela();
  }
  *sig_tipo++ = byte;
}

/*
** Procesa las decoraciones de tipo.
*/
p_tipo_3(nombre, decoracion, anidamiento)
  char *nombre;
  int *decoracion, anidamiento;
{
  int p, tam;
  if(match("*")) {
    p = p_tipo_3(nombre, decoracion, anidamiento);
    *decoracion = 1;
    guarda_tipo(APUNTADOR);
    return p;
  }
  if(match("(")) {
    if(nombre == NULL && match(")")) {
      *decoracion = 1;
      guarda_tipo(FUNCION);
      return 0;
    }
    p = p_tipo_3(nombre, decoracion, 1);
    pide(")");
  } else if(nombre != NULL) {
    if(nombre_legal(nombre) == 0)
      nombre_ilegal();
  }
  if(match("(")) {
    *decoracion = 1;
    if(anidamiento == 0 && nombre != NULL) {
      if(match(")") == 0)
        p = 1;
      else
        p = 0;
    } else
      pide(")");
    guarda_tipo(FUNCION);
    return p;
  }
  while(match("[")) {
    *decoracion = 1;
    tam = subindice();
    guarda_tipo(MATRIZ);
    guarda_tipo(tam);
    guarda_tipo(tam >> 8);
    guarda_tipo(tam >> 16);
    guarda_tipo(tam >> 24);
  }
  return 0;
}

/*
** Procesa una declaración de estructura o unión.
*/
p_estructura(es_union)
  int es_union;
{
  char rotulo[TAM_NOMBRE];
  char nombre_miembro[TAM_NOMBRE];
  char *miembro;
  char *estructura;
  char *lista, *ultima_franja;
  int posicion, tam, numero_bits;
  char *tipo_optimo;

  if(nombre_legal(rotulo)) {
    if((estructura = busca_estructura(rotulo)) != NULL) {
      if(estructura[EST_QUE_ES] == ENUM)
        redefinido(rotulo);
      if(es_union != estructura[EST_ES_UNION])
        if(es_union)
          error("Se uso union en lugar de struct");
        else
          error("Se uso struct en lugar de union");
      if(lee_entero(estructura + EST_TAM)) {
        tipo_basico = sig_tipo;
        guarda_tipo(STRUCT);
        posicion = estructura;
        guarda_tipo(posicion);
        guarda_tipo(posicion >> 8);
        guarda_tipo(posicion >> 16);
        guarda_tipo(posicion >> 24);
        return;
      }
    } else {
      estructura = nueva_estructura(rotulo);
      estructura[EST_QUE_ES] = STRUCT;
      estructura[EST_ES_UNION] = es_union;
    }
  } else {
    estructura = nueva_estructura("");
    estructura[EST_QUE_ES] = STRUCT;
    estructura[EST_ES_UNION] = es_union;
  }
  if(match("{") == 0) {
    tipo_basico = sig_tipo;
    guarda_tipo(STRUCT);
    posicion = estructura;
    guarda_tipo(posicion);
    guarda_tipo(posicion >> 8);
    guarda_tipo(posicion >> 16);
    guarda_tipo(posicion >> 24);
    return;
  }
  lista = NULL;
  posicion = tam = 0;
  while(p_tipo_1(NO)) {
    while(1) {
      if(es_union)
        posicion = 0;
      if(fin_sentencia())
        break;
      numero_bits = 0;
      if(match(":")) {
        *nombre_miembro = 0;
        numero_bits = expr_constante();
        if(tipo_basico != t_int && tipo_basico != t_uint)
          error("No es de tipo int o unsigned int");
        if(numero_bits > 32)
          error("Más de 32 bits en el miembro");
        if(numero_bits == 0) {
          posicion = ((posicion + 3) & ~3) + 4;
        } else if(numero_bits > 16) {
          posicion += 4;
        } else if(numero_bits > 8) {
          posicion += 2;
        } else {
          posicion++;
        }
      } else {
        if(p_tipo_2(nombre_miembro))
          pide(")");
        if(match(":")) {
          numero_bits = expr_constante();
          if(tipo_proc != t_int && tipo_proc != t_uint)
            error("No es de tipo int o unsigned int");
          if(numero_bits == 0)
            error("Miembro vacio");
          if(numero_bits > 32)
            error("Más de 32 bits en el miembro");
          if(numero_bits > 16) {
            posicion = (posicion + 3) & ~3;
          } else if(numero_bits > 8) {
            posicion = (posicion + 1) & ~1;
            if(tipo_proc == t_int)
              tipo_proc = t_short;
            else
              tipo_proc = t_ushort;
          } else {
            tipo_proc = t_char;
          }
          numero_bits = 0;
        } else {
          tipo_optimo = tipo_proc;
          while (*tipo_optimo == MATRIZ)
            tipo_optimo += 5;
          if(*tipo_optimo == *t_short || *tipo_optimo == *t_ushort)
            posicion = (posicion + 1) & ~1;
          else if(*tipo_optimo != *t_char)
            posicion = (posicion + 3) & ~3;
        }
      }
      if(*nombre_miembro) {
        if(miembro = busca_miembro(lista, nombre_miembro))
          redefinido(nombre_miembro);
        else
          miembro = nuevo_miembro(&lista, nombre_miembro);
        escribe_entero(miembro + MIE_TIPO, tipo_proc);
        escribe_entero(miembro + MIE_POSICION, posicion);
        posicion += tam_tipo(tipo_proc);
      }
      if(es_union)
        tam = (posicion > tam) ? posicion : tam;
      else
        tam = posicion;
      if(match(",") == 0) break;
    }
    punto_y_coma();
  }
  if(lista != NULL)
    escribe_entero(estructura + EST_LISTA, lista);
  escribe_entero(estructura + EST_TAM, tam);
  if(tam == 0)
    if(es_union) error("Unión vacia");
    else error("Estructura vacia");
  tipo_basico = sig_tipo;
  guarda_tipo(STRUCT);
  posicion = estructura;
  guarda_tipo(posicion);
  guarda_tipo(posicion >> 8);
  guarda_tipo(posicion >> 16);
  guarda_tipo(posicion >> 24);
  pide("}");
}

/*
** Procesa un enumerador.
*/
p_enumerador() {
  char rotulo[TAM_NOMBRE];
  char nombre_miembro[TAM_NOMBRE];
  char *enumerador;
  int valor;

  if(nombre_legal(rotulo)) {
    if((enumerador = busca_estructura(rotulo)) != NULL) {
      if(enumerador[EST_QUE_ES] != ENUM)
        redefinido(rotulo);
      if(lee_entero(enumerador + EST_TAM)) {
        tipo_basico = t_int;
        return;
      }
    } else {
      enumerador = nueva_estructura(rotulo);
      enumerador[EST_QUE_ES] = ENUM;
    }
  } else {
    enumerador = nueva_estructura("");
    enumerador[EST_QUE_ES] = ENUM;
  }
  if(match("{") == 0) {
    tipo_basico = t_int;
    return;
  }
  escribe_entero(enumerador + EST_TAM, 1);
  valor = 0;
  while(1) {
    if(nombre_legal(nombre_miembro) == 0)
      break;
    if(match("="))
      valor = expr_constante();
    if(busca_enum(nombre_miembro) != NULL)
      redefinido(nombre_miembro);
    else
      nuevo_enum(nombre_miembro, valor);
    if(match(",") == 0)
      break;
  }
  pide("}");
  tipo_basico = t_int;
}

/*
** Obtiene el tamaño de un tipo.
*/
tam_tipo(tipo)
  char *tipo;
{
  int tam;
  switch(*tipo) {
    case CHAR:   return 1;
    case USHORT:
    case SHORT:  return 2;
    case APUNTADOR:
    case UINT:
    case INT:
    case FLOAT:  return 4;
    case DOUBLE: return 8;
    case VOID:   error("Uso incorrecto de void");
    case FUNCION:
                 error("Uso incorrecto de tipo de función");
                 return 0;
    case MATRIZ: tam = lee_entero(tipo + 1);
                 if(tam == 0)
                   error("Tamaño nulo de matriz");
                 return tam_tipo(tipo + 5) * tam;
    case STRUCT: tam = lee_entero(lee_entero(tipo + 1) + EST_TAM);
                 if(tam == 0)
                   error("Estructura o unión incompleta");
                 return (tam + 3) & ~3;
  }
}

/*
** Obtiene el tamaño de una matriz.
**
** Invocada cuando una declaración es seguida
** por "[".
*/
subindice()
{
  int num;

  if (match("]"))
    return 0;                   /* Tamaño nulo */
  num = expr_constante();       /* Procesa una expresión constante */
  if (num == 0) {
    error("No se acepta una dimensión cero");
    num = 1;                    /* Forza a 1 */
  }
  if (num < 0) {
    error("Tamaño negativo");
    num = -num;
  }
  pide("]");                    /* Forza una dimensión */
  return num;                   /* y retorna el tamaño */
}

/*
** Compila una función.
**
** Invocada por "decl_glb", esta función intenta compilar una funcion
** a partir de la entrada.
*/
nueva_func(n, parentesis)
  char *n;
  int parentesis;
{
  int num_args;
                                   /* Recuerda el comienzo de la función */
  comienzo_funcion = linea_actual;
  dentro_funcion = SI;          /* Indica que esta dentro de una función */
                                /* ¿ Ya estaba en la tabla de nombres ? */
  if(funcion_actual = busca_glb(n)) {
    if (funcion_actual[IDENT] != FUNCION)
      redefinido(n);            /* Ya hay una variable con ese nombre */
    else if (funcion_actual[POSICION] == FUNC_DEF)
      redefinido(n);            /* Se redefinio una función. */
    else {                      /* Es una función referenciada antes */
      funcion_actual[POSICION] = FUNC_DEF;
      escribe_entero(funcion_actual + TIPO, tipo_proc);
    }
  }

  /* No estaba en la tabla, definir cómo una función */

  else
    funcion_actual = nueva_glb(n, FUNCION, STATIC, tipo_proc, FUNC_DEF);

  hacia_consola();
  emite_texto("Compilando ");
  emite_texto(n);
  emite_texto("()...");
  emite_nueva_linea();
  hacia_archivo();

  emite_nombre(n);              /* Imprime el nombre de la función */
  dos_puntos();
  emite_nueva_linea();

  ap_loc = INICIO_LOC;          /* Limpia la tabla de variables locales */
  pila_args = 0;                /* Inicia la cuenta de argumentos */
  while(parentesis
    && (match(")") == 0)) {     /* Empieza a contar */

    /* Cualquier nombre legal incrementa la cuenta */

    if (nombre_legal(n)) {
      if (busca_loc(n))
        redefinido(n);
      else {
        nueva_loc(n, VARIABLE, AUTO, 0, 0);
        ++pila_args;
      }
    } else {
      error("Nombre ilegal para el argumento");
      basura();
    }
    espacios();

    /* Si no es parentesis de cierre, debe ser coma */

    if (car_act != ')') {
      if (match(",") == 0)
        error("Se requiere una coma");
    }
    if (fin_sentencia())
      break;
  }

  num_args = pila_args;

  while (pila_args > 0) {

    /* Ahora el usuario declara los tipos de los argumentos */

    if (p_tipo_1(NO)) {
      tipos_args();
      punto_y_coma();
    } else {
      error("Número incorrecto de argumentos");
      break;
    }
  }

  ordena_args(num_args);

  pila = 0;                 /* Inicializa el apuntador de la pila */

  etiq_lit = nueva_etiq;  /* Etiqueta para el buffer literal */
  ap_lit = 0;               /* Limpia el buffer literal */

  /* Procesa una sentencia, si es un retorno */
  /* entonces no limpia la pila */

  if(sentencia() != E_RETURN) {
    desp_pila(0);
    retorno();
  }
  vacia_lits();

  pila = 0;                /* Limpia la pila de nuevo */
  ap_loc = INICIO_LOC;     /* Elimina todas las variables locales */
  dentro_funcion = NO;     /* Ahora no esta dentro de una función */
}

/*
** Declara los tipos de los argumentos.
*/
tipos_args()
{
  char n[TAM_NOMBRE], *ap_arg;
  char *nuevo_tipo;
  int p;

  while (1) {
    p = p_tipo_2(n);
    if(*tipo_proc == FUNCION) {
      if(p) pide(")");
      error("No se puede usar una función cómo argumento");
    }
    if(*tipo_proc == MATRIZ) {
      nuevo_tipo = sig_tipo;
      guarda_tipo(APUNTADOR);
      copia_tipo(tipo_proc + 5);
      tipo_proc = nuevo_tipo;
    }
    if(*tipo_proc == FLOAT)
      tipo_proc = t_double;
    if (ap_arg = busca_loc(n)) {

       /* Pone el tipo correcto al argumento */

      if(lee_entero(ap_arg + TIPO))
        error("Argumento redefinido");
      escribe_entero(ap_arg + TIPO, tipo_proc);
    } else
      error("Se requiere el nombre de un argumento");
    --pila_args;                   /* cuenta hacia atras */
    if (fin_sentencia())
      return;
    if (match(",") == 0)
      error("Se requiere una coma");
  }
}

/*
** Ordena los argumentos.
*/
ordena_args(cuantos)
  int cuantos;
{
  char *ap_arg, *tipo, *tipo_func;
  int pos;

  args[0] = NO;
  args[1] = NO;
  tipo_func = lee_entero(funcion_actual + TIPO);
  if(*++tipo_func != STRUCT)    /* El primer byte es FUNCION */
    pos = 2;                    /* Función comun */
  else {
    args[0] = SI;               /* Separa espacio para el resultado */
    pos = 4 + (tam_tipo(tipo_func) + 3) / 4;
  }
  ap_arg = INICIO_LOC;
  while(cuantos--) {
    tipo = lee_entero(ap_arg + TIPO);
    if(*tipo == STRUCT || *tipo == DOUBLE) {
      if(pos < 4)
        pos = 4;
    }
    escribe_entero(ap_arg + POSICION, pos);
    if(*tipo == STRUCT)
      pos += (tam_tipo(tipo) + 3) / 4;
    else if(*tipo == DOUBLE)
      pos += 2;
    else {
      if (pos == 2)
        args[0] = SI;
      else if (pos == 3)
        args[1] = SI;
      pos++;
    }
    ap_arg += TAM_SIM;
  }
}

/*
** Analizador de sentencias.
**
** Llamado cuando la sintaxis requiere una
** sentencia, retorna un número que indica
** la última sentencia procesada.
*/
sentencia()
{
  if ((car_act == 0) && (eof))
    return;
  else if (match("{"))
    p_bloque();
  else if (amatch("if", 2)) {
    s_if();
    ultima_sentencia = E_IF;
  } else if (amatch("while", 5)) {
    s_while();
    ultima_sentencia = E_WHILE;
  } else if (amatch("do", 5)) {
    s_do();
    punto_y_coma();
    ultima_sentencia = E_WHILE;
  } else if (amatch("for", 3)) {
    s_for();
    ultima_sentencia = E_WHILE;
  } else if (amatch("switch", 6)) {
    s_switch();
    ultima_sentencia = E_WHILE;
  } else if (amatch("case", 4)) {
    s_case();
    ultima_sentencia = E_EXPR;
  } else if (amatch("default", 7)) {
    s_default();
    ultima_sentencia = E_EXPR;
  } else if (amatch("goto", 4)) {
    s_goto();
    punto_y_coma();
    ultima_sentencia = E_BREAK;
  } else if (p_etiqueta()) {
    ultima_sentencia = E_EXPR;
  } else if (amatch("return", 6)) {
    s_return();
    punto_y_coma();
    ultima_sentencia = E_RETURN;
  } else if (amatch("break", 5)) {
    s_break();
    punto_y_coma();
    ultima_sentencia = E_BREAK;
  } else if (amatch("continue", 8)) {
    s_cont();
    punto_y_coma();
    ultima_sentencia = E_CONT;
  } else if (match(";"))
    ultima_sentencia = E_EXPR;
  /* Asume que es una expresión */
  else {
    usa_expr = NO;
    expresion();
    punto_y_coma();
    ultima_sentencia = E_EXPR;
  }
  return ultima_sentencia;
}

/*
** Checa punto y coma.
**
** Llamado cuando la sintaxis lo requiere.
*/
punto_y_coma()
{
  if (match(";") == 0)
    error("Falta punto y coma");
}

/*
** Bloque de sentencias.
*/
p_bloque()
{
  int p, nodo_vars, c_dentro_switch;
  char *local, *local2, *local3;
  char *pos_tipo;

  c_dentro_switch = dentro_switch;
  if(nivel - dentro_switch >= 1)
    dentro_switch = 0;          /* Vuelve a permitir declaraciones */
  pos_tipo = sig_tipo;          /* Tabla de tipos */
  local = ap_loc;               /* Variables locales */
  p = pila;                     /* Pila actual */
  ++nivel;                      /* Un nuevo nivel */
  nodo_vars = ultimo_nodo;
  vars_inicializadas = 0;
  decl_loc();                   /* Procesa declaraciones locales */
  inic_loc();                   /* Inicializa variables locales */
  ultimo_nodo = nodo_vars;
  while (match("}") == 0)
    sentencia();                /* Procesa sentencias */
  --nivel;                      /* Cierra el nivel */
  local2 = ap_loc;              /* Checa el número de vars. locales */
  if(nivel) {                   /* Mantiene las etiquetas para goto */
    ap_loc = local;
    while(local < local2) {
      local3 = local + TAM_SIM;
      if(local[IDENT] == ETIQUETA) {
        while(local < local3)
          *ap_loc++ = *local++;
      } else
        local = local3;
    }
  } else
    ap_loc = local;             /* Limpia las variables locales */
  if((ultima_sentencia != E_RETURN) &&
     (ultima_sentencia != E_BREAK) &&
     (ultima_sentencia != E_CONT))
    pila = desp_pila(p);        /* Limpia la pila */
  else
    pila = p;
  sig_tipo = pos_tipo;
  dentro_switch = c_dentro_switch;
}

/*
** Sentencia "if"
*/
s_if()
{
  int etiq1, etiq2;

  etiq1 = nueva_etiq;           /* Etiqueta para el salto falso */
  prueba(etiq1, SI);            /* Prueba la expresión y salta si es falsa */
  sentencia();                  /* Verdadera, procesa sentencias */
  if (amatch("else", 4) == 0)   /* ¿ if...else ? */
                                /* "if" simple ... imprimir etiqueta de falso */
  {
    emite_etiq(etiq1);
    dos_puntos();
    emite_nueva_linea();
    return;                     /* Y vuelve */
  }
                                /* Una sentencia "if...else" */
  if((ultima_sentencia != E_RETURN) &&
     (ultima_sentencia != E_BREAK) &&
     (ultima_sentencia != E_CONT))
    salto(etiq2 = nueva_etiq);  /* Salta alrededor del codigo de else */
  else
    etiq2 = 0;
  emite_etiq(etiq1);
  dos_puntos();
  emite_nueva_linea();          /* Imprime etiqueta falsa */
  sentencia();                  /* Procesa el else */
  if(etiq2) {
    emite_etiq(etiq2);
    dos_puntos();
    emite_nueva_linea();        /* Imprime etiqueta verdadera */
  }
}

/*
** Sentencia "while"
*/
s_while()
{
  int bucle[4];                    /* Crea una entrada */
  int *anterior;

  anterior = ultimo_bucle;
  nuevo_bucle(bucle);              /* Agrega a la cola (para el break) */
  emite_etiq(bucle[B_BUCLE]);      /* Etiqueta del bucle */
  dos_puntos();
  emite_nueva_linea();
  prueba(bucle[B_FIN], SI);        /* Checa la expresión */
  sentencia();                     /* Procesa una sentencia */
  if((ultima_sentencia != E_RETURN) &&
     (ultima_sentencia != E_CONT) &&
     (ultima_sentencia != E_BREAK))
    salto(bucle[B_BUCLE]);         /* Vuelve al bucle */
  emite_etiq(bucle[B_FIN]);        /* Etiqueta de salida */
  dos_puntos();
  emite_nueva_linea();
  ultimo_bucle = anterior;         /* Borra de la cola */
}

/*
** Sentencia "do"
*/
s_do()
{
  int bucle[4];                    /* Crea una entrada */
  int *anterior;

  anterior = ultimo_bucle;
  nuevo_bucle(bucle);              /* Agrega a la cola (para el break) */
  emite_etiq(bucle[B_BUCLE]);      /* Etiqueta del bucle */
  dos_puntos();
  emite_nueva_linea();
  sentencia();                     /* Procesa una sentencia */
  if(amatch("while", 5) == 0)
    error("Falta el while");
  prueba(bucle[B_FIN], SI);        /* Checa la expresión */
  salto(bucle[B_BUCLE]);
  emite_etiq(bucle[B_FIN]);        /* Etiqueta de salida */
  dos_puntos();
  emite_nueva_linea();
  ultimo_bucle = anterior;         /* Borra de la cola */
}

/*
** Sentencia "for"
*/
s_for()
{
  int bucle[4];
  int *anterior;
  int etiq, origen;
  int expr;

  anterior = ultimo_bucle;
  nuevo_bucle(bucle);
  etiq = nueva_etiq;
  pide("(");
  if(match(";") == 0) {
    usa_expr = NO;
    expresion();
    punto_y_coma();
  }
  emite_etiq(etiq);
  dos_puntos();
  emite_nueva_linea();
  if(match(";") == 0) {
    prueba(bucle[B_FIN], NO);
    punto_y_coma();
  }
  if(match(")") == 0) {
    origen = ultimo_nodo;
    expr = SI;
    usa_expr = NO;
    almacena_expresion(SI);
    pide(")");
  } else
    expr = NO;
  sentencia();
  emite_etiq(bucle[B_BUCLE]);
  dos_puntos();
  emite_nueva_linea();
  if(expr) {
    usa_expr = NO;
    evalua_arbol(NO);
    ultimo_nodo = origen;
  }
  salto(etiq);
  emite_etiq(bucle[B_FIN]);
  dos_puntos();
  emite_nueva_linea();
  ultimo_bucle = anterior;
}

/*
** Sentencia "switch"
*/
s_switch()
{
  int bucle[4];
  int *anterior;
  int etiq, origen;
  int c_dentro_switch;
  int c_etiqueta_default;
  int *c_sig_case;
  int *c_inicio_lista;

  anterior = ultimo_bucle;
  c_dentro_switch = dentro_switch;
  c_etiqueta_default = etiqueta_default;
  c_sig_case = sig_case;
  c_inicio_lista = inicio_lista;
  inicio_lista = sig_case;
  etiqueta_default = 0;
  dentro_switch = nivel;
  nuevo_bucle(bucle);
  bucle[B_BUCLE] = 0;
  pide("(");
  origen = ultimo_nodo;
  usa_expr = SI;
  checa_entero(almacena_expresion(SI));
  pide(")");
  salto(etiq = nueva_etiq);
  sentencia();
  if((ultima_sentencia != E_RETURN) &&
     (ultima_sentencia != E_CONT) &&
     (ultima_sentencia != E_BREAK))
    salto(bucle[B_FIN]);
  emite_etiq(etiq);
  dos_puntos();
  emite_nueva_linea();
  usa_expr = SI;
  evalua_arbol(NO);
  ultimo_nodo = origen;
  while(inicio_lista != sig_case) {
    compara_y_salta(*inicio_lista, *(inicio_lista+1));
    inicio_lista = inicio_lista + 2;
  }
  if(etiqueta_default) {
    desp_pila(0);
    salto_no_int(etiqueta_default);
  }
  emite_etiq(bucle[B_FIN]);
  dos_puntos();
  emite_nueva_linea();
  dentro_switch = c_dentro_switch;
  etiqueta_default = c_etiqueta_default;
  sig_case = c_sig_case;
  inicio_lista = c_inicio_lista;
  ultimo_bucle = anterior;
}

/*
** Sentencia "case"
*/
s_case()
{
  int num;
  int *ultimo;

  if(!dentro_switch)
    error("El case no esta en un switch");
  if(sig_case == casos + MAX_CASOS) {
    error("Demasiados case");
    return;
  }
  num = expr_constante();       /* Busca el número */
  ultimo = inicio_lista;
  while(ultimo != sig_case) {
    if(*ultimo == num)
      error("El valor del case esta repetido");
    ultimo += 2;
  }
  *sig_case++ = num;
  emite_etiq(*sig_case++ = nueva_etiq);
  dos_puntos();
  emite_nueva_linea();
  pide(":");
}

/*
** Sentencia "default"
*/
s_default()
{
  int pos_pila;

  if (!dentro_switch)
    error("El default no esta en un switch");
  else if (etiqueta_default)
    error("El default esta repetido");
  pide(":");
  pos_pila = pila;
  pila = desp_pila(0);
  emite_etiq(etiqueta_default = nueva_etiq);
  dos_puntos();
  emite_nueva_linea();
  pila = desp_pila(pos_pila);
}

/*
** Sentencia "goto"
*/
s_goto()
{
  char n[TAM_NOMBRE];

  if (nombre_legal(n)) {
    desp_pila(0);
    salto_no_int(agrega_etiqueta(n));
  } else
    error("Etiqueta incorrecta");
}

/*
** Procesa una posible definición de etiqueta para goto
*/
p_etiqueta()
{
  char *c_pos_linea;
  char n[TAM_NOMBRE];
  int pos_pila;

  espacios();
  c_pos_linea = pos_linea;
  if(nombre_legal(n)) {
    if(car_act == ':') {
      pos_linea++;
      pos_pila = pila;
      pila = desp_pila(0);
      emite_etiq(agrega_etiqueta(n));
      dos_puntos();
      emite_nueva_linea();
      pila = desp_pila(pos_pila);
      return 1;
    }
    else pos_linea = c_pos_linea;
  }
  return 0;
}

agrega_etiqueta(nombre)
  char *nombre;
{
  char *ap;

  if (ap = busca_loc(nombre)) {
    if (ap[IDENT] != ETIQUETA)
      error("No es una etiqueta");
  } else
    ap = nueva_loc(nombre, ETIQUETA, AUTO, 0, nueva_etiq);
  return lee_entero(ap + POSICION);
}

/*
** Sentencia "return"
*/
s_return()
{
  int origen, pila_retorno, nodo_expr;
  char *tipo, *tipo2;

  /* Checa si hay una expresión */
  if (fin_sentencia() == 0) {
    origen = ultimo_nodo;
    usa_expr = SI;
    tipo2 = almacena_expresion(SI);
    tipo = lee_entero(funcion_actual + TIPO);
    if(*++tipo == STRUCT) {                /* el primer byte es FUNCION */
      if(*tipo2 != STRUCT)
        error("El resultado no tiene tipo de estructura");
      else if(lee_entero(tipo + 1) != lee_entero(tipo2 + 1))
        error("Estructuras incompatibles");
      copia_resultado(tam_tipo(tipo2));
      usa_expr = NO;
      evalua_arbol(NO);
      ultimo_nodo = origen;
      desp_pila(0);
      retorno();
      return;
    } else if(*tipo2 == STRUCT)
      error("La función no tiene tipo de estructura");
    else {
      nodo_expr = ultimo_nodo;
      convierte_tipo(&nodo_expr, tipo2, tipo);
    }
    evalua_arbol(NO);
    ultimo_nodo = origen;
  }
  desp_pila(0);                 /* Limpia la pila */
  retorno();                    /* Sale de la función */
}

/*
** Sentencia "break"
*/
s_break()
{
  /* Ve si hay un bucle abierto */
  if (ultimo_bucle == NULL) {
    error("No hay ningun bucle abierto");
    return;                         /* No */
  }
  desp_pila(ultimo_bucle[B_PILA]);  /* Si, arregla la pila */
  salto(ultimo_bucle[B_FIN]);       /* Salta a la etiqueta de salida */
}

/*
** Sentencia "continue"
*/
s_cont()
{
  int *u_bucle;

  /* Ve si hay un bucle abierto */
  u_bucle = ultimo_bucle;
  while(1) {
    if (u_bucle == NULL) {
      error("No hay ningun bucle abierto");
      return;                         /* No */
    }
    if (u_bucle[B_BUCLE]) break;
    u_bucle = u_bucle[B_ANTERIOR];
  }

  desp_pila(u_bucle[B_PILA]);         /* Si, arregla la pila */
  salto(u_bucle[B_BUCLE]);            /* Salta a la etiqueta de salida */
}

/*
** Detecta el fin de una sentencia, un punto y coma
** o el fin de archivo.
*/
fin_sentencia()
{
  espacios();
  return ((car_act == ';') || (car_act == 0));
}

nombre_ilegal()
{
  error("Nombre ilegal");
  basura();
}

redefinido(nombre)
  char *nombre;
{
  error("Nombre redefinido");
  comentario();
  emite_texto(nombre);
  emite_nueva_linea();
}

pide(cadena)
  char *cadena;
{
  if (match(cadena) == 0) {
    error("Falta un parentesis, llave o corchete");
    comentario();
    emite_texto(cadena);
    emite_nueva_linea();
  }
}

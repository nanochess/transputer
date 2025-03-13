/*
** Compilador de C para transputer.
** Interfaz con el usuario.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 4 de junio de 1995.
** Revisión: 26 de julio de 1995. Ahora ap_mac se inicializa a 1.
** Revisión: 27 de julio de 1995. Se modifica p_include() para que sólo
**                                acepte la sintaxis estandard.
** Revisión: 10 de agosto de 1995. Predefine los tipos estandares.
** Revisión: 12 de agosto de 1995. Soporte para #include anidado.
** Revisión: 22 de agosto de 1995. Inicialización de lista_estruct.
** Revisión: 23 de agosto de 1995. Inicialización de ultima_estruct.
** Revisión: 24 de agosto de 1995. Inicialización de lista_enum y ultimo_enum.
** Revisión: 7 de septiembre de 1995. Nueva función. inicializa().
** Revisión: 22 de noviembre de 1995. Inicialización de const_definidas, y
**                                    inicialización de pos_globales a 3.
*/

/*
** El compilador comienza su ejecución aquí.
*/
main()
{
  presentacion();           /* Presentacion */
  opciones();               /* Determina las opciones */
  abre_entrada();           /* Primer archivo a procesar */
  if(entrada != 0) {
    inicializa();           /* Inicializa todo */
    abre_salida();          /* Prepara el archivo de salida */
    prologo();              /* Emite el prologo */
    analiza();              /* Hace la compilación */
    if (nivel)
      error("Falta llave de cierre");
    epilogo();              /* Emite el epilogo */
    cierra_salida();        /* Cierra la salida */
    reporta_errores();      /* Reporta errores detectados */
  }
}

/*
** Inicializa todo.
*/
inicializa()
{
  ap_glb = INICIO_GLB;    /* Limpia la tabla global */
  ap_loc = INICIO_LOC;    /* Limpia la tabla local */
  ultimo_bucle = NULL;    /* Limpia la cola de bucles */
  pila =                  /* Apuntador de pila */
  errores =               /* No hay errores */
  eof =                   /* No se ha alcanzado el fin del archivo */
  desvio_salida =         /* No se ha desviado la salida */
  nivel =                 /* No hay bloques abiertos */
  ultima_sentencia =      /* Ninguna sentencia compilada aún */
  comienzo_funcion =      /* La función actual empezó en la linea 0 */
  linea_actual =          /* No se han leido líneas del archivo */
  dentro_funcion =        /* No esta dentro de una función */
  sig_etiq =              /* Inicia números de etiquetas */
  nivel_if =              /* No esta dentro de un #if... */
  nivel_incl =            /* No esta dentro de un #include */
  evadir_nivel =          /* No esta evadiendo ningun texto de la entrada */
  ultimo_nodo = 0;        /* Ultimo nodo usado del arbol */
  dentro_pp = NO;         /* No esta dentro del preprocesador */
  ap_mac = 1;             /* Limpia la tabla de macros */
  pos_global = 3;         /* Reserva dos palabras para el limpiador, una */
                          /* palabra extra para apuntar a la tabla de punto */
                          /* flotante */
  sig_case = casos;       /* Ningún case aún */
  funcion_actual = NULL;  /* Ninguna función aún */
  lista_estruct = NULL;   /* Ninguna estructura definida */
  ultima_estruct = NULL;  /* No hay última estructura definida */
  lista_enum = NULL;      /* No hay lista de enumeradores */
  ultimo_enum = NULL;     /* No hay último enumerador definido */
  sig_tipo = tipos;       /* Ningún tipo aún */
                          /* Prepara los tipos predefinidos */
  t_achar = sig_tipo;     /* Apuntador a char */
  *sig_tipo++ = APUNTADOR;
  t_char = sig_tipo;      /* char */
  *sig_tipo++ = CHAR;
  t_short = sig_tipo;     /* short */
  *sig_tipo++ = SHORT;
  t_func = sig_tipo;      /* Función que retorna int */
  *sig_tipo++ = FUNCION;
  t_int = sig_tipo;       /* int */
  *sig_tipo++ = INT;
  t_ushort = sig_tipo;    /* unsigned short */
  *sig_tipo++ = USHORT;
  t_uint = sig_tipo;      /* unsigned int */
  *sig_tipo++ = UINT;
  t_float = sig_tipo;     /* float */
  *sig_tipo++ = FLOAT;
  t_double = sig_tipo;    /* double */
  *sig_tipo++ = DOUBLE;
  t_void = sig_tipo;      /* void */
  *sig_tipo++ = VOID;

  const_definidas = 0;    /* Ninguna constante aún */
}

/*
** Selecciona un color
*/
color(col) int col; {
  putchar(0x1b);
  putchar(1);
  putchar(col);
}

/*
** Cancela la compilación.
*/
cancela()
{
  while (nivel_incl)
    fin_include();
  if (entrada)
    fclose(entrada);
  cierra_salida();
  hacia_consola();
  color(15);
  mensaje("Compilación cancelada.");
  emite_nueva_linea();
  exit(1);
}

/*
** Reporta los errores
*/
reporta_errores()
{
  emite_nueva_linea();
  color(11);
  emite_texto("Hubo ");
  emite_numero(errores);       /* No. total de errores */
  emite_texto(" errores en la compilación.");
  emite_nueva_linea();
}

/*
** Presentación.
*/
presentacion()
{
  color(15);
  mensaje(PROGRAMA);
  emite_nueva_linea();
}

/*
** Opciones de compilación.
*/
opciones()
{
  color(10);
  mensaje("¿ Desea una pausa despues de un error (S/N) ? ");
  gets(linea);
  pausa = NO;
  if ((car_act == 'S') || (car_act == 's'))
    pausa = SI;

  color(10);
  mensaje("¿ Desea que aparezca el listado C en la salida (S/N) ? ");
  gets(linea);
  intercala_fuente = NO;
  if ((car_act == 'S') | (car_act == 's'))
    intercala_fuente = SI;
}

/*
** Obtiene el nombre del archivo de salida.
*/
abre_salida()
{
  salida = 0;           /* Por defecto la salida a la consola */
  while (salida == 0) {
    descarta();
    color(10);
    mensaje("¿ Archivo de salida ? ");
    gets(linea);        /* Obtiene el nombre */
    if (car_act == 0)
      break;            /* Ninguno... */
    if ((salida = fopen(linea, "w")) == NULL) {  /* Intenta crear */
      salida = 0;       /* No pudo crearse */
      error("No se pudo crear el archivo");
    }
  }
  hacia_consola();
  emite_nueva_linea();
  hacia_archivo();
  descarta();           /* Limpia la línea */
}

/*
** Obtiene el archivo de entrada
*/
abre_entrada()
{
  entrada = 0;          /* Ninguno aún */
  while (entrada == 0) {
    descarta();         /* Limpia la línea de entrada */
    color(10);
    mensaje("¿ Archivo de entrada ? ");
    gets(linea);        /* Obtiene un nombre */
    if (car_act == 0)
      break;
    if ((entrada = fopen(linea, "r")) != NULL)
      nuevo_archivo();
    else {
      entrada = 0;      /* No se pudo leer */
      color(15);
      mensaje("No se pudo leer el archivo");
    }
  }
  descarta();           /* Limpia la línea */
}

/*
** Inicia el contador de líneas.
*/
nuevo_archivo()
{
  linea_actual = 0;     /* Ninguna línea leida */
  comienzo_funcion = 0; /* Ninguna función aún */
  funcion_actual = NULL;
  dentro_funcion = NO;
}

/*
** Procesa #include, abre el nuevo archivo.
*/
p_include()
{
  char *rastreo, *comienzo;
  int estatus;

  espacios();           /* Salta los espacios */

  hacia_consola();
  color(11);
  emite_texto("#include ");
  emite_texto(linea + pos_linea);
  emite_nueva_linea();
  hacia_archivo();

  rastreo = linea + pos_linea;
  if (*rastreo == '<') {
    ++rastreo;
    comienzo = rastreo;
    estatus = 1;
  } else if (*rastreo == '"') {
    ++rastreo;
    comienzo = rastreo;
    estatus = 2;
  } else {
    estatus = 0;
    comienzo = rastreo;
    error("Error de sintaxis");
  }
  while(*rastreo != '>' && *rastreo != '"' && *rastreo)
    ++rastreo;
  if(*rastreo == '>' && estatus == 1) *rastreo = 0;
  else if(*rastreo == '"' && estatus == 2) *rastreo = 0;
  else if(estatus != 0)
    error("Falta > o \" al final");
  if (nivel_incl == MAX_INCL)
    error("Demasiados #include");
  else if ((entrada2 = fopen(comienzo, "r")) == NULL)
    error("No se pudo leer el archivo");
  else {
    incl[nivel_incl++] = entrada;
    incl[nivel_incl++] = funcion_actual;
    incl[nivel_incl++] = comienzo_funcion;
    incl[nivel_incl++] = linea_actual;
    incl[nivel_incl++] = dentro_funcion;
    entrada = entrada2;
    nuevo_archivo();
  }
  descarta();           /* La siguiente entrada será del */
                        /* nuevo archivo. */
}

/*
** Fin de un archivo #include
*/
fin_include()
{
  hacia_consola();
  color(11);
  emite_texto("#fin include");
  emite_nueva_linea();
  hacia_archivo();

  fclose(entrada);
  dentro_funcion = incl[--nivel_incl];
  linea_actual = incl[--nivel_incl];
  comienzo_funcion = incl[--nivel_incl];
  funcion_actual = incl[--nivel_incl];
  entrada = incl[--nivel_incl];
}

/*
** Cierra el archivo de salida.
*/
cierra_salida()
{
  hacia_archivo();      /* Si esta desviado, volver al archivo */
  if (salida)
    fclose(salida);     /* Si esta abierto, cerrarlo */
  salida = 0;           /* Marcar como cerrado */
}

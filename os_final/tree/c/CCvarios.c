/*
** Compilador de C para transputer.
** Preprocesador y funciones varias.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 1 de junio de 1995.
** Revisión: 25 de julio de 1995. Nueva función, isspace().
** Revisión: 25 de julio de 1995. Se agrega #ifdef, #ifndef, #else y #endif
**                                al preprocesador.
** Revisión: 26 de julio de 1995. Nueva función. encuentra().
** Revisión: 26 de julio de 1995. Se agrega #undef y #line al preprocesador.
**                                Nueva función. borra_macro().
**                                Modifico busca_macro() para que retorne
**                                la posición del principio de la macro.
** Revisión: 26 de julio de 1995. Se agrega #if.
** Revisión: 26 de julio de 1995. Se pasa #define y #include al preprocesador.
** Revisión: 26 de julio de 1995. Se pasa #asm al preprocesador.
** Revisión: 27 de julio de 1995. Soporte para concatenar líneas usando \.
** Revisión: 27 de julio de 1995. Nuevas funciones. strcpy(), strcat().
** Revisión: 27 de julio de 1995. Ahora soporta #include y #define estandard.
**                                El preprocesador ya esta casí completo,
**                                faltan los #include anidados.
** Revisión: 27 de julio de 1995. Corrección de un defecto en encuentra().
** Revisión: 10 de agosto de 1995. Nueva función. lee_entero().
** Revisión: 10 de agosto de 1995. Modificación de nueva_glb y nueva_loc,
**                                 para el soporte de tipos complejos.
** Revisión: 10 de agosto de 1995. Nueva función. escribe_entero().
** Revisión: 12 de agosto de 1995. Soporte para #include anidado.
** Revisión: 23 de agosto de 1995. Nuevas funciones, busca_miembro(),
**                                 nuevo_miembro(), busca_estructura(),
**                                 nueva_estructura().
** Revisión: 24 de agosto de 1995. Nuevas funciones, busca_enum(),
**                                 nuevo_enum().
** Revisión: 29 de noviembre de 1995. Se pasa la funcion car_act() al archivo
**                                    CCVARS.C
** Revisión: 27 de diciembre de 1995. Corrección de un defecto horrible en el
**                                    preprocesamiento de macros con pars.
** Revisión: 28 de diciembre de 1995. Corrección de un error que hacia que
**                                    quedara un archivo abierto al salir por
**                                    un error.
** Revisión: 20 de junio de 1996. Hago que nueva_glb() y nueva_loc agregen
**                                el nivel de profundidad.
*/

/*
** Un nuevo bucle, lo agrega al final de la lista enlazada
*/
nuevo_bucle(ap)
  int ap[];
{
  ap[B_ANTERIOR] = ultimo_bucle;  /* Bucle anterior */
  ap[B_PILA] = pila;              /* Nivel de la pila */
  ap[B_BUCLE] = nueva_etiq;       /* Etiqueta del bucle */
  ap[B_FIN] = nueva_etiq;         /* Etiqueta de salida */
  ultimo_bucle = ap;
}

/*
** Una nueva variable/función global.
*/
nueva_glb(nombre, id, clase, tipo, valor)
  char *nombre, *tipo;
  int valor, clase, id;
{
  char *ap;

  if (ap_glb >= FIN_GLB) {
    error("Tabla global llena");
    return 0;
  }
  ap = ap_glb;
  while (alfanum(*ap++ = *nombre++));  /* Copia el nombre */
  ap = ap_glb;
  ap[IDENT] = id;
  ap[CLASE] = clase;
  ap[NIVEL] = 0;
  escribe_entero(ap + TIPO, tipo);
  escribe_entero(ap + POSICION, valor);
  ap_glb += TAM_SIM;
  return ap;
}

/*
** Una nueva variable local.
*/
nueva_loc(nombre, id, clase, tipo, valor)
  char *nombre, *tipo;
  int valor, clase, id;
{
  char *ap;

  if (ap_loc >= FIN_LOC) {
    error("Tabla local llena");
    return 0;
  }
  ap = ap_loc;
  while (alfanum(*ap++ = *nombre++));  /* Copia el nombre */
  ap = ap_loc;
  ap[IDENT] = id;
  ap[CLASE] = clase;
  ap[NIVEL] = nivel;
  escribe_entero(ap + TIPO, tipo);
  escribe_entero(ap + POSICION, valor);
  ap_loc += TAM_SIM;
  return ap;
}

/*
** Una nueva estructura.
*/
nueva_estructura(nombre)
  char *nombre;
{
  char *ap;
  int conteo;

  if(ultima_estruct != NULL)
    escribe_entero(ultima_estruct + EST_SIG, sig_tipo);
  ultima_estruct = sig_tipo;
  if(lista_estruct == NULL)
    lista_estruct = sig_tipo;
  conteo = 0;
  while(conteo++ < EST_NOMBRE)
    guarda_tipo(0);
  while(*nombre)
    guarda_tipo(*nombre++);
  guarda_tipo(0);
  return ultima_estruct;
}

/*
** Un nuevo miembro de una estructura.
*/
nuevo_miembro(lista, nombre)
  char **lista, *nombre;
{
  char *sig, *nuevo;
  int conteo;

  if(*lista == NULL)
    *lista = sig_tipo;
  else {
    sig = *lista;
    while(lee_entero(sig + MIE_SIG) != NULL)
      sig = lee_entero(sig + MIE_SIG);
    escribe_entero(sig + MIE_SIG, sig_tipo);
  }
  nuevo = sig_tipo;
  conteo = 0;
  while(conteo++ < MIE_NOMBRE)
    guarda_tipo(0);
  while(*nombre)
    guarda_tipo(*nombre++);
  guarda_tipo(0);
  return nuevo;
}

/*
** Una nueva constante de un enumerador.
*/
nuevo_enum(nombre, valor)
  char *nombre;
  int valor;
{
  int conteo;

  if(ultimo_enum != NULL)
    escribe_entero(ultimo_enum + ENUM_SIG, sig_tipo);
  ultimo_enum = sig_tipo;
  if(lista_enum == NULL)
    lista_enum = sig_tipo;
  guarda_tipo(valor);
  guarda_tipo(valor >> 8);
  guarda_tipo(valor >> 16);
  guarda_tipo(valor >> 24);
  conteo = 0;
  while(conteo++ < 4)
    guarda_tipo(0);
  while(*nombre)
    guarda_tipo(*nombre++);
  guarda_tipo(0);
}

/*
** Busca una variable/función global.
*/
busca_glb(nombre)
  char *nombre;
{
  char *ap;

  ap = INICIO_GLB;
  while (ap != ap_glb) {
    if (astreq(nombre, ap, MAX_NOMBRE))
      return ap;
    ap += TAM_SIM;
  }
  return NULL;
}

/*
** Busca una variable/función local.
*/
busca_loc(nombre)
  char *nombre;
{
  char *ap;

  ap = ap_loc;
  while (ap != INICIO_LOC) {
    ap -= TAM_SIM;
    if (astreq(nombre, ap, MAX_NOMBRE))
      return ap;
  }
  return NULL;
}

/*
** Busca una estructura.
*/
busca_estructura(nombre)
  char *nombre;
{
  char *ap;

  ap = lista_estruct;
  while (ap != NULL) {
    if (astreq(nombre, ap + EST_NOMBRE, MAX_NOMBRE))
      return ap;
    ap = lee_entero(ap + EST_SIG);
  }
  return NULL;
}

/*
** Busca un miembro de estructura.
*/
busca_miembro(lista, nombre)
  char *lista, *nombre;
{
  while (lista != NULL) {
    if (astreq(nombre, lista + MIE_NOMBRE, MAX_NOMBRE))
      return lista;
    lista = lee_entero(lista + MIE_SIG);
  }
  return NULL;
}

/*
** Busca un enumerador.
*/
busca_enum(nombre)
  char *nombre;
{
  char *ap;

  ap = lista_enum;
  while (ap != NULL) {
    if (astreq(nombre, ap + ENUM_NOMBRE, MAX_NOMBRE))
      return ap;
    ap = lee_entero(ap + ENUM_SIG);
  }
  return NULL;
}

/*
** Checa si la proxima cadena de entrada es un nombre legal.
*/
nombre_legal(nombre)
  char *nombre;
{
  int k;
  char c;

  espacios();
  if (letra(car_act) == 0)
    return (*nombre = 0);
  k = 0;
  while (alfanum(car_act)) {
    if(k < MAX_NOMBRE)
      nombre[k++] = obt_car();
    else
      obt_car();
  }
  nombre[k] = 0;
  return 1;
}

/*
** Imprime un retorno de carro y una cadena a la consola.
*/
mensaje(cad)
  char *cad;
{
  puts("\r\n");
  puts(cad);
}

/*
** Siguiente caracter en la línea.
*/
prox_car()
{
  if (car_act == 0)
    return 0;
  else
    return linea[pos_linea + 1];
}

/*
** Pasa al siguiente caracter, retorna el caracter anterior.
*/
obt_car()
{
  if (car_act == 0)
    return 0;
  else
    return linea[pos_linea++];
}

/*
** Descarta la línea actual.
*/
descarta()
{
  pos_linea = 0;
  linea[pos_linea] = 0;
}

/*
** Obtiene un caracter, si era el último en la línea, carga
** otra línea.
*/
lee_car()
{
  while (car_act == 0) {
    if (eof)
      return 0;
    preprocesa();
  }
  return obt_car();
}

/*
** Obtiene otra linea de la entrada.
*/
lee_linea()
{
  int k, unidad;

  while (1) {
    descarta();
    if (entrada == 0) {
      eof = 1;
      return;
    }
    unidad = entrada;
    while ((k = fgetc(unidad)) > 0) {
      if (k == 13)
        continue;
      if ((k == '\n') || (pos_linea >= MAX_LINEA))
        break;
      linea[pos_linea++] = k;
    }
    linea[pos_linea] = 0;   /* Agrega un caracter nulo */
    linea_actual++;         /* Se ha leido una línea más */
    if (k <= 0) {
      if (nivel_incl)
        fin_include();
      else {
        fclose(entrada);
        entrada = 0;
      }
    }
    if (pos_linea) {
      if (intercala_fuente) {
        comentario();
        emite_texto(linea);
        emite_nueva_linea();
      }
      pos_linea = 0;
      return;
    }
  }
}

/*
** Hace el preprocesamiento.
*/
preprocesa()
{
  int k, car, hay_if;
  char c, nombre[TAM_NOMBRE], *def, *busqueda;
  int subs, pars, args, paren, m;

  hay_if = 0;
  dentro_pp = SI;
  while(1) {
    primer_paso();
    if (eof) {
      dentro_pp = NO;
      return;
    }
    espacios();
    if(car_act == '#') {
      pos_linea++;
      espacios();
      if(match("ifdef")) {
        ++nivel_if;
        if(evadir_nivel) continue;
        nombre_legal(nombre);
        if(busca_macro(nombre) == 0)
          evadir_nivel = nivel_if;
        continue;
      }
      if(match("ifndef")) {
        ++nivel_if;
        if(evadir_nivel) continue;
        nombre_legal(nombre);
        if(busca_macro(nombre))
          evadir_nivel = nivel_if;
        continue;
      }
      if(match("if")) {
        ++nivel_if;
        if(evadir_nivel) continue;
        hay_if = pos_linea;
        break;
      }
      if(match("else")) {
        if(nivel_if) {
          if(evadir_nivel == nivel_if)
            evadir_nivel = 0;
          else if(evadir_nivel == 0)
            evadir_nivel = nivel_if;
        } else error("No hay #if...");
        continue;
      }
      if(match("endif")) {
        if(nivel_if) {
          if(evadir_nivel == nivel_if)
            evadir_nivel = 0;
          --nivel_if;
        } else error("No hay #if...");
        continue;
      }
      if(evadir_nivel) continue;
      if(match("asm")) {
        while(1) {
          lee_linea();
          if(eof) {
            dentro_pp = NO;
            return;
          }
          espacios();
          if(car_act == '#')
            break;
          emite_linea(linea + pos_linea);
        }
        continue;
      }
      if(match("include")) {
        p_include();
        continue;
      }
      if(match("define")) {
        nueva_macro();
        continue;
      }
      if(match("undef")) {
        nombre_legal(nombre);
        borra_macro(nombre);
        continue;
      }
      if(match("line")) continue;
    }
    if(evadir_nivel) continue;
    break;
  }
  pos_linea = hay_if;
  subs = SI;
  while(subs) {
    subs = NO;
    pos_linea_m = 0;
    while (car = car_act) {
      if (isspace(car))
        pp_espacios();
      else if (car == '"')
        pp_comillas();
      else if (car == 39)
        pp_apostrofe();
      else if (letra(car)) {
        k = 0;
        while (alfanum(car_act)) {
          if (k < MAX_NOMBRE)
            nombre[k++] = car_act;
          obt_car();
        }
        nombre[k] = 0;
        if(k = busca_macro(nombre)) {
          m = 0;
          while (macs[k++]);
          pars = macs[k++];
          def = macs + k;
          if(pars) {
            espacios();
            if(car_act != '(') error("falta (");
            obt_car();
            args = paren = 0;
            while(car_act && car_act != ')') {
              espacios();
              while(car_act) {
                if(car_act == ',' && paren == 0)
                  break;
                if(car_act == '(')
                  ++paren;
                if(car_act == ')') {
                  if(paren == 0) break;
                  --paren;
                }
                if(m < MAX_AMAC)
                  amacs[m++] = obt_car();
                else {
                  error("Tabla de parametros de macros llena");
                  cancela();
                }
              }
              amacs[m++] = 0;
              if(car_act == ',')
                obt_car();
              ++args;
            }
            if(args != pars)
              error("Número incorrecto de argumentos");
            if(car_act != ')')
              error("Falta )");
            obt_car();
          }
          while(*def) {
            if(*def != 127) almacena_car(*def++);
            else {
              busqueda = amacs;
              k = *++def;
              while(--k)
                while(*busqueda++) ;
              while(*busqueda)
                almacena_car(*busqueda++);
              ++def;
            }
          }
          subs = SI;
        } else {
          k = 0;
          while(c = nombre[k++])
            almacena_car(c);
        }
      } else
        almacena_car(obt_car());
    }
    if (hay_if)                /* Si hay #if, agrega un ; para que no vaya */
      almacena_car(';');       /* a salirse de la linea en caso de error. */
    almacena_car(0);
    strcpy(linea, linea_m);
    pos_linea = 0;
    if (pos_linea_m >= MAX_LINEA) {
      error("Línea muy larga");
      break;
    }
  }
  if (hay_if) {
    if(expr_constante() == 0)
      evadir_nivel = nivel_if;
    descarta();
  }
  dentro_pp = NO;
}

/*
** Primer paso del preprocesamiento, pega líneas terminadas en \, y
** elimina los comentarios.
*/
primer_paso() {
  int car;

  lee_linea();
  pos_linea = pos_linea_m = 0;
  if(eof) return;
  while (car = car_act) {
    if (isspace(car))
      pp_espacios();
    else if ((car == '\\') && (prox_car() == 0)) {
      lee_linea();
      if(eof) return;
    } else if (car == '"')
      pp_comillas();
    else if (car == 39)
      pp_apostrofe();
    else if ((car == '/') && (prox_car() == '*')) {
      almacena_car(' ');
      pp_comentarios();
    } else
      almacena_car(obt_car());
  }
  almacena_car(0);
  if (pos_linea_m >= MAX_LINEA)
    error("Línea muy larga");
  pos_linea = pos_linea_m = 0;
  while (linea[pos_linea++] = linea_m[pos_linea_m++]);
  pos_linea = 0;
}

almacena_car(c)
  char c;
{
  linea_m[pos_linea_m] = c;
  if (pos_linea_m < MAX_LINEA)
    pos_linea_m++;
  return c;
}

/*
** elimina espacios y tabuladores extras.
*/
pp_espacios()
{
  almacena_car(' ');
  while (isspace(car_act))
    obt_car();
}

/*
** Procesa cadenas de caracteres.
*/
pp_comillas()
{
  almacena_car(obt_car());
  while ((car_act != '"') ||
        ((linea[pos_linea - 1] == 92) && (linea[pos_linea - 2] != 92))) {
    if (car_act == 0) {
      error("Faltan comillas");
      break;
    }
    almacena_car(obt_car());
  }
  obt_car();
  almacena_car('"');
}

/*
** Procesa caracteres encerrados entre '
*/
pp_apostrofe()
{
  almacena_car(39);
  obt_car();
  while ((car_act != 39) ||
        ((linea[pos_linea - 1] == 92) && (linea[pos_linea - 2] != 92))) {
    if (car_act == 0) {
      error("Falta un apostrofe");
      break;
    }
    almacena_car(obt_car());
  }
  obt_car();
  almacena_car(39);
}

/*
** Procesa y elimina comentarios.
*/
pp_comentarios()
{
  pos_linea = pos_linea + 2;
  while ((car_act != '*') ||
         (prox_car() != '/')) {
    if (car_act == 0)
      lee_linea();
    else
      ++pos_linea;
    if (eof)
      break;
  }
  pos_linea = pos_linea + 2;
}

/*
** Añade una nueva macro en la tabla.
*/
nueva_macro()
{
  char nombre[TAM_NOMBRE];
  int k, car;
  int num_args;
  int l;        /* indice en la tabla de argumentos de macros */
  int numero;
  char *busqueda;

  if (nombre_legal(nombre) == 0) {
    nombre_ilegal();
    descarta();
    return;
  }
  borra_macro(nombre);
  k = 0;
  while (pone_macro(nombre[k++]));
  num_args = l = 0;
  if(car_act == '(') {   /* genera una lista de nombres de parametros */
    obt_car();
    while(car_act != ')') {
      espacios();
      if(letra(car_act)) {
        while(alfanum(car_act)) {
          if(l < MAX_AMAC) amacs[l++] = obt_car();
          else {
            error("Tabla de parametros de macros llena");
            cancela();
          }
        }
        amacs[l++] = 0;
        num_args++;
      }
      espacios();
      if(car_act == ',') obt_car();
      else if(car_act != ')') {
        error("Falta ) en #define");
        break;
      }
    }
    obt_car();
    amacs[l++] = 0;
  }
  espacios();
  pone_macro(num_args);  /* Ahora substituye los nombres por */
  while(car_act) {     /* secuencias 0x7f num */
    if(num_args && letra(car_act)) {
      numero = 1;
      busqueda = amacs;
      while(*busqueda) {         /* rastrea nombres */
        k = 0;
        while(1) {
          if(busqueda[k] != linea[pos_linea + k]) break;
          if(linea[pos_linea + k] < ' ') break;
          k++;
        }
        if(alfanum(busqueda[k]) || alfanum(linea[pos_linea + k])) {
          ++numero;
          while(*busqueda++) ;
        } else {
          pos_linea += k;
          pone_macro(127);
          pone_macro(numero);
          break;
        }
      }
      if(*busqueda == 0)
        while(alfanum(car_act))
          pone_macro(obt_car());
    } else {
      pone_macro(obt_car());
    }
  }
  pone_macro(0);
  if (ap_mac >= MAX_MAC)
    error("Tabla de macros llena");
}

/*
** Elimina una macro de la tabla.
*/
borra_macro(nombre) char *nombre; {
  int k, l, m;

  if(k = busca_macro(nombre)) {    /* Obtiene el comienzo de la macro */
    l = k;
    while (macs[k++]);             /* Busca el comienzo de la siguiente */
    k++;                           /* Evade la cuenta de argumentos */
    while (macs[k++]);
    m = k - l;
    while (k != ap_mac)            /* Mueve el bloque hacia atras */
      macs[l++] = macs[k++];
    ap_mac = ap_mac - m;           /* Ahora hay más espacio libre */
  }
}

pone_macro(c)
  char c;
{
  macs[ap_mac] = c;
  if (ap_mac < MAX_MAC)
    ap_mac++;
  return c;
}

/*
** Busca una macro en la tabla.
*/
busca_macro(nombre)
  char *nombre;
{
  int k;

  k = 1;
  while (k < ap_mac) {
    if (astreq(nombre, macs + k, MAX_NOMBRE))
      return k;
    while (macs[k++]);
    k++;
    while (macs[k++]);
  }
  return 0;
}

/*
** Desvia la salida a la consola.
*/
hacia_consola()
{
  desvio_salida = salida;
  salida = 0;
  color(15);
}

/*
** Regresa la salida al archivo.
*/
hacia_archivo()
{
  if (desvio_salida)
    salida = desvio_salida;
  desvio_salida = 0;
}

/*
** Manda un caracter a la salida.
*/
emite_car(c)
  char c;
{
  if (c == 0)
    return 0;
  if (salida) {
    if (fputc(c, salida) <= 0) {
      cierra_salida();
      error("Error al escribir");
      cancela();
    }
  } else
    putchar(c);
  return c;
}

/*
** Cambio de linea a la salida.
*/
emite_nueva_linea()
{
  emite_car(13);
  emite_car(10);
}

/*
** Ilustra los mensajes de error.
*/
error(ap)
  char ap[];
{
  int k;
  char entrada[81];

  hacia_consola();
  color(11);
  emite_texto("Línea ");
  emite_numero(linea_actual);
  emite_texto(", ");
  if (!dentro_funcion)
    emite_car('(');
  if (funcion_actual == NULL)
    emite_texto("comienzo del archivo");
  else
    emite_texto(funcion_actual + NOMBRE);
  if (!dentro_funcion)
    emite_car(')');
  emite_texto(" + ");
  emite_numero(linea_actual - comienzo_funcion);
  emite_texto(": ");
  color(15);
  emite_texto(ap);
  emite_nueva_linea();

  color(14);
  emite_texto(linea);
  emite_nueva_linea();

  k = 0;                /* Busca la posición del error */
  while (k < pos_linea) {
    if (linea[k++] == 9)
      emite_car(9);
    else
      emite_car(' ');
  }
  emite_car('^');
  emite_nueva_linea();
  ++errores;

  hacia_archivo();
  if (pausa) {
    color(10);
    mensaje("¿ Continuar (Si, No, Pasar de largo) ? ");
    gets(entrada);
    k = entrada[0];
    if ((k == 'N') || (k == 'n'))
      cancela();
    if ((k == 'P') || (k == 'p'))
      pausa = NO;
  }
}

/*
** Manda una línea a la salida, hace un cambio de linea también.
*/
emite_linea(ap)
  char *ap;
{
  emite_texto(ap);
  emite_nueva_linea();
}

/*
** Manda un texto a la salida.
*/
emite_texto(ap)
  char *ap;
{
  while (emite_car(*ap++));
}

/*
** Checa si encuentra un operador de expresión.
*/
encuentra(op) char *op; {
  int tam_op;
  espacios();
  if(tam_op = streq(linea + pos_linea, op))
    if((*(linea + pos_linea + tam_op) != '=') &&
       (*(linea + pos_linea + tam_op) != *(linea + pos_linea + tam_op - 1)))
      return 1;
  return 0;
}

streq(cad1, cad2)
  char cad1[], cad2[];
{
  int k;

  k = 0;
  while (cad2[k]) {
    if ((cad1[k]) != (cad2[k]))
      return 0;
    k++;
  }
  return k;
}

astreq(cad1, cad2, len)
  char cad1[], cad2[];
  int len;
{
  int k;

  k = 0;
  while (k < len) {
    if ((cad1[k]) != (cad2[k]))
      break;
    if (cad1[k] == 0)
      break;
    if (cad2[k] == 0)
      break;
    k++;
  }
  if (alfanum(cad1[k]))
    return 0;
  if (alfanum(cad2[k]))
    return 0;
  return k;
}

match(lit)
  char *lit;
{
  int k;

  espacios();
  if (k = streq(linea + pos_linea, lit)) {
    pos_linea = pos_linea + k;
    return 1;
  }
  return 0;
}

amatch(lit, len)
  char *lit;
  int len;
{
  int k;

  espacios();
  if (k = astreq(linea + pos_linea, lit, len)) {
    pos_linea = pos_linea + k;
    while (alfanum(car_act))
      lee_car();
    return 1;
  }
  return 0;
}

/*
** Salta los espacios en la entrada.
*/
espacios()
{
  while (1) {
    while (car_act == 0) {
      if (dentro_pp) return;
      preprocesa();
      if (eof)
        break;
    }
    if (isspace(car_act))
      obt_car();
    else
      return;
  }
}

/*
** Compone un entero.
*/
lee_entero(dir)
  char *dir;
{
  return *dir | (*(dir+1) << 8) | (*(dir+2) << 16) | (*(dir+3) << 24);
}

/*
** Escribe un entero en una dirección.
*/
escribe_entero(dir, dato)
  char *dir;
  int dato;
{
  *dir++ = dato;
  *dir++ = dato >> 8;
  *dir++ = dato >> 16;
  *dir++ = dato >> 24;
}

/*
** Saca un número decimal en la salida.
*/
emite_numero(numero)
  int numero;
{
  if (numero < 0) {
    emite_car('-');
    if (numero < -9)
      emite_numero(-(numero / 10));
    emite_car(-(numero % 10) + '0');
  } else {
    if (numero > 9)
      emite_numero(numero / 10);
    emite_car((numero % 10) + '0');
  }
}

/*
** Prueba si el caracter dado es una letra.
*/
letra(c)
  int c;
{
  c = c & 255;
  return (((c >= 'a') && (c <= 'z')) ||
          ((c >= 'A') && (c <= 'Z')) ||
           (c == '_'));
}

/*
** Prueba si el caracter dado es alfanumérico.
*/
alfanum(c)
  char c;
{
  return ((letra(c)) || (isdigit(c)));
}

/*
** Evade basura en la entrada.
*/
basura()
{
  if (alfanum(lee_car()))
    while (alfanum(car_act))
      obt_car();
  else
    while (alfanum(car_act) == 0) {
      if (car_act == 0)
        break;
      obt_car();
    }
  espacios();
}

/*
** Prueba si el caracter dado es un número.
*/
isdigit(c)
  int c;
{
  return ((c >= '0') && (c <= '9'));
}

/*
** Checa si es un número hexadecimal.
*/
isxdigit(c) char c; {
  return (((c >= '0') && (c <= '9')) ||
          ((c >= 'A') && (c <= 'F')) ||
          ((c >= 'a') && (c <= 'f')));
}

/*
** Checa si es un espacio.
*/
isspace(c) char c; {
  return (c == ' ') || (c == 9);
}

/*
** Conversión a máyusculas.
*/
toupper(c)
  char c;
{
  if ((c >= 'a') && (c <= 'z'))
    c = c + ('A' - 'a');
  return (c);
}

/*
** Retorna el tamaño de una cadena.
*/
strlen(s)
  char *s;
{
  char *t;

  t = s;
  while (*s)
    s++;
  return (s - t);
}

/*
** Copia una cadena.
*/
strcpy(destino, origen)
  char *destino, *origen;
{
  while (*destino++ = *origen++);
}

/*
** Concatena una cadena.
*/
strcat(destino, origen)
  char *destino, *origen;
{
  while (*destino) ++destino;
  strcpy(destino, origen);
}

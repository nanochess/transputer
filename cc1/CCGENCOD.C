/*
** Compilador de C para el G10.
** Generador de Codigo.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 3 de junio de 1995.
** Revisión: 20 de julio de 1995. Optimación de sumas de constantes que son
**                                multiplos de 4.
** Revisión: 20 de julio de 1995. Mejor optimación de pos-incrementos.
** Revisión: 24 de julio de 1995. Nueva función, compara_y_salta().
** Revisión: 25 de julio de 1995. Generación de codigo para N_ANDB y N_ORB.
** Revisión: 25 de julio de 1995. Generación de codigo para N_TRI.
** Revisión: 25 de julio de 1995. Nueva función, salto_no_int().
** Revisión: 26 de julio de 1995. Generación de codigo para N_ASIGNA, N_AOR,
**                                N_AXOR, N_AAND, N_ACI, N_ACD, N_ASUMA,
**                                N_ARESTA, N_AMUL, N_ADIV, y N_AMOD.
** Revisión: 26 de julio de 1995. Optimación más amplia sobre N_PINC, N_INC,
**                                y los nodos de asignación, calculando
**                                con exactitud el número de registros usados.
** Revisión: 26 de julio de 1995. Generación de codigo para N_AIXP y
**                                optimación de N_ASUMA con constantes.
** Revisión: 27 de julio de 1995. Generación de codigo para N_COMA.
** Revisión: 28 de julio de 1995. Optimación mejor de N_PINC y N_INC.
** Revisión: 29 de julio de 1995. Corrección de un defecto en N_ASIGNA.
** Revisión: 11 de agosto de 1995. Soporte para short y unsigned short.
** Revisión: 11 de agosto de 1995. Nuevas funciones. almacena(), carga(),
**                                 libreria().
** Revisión: 5 de septiembre de 1995. Soporte para N_COPIA.
** Revisión: 5 de septiembre de 1995. Soporte para paso de estructuras cómo
**                                    parametros de funciones, y también
**                                    cómo resultados.
** Revisión: 5 de septiembre de 1995. Nuevas funciones. copia_resultado(),
**                                    y estructura().
** Revisión: 6 de septiembre de 1995. Nueva función. asigna().
** Revisión: 22 de septiembre de 1995. Generación de codigo para N_CFLOAT,
**                                     N_CDOUBLE, N_IGUALPF, N_MAYORPF,
**                                     N_SUMAPF, N_RESTAPF, N_MULPF, N_DIVPF,
**                                     N_CEROPF, N_ENTPF, N_PFENT, y N_IXF. 
** Revisión: 23 de septiembre de 1995. Generación de codigo para N_PARF y
**                                     N_NUMPF, nueva función. copia_reg().
** Revisión: 23 de septiembre de 1995. Simplifico algunas comparaciones.
** Revisión: 27 de septiembre de 1995. Añado el codigo faltante para cargar
**                                     short y unsigned short.
** Revisión: 27 de septiembre de 1995. Corrección de un defecto en el manejo
**                                     de operadores de asignación, manejaba
**                                     incorrectamente la estructura del
**                                     arbol.
** Revisión: 27 de septiembre de 1995. Optimación de N_PFENT y N_ENTPF.
** Revisión: 22 de noviembre de 1995. Generación de codigo final para N_NUMPF.
** Revisión: 24 de noviembre de 1995. Corrección de un defecto en la generación
**                                    de codigo para N_NUMPF.
** Revisión: 25 de noviembre de 1995. Corrección de un defecto en la función
**                                    gen_oper(), no generaba fprev sino rev.
** Revisión: 29 de noviembre de 1995. Se pasa la función nueva_etiq() al
**                                    archivo CCVARS.C
** Revisión: 29 de noviembre de 1995. Optimación de expresiones de punto
**                                    flotante, para evaluar en precisión
**                                    simple.
** Revisión: 30 de noviembre de 1995. Optimación extra de evaluación de
**                                    expresiones de punto flotante.
** Revisión: 1o. de diciembre de 1995. Generación de codigo para N_CEROF,
**                                     N_ENTF y N_CONVDF.
** Revisión: 2 de diciembre de 1995. Optimación de N_CEROPF-N_CONVDF a N_CEROF.
** Revisión: 9 de diciembre de 1995. Corrección de un defecto en la función
**                                   asigna(), no recibia el parametro de tipo.
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en la generación
**                                    de codigo para unsigned int.
** Revisión: 8 de marzo de 1996. Pasa los argumentos argc y argv a main().
** Revisión: 8 de abril de 1996. Corrección de un defecto en paso de
**                               estructuras como argumentos.
** Revisión: 9 de abril de 1996. Corrección de defectos en manejo de
**                               estructuras.
** Revisión: 11 de abril de 1996. Corrección de un defecto tremendo en la
**                                generación de codigo para short y unsigned
**                                short.
** Revisión: 15 de abril de 1996. Corrección de defectos en manejo de
**                                estructuras.
** Revisión: 19 de abril de 1996. Corrección de un defecto en el paso de
**                                parametros reales a una función, cuando no
**                                era una expresión simple.
** Revisión: 20 de junio de 1996. Corrección de un defecto en la generación
**                                de la dirección para N_COPIA, cuando N_COPIA
**                                era descendiente de otro N_COPIA.
** Revisión: 7 de mayo de 1998. Los árboles de expresiones ahora son
**                              dinámicos, nueva función libera_arbol()
*/

/*
** Libera un arbol.
*/
libera_arbol(nodo)
  struct nodo *nodo;
{
  int op;

  op = nodo->oper;
  if (nodo->izq != NULL)
    libera_arbol(nodo->izq);
  if ((op != N_INC) && (op != N_PINC) && (op != N_RESULTA)
   && (op != N_PAR) && (op != N_PARF) && (nodo->der != NULL))
    libera_arbol(nodo->der);
  if ((op == N_FUNCI) || (op == N_PAR) || (op == N_PARF) ||
      (op == N_TRI) || (op == N_RESULTA))
    if (nodo->esp != NULL)
      libera_arbol(nodo->esp);
  free(nodo);
}

/*
** Crea un nodo del arbol de expresiones.
*/
crea_nodo(op, izq, der, val)
  int op, val;
  struct nodo *izq, *der;
{
  struct nodo *temp;

  ultimo_nodo = malloc(sizeof(struct nodo));
  if (ultimo_nodo == NULL) {
    error("Expresión muy compleja");
    cancela();
  }
  if (op == N_CSUMA) {
    if (izq->oper == N_CSUMA) {
      temp = izq;
      val += izq->esp;
      izq = izq->izq;
      free(temp);
    } else if (izq->oper == N_LDNLP) {
      temp = izq;
      val += izq->esp * 4;
      izq = izq->izq;
      free(temp);
    }
    if (val % 4 == 0) {
      op = N_LDNLP;
      val /= 4;
    }
  }
  if (op == N_LDNLP) {
    if (izq->oper == N_LDNLP) {
      temp = izq;
      val += izq->esp;
      izq = izq->izq;
      free(temp);
    } else if (izq->oper == N_LDLP) {
      val += izq->esp;
      free(izq);
      izq = NULL;
      op = N_LDLP;
    }
  } else if (op == N_CIGUAL) {
    if (val == 0)
      op = N_NOT;
  } else if (op == N_CONVDF && izq->oper == N_CEROPF) {
    op = N_CEROF;
    free(izq);
    izq = NULL;
  }
  ultimo_nodo->izq = izq;
  ultimo_nodo->der = der;
  ultimo_nodo->oper = op;
  ultimo_nodo->esp = val;
  ultimo_nodo->regs = 0;
  ultimo_nodo->regsf = 0;
}

/*
** Genera codigo para todo un arbol.
*/
gen_codigo(nodo)
  struct nodo *nodo;
{
  raiz_arbol = nodo;
  etiqueta(nodo);
  if (((nodo->oper == N_ANDB) || (nodo->oper == N_ORB)) && (es_control))
    corto_circuito(nodo, etiq_and, etiq_or);
  else
    gen_nodo(nodo);
}

/*
** Etiqueta un arbol.
**
** Cada nodo es etiquetado con el número de registros
** que requiere para evaluarse.
*/
etiqueta(nodo)
  struct nodo *nodo;
{
  int min, max, op, req_res;

  req_res = (nodo != raiz_arbol) || (usa_expr == SI);
  op = nodo->oper;
  if (nodo->izq != NULL)
    etiqueta(nodo->izq);
  if ((op != N_INC) && (op != N_PINC) && (op != N_RESULTA)
   && (op != N_PAR) && (op != N_PARF) && (nodo->der != NULL))
    etiqueta(nodo->der);
  if ((op == N_FUNCI) || (op == N_PAR) || (op == N_PARF) ||
      (op == N_TRI) || (op == N_RESULTA))
    if (nodo->esp != NULL)
      etiqueta(nodo->esp);
  if ((op == N_FUNCI) || (op == N_FUNC) ||
      (op == N_ANDB) || (op == N_ORB) ||
      (op == N_TRI) || (op == N_COMA) ||
      (op == N_COPIA)) {
    nodo->regs = 3;
    nodo->regsf = 3;
  } else if (op >= N_ASIGNA && op <= N_AIXP) {
    min = nodo->izq->regs;
    if (nodo->izq->oper == N_CONST && op == N_ASUMA)
      min = 0;
    max = nodo->der->regs;
    if (nodo->der->oper == N_LDLP && op == N_ASIGNA &&
        nodo->der->esp != SHORT && nodo->der->esp != USHORT)
      max = 0;
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    if (req_res && nodo->esp != FLOAT && nodo->esp != DOUBLE)
      max++;
    if (nodo->esp == FLOAT)
      max++;
    nodo->regs = max;
    min = nodo->izq->regsf;
    max = nodo->der->regsf;
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    if (req_res && (nodo->esp == FLOAT || nodo->esp == DOUBLE))
      max++;
    nodo->regsf = max;
  } else if ((op == N_INC) || (op == N_PINC)) {
    max = nodo->izq->regs;
    if (nodo->izq->oper != N_LDLP ||
        nodo->esp == SHORT || nodo->esp == USHORT)
      max++;
    if (req_res)
      max++;
    nodo->regs = max;
    nodo->regsf = nodo->izq->regsf;
  } else if (op == N_CEROPF || op == N_CEROF)
    nodo->regsf = 1;
  else if (op == N_NUMPF) {
    nodo->regs = 1;
    nodo->regsf = 1;
  } else if (nodo->izq == 0)
    nodo->regs = 1;
  else if (nodo->der == 0) {
    nodo->regs = nodo->izq->regs;
    nodo->regsf = nodo->izq->regsf;
    if (op == N_CUENTA || op == N_PFENT)
      nodo->regs++;
    if (op == N_ENTPF || op == N_ENTF || op == N_CFLOAT || op == N_CDOUBLE)
      nodo->regsf++;
  } else {
    min = nodo->izq->regs;
    max = nodo->der->regs;
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    if (op == N_SMAYOR)
      max++;
    nodo->regs = max;
    min = nodo->izq->regsf;
    max = nodo->der->regsf;
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    nodo->regsf = max;
    if (op == N_IGUALPF || op == N_MAYORPF)
      nodo->regs++;
  }
}

/*
** Codigo para cada operador binario, y algunos unarios.
*/
gen_oper(oper, rev)
  int oper, rev;
{
  if (oper == N_NULO) return;
  if (oper == N_CUENTA)
    emite_linea("wcnt");
  else if (oper == N_OR)
    emite_linea("or");
  else if (oper == N_XOR)
    emite_linea("xor");
  else if (oper == N_AND)
    emite_linea("and");
  else if (oper == N_IGUAL) {
    emite_linea("diff");
    emite_linea("eqc 0");
  } else if (oper == N_SUMA)
    emite_linea("bsub");
  else if (oper == N_MUL)
    emite_linea("prod");
  else if (oper == N_NEG) {
    emite_linea("not");
    emite_linea("adc 1");
  } else if (oper == N_COM)
    emite_linea("not");
  else if (oper == N_NOT)
    emite_linea("eqc 0");
  else if (oper == N_CBYTE)
    emite_linea("lb");
  else if (oper == N_CPAL)
    emite_linea("ldnl 0");
  else if (oper == N_CSHORT)
    emite_linea("call LIB_CSHORT");
  else if (oper == N_CUSHORT)
    emite_linea("call LIB_CUSHORT");
  else if (oper == N_IGUALPF)
    emite_linea("fpeq");
  else if (oper == N_SUMAPF)
    emite_linea("fpadd");
  else if (oper == N_MULPF)
    emite_linea("fpmul");
  else if (oper == N_CFLOAT)
    emite_linea("fpldnlsn");
  else if (oper == N_CDOUBLE)
    emite_linea("fpldnldb");
  else if (oper == N_CONVFD)
    emite_linea("fpur32tor64");
  else if (oper == N_CONVDF)
    emite_linea("fpur64tor32");
  else if (oper == N_ENTF) {
    emite_texto("ajw -1\nstl 0\nldlp 0\nfpi32tor32\najw 1\n");
  } else if (oper == N_ENTPF) {
    emite_texto("ajw -1\nstl 0\nldlp 0\nfpi32tor64\najw 1\n");
  } else if (oper == N_PFENT) {
    emite_texto("ajw -1\nldlp 0\nfpstnli32\nldl 0\najw 1\n");
  } else if (oper == N_SMAYOR) {
    if (!rev)
      emite_linea("rev");
    emite_texto("mint\nxor\nrev\nmint\nxor\ngt\n");
  } else {
    if (rev) {
      if (oper == N_MAYORPF || oper == N_RESTAPF || oper == N_DIVPF)
        emite_linea("fprev");
      else
        emite_linea("rev");
    }
    if (oper == N_MAYOR)
      emite_linea("gt");
    else if (oper == N_CD)
      emite_linea("shr");
    else if (oper == N_CI)
      emite_linea("shl");
    else if (oper == N_RESTA)
      emite_linea("diff");
    else if (oper == N_DIV)
      emite_linea("div");
    else if (oper == N_MOD)
      emite_linea("rem");
    else if (oper == N_IXP)
      emite_linea("wsub");
    else if (oper == N_IXF)
      emite_linea("wsubdb");
    else if (oper == N_CFI)
      emite_linea("fpldnlsni");
    else if (oper == N_CDI)
      emite_linea("fpldnldbi");
    else if (oper == N_MAYORPF)
      emite_linea("fpgt");
    else if (oper == N_RESTAPF)
      emite_linea("fpsub");
    else if (oper == N_DIVPF)
      emite_linea("fpdiv");
  }
}

/*
** Genera codigo para el nodo del arbol.
*/
gen_nodo(nodo)
  struct nodo *nodo;
{
  int conteo, pals, par, rev, op, req, req_res;
  int regb, regc, regbf, regcf, evitar_rev, temp;
  int etiq_and, etiq_or, pila_extra, que_onda, reqf;
  int opt_conv;
  struct nodo *nodo_temp, *nodo_b, *nodo_c;

  req_res = (nodo != raiz_arbol) || (usa_expr == SI);
  op = nodo->oper;
  if ((op == N_FUNC) || (op == N_FUNCI)) {
    pals = conteo = 0;
    nodo_b = nodo_c = NULL;
    nodo_temp = nodo->izq;
    req = SI;
    req_res = 0;                      /* Inicio de argumentos */
    while (nodo_temp != NULL) {
      if (nodo_temp->oper == N_RESULTA) {  /* Función que retorna estructura */
        pals += (req_res = nodo_temp->der);
        nodo_b = -1;
        req = NO;
      } else {
        if (nodo_temp->der)           /* garantiza que una estructura se */
          req = NO;                   /* alinea en la frontera de 4 palabras */
                                      /* después del call */
        if (nodo_temp->oper == N_PARF)/* Garantiza que un parametro de tipo */
          req = NO;                   /* double se alinea en la frontera de */
                                      /* 4 palabras después del call */
        if (nodo_b == NULL && req)
          nodo_b = nodo_temp->izq;
        else if (nodo_c == NULL && req)
          nodo_c = nodo_temp->izq;
        else {
          if (nodo_temp->der == NULL) {
            if (nodo_temp->oper != N_PARF)
              pals++;                 /* parametro simple */
            else
              pals += 2;              /* parametro de tipo double */
          } else
            pals += nodo_temp->der;   /* tamaño de la estructura */
          conteo++;
        }
      }
      nodo_temp = nodo_temp->esp;
    }
    pila = desp_pila(pila - pals);    /* asigna espacio a los parametros */
    if (conteo) {                     /* Procesa los parametros que no puede */
      nodo_temp = nodo->izq;          /* poner en registros. */
      if (nodo_b != NULL)             /* nodo_b y nodo_c pueden quedar sin uso, */
        nodo_temp = nodo_temp->esp;   /* sólo si se pasa una estructura. */
      if (nodo_c != NULL)
        nodo_temp = nodo_temp->esp;
      par = req_res;
      while (conteo--) {
        if (nodo_temp->der == NULL) {
          if (nodo_temp->oper != N_PARF) { /* parametro simple */
            gen_nodo(nodo_temp->izq);
            ins("stl ", par++);
          } else {
            gen_nodo(nodo_temp->izq);
            ins("ldlp ", par);
            emite_linea("fpstnldb");
            par += 2;
          }
        } else {                      /* estructura */
          pila_extra = pila;
          estructura(nodo_temp->izq);
          ins("ldlp ", par - (pila - pila_extra));
          ins("ldc ", (int) nodo_temp->der * 4);
          emite_linea("move");
          pila = desp_pila(pila_extra);
          par += nodo_temp->der;
        }
        nodo_temp = nodo_temp->esp;             /* siguiente parametro */
      }
    }
    if (op == N_FUNC) {               /* función directa, se aceptan params. */
      if (nodo_c == NULL) {           /* en registros, seleccionar carga */
        if (nodo_b != NULL) {         /* óptima. */
          if (nodo_b == -1)
            emite_linea("ldlp 0");
          else
            gen_nodo(nodo_b);
        }
      } else {
        if (nodo_b == -1) {
          gen_nodo(nodo_c);
          emite_linea("ldlp 0");
        } else if ((nodo_c->regs >= nodo_b->regs) &&
                   (nodo_b->regs < 3)) {
          gen_nodo(nodo_c);
          gen_nodo(nodo_b);
        } else if ((nodo_b->regs > nodo_c->regs) &&
                   (nodo_c->regs < 3)) {
          gen_nodo(nodo_b);
          gen_nodo(nodo_c);
          emite_linea("rev");
        } else {
          gen_nodo(nodo_b);
          salva(0);
          gen_nodo(nodo_c);
          recupera(0);
        }
      }
      ins("ldl ", 1 - pila);
      llamada(nodo->esp);
    } else {                          /* Llamada indirecta, simular */
      pila = desp_pila(pila - 4);     /* llamada con parametros en regs. */
      if (nodo_b != NULL) {
        if (nodo_b == -1)
          emite_linea("ldlp 4");
        else
          gen_nodo(nodo_b);
        emite_linea("stl 2");
      }
      if (nodo_c != NULL) {
        gen_nodo(nodo_c);
        emite_linea("stl 3");
      }
      gen_nodo(nodo->esp);
      ins("ldl ", 1 - pila);
      emite_texto("ldc 3\nldpi\nstl 0\nstl 1\ngcall\n");
      pila += 4;
    }
    if (nodo->der)
      emite_linea("ldlp 0");
    else
      pila = desp_pila(pila + pals);
    return;
  }
  if (op == N_ASIGNA) {
    que_onda = (nodo->esp == FLOAT || nodo->esp == DOUBLE);
    opt_conv = NO;
    if (req_res) {
      req = que_onda ? 1000000 : 2;
      reqf = que_onda ? 2 : 1000000;
    } else {
      req = que_onda ? 1000000 : 3;
      reqf = que_onda ? 3 : 1000000;
    }
    if (nodo->der->regs < req
     && nodo->der->regsf < reqf) {
      if (nodo->izq->oper == N_PFENT && !req_res
      && (nodo->esp == INT || nodo->esp == UINT)) {
        opt_conv = SI;
        gen_nodo(nodo->izq->izq);
      } else
        gen_nodo(nodo->izq);
      if (req_res)
        copia_reg(que_onda);
      if (!opt_conv)
        accesa_nodo(nodo->esp, nodo->der, NO);
      else {
        gen_nodo(nodo->der);
        emite_linea("fpstnli32");
      }
      return;
    }
    gen_nodo(nodo->der);
    req = que_onda ? 3 : (req_res ? 2 : 3);
    if (nodo->izq->regs < req) {
      if (nodo->izq->oper == N_PFENT && !req_res
      && (nodo->esp == INT || nodo->esp == UINT)) {
        opt_conv = SI;
        gen_nodo(nodo->izq->izq);
      } else
        gen_nodo(nodo->izq);
      if (req_res) {
        copia_reg(que_onda);
        if (!que_onda)
          emite_linea("pop");
      }
      if (!que_onda && !opt_conv)
        emite_linea("rev");
      if (opt_conv) {
        emite_linea("fpstnli32");
        return;
      }
    } else {
      salva(0);
      if (nodo->izq->oper == N_PFENT && !req_res
      && (nodo->esp == INT || nodo->esp == UINT)) {
        opt_conv = SI;
        gen_nodo(nodo->izq->izq);
      } else
        gen_nodo(nodo->izq);
      if (req_res)
        copia_reg(que_onda);
      recupera(0);
      if (opt_conv) {
        emite_linea("fpstnli32");
        return;
      }
    }
    almacena(nodo->esp);
    return;
  }
  if (op > N_ASIGNA && op <= N_AIXP) {
    que_onda = (nodo->esp == FLOAT || nodo->esp == DOUBLE);
    regb = nodo->izq->regs;
    regc = nodo->der->regs;
    regbf = nodo->izq->regsf;
    regcf = nodo->der->regsf;
    rev = NO;
    if (regc > 1) {    /* Esta es una dirección compleja */
      if ((nodo->izq->oper == N_CONST) && (op == N_ASUMA)) {
        gen_nodo(nodo->der);
        emite_linea("dup");
        carga(nodo->esp);
      } else {
        if (regc < 3 && (regcf < 3 || !que_onda)) {
          gen_nodo(nodo->izq);
          gen_nodo(nodo->der);
        } else if (regb < 3) {
          gen_nodo(nodo->der);
          gen_nodo(nodo->izq);
          emite_linea("rev");
        } else {
          gen_nodo(nodo->der);
          salva(0);
          gen_nodo(nodo->izq);
          recupera(0);
        }
        emite_linea("dup");
        if (!que_onda)
          emite_linea("pop");
        carga(nodo->esp);
        rev = SI;
      }
    } else {          /* Una dirección simple */
      if ((nodo->izq->oper == N_CONST) && (op == N_ASUMA)) {
        accesa_nodo(nodo->esp, nodo->der, SI);
      } else if (regb < 3) {
        accesa_nodo(nodo->esp, nodo->der, SI);
        gen_nodo(nodo->izq);
        rev = NO;
      } else {
        gen_nodo(nodo->izq);
        accesa_nodo(nodo->esp, nodo->der, SI);
        rev = SI;
      }
    }
    if (op == N_AOR)
      emite_linea("or");
    else if (op == N_AAND)
      emite_linea("and");
    else if (op == N_AXOR)
      emite_linea("xor");
    else if (op == N_ASUMA) {
      if (nodo->izq->oper == N_CONST) {
        if (temp = nodo->izq->esp) {
          if (temp % 4 == 0)
            ins("ldnlp ", temp / 4);
          else
            ins("adc ", temp);
        }
      } else if (que_onda)
        emite_linea("fpadd");
      else
        emite_linea("bsub");
    } else if (op == N_AMUL) {
      if (que_onda)
        emite_linea("fpmul");
      else
        emite_linea("prod");
    } else {
      if (rev) {
        if (que_onda)
          emite_linea("fprev");
        else
          emite_linea("rev");
      }
      if (op == N_ACI)
        emite_linea("shl");
      else if (op == N_ACD)
        emite_linea("shr");
      else if (op == N_ARESTA) {
        if (que_onda)
          emite_linea("fpsub");
        else
          emite_linea("diff");
      } else if (op == N_ADIV) {
        if (que_onda)
          emite_linea("fpdiv");
        else
          emite_linea("div");
      } else if (op == N_AMOD)
        emite_linea("rem");
      else if (op == N_AIXP)
        emite_linea("wsub");
    }
    if (req_res)
      copia_reg(que_onda);
    if (regc > 1) {    /* Esta es una dirección compleja */
      if (!que_onda) {
        if (req_res)
          emite_linea("pop");
        emite_linea("rev");
      }
      almacena(nodo->esp);
    } else {          /* Una dirección simple */
      accesa_nodo(nodo->esp, nodo->der, NO);
    }
    return;
  }
  if (op == N_APFUNC) {
    emite_texto("ldc ");
    emite_nombre(nodo->esp);
    emite_texto("-");
    emite_etiq(temp = nueva_etiq);
    emite_nueva_linea();
    emite_linea("ldpi");
    emite_etiq(temp);
    dos_puntos();
    emite_nueva_linea();
    return;
  }
  if (op == N_CEROF) {
    emite_linea("fpldzerosn");
    return;
  }
  if (op == N_CEROPF) {
    emite_linea("fpldzerodb");
    return;
  }
  if (op == N_NUMPF) {
    ins("ldl ", 1 - pila);
    ins("ldnl ", 2);
    if (nodo->esp)
      ins("ldnlp ", nodo->esp * 2);
    emite_linea("fpldnldb");
    return;
  }
  if (op == N_CONST) {
    ins("ldc ", nodo->esp);
    return;
  }
  if (op == N_LIT) {
    emite_texto("ldc ");
    emite_etiq(etiq_lit);
    emite_texto("-");
    emite_etiq(temp = nueva_etiq);
    emite_texto("+");
    emite_numero(nodo->esp);
    emite_nueva_linea();
    emite_linea("ldpi");
    emite_etiq(temp);
    dos_puntos();
    emite_nueva_linea();
    return;
  }
  if (op == N_LDLP) {
    ins("ldlp ", nodo->esp - pila);
    return;
  }
  if (op == N_LDL) {
    ins("ldl ", nodo->esp - pila);
    return;
  }
  if (op == N_STL) {
    ins("stl ", nodo->esp - pila);
    return;
  }
  if ((op == N_INC) || (op == N_PINC)) {
    if (nodo->izq->regs == 1) {
      accesa_nodo(nodo->esp, nodo->izq, SI);
    } else {
      gen_nodo(nodo->izq);
      emite_linea("dup");
      carga(nodo->esp);
    }
    if (op == N_PINC)
      if (req_res)
        emite_linea("dup");
    ins("adc ", nodo->der);
    if (op == N_INC)
      if (req_res)
        emite_linea("dup");
    if (nodo->izq->regs == 1) {
      accesa_nodo(nodo->esp, nodo->izq, NO);
    } else {
      if (req_res) {
        emite_linea("pop");
        if (op == N_INC)
          emite_linea("rev");
        else
          emite_linea("pop");
      } else
        emite_linea("rev");
      almacena(nodo->esp);
    }
    return;
  }
  if (op == N_ANDB) {
    gen_nodo(nodo->izq);
    salta_si_falso(temp = nueva_etiq);
    gen_nodo(nodo->der);
    salta_si_falso(temp);
    emite_linea("ldc 1");
    emite_etiq(temp);
    dos_puntos();
    emite_nueva_linea();
    return;
  }
  if (op == N_ORB) {
    if (nodo->izq->oper == N_NOT)
      gen_nodo(nodo->izq->izq);
    else {
      gen_nodo(nodo->izq);
      emite_linea("eqc 0");
    }
    salta_si_falso(temp = nueva_etiq);
    if (nodo->der->oper == N_NOT)
      gen_nodo(nodo->der->izq);
    else {
      gen_nodo(nodo->der);
      emite_linea("eqc 0");
    }
    salta_si_falso(temp);
    emite_linea("ldc 1");
    emite_etiq(temp);
    dos_puntos();
    emite_nueva_linea();
    emite_linea("eqc 0");
    return;
  }
  if (op == N_TRI) {
    temp = nueva_etiq;
    nodo_temp = nodo->esp;
    if ((nodo_temp->oper == N_ANDB) || (nodo_temp->oper == N_ORB)) {
      etiq_and = nueva_etiq;
      etiq_or = nueva_etiq;
      corto_circuito(nodo->esp, etiq_and, etiq_or);
      salta_si_falso(etiq_and);
      emite_etiq(etiq_or);
      dos_puntos();
      emite_nueva_linea();
    } else {
      gen_nodo(nodo->esp);
      salta_si_falso(etiq_and = nueva_etiq);
    }
    gen_nodo(nodo->izq);
    salto_no_int(temp);
    emite_etiq(etiq_and);
    dos_puntos();
    emite_nueva_linea();
    gen_nodo(nodo->der);
    emite_linea("dup");
    emite_etiq(temp);
    dos_puntos();
    emite_nueva_linea();
    emite_linea("rev");
    return;
  }
  if (op == N_COMA) {
    gen_nodo(nodo->izq);
    gen_nodo(nodo->der);
    return;
  }
  if (op == N_COPIA) {
    pila_extra = pila;
    if ((nodo->der->regs >= nodo->izq->regs) &&
        (nodo->izq->regs < 3)) {
      estructura(nodo->der);
      gen_nodo(nodo->izq);
    } else if ((nodo->izq->regs > nodo->der->regs) &&
               (nodo->der->regs < 3)) {
      gen_nodo(nodo->izq);
      estructura(nodo->der);
      emite_linea("rev");
    } else {
      gen_nodo(nodo->izq);
      salva(0);
      estructura(nodo->der);
      recupera(0);
    }
    if (req_res)
      emite_texto("ajw -1\ndup\nstl 0\n");
    ins("ldc ", nodo->esp);
    emite_linea("move");
    if (req_res)
      emite_texto("ldl 0\najw 1\n");
    pila = desp_pila(pila_extra);
    return;
  }
  if (op == N_ENTPF && nodo->izq->oper == N_CPAL) {
    gen_nodo(nodo->izq->izq);
    emite_linea("fpi32tor64");
    return;
  }
  if (op == N_CFLOAT && nodo->izq->oper == N_IXP) {
    nodo->izq->oper = N_CFI;
    gen_nodo(nodo->izq);
    nodo->izq->oper = N_IXP;
    return;
  }
  if (op == N_CDOUBLE && nodo->izq->oper == N_IXF) {
    nodo->izq->oper = N_CDI;
    gen_nodo(nodo->izq);
    nodo->izq->oper = N_IXF;
    return;
  }
  if (op == N_CPAL) {
    if (nodo->izq->oper == N_LDLP) {
      nodo->izq->oper = N_LDL;
      gen_nodo(nodo->izq);
      nodo->izq->oper = N_LDLP;
      return;
    } else if (nodo->izq->oper == N_LDNLP) {
      nodo->izq->oper = N_LDNL;
      gen_nodo(nodo->izq);
      nodo->izq->oper = N_LDNLP;
      return;
    }
  }
  rev = NO;
  if (nodo->der) {
    if (op >= N_IGUALPF && op <= N_DIVPF) {
      if ((nodo->izq->regsf >= nodo->der->regsf) &&
          (nodo->der->regsf < 3)) {
        gen_nodo(nodo->izq);
        if ((op == N_SUMAPF || op == N_MULPF)
        && (nodo->der->oper == N_CFLOAT
         || nodo->der->oper == N_CDOUBLE)) {
          gen_nodo(nodo->der->izq);
          if (nodo->der->oper == N_CFLOAT) {
            if (op == N_SUMAPF)
              emite_linea("fpldnladdsn");
            else
              emite_linea("fpldnlmulsn");
          } else {
            if (op == N_SUMAPF)
              emite_linea("fpldnladddb");
            else
              emite_linea("fpldnlmuldb");
          }
          return;
        } else
          gen_nodo(nodo->der);
      } else if ((nodo->der->regsf > nodo->izq->regsf) &&
                 (nodo->izq->regsf < 3)) {
        gen_nodo(nodo->der);
        if ((op == N_SUMAPF || op == N_MULPF)
        && (nodo->izq->oper == N_CFLOAT
         || nodo->izq->oper == N_CDOUBLE)) {
          gen_nodo(nodo->izq->izq);
          if (nodo->izq->oper == N_CFLOAT) {
            if (op == N_SUMAPF)
              emite_linea("fpldnladdsn");
            else
              emite_linea("fpldnlmulsn");
          } else {
            if (op == N_SUMAPF)
              emite_linea("fpldnladddb");
            else
              emite_linea("fpldnlmuldb");
          }
          return;
        } else
          gen_nodo(nodo->izq);
        rev = SI;
      } else {
        gen_nodo(nodo->der);
        salva(1);
        gen_nodo(nodo->izq);
        if (op == N_SUMAPF) {
          recupera(2);
          return;
        } else if (op == N_MULPF) {
          recupera(3);
          return;
        }
        recupera(1);
      }
    } else {
      if ((nodo->izq->regs >= nodo->der->regs) &&
          (nodo->der->regs < 3)) {
        gen_nodo(nodo->izq);
        gen_nodo(nodo->der);
      } else if ((nodo->der->regs > nodo->izq->regs) &&
                 (nodo->izq->regs < 3)) {
        gen_nodo(nodo->der);
        gen_nodo(nodo->izq);
        rev = SI;
      } else {
        gen_nodo(nodo->der);
        salva(0);
        gen_nodo(nodo->izq);
        recupera(0);
      }
    }
  } else
    gen_nodo(nodo->izq);
  if (op == N_CIGUAL) {
    ins("eqc ", nodo->esp);
    return;
  }
  if (op == N_CSUMA) {
    if (nodo->esp)
      ins("adc ", nodo->esp);
    return;
  }
  if (op == N_LDNLP) {
    if (nodo->esp)
      ins("ldnlp ", nodo->esp);
    return;
  }
  if (op == N_LDNL) {
    ins("ldnl ", nodo->esp);
    return;
  }
  if (op == N_STNL) {
    ins("stnl ", nodo->esp);
    return;
  }
  gen_oper(op, rev);
}

/*
** Genera una secuencia optima para && y ||
*/
corto_circuito(nodo, etiq_and, etiq_or)
  int etiq_and, etiq_or;
  struct nodo *nodo;
{
  int nueva, etiq, opt;

  nueva = NO;
  if (nodo->oper == N_ANDB) {
    if (nodo->izq->oper == N_ANDB)
      corto_circuito(nodo->izq, etiq_and, etiq_or);
    else if (nodo->izq->oper == N_ORB) {
      etiq = nueva_etiq;
      nueva = SI;
      corto_circuito(nodo->izq, etiq_and, etiq);
    } else {
      gen_nodo(nodo->izq);
    }
    salta_si_falso(etiq_and);
    if (nueva) {
      emite_etiq(etiq);
      dos_puntos();
      emite_nueva_linea();
    }
    if (nodo->der->oper == N_ANDB)
      corto_circuito(nodo->der, etiq_and, etiq_or);
    else if (nodo->der->oper == N_ORB)
      corto_circuito(nodo->der, etiq_and, etiq_or);
    else
      gen_nodo(nodo->der);
  } else if (nodo->oper == N_ORB) {
    opt = NO;
    if (nodo->izq->oper == N_ANDB) {
      etiq = nueva_etiq;
      nueva = SI;
      corto_circuito(nodo->izq, etiq, etiq_or);
    } else if (nodo->izq->oper == N_ORB)
      corto_circuito(nodo->izq, etiq_and, etiq_or);
    else
      if (nodo->izq->oper == N_NOT) {
        opt = SI;
        gen_nodo(nodo->izq->izq);
      } else
        gen_nodo(nodo->izq);
    if (!opt)
      emite_linea("eqc 0");
    salta_si_falso(etiq_or);
    if (nueva) {
      emite_etiq(etiq);
      dos_puntos();
      emite_nueva_linea();
    }
    if (nodo->der->oper == N_ANDB)
      corto_circuito(nodo->der, etiq_and, etiq_or);
    else if (nodo->der->oper == N_ORB)
      corto_circuito(nodo->der, etiq_and, etiq_or);
    else
      gen_nodo(nodo->der);
  }
}

accesa_nodo(tipo, nodo, lectura)
  int tipo, lectura;
  struct nodo *nodo;
{
  if ((tipo == INT || tipo == UINT) && (nodo->oper == N_LDLP)) {
    nodo->oper = lectura ? N_LDL : N_STL;
    gen_nodo(nodo);
    nodo->oper = N_LDLP;
  } else if ((tipo == INT || tipo == UINT) && (nodo->oper == N_LDNLP)) {
    nodo->oper = lectura ? N_LDNL : N_STNL;
    gen_nodo(nodo);
    nodo->oper = N_LDNLP;
  } else {
    gen_nodo(nodo);
    if (lectura)
      carga(tipo);
    else
      almacena(tipo);
  }
}

/*
** Genera el codigo correcto para copias de estructuras.
*/
estructura(nodo)
  struct nodo *nodo;
{
  if (nodo->oper == N_FUNC || nodo->oper == N_FUNCI)
    nodo->der = 1;
  gen_nodo(nodo);
}

carga(tipo)
  int tipo;
{
  if (tipo == INT || tipo == UINT)
    emite_linea("ldnl 0");
  else if (tipo == SHORT)
    emite_linea("call LIB_CSHORT");
  else if (tipo == USHORT)
    emite_linea("call LIB_CUSHORT");
  else if (tipo == DOUBLE)
    emite_linea("fpldnldb");
  else if (tipo == FLOAT)
    emite_linea("fpldnlsn");
  else
    emite_linea("lb");
}

almacena(tipo)
  int tipo;
{
  if (tipo == INT || tipo == UINT)
    emite_linea("stnl 0");
  else if (tipo == SHORT || tipo == USHORT)
    emite_linea("call LIB_GSHORT");
  else if (tipo == DOUBLE)
    emite_linea("fpstnldb");
  else if (tipo == FLOAT)
    emite_linea("fpstnlsn");
  else
    emite_linea("sb");
}

/*
** Copia una estructura para resultado de función.
*/
copia_resultado(tam)
  int tam;
{
  struct nodo *izq;

  izq = ultimo_nodo;
  crea_nodo(N_LDL, NULL, NULL, 2);
  crea_nodo(N_COPIA, ultimo_nodo, izq, tam);
}

asigna(nodo, posicion, tipo)
  struct nodo *nodo;
  int posicion;
  unsigned char *tipo;
{
  es_control = NO;
  gen_codigo(nodo);
  if (*tipo == FLOAT) {
    ins("ldlp ", posicion - pila);
    emite_linea("fpstnlsn");
  } else if (*tipo == DOUBLE) {
    ins("ldlp ", posicion - pila);
    emite_linea("fpstnldb");
  } else
    ins("stl ", posicion - pila);
}

/*
** Genera una instrucción con operando.
*/
ins(codigo, valor)
  unsigned char *codigo;
  int valor;
{
  emite_texto(codigo);
  emite_numero(valor);
  emite_nueva_linea();
}

/*
** Comienza una linea de comentarios para el ensamblador.
*/
comentario()
{
  emite_car(';');
}

/*
** Pone el prologo para el codigo generado.
*/
prologo()
{
  comentario();
  emite_texto(PROGRAMA);
  emite_nueva_linea();
  comentario();
  emite_nueva_linea();
  emite_linea("COMIENZO:");
  emite_linea("j INICIO");
}

/*
** Pone el epilogo para el codigo generado.
*/
epilogo()
{
  int temp;
  int pos, byte;
  int pos_total;

  emite_nueva_linea();
  comentario();
  emite_texto(" >>>>> Fin de compilación <<<<<");
  emite_nueva_linea();
  emite_texto("INICIO");
  dos_puntos();
  emite_nueva_linea();
  pos_total = pos_global + const_definidas * 2;
  ins("ajw ", -pos_total);
  if (const_definidas) {
    emite_texto("ldc TABLA-");
    emite_etiq(temp = nueva_etiq);
    emite_nueva_linea();
    emite_linea("ldpi");
    emite_etiq(temp);
    dos_puntos();
    emite_nueva_linea();
    ins("ldlp ", pos_global);
    ins("ldc ", const_definidas * 8);
    emite_linea("move");
    ins("ldlp ", pos_global);
    ins("stl ", 2);
  }
  if (pos_global > 3) {
    emite_linea("ldlp 3");
    emite_linea("stl 0");
    ins("ldc ", pos_global - 3);
    emite_linea("stl 1");
    emite_texto("INICIO2:\nldc 0\nldl 0\nstnl 0\nldl 0\n");
    emite_texto("adc 4\nstl 0\nldl 1\nadc -1\nstl 1\n");
    emite_texto("ldl 1\neqc 0\ncj INICIO2\n");
  }
  ins("ldl ", pos_total + 3);
  ins("ldl ", pos_total + 2);
  emite_linea("ldlp 0");
  emite_linea("call qmain");
  ins("ajw ", pos_total);
  emite_linea("ret");
  libreria();
  if (const_definidas) {
    emite_linea("TABLA:");
    for(pos = 0; pos < const_definidas; ++pos) {
      def_byte();
      for(byte = 0; byte < TAM_DOUBLE; ++byte) {
        emite_numero(constantes[pos].byte[byte]);
        if (byte != TAM_DOUBLE - 1)
          emite_car(',');
      }
      emite_nueva_linea();
    }
  }
}

/*
** Libreria requerida siempre.
**
** LIB_CSHORT   Carga un entero de tipo short con signo.
** LIB_CUSHORT  Carga un entero de tipo short sin signo.
** LIB_GSHORT   Almacena un entero de tipo short.
*/
libreria()
{
  emite_texto("LIB_CSHORT:\nldl 1\nlb\nldl 1\nadc 1\n");
  emite_texto("lb\nldc 8\nshl\nor\nldc 0x8000\nxword\n");
  emite_texto("stl 1\nldl 3\nldl 2\nldl 1\nret\n");
  emite_texto("LIB_CUSHORT:\nldl 1\nlb\nldl 1\nadc 1\n");
  emite_texto("lb\nldc 8\nshl\nor\nstl 1\nldl 3\n");
  emite_texto("ldl 2\nldl 1\nret\n");
  emite_texto("LIB_GSHORT:\nldl 2\nldl 1\nsb\nldl 2\n");
  emite_texto("ldc 8\nshr\nldl 1\nadc 1\nsb\nldl 3\nret\n");
}

/*
** Emite un nombre que no entre en conflicto con las
** palabras reservadas del ensamblador.
*/
emite_nombre(nombre)
  unsigned char *nombre;
{
  emite_texto("q");
  emite_texto(nombre);
}

/*
** Salva el registro A en la pila.
*/
salva(flotante)
  int flotante;
{
  if (flotante) {
    emite_texto("ajw -2\nldlp 0\nfpstnldb\n");
    pila -= 2;
  } else {
    emite_texto("ajw -1\nstl 0\n");
    --pila;
  }
}

/*
** Recupera el registro A desde la pila.
*/
recupera(flotante)
  int flotante;
{
  if (flotante) {
    emite_linea("ldlp 0");
    if (flotante == 1)
      emite_linea("fpldnldb");
    else if (flotante == 2)
      emite_linea("fpldnladddb");
    else if (flotante == 3)
      emite_linea("fpldnlmuldb");
    emite_linea("ajw 2");
    pila += 2;
  } else {
    emite_texto("ldl 0\najw 1\n");
    ++pila;
  }
}

/*
** Copia un registro entero o de punto flotante.
*/
copia_reg(flotante)
  int flotante;
{
  if (flotante)
    emite_linea("fpdup");
  else
    emite_linea("dup");
}

/*
** Llama a la función especificada.
*/
llamada(nombre)
  unsigned char *nombre;
{
  emite_texto("call ");
  emite_nombre(nombre);
  emite_nueva_linea();
}

/*
** Retorna de una función.
*/
retorno()
{
  emite_linea("ret");
}

/*
** Salta a la etiqueta interna especificada.
*/
salto(etiq)
  int etiq;
{
  ins("j c", etiq);
}

/*
** Prueba el registro A y salta si es falso.
*/
salta_si_falso(etiq)
  int etiq;
{
  ins("cj c", etiq);
}

/*
** Ejecuta un salto no interrumpible.
*/
salto_no_int(etiq)
  int etiq;
{
  emite_linea("ldc 0");
  ins("cj c", etiq);
}

/*
** Imprime el número especificado cómo una etiqueta.
*/
emite_etiq(etiq)
  int etiq;
{
  emite_texto("c");
  emite_numero(etiq);
}

dos_puntos()
{
  emite_car(58);
}

/*
** Seudo-operacion para definir un byte.
*/
def_byte()
{
  emite_texto("db ");
}

/*
** Desplaza la posición de la pila.
*/
desp_pila(nueva_pos)
  int nueva_pos;
{
  int k;

  if (k = nueva_pos - pila)
    ins("ajw ", k);
  return nueva_pos;
}

/*
** Hace una comparación y un salto. (para switch)
** No pierde el valor con el que esta comparando.
*/
compara_y_salta(valor, etiqueta)
  int valor, etiqueta;
{
  emite_linea("dup");
  ins("ldc ", valor);
  emite_linea("diff");
  salta_si_falso(etiqueta);
}

/*
** Vacia el almacenamiento de cadenas
*/
vacia_lits()
{
  int j, k;

  if (ap_lit == 0)
    return;             /* No hay nada, volver... */
  emite_etiq(etiq_lit); /* Imprime la etiqueta */
  dos_puntos();
  emite_nueva_linea();
  k = 0;                /* Inicia un indice... */
  while (k < ap_lit) {  /* para vaciar el almacenamiento */
    def_byte();         /* Define byte */
    j = 5;              /* Bytes por línea */
    while (j--) {
      emite_numero(lits[k++]);
      if ((j == 0) || (k >= ap_lit)) {
        emite_nueva_linea();  /* Otra línea */
        break;
      }
      emite_car(',');   /* Separa los bytes */
    }
  }
}

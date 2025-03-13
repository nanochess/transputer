/*
** Compilador de C para transputer.
** Generador de Codigo.
**
** (c) Copyright 1995 Oscar Toledo G.
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
** Revisión: 11 de marzo de 2025. Parche para temporales float.
*/

/*
** Crea un nodo del arbol de expresiones.
*/
crea_nodo(op, izq, der, val)
  int op, izq, der, val;
{
  if(op == N_CSUMA) {
    if(oper[izq] == N_CSUMA) {
      val = val + esp[izq];
      izq = nodo_izq[izq];
    } else if(oper[izq] == N_LDNLP) {
      val = val + esp[izq] * 4;
      izq = nodo_izq[izq];
    }
    if(val % 4 == 0) {
      op = N_LDNLP;
      val = val / 4;
    }
  }
  if(op == N_LDNLP) {
    if(oper[izq] == N_LDNLP) {
      val = val + esp[izq];
      izq = nodo_izq[izq];
    } else if(oper[izq] == N_LDLP) {
      val = val + esp[izq];
      izq = 0;
      op = N_LDLP;
    }
  } else if(op == N_CIGUAL) {
    if(val == 0) op = N_NOT;
  } else if(op == N_CONVDF && oper[izq] == N_CEROPF) {
    op = N_CEROF;
    izq = 0;
  }
  ++ultimo_nodo;
  if(ultimo_nodo == TAM_ARBOL) {
    error("Expresión muy compleja");
    cancela();
  }
  nodo_izq[ultimo_nodo] = izq;
  nodo_der[ultimo_nodo] = der;
  oper[ultimo_nodo] = op;
  esp[ultimo_nodo] = val;
  regs[ultimo_nodo] = 0;
  regsf[ultimo_nodo] = 0;
}

/*
** Genera codigo para todo un arbol.
*/
gen_codigo(nodo)
  int nodo;
{
  raiz_arbol = nodo;
  etiqueta(nodo);
  if(((oper[nodo] == N_ANDB) || (oper[nodo] == N_ORB)) && (es_control))
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
  int nodo;
{
  int min, max, op, req_res;

  req_res = (nodo != raiz_arbol) || (usa_expr == SI);
  op = oper[nodo];
  if (nodo_izq[nodo])
    etiqueta(nodo_izq[nodo]);
  if ((op != N_INC) && (op != N_PINC) && (op != N_RESULTA)
  && (op != N_PAR) && (op != N_PARF) && nodo_der[nodo])
    etiqueta(nodo_der[nodo]);
  if ((op == N_FUNCI) || (op == N_PAR) || (op == N_PARF) ||
      (op == N_TRI) || (op == N_RESULTA))
    if (esp[nodo])
      etiqueta(esp[nodo]);
  if ((op == N_FUNCI) || (op == N_FUNC) ||
      (op == N_ANDB) || (op == N_ORB) ||
      (op == N_TRI) || (op == N_COMA) ||
      (op == N_COPIA)) {
    regs[nodo] = 3;
    regsf[nodo] = 3;
  } else if(op >= N_ASIGNA && op <= N_AIXP) {
    min = regs[nodo_izq[nodo]];
    if (oper[nodo_izq[nodo]] == N_CONST && op == N_ASUMA)
      min = 0;
    max = regs[nodo_der[nodo]];
    if (oper[nodo_der[nodo]] == N_LDLP && op == N_ASIGNA &&
        esp[nodo_der[nodo]] != SHORT && esp[nodo_der[nodo]] != USHORT)
      max = 0;
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    if(req_res && esp[nodo] != FLOAT && esp[nodo] != DOUBLE)
      max++;
    if(esp[nodo] == FLOAT)
      max++;
    regs[nodo] = max;
    min = regsf[nodo_izq[nodo]];
    max = regsf[nodo_der[nodo]];
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    if(req_res && (esp[nodo] == FLOAT || esp[nodo] == DOUBLE))
      max++;
    regsf[nodo] = max;
  } else if ((op == N_INC) || (op == N_PINC)) {
    max = regs[nodo_izq[nodo]];
    if(oper[nodo_izq[nodo]] != N_LDLP ||
       esp[nodo] == SHORT || esp[nodo] == USHORT)
      max++;
    if(req_res)
      max++;
    regs[nodo] = max;
    regsf[nodo] = regsf[nodo_izq[nodo]];
  } else if (op == N_CEROPF || op == N_CEROF)
    regsf[nodo] = 1;
  else if (op == N_NUMPF) {
    regs[nodo] = 1;
    regsf[nodo] = 1;
  } else if (nodo_izq[nodo] == 0)
    regs[nodo] = 1;
  else if (nodo_der[nodo] == 0) {
    regs[nodo] = regs[nodo_izq[nodo]];
    regsf[nodo] = regsf[nodo_izq[nodo]];
    if(op == N_CUENTA || op == N_PFENT)
      regs[nodo]++;
    if(op == N_ENTPF || op == N_ENTF || op == N_CFLOAT || op == N_CDOUBLE)
      regsf[nodo]++;
  } else {
    min = regs[nodo_izq[nodo]];
    max = regs[nodo_der[nodo]];
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    if(op == N_SMAYOR)
      max++;
    regs[nodo] = max;
    min = regsf[nodo_izq[nodo]];
    max = regsf[nodo_der[nodo]];
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    regsf[nodo] = max;
    if(op == N_IGUALPF || op == N_MAYORPF)
      regs[nodo]++;
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
    emite_texto("ajw -1\r\nstl 0\r\nldlp 0\r\nfpi32tor32\r\najw 1\r\n");
  } else if (oper == N_ENTPF) {
    emite_texto("ajw -1\r\nstl 0\r\nldlp 0\r\nfpi32tor64\r\najw 1\r\n");
  } else if (oper == N_PFENT) {
    emite_texto("ajw -1\r\nldlp 0\r\nfpstnli32\r\nldl 0\r\najw 1\r\n");
  } else if (oper == N_SMAYOR) {
    if (!rev)
      emite_linea("rev");
    emite_texto("mint\r\nxor\r\nrev\r\nmint\r\nxor\r\ngt\r\n");
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
  int nodo;
{
  int temp, conteo, pals, par, rev, op, req, req_res;
  int regb, regc, regbf, regcf, evitar_rev;
  int etiq_and, etiq_or, pila_extra, que_onda, reqf;
  int opt_conv;

  req_res = (nodo != raiz_arbol) || (usa_expr == SI);
  op = oper[nodo];
  if ((op == N_FUNC) || (op == N_FUNCI)) {
    pals = conteo = 0;
    regb = regc = 0;
    temp = nodo_izq[nodo];
    req = SI;
    req_res = 0;                      /* Inicio de argumentos */
    while (temp) {
      if(oper[temp] == N_RESULTA) {   /* Función que retorna estructura */
        pals += (req_res = nodo_der[temp]);
        regb = -1;
        req = NO;
      } else {
        if (nodo_der[temp])           /* garantiza que una estructura se */
          req = NO;                   /* alinea en la frontera de 4 palabras */
                                      /* después del call */
        if (oper[temp] == N_PARF)     /* Garantiza que un parametro de tipo */
          req = NO;                   /* double se alinea en la frontera de */
                                      /* 4 palabras después del call */
        if(regb == 0 && req)
          regb = nodo_izq[temp];
        else if(regc == 0 && req)
          regc = nodo_izq[temp];
        else {
          if(nodo_der[temp] == 0) {
            if(oper[temp] != N_PARF)
              pals++;                 /* parametro simple */
            else
              pals += 2;              /* parametro de tipo double */
          } else
            pals += nodo_der[temp];   /* tamaño de la estructura */
          conteo++;
        }
      }
      temp = esp[temp];
    }
    pila = desp_pila(pila - pals);    /* asigna espacio a los parametros */
    if(conteo) {                      /* Procesa los parametros que no puede */
      temp = nodo_izq[nodo];          /* poner en registros. */
      if (regb)                       /* regb y regc pueden quedar sin uso, */
        temp = esp[temp];             /* sólo si se pasa una estructura. */
      if (regc)
        temp = esp[temp];
      par = req_res;
      while (conteo--) {
        if(nodo_der[temp] == 0) {
          if(oper[temp] != N_PARF) {  /* parametro simple */
            gen_nodo(nodo_izq[temp]);
            ins("stl ", par++);
          } else {
            gen_nodo(nodo_izq[temp]);
            ins("ldlp ", par);
            emite_linea("fpstnldb");
            par += 2;
          }
        } else {                      /* estructura */
          pila_extra = pila;
          estructura(nodo_izq[temp]);
          ins("ldlp ", par - (pila - pila_extra));
          ins("ldc ", nodo_der[temp] * 4);
          emite_linea("move");
          pila = desp_pila(pila_extra);
          par += nodo_der[temp];
        }
        temp = esp[temp];             /* siguiente parametro */
      }
    }
    if (op == N_FUNC) {               /* función directa, se aceptan params. */
      if(regc == 0) {                 /* en registros, seleccionar carga */
        if(regb) {                    /* optima. */
          if(regb == -1)
            emite_linea("ldlp 0");
          else
            gen_nodo(regb);
        }
      } else {
        if(regb == -1) {
          gen_nodo(regc);
          emite_linea("ldlp 0");
        } else if ((regs[regc] >= regs[regb]) &&
            (regs[regb] < 3)) {
          gen_nodo(regc);
          gen_nodo(regb);
        } else if ((regs[regb] > regs[regc]) &&
                   (regs[regc] < 3)) {
          gen_nodo(regb);
          gen_nodo(regc);
          emite_linea("rev");
        } else {
          gen_nodo(regb);
          salva(0);
          gen_nodo(regc);
          recupera(0);
        }
      }
      ins("ldl ", 1 - pila);
      llamada(esp[nodo]);
    } else {                          /* Llamada indirecta, simular */
      pila = desp_pila(pila - 4);     /* llamada con parametros en regs. */
      if(regb) {
        if(regb == -1)
          emite_linea("ldlp 4");
        else
          gen_nodo(regb);
        emite_linea("stl 2");
        }
      if(regc) {
        gen_nodo(regc);
        emite_linea("stl 3");
        }
      gen_nodo(esp[nodo]);
      ins("ldl ", 1 - pila);
      emite_texto("ldc 3\r\nldpi\r\nstl 0\r\nstl 1\r\ngcall\r\n");
      pila += 4;
    }
    if(nodo_der[nodo])
      emite_linea("ldlp 0");
    else
      pila = desp_pila(pila + pals);
    return;
  }
  if (op == N_ASIGNA) {
    que_onda = (esp[nodo] == FLOAT || esp[nodo] == DOUBLE);
    opt_conv = NO;
    if(req_res) {
      req = que_onda ? 1000000 : 2;
      reqf = que_onda ? 2 : 1000000;
    } else {
      req = que_onda ? 1000000 : 3;
      reqf = que_onda ? 3 : 1000000;
    }
    if (regs[nodo_der[nodo]] < req
     && regsf[nodo_der[nodo]] < reqf) {
      if(oper[nodo_izq[nodo]] == N_PFENT && !req_res
      && (esp[nodo] == INT || esp[nodo] == UINT)) {
        opt_conv = SI;
        gen_nodo(nodo_izq[nodo_izq[nodo]]);
      } else
        gen_nodo(nodo_izq[nodo]);
      if(req_res)
        copia_reg(que_onda);
      if(!opt_conv)
        accesa_nodo(esp[nodo], nodo_der[nodo], NO);
      else {
        gen_nodo(nodo_der[nodo]);
        emite_linea("fpstnli32");
      }
      return;
    }
    gen_nodo(nodo_der[nodo]);
    req = que_onda ? 3 : (req_res ? 2 : 3);
    if (regs[nodo_izq[nodo]] < req) {
      if(oper[nodo_izq[nodo]] == N_PFENT && !req_res
      && (esp[nodo] == INT || esp[nodo] == UINT)) {
        opt_conv = SI;
        gen_nodo(nodo_izq[nodo_izq[nodo]]);
      } else
        gen_nodo(nodo_izq[nodo]);
      if(req_res) {
        copia_reg(que_onda);
        if(!que_onda)
          emite_linea("pop");
      }
      if(!que_onda && !opt_conv)
        emite_linea("rev");
      if(opt_conv) {
        emite_linea("fpstnli32");
        return;
      }
    } else {
      salva(0);
      if(oper[nodo_izq[nodo]] == N_PFENT && !req_res
      && (esp[nodo] == INT || esp[nodo] == UINT)) {
        opt_conv = SI;
        gen_nodo(nodo_izq[nodo_izq[nodo]]);
      } else
        gen_nodo(nodo_izq[nodo]);
      if(req_res)
        copia_reg(que_onda);
      recupera(0);
      if(opt_conv) {
        emite_linea("fpstnli32");
        return;
      }
    }
    almacena(esp[nodo]);
    return;
  }
  if(op > N_ASIGNA && op <= N_AIXP) {
    que_onda = (esp[nodo] == FLOAT || esp[nodo] == DOUBLE);
    regb = regs[nodo_izq[nodo]];
    regc = regs[nodo_der[nodo]];
    regbf = regsf[nodo_izq[nodo]];
    regcf = regsf[nodo_der[nodo]];
    rev = NO;
    if(regc > 1) {    /* Esta es una dirección compleja */
      if((oper[nodo_izq[nodo]] == N_CONST) && (op == N_ASUMA)) {
        gen_nodo(nodo_der[nodo]);
        emite_linea("dup");
        carga(esp[nodo]);
      } else {
        if(regc < 3 && (regcf < 3 || !que_onda)) {
          gen_nodo(nodo_izq[nodo]);
          gen_nodo(nodo_der[nodo]);
        } else if(regb < 3) {
          gen_nodo(nodo_der[nodo]);
          gen_nodo(nodo_izq[nodo]);
          emite_linea("rev");
        } else {
          gen_nodo(nodo_der[nodo]);
          salva(0);
          gen_nodo(nodo_izq[nodo]);
          recupera(0);
        }
        emite_linea("dup");
        if(!que_onda)
          emite_linea("pop");
        carga(esp[nodo]);
        rev = SI;
      }
    } else {          /* Una dirección simple */
      if((oper[nodo_izq[nodo]] == N_CONST) && (op == N_ASUMA)) {
        accesa_nodo(esp[nodo], nodo_der[nodo], SI);
      } else if(regb < 3) {
        accesa_nodo(esp[nodo], nodo_der[nodo], SI);
        gen_nodo(nodo_izq[nodo]);
        rev = NO;
      } else {
        gen_nodo(nodo_izq[nodo]);
        accesa_nodo(esp[nodo], nodo_der[nodo], SI);
        rev = SI;
      }
    }
    if(op == N_AOR) emite_linea("or");
    else if(op == N_AAND) emite_linea("and");
    else if(op == N_AXOR) emite_linea("xor");
    else if(op == N_ASUMA) {
      if(oper[nodo_izq[nodo]] == N_CONST) {
        if(temp = esp[nodo_izq[nodo]]) {
          if(temp % 4 == 0)
            ins("ldnlp ", temp / 4);
          else
            ins("adc ", temp);
        }
      } else if(que_onda)
        emite_linea("fpadd");
      else
        emite_linea("bsub");
    } else if(op == N_AMUL) {
      if (que_onda)
        emite_linea("fpmul");
      else
        emite_linea("prod");
    } else {
      if(rev) {
        if (que_onda)
          emite_linea("fprev");
        else
          emite_linea("rev");
      }
      if(op == N_ACI) emite_linea("shl");
      else if(op == N_ACD) emite_linea("shr");
      else if(op == N_ARESTA) {
        if (que_onda)
          emite_linea("fpsub");
        else
          emite_linea("diff");
      } else if(op == N_ADIV) {
        if (que_onda)
          emite_linea("fpdiv");
        else
          emite_linea("div");
      } else if(op == N_AMOD) emite_linea("rem");
      else if(op == N_AIXP) emite_linea("wsub");
    }
    if(req_res)
      copia_reg(que_onda);
    if(regc > 1) {    /* Esta es una dirección compleja */
      if(!que_onda) {
        if(req_res)
          emite_linea("pop");
        emite_linea("rev");
      }
      almacena(esp[nodo]);
    } else {          /* Una dirección simple */
      accesa_nodo(esp[nodo], nodo_der[nodo], NO);
    }
    return;
  }
  if (op == N_APFUNC) {
    emite_texto("ldc ");
    emite_nombre(esp[nodo]);
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
    if(esp[nodo])
      ins("ldnlp ", esp[nodo] * 2);
    emite_linea("fpldnldb");
    return;
  }
  if (op == N_CONST) {
    ins("ldc ", esp[nodo]);
    return;
  }
  if (op == N_LIT) {
    emite_texto("ldc ");
    emite_etiq(etiq_lit);
    emite_texto("-");
    emite_etiq(temp = nueva_etiq);
    emite_texto("+");
    emite_numero(esp[nodo]);
    emite_nueva_linea();
    emite_linea("ldpi");
    emite_etiq(temp);
    dos_puntos();
    emite_nueva_linea();
    return;
  }
  if (op == N_LDLP) {
    ins("ldlp ", esp[nodo] - pila);
    return;
  }
  if (op == N_LDL) {
    ins("ldl ", esp[nodo] - pila);
    return;
  }
  if (op == N_STL) {
    ins("stl ", esp[nodo] - pila);
    return;
  }
  if ((op == N_INC) || (op == N_PINC)) {
    if (regs[nodo_izq[nodo]] == 1) {
      accesa_nodo(esp[nodo], nodo_izq[nodo], SI);
    } else {
      gen_nodo(nodo_izq[nodo]);
      emite_linea("dup");
      carga(esp[nodo]);
    }
    if (op == N_PINC)
      if(req_res)
        emite_linea("dup");
    ins("adc ", nodo_der[nodo]);
    if (op == N_INC)
      if(req_res)
        emite_linea("dup");
    if (regs[nodo_izq[nodo]] == 1) {
      accesa_nodo(esp[nodo], nodo_izq[nodo], NO);
    } else {
      if (req_res) {
        emite_linea("pop");
        if (op == N_INC)
          emite_linea("rev");
        else
          emite_linea("pop");
      } else
        emite_linea("rev");
      almacena(esp[nodo]);
    }
    return;
  }
  if (op == N_ANDB) {
    gen_nodo(nodo_izq[nodo]);
    salta_si_falso(temp = nueva_etiq);
    gen_nodo(nodo_der[nodo]);
    salta_si_falso(temp);
    emite_linea("ldc 1");
    emite_etiq(temp);
    dos_puntos();
    emite_nueva_linea();
    return;
  }
  if (op == N_ORB) {
    if(oper[nodo_izq[nodo]] == N_NOT)
      gen_nodo(nodo_izq[nodo_izq[nodo]]);
    else {
      gen_nodo(nodo_izq[nodo]);
      emite_linea("eqc 0");
    }
    salta_si_falso(temp = nueva_etiq);
    if(oper[nodo_der[nodo]] == N_NOT)
      gen_nodo(nodo_izq[nodo_der[nodo]]);
    else {
      gen_nodo(nodo_der[nodo]);
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
    if((oper[esp[nodo]] == N_ANDB) || (oper[esp[nodo]] == N_ORB)) {
      etiq_and = nueva_etiq;
      etiq_or = nueva_etiq;
      corto_circuito(esp[nodo], etiq_and, etiq_or);
      salta_si_falso(etiq_and);
      emite_etiq(etiq_or);
      dos_puntos();
      emite_nueva_linea();
    } else {
      gen_nodo(esp[nodo]);
      salta_si_falso(etiq_and = nueva_etiq);
    }
    gen_nodo(nodo_izq[nodo]);
    salto_no_int(temp);
    emite_etiq(etiq_and);
    dos_puntos();
    emite_nueva_linea();
    gen_nodo(nodo_der[nodo]);
    emite_linea("dup");
    emite_etiq(temp);
    dos_puntos();
    emite_nueva_linea();
    emite_linea("rev");
    return;
  }
  if (op == N_COMA) {
    gen_nodo(nodo_izq[nodo]);
    gen_nodo(nodo_der[nodo]);
    return;
  }
  if (op == N_COPIA) {
    pila_extra = pila;
    if ((regs[nodo_der[nodo]] >= regs[nodo_izq[nodo]]) &&
        (regs[nodo_izq[nodo]] < 3)) {
      estructura(nodo_der[nodo]);
      gen_nodo(nodo_izq[nodo]);
    } else if ((regs[nodo_izq[nodo]] > regs[nodo_der[nodo]]) &&
               (regs[nodo_der[nodo]] < 3)) {
      gen_nodo(nodo_izq[nodo]);
      estructura(nodo_der[nodo]);
      emite_linea("rev");
    } else {
      gen_nodo(nodo_izq[nodo]);
      salva(0);
      estructura(nodo_der[nodo]);
      recupera(0);
    }
    if(req_res)
      emite_texto("ajw -1\r\ndup\r\nstl 0\r\n");
    ins("ldc ", esp[nodo]);
    emite_linea("move");
    if(req_res)
      emite_texto("ldl 0\r\najw 1\r\n");
    pila = desp_pila(pila_extra);
    return;
  }
  if (op == N_ENTPF && oper[nodo_izq[nodo]] == N_CPAL) {
    gen_nodo(nodo_izq[nodo_izq[nodo]]);
    emite_linea("fpi32tor64");
    return;
  }
  if (op == N_CFLOAT && oper[nodo_izq[nodo]] == N_IXP) {
    oper[nodo_izq[nodo]] = N_CFI;
    gen_nodo(nodo_izq[nodo]);
    oper[nodo_izq[nodo]] = N_IXP;
    return;
  }
  if (op == N_CDOUBLE && oper[nodo_izq[nodo]] == N_IXF) {
    oper[nodo_izq[nodo]] = N_CDI;
    gen_nodo(nodo_izq[nodo]);
    oper[nodo_izq[nodo]] = N_IXF;
    return;
  }
  if (op == N_CPAL) {
    if (oper[nodo_izq[nodo]] == N_LDLP) {
      oper[nodo_izq[nodo]] = N_LDL;
      gen_nodo(nodo_izq[nodo]);
      oper[nodo_izq[nodo]] = N_LDLP;
      return;
    } else if (oper[nodo_izq[nodo]] == N_LDNLP) {
      oper[nodo_izq[nodo]] = N_LDNL;
      gen_nodo(nodo_izq[nodo]);
      oper[nodo_izq[nodo]] = N_LDNLP;
      return;
    }
  }
  rev = NO;
  if (nodo_der[nodo]) {
    if (op >= N_IGUALPF && op <= N_DIVPF) {
      if ((regsf[nodo_izq[nodo]] >= regsf[nodo_der[nodo]]) &&
          (regsf[nodo_der[nodo]] < 3)) {
        gen_nodo(nodo_izq[nodo]);
        if((op == N_SUMAPF || op == N_MULPF)
        && (oper[nodo_der[nodo]] == N_CFLOAT
         || oper[nodo_der[nodo]] == N_CDOUBLE)) {
          gen_nodo(nodo_izq[nodo_der[nodo]]);
          if(oper[nodo_der[nodo]] == N_CFLOAT) {
            if(op == N_SUMAPF)
              emite_linea("fpldnladdsn");
            else
              emite_linea("fpldnlmulsn");
          } else {
            if(op == N_SUMAPF)
              emite_linea("fpldnladddb");
            else
              emite_linea("fpldnlmuldb");
          }
          return;
        } else
          gen_nodo(nodo_der[nodo]);
      } else if ((regsf[nodo_der[nodo]] > regsf[nodo_izq[nodo]]) &&
                 (regsf[nodo_izq[nodo]] < 3)) {
        gen_nodo(nodo_der[nodo]);
        if((op == N_SUMAPF || op == N_MULPF)
        && (oper[nodo_izq[nodo]] == N_CFLOAT
         || oper[nodo_izq[nodo]] == N_CDOUBLE)) {
          gen_nodo(nodo_izq[nodo_izq[nodo]]);
          if(oper[nodo_izq[nodo]] == N_CFLOAT) {
            if(op == N_SUMAPF)
              emite_linea("fpldnladdsn");
            else
              emite_linea("fpldnlmulsn");
          } else {
            if(op == N_SUMAPF)
              emite_linea("fpldnladddb");
            else
              emite_linea("fpldnlmuldb");
          }
          return;
        } else
          gen_nodo(nodo_izq[nodo]);
        rev = SI;
      } else {
        gen_nodo(nodo_der[nodo]);
        salva(esp[nodo] ? 2 : 1);
        gen_nodo(nodo_izq[nodo]);
        if(op == N_SUMAPF) {
          recupera(esp[nodo] ? 5 : 2);
          return;
        } else if(op == N_MULPF) {
          recupera(esp[nodo] ? 6 : 3);
          return;
        }
        recupera(esp[nodo] ? 4 : 1);
      }
    } else {
      if ((regs[nodo_izq[nodo]] >= regs[nodo_der[nodo]]) &&
          (regs[nodo_der[nodo]] < 3)) {
        gen_nodo(nodo_izq[nodo]);
        gen_nodo(nodo_der[nodo]);
      } else if ((regs[nodo_der[nodo]] > regs[nodo_izq[nodo]]) &&
                 (regs[nodo_izq[nodo]] < 3)) {
        gen_nodo(nodo_der[nodo]);
        gen_nodo(nodo_izq[nodo]);
        rev = SI;
      } else {
        gen_nodo(nodo_der[nodo]);
        salva(0);
        gen_nodo(nodo_izq[nodo]);
        recupera(0);
      }
    }
  } else
    gen_nodo(nodo_izq[nodo]);
  if (op == N_CIGUAL) {
    ins("eqc ", esp[nodo]);
    return;
  }
  if (op == N_CSUMA) {
    if(esp[nodo])
      ins("adc ", esp[nodo]);
    return;
  }
  if (op == N_LDNLP) {
    if(esp[nodo])
      ins("ldnlp ", esp[nodo]);
    return;
  }
  if (op == N_LDNL) {
    ins("ldnl ", esp[nodo]);
    return;
  }
  if (op == N_STNL) {
    ins("stnl ", esp[nodo]);
    return;
  }
  gen_oper(op, rev);
}

/*
** Genera una secuencia optima para && y ||
*/
corto_circuito(nodo, etiq_and, etiq_or)
  int nodo, etiq_and, etiq_or;
{
  int nueva, etiq, opt;
  nueva = NO;
  if(oper[nodo] == N_ANDB) {
    if(oper[nodo_izq[nodo]] == N_ANDB)
      corto_circuito(nodo_izq[nodo], etiq_and, etiq_or);
    else if(oper[nodo_izq[nodo]] == N_ORB) {
      etiq = nueva_etiq;
      nueva = SI;
      corto_circuito(nodo_izq[nodo], etiq_and, etiq);
    } else {
      gen_nodo(nodo_izq[nodo]);
    }
    salta_si_falso(etiq_and);
    if(nueva) {
      emite_etiq(etiq);
      dos_puntos();
      emite_nueva_linea();
    }
    if(oper[nodo_der[nodo]] == N_ANDB)
      corto_circuito(nodo_der[nodo], etiq_and, etiq_or);
    else if(oper[nodo_der[nodo]] == N_ORB)
      corto_circuito(nodo_der[nodo], etiq_and, etiq_or);
    else
      gen_nodo(nodo_der[nodo]);
  } else if(oper[nodo] == N_ORB) {
    opt = NO;
    if(oper[nodo_izq[nodo]] == N_ANDB) {
      etiq = nueva_etiq;
      nueva = SI;
      corto_circuito(nodo_izq[nodo], etiq, etiq_or);
    } else if(oper[nodo_izq[nodo]] == N_ORB)
      corto_circuito(nodo_izq[nodo], etiq_and, etiq_or);
    else
      if(oper[nodo_izq[nodo]] == N_NOT) {
        opt = SI;
        gen_nodo(nodo_izq[nodo_izq[nodo]]);
      } else gen_nodo(nodo_izq[nodo]);
    if(!opt)
      emite_linea("eqc 0");
    salta_si_falso(etiq_or);
    if(nueva) {
      emite_etiq(etiq);
      dos_puntos();
      emite_nueva_linea();
    }
    if(oper[nodo_der[nodo]] == N_ANDB)
      corto_circuito(nodo_der[nodo], etiq_and, etiq_or);
    else if(oper[nodo_der[nodo]] == N_ORB)
      corto_circuito(nodo_der[nodo], etiq_and, etiq_or);
    else
      gen_nodo(nodo_der[nodo]);
  }
}

accesa_nodo(tipo, nodo, lectura)
  int tipo, nodo, lectura;
{
  if ((tipo == INT || tipo == UINT) && (oper[nodo] == N_LDLP)) {
    oper[nodo] = lectura ? N_LDL : N_STL;
    gen_nodo(nodo);
    oper[nodo] = N_LDLP;
  } else if ((tipo == INT || tipo == UINT) && (oper[nodo] == N_LDNLP)) {
    oper[nodo] = lectura ? N_LDNL : N_STNL;
    gen_nodo(nodo);
    oper[nodo] = N_LDNLP;
  } else {
    gen_nodo(nodo);
    if(lectura)
      carga(tipo);
    else
      almacena(tipo);
  }
}

/*
** Genera el codigo correcto para copias de estructuras.
*/
estructura(nodo)
  int nodo;
{
  if(oper[nodo] == N_FUNC || oper[nodo] == N_FUNCI)
    nodo_der[nodo] = 1;
  gen_nodo(nodo);
}

carga(tipo)
  int tipo;
{
  if(tipo == INT || tipo == UINT)
    emite_linea("ldnl 0");
  else if(tipo == SHORT)
    emite_linea("call LIB_CSHORT");
  else if(tipo == USHORT)
    emite_linea("call LIB_CUSHORT");
  else if(tipo == DOUBLE)
    emite_linea("fpldnldb");
  else if(tipo == FLOAT)
    emite_linea("fpldnlsn");
  else
    emite_linea("lb");
}

almacena(tipo)
  int tipo;
{
  if(tipo == INT || tipo == UINT)
    emite_linea("stnl 0");
  else if(tipo == SHORT || tipo == USHORT)
    emite_linea("call LIB_GSHORT");
  else if(tipo == DOUBLE)
    emite_linea("fpstnldb");
  else if(tipo == FLOAT)
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
  int izq;

  izq = ultimo_nodo;
  crea_nodo(N_LDL, 0, 0, 2);
  crea_nodo(N_COPIA, ultimo_nodo, izq, tam);
}

asigna(nodo, posicion, tipo)
  int nodo, posicion;
  char *tipo;
{
  es_control = NO;
  gen_codigo(nodo);
  if(*tipo == FLOAT) {
    ins("ldlp ", posicion - pila);
    emite_linea("fpstnlsn");
  } else if(*tipo == DOUBLE) {
    ins("ldlp ", posicion - pila);
    emite_linea("fpstnldb");
  } else
    ins("stl ", posicion - pila);
}

/*
** Genera una instrucción con operando.
*/
ins(codigo, valor)
  char *codigo;
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
    emite_texto("INICIO2:\r\nldc 0\r\nldl 0\r\nstnl 0\r\nldl 0\r\n");
    emite_texto("adc 4\r\nstl 0\r\nldl 1\r\nadc -1\r\nstl 1\r\n");
    emite_texto("ldl 1\r\neqc 0\r\ncj INICIO2\r\n");
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
        if(byte != TAM_DOUBLE - 1)
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
  emite_texto("LIB_CSHORT:\r\nldl 1\r\nlb\r\nldl 1\r\nadc 1\r\n");
  emite_texto("lb\r\nldc 8\r\nshl\r\nor\r\nldc 0x8000\r\nxword\r\n");
  emite_texto("stl 1\r\nldl 3\r\nldl 2\r\nldl 1\r\nret\r\n");
  emite_texto("LIB_CUSHORT:\r\nldl 1\r\nlb\r\nldl 1\r\nadc 1\r\n");
  emite_texto("lb\r\nldc 8\r\nshl\r\nor\r\nstl 1\r\nldl 3\r\n");
  emite_texto("ldl 2\r\nldl 1\r\nret\r\n");
  emite_texto("LIB_GSHORT:\r\nldl 2\r\nldl 1\r\nsb\r\nldl 2\r\n");
  emite_texto("ldc 8\r\nshr\r\nldl 1\r\nadc 1\r\nsb\r\nldl 3\r\nret\r\n");
}

/*
** Emite un nombre que no entre en conflicto con las
** palabras reservadas del ensamblador.
*/
emite_nombre(nombre)
  char *nombre;
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
  if(flotante == 1) {
    emite_texto("ajw -2\r\nldlp 0\r\nfpstnldb\r\n");
    pila -= 2;
  } else if(flotante == 2) {
    emite_texto("ajw -1\r\nldlp 0\r\nfpstnlsn\r\n");
    --pila;
  } else {
    emite_texto("ajw -1\r\nstl 0\r\n");
    --pila;
  }
}

/*
** Recupera el registro A desde la pila.
*/
recupera(flotante)
  int flotante;
{
  if(flotante) {
    emite_linea("ldlp 0");
    if(flotante == 1)
      emite_linea("fpldnldb");
    else if(flotante == 2)
      emite_linea("fpldnladddb");
    else if(flotante == 3)
      emite_linea("fpldnlmuldb");
    else if(flotante == 4)
      emite_linea("fpldnlsn");
    else if(flotante == 5)
      emite_linea("fpldnladdsn");
    else if(flotante == 6)
      emite_linea("fpldnlmulsn");
    if(flotante < 4) {
      emite_linea("ajw 2");
      pila += 2;
    } else {
      emite_linea("ajw 1");
      ++pila;
    }
  } else {
    emite_texto("ldl 0\r\najw 1\r\n");
    ++pila;
  }
}

/*
** Copia un registro entero o de punto flotante.
*/
copia_reg(flotante)
  int flotante;
{
  if(flotante)
    emite_linea("fpdup");
  else
    emite_linea("dup");
}

/*
** Llama a la función especificada.
*/
llamada(nombre)
  char *nombre;
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

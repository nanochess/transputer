/*
** Compilador de C para transputer.
** Evaluador de Expresiones.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 4 de junio de 1995.
** Revisión: 20 de julio de 1995. Ahora efectua correctamente suma y resta
**                                con apuntadores.
** Revisión: 25 de julio de 1995. Soporte para || y &&.
** Revisión: 25 de julio de 1995. Soporte para el operador trinario ?.
** Revisión: 26 de julio de 1995. Ahora da correctamente el resultado de una
**                                resta de dos apuntadores.
** Revisión: 26 de julio de 1995. Soporte para |= ^= &= += -= /= %= <<= >>=.
** Revisión: 26 de julio de 1995. Nueva función. expr_constante().
** Revisión: 27 de julio de 1995. Soporte para el operador coma.
** Revisión: 29 de julio de 1995. Corrección de los operadores ++ y -- para
**                                variables de tipo char.
** Revisión: 31 de julio de 1995. Corrección de un defecto en el posdecremento.
** Revisión: 10 de agosto de 1995. Soporte para tipos complejos.
** Revisión: 11 de agosto de 1995. Desaparece nivel14(), se combina con
**                                 primaria().
** Revisión: 11 de agosto de 1995. Soporte para short y unsigned short.
** Revisión: 11 de agosto de 1995. Soporte para sizeof.
** Revisión: 11 de agosto de 1995. Soporte para conversiones de tipo.
** Revisión: 12 de agosto de 1995. Corrección de defectos en sizeof y
**                                 conversiones de tipo.
** Revisión: 23 de agosto de 1995. Soporte para los operadores -> y .
** Revisión: 24 de agosto de 1995. Soporte para enumeradores.
** Revisión: 5 de septiembre de 1995. Soporte para asignación, paso cómo
**                                    parametros y resultados de estructuras.
** Revisión: 6 de septiembre de 1995. Nueva función. nivel13dir().
** Revisión: 12 de septiembre de 1995. Soporte para float y double.
** Revisión: 12 de septiembre de 1995. Nuevas funciones. convierte_tipo().
**                                     checa_entero(), checa_numerico(),
**                                     compara_no_cero(), compara_cero(),
**                                     checa_entero_o_apuntador(),
**                                     haz_compatible().
** Revisión: 22 de septiembre de 1995. Corrección de varios errores en
**                                     convierte_tipo y en nivel1().
** Revisión: 27 de septiembre de 1995. Corrección de varios errores.
** Revisión: 22 de noviembre de 1995. Soporte para números de punto flotante.
** Revisión: 26 de noviembre de 1995. Corrección de un defecto en el manejo
**                                    del exponente en números de punto
**                                    flotante.
** Revisión: 26 de noviembre de 1995. Corrección de un defecto en el manejo
**                                    de estructuras.
** Revisión: 30 de noviembre de 1995. Generación de nodos para optimación
**                                    de operaciones con float.
** Revisión: 1o. de diciembre de 1995. Corrección de varios defectos.
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en el manejo
**                                    de unsigned int, no efectuaba bien las
**                                    comparaciones.
** Revisión: 28 de diciembre de 1995. Ahora soporta la letra L en los números.
** Revisión: 28 de diciembre de 1995. Ahora balancea correctamente unsigned.
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en sizeof.
** Revisión: 3 de enero de 1996. Ahora evalua en tiempo de compilación las
**                               expresiones constantes de | & y ^. (¿cómo se
**                               me fue a olvidar?).
** Revisión: 18 de febrero de 1996. Soporte para el operador coma en sentencias
**                                  condicionales. (¿cómo se me fue a olvidar?)
** Revisión: 19 de febrero de 1996. Corrección de defectos en la sintaxis de
**                                  las expresiones.
** Revisión: 9 de marzo de 1996. Genera árboles más óptimos para comparaciones
**                               entre apuntadores.
** Revisión: 8 de abril de 1996. Corrección de un defecto en el procesamiento
**                               de números reales.
** Revisión: 10 de abril de 1996. Corrección de un defecto en la generación de
**                                codigo para el operador trinario.
** Revisión: 15 de abril de 1996. Corrección de un defecto que no permitia el
**                                acceso a estructuras en matrices.
** Revisión: 11 de marzo de 2025. Parche para temporales float.
*/

/*
** La matriz info contiene información sobre la expresión.
**
** info[0] - Tipo de la expresión hasta el momento.
*/

/*
** Evalua el último arbol generado.
*/
evalua_arbol(control)
  int control;
{
  es_control = control;
  gen_codigo(ultimo_nodo);
}

/*
** Analiza una expresión, y genera el codigo.
*/
expresion()
{
  int origen;
  char *tipo;

  origen = ultimo_nodo;
  tipo = almacena_expresion(SI);
  evalua_arbol(NO);
  ultimo_nodo = origen;
  return tipo;
}

/*
** Procesa una expresión constante.
*/
expr_constante()
{
  int origen, valor;
  char *tipo;

  origen = ultimo_nodo;
  checa_entero(almacena_expresion(SI));
  if (oper[ultimo_nodo] != N_CONST) {
    ultimo_nodo = origen;
    error("No es una expresión constante");
    return 1;
  } else {
    valor = esp[ultimo_nodo];
    ultimo_nodo = origen;
    return valor;
  }
}

/*
** Analiza una expresión y la mantiene en memoria.
*/
almacena_expresion(operador_coma)
  int operador_coma;
{
  int info[1], izq;

  if (operador_coma) {
    if (nivel0(info))
      carga_valor(info);
  } else {
    if (nivel1(info))
      carga_valor(info);
  }
  return info[0];
}

nivel0(info)
  int info[];
{
  int k, izq;

  k = nivel1(info);
  while (match(",")) {
    if (k)
      carga_valor(info);
    izq = ultimo_nodo;
    k = nivel1(info);
    crea_nodo(N_COMA, izq, ultimo_nodo, 0);
  }
  return k;
}

nivel1(info)
  int info[];
{
  int k, info2[1];
  int der, izq, op;
  char *tipo, *tipo2;

  k = nivel2(info);
  if (match("="))
    op = N_ASIGNA;
  else if (match("|="))
    op = N_AOR;
  else if (match("^="))
    op = N_AXOR;
  else if (match("&="))
    op = N_AAND;
  else if (match("<<="))
    op = N_ACI;
  else if (match(">>="))
    op = N_ACD;
  else if (match("+="))
    op = N_ASUMA;
  else if (match("-="))
    op = N_ARESTA;
  else if (match("*="))
    op = N_AMUL;
  else if (match("/="))
    op = N_ADIV;
  else if (match("%="))
    op = N_AMOD;
  else return k;
  der = ultimo_nodo;
  tipo = info[0];
  if (k == 0)
    if(*tipo != STRUCT)
      req_valorl();
  if (nivel1(info2)) {
    if(*tipo == STRUCT)
      req_valorl();
    carga_valor(info2);
    izq = ultimo_nodo;
  } else if(*tipo == STRUCT) {
    tipo2 = info2[0];
    if(*tipo2 != STRUCT)
      error("Se requiere una estructura o unión");
    else {
      if(lee_entero(tipo + 1) != lee_entero(tipo2 + 1))
        error("Estructuras incompatibles");
    }
    if(op != N_ASIGNA)
      error("Asignación incompatible");
    crea_nodo(N_COPIA, der, ultimo_nodo, tam_tipo(tipo));
    return 0;
  } else
    izq = ultimo_nodo;
  tipo2 = info2[0];
  convierte_tipo(&izq, tipo2, tipo);
  if((*tipo == DOUBLE || *tipo == FLOAT)
  && (op == N_AOR || op == N_AXOR
   || op == N_AAND || op == N_ACI
   || op == N_ACD || op == N_AMOD))
    error("No se puede hacer esta operación con reales");
  if((op == N_ASUMA) || (op == N_ARESTA)) {
    if(k = dobla(tipo, izq)) {
      if(k == 2) {
        if(multi == 4) {
          if(op == N_ARESTA) {
            crea_nodo(N_NEG, izq, 0, 0);
            izq = ultimo_nodo;
          }
          op = N_AIXP;
        } else if(multi != 1) {
          crea_nodo(N_CONST, 0, 0, multi);
          crea_nodo(N_MUL, izq, ultimo_nodo, 0);
          izq = ultimo_nodo;
        }
      }
    }
  }
  if((op == N_ARESTA) && (oper[izq] == N_CONST)) {
    op = N_ASUMA;
    esp[izq] = -esp[izq];
  }
  if(*tipo == APUNTADOR)
    tipo = t_int;
  crea_nodo(op, izq, der, *tipo);
  return 0;
}

nivel2(info)
  int info[];
{
  int k, info2[1], info3[1];
  int ext, izq, der;

  k = nivel3(info);
  espacios();
  if (car_act != '?')
    return k;
  if (k)
    carga_valor(info);
  ext = ultimo_nodo;
  pos_linea++;
  if (nivel0(info2))
    carga_valor(info2);
  izq = ultimo_nodo;
  pide(":");
  if (nivel2(info3))
    carga_valor(info3);
  der = ultimo_nodo;
  if((oper[ext] == N_CONST) &&
     (oper[izq] == N_CONST) &&
     (oper[der] == N_CONST)) {
    if(esp[ext]) esp[ultimo_nodo] = esp[izq];
  } else {
    haz_compatible(&izq, info2, &der, info3);
    crea_nodo(N_TRI, izq, der, ext);
  }
  info[0] = info2[0];
  return 0;
}

nivel3(info)
  int info[];
{
  int k, info2[1];
  int izq;

  k = nivel4(info);
  espacios();
  if (streq(linea + pos_linea, "||") == 0)
    return k;
  if (k)
    carga_valor(info);
  while (match("||")) {
    checa_numerico(info[0]);
    compara_no_cero(info[0]);
    izq = ultimo_nodo;
    if (nivel4(info2))
      carga_valor(info2);
    checa_numerico(info2[0]);
    compara_no_cero(info2[0]);
    crea_nodo(N_ORB, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel4(info)
  int info[];
{
  int k, info2[1];
  int izq;

  k = nivel5(info);
  espacios();
  if (streq(linea + pos_linea, "&&") == 0)
    return k;
  if (k)
    carga_valor(info);
  while (match("&&")) {
    checa_numerico(info[0]);
    compara_no_cero(info[0]);
    izq = ultimo_nodo;
    if (nivel5(info2))
      carga_valor(info2);
    checa_numerico(info2[0]);
    compara_no_cero(info2[0]);
    crea_nodo(N_ANDB, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel5(info)
  int info[];
{
  int k, info2[1];
  int izq;

  k = nivel6(info);
  if (encuentra("|") == 0) return k;
  if (k)
    carga_valor(info);
  while (encuentra("|")) {
    pos_linea++;
    izq = ultimo_nodo;
    checa_entero(info[0]);
    if (nivel6(info2))
      carga_valor(info2);
    checa_entero(info2[0]);
    if(oper[izq] == N_CONST && oper[ultimo_nodo] == N_CONST)
      esp[ultimo_nodo] |= esp[izq];
    else
      crea_nodo(N_OR, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel6(info)
  int info[];
{
  int k, info2[1];
  int izq;

  k = nivel7(info);
  if (encuentra("^") == 0) return k;
  if (k)
    carga_valor(info);
  while (encuentra("^")) {
    pos_linea++;
    izq = ultimo_nodo;
    checa_entero(info[0]);
    if (nivel7(info2))
      carga_valor(info2);
    checa_entero(info2[0]);
    if(oper[izq] == N_CONST && oper[ultimo_nodo] == N_CONST)
      esp[ultimo_nodo] ^= esp[izq];
    else
      crea_nodo(N_XOR, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel7(info)
  int info[];
{
  int k, info2[1];
  int izq;

  k = nivel8(info);
  if (encuentra("&") == 0) return k;
  if (k)
    carga_valor(info);
  while (encuentra("&")) {
    pos_linea++;
    izq = ultimo_nodo;
    checa_entero(info[0]);
    if (nivel8(info2))
      carga_valor(info2);
    checa_entero(info2[0]);
    if(oper[izq] == N_CONST && oper[ultimo_nodo] == N_CONST)
      esp[ultimo_nodo] &= esp[izq];
    else
      crea_nodo(N_AND, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel8(info) int info[]; {
  int k, info2[1];
  int izq, der;

  k = nivel9(info);
  if ((encuentra("==") == 0) &&
      (encuentra("!=") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra("==")) {
      pos_linea += 2;
      if (nivel9(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if(haz_compatible(&izq, info, &der, info2))
        crea_nodo(N_IGUALPF, izq, der, *((char *) info[0]) == FLOAT);
      else if(oper[der] == N_CONST) {
        if(oper[izq] == N_CONST)
          crea_nodo(N_CONST, 0, 0, esp[izq] == esp[der]);
        else
          crea_nodo(N_CIGUAL, izq, 0, esp[der]);
      } else if(oper[izq] == N_CONST)
        crea_nodo(N_CIGUAL, der, 0, esp[izq]);
      else crea_nodo(N_IGUAL, izq, der, 0);
    } else if (encuentra("!=")) {
      pos_linea += 2;
      if (nivel9(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if(haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_IGUALPF, izq, der, *((char *) info[0]) == FLOAT);
        crea_nodo(N_NOT, ultimo_nodo, 0, 0);
      } else if(oper[der] == N_CONST) {
        if(oper[izq] == N_CONST)
          crea_nodo(N_CONST, 0, 0, esp[izq] != esp[der]);
        else {
          crea_nodo(N_CIGUAL, izq, 0, esp[der]);
          crea_nodo(N_NOT, ultimo_nodo, 0, 0);
        }
      } else if(oper[izq] == N_CONST) {
        crea_nodo(N_CIGUAL, der, 0, esp[izq]);
        crea_nodo(N_NOT, ultimo_nodo, 0, 0);
      } else {
        crea_nodo(N_IGUAL, izq, der, 0);
        crea_nodo(N_NOT, ultimo_nodo, 0, 0);
      }
    } else
      return 0;
    info[0] = t_int;
  }
}

nivel9(info)
  int info[];
{
  int k;

  k = nivel10(info);
  if ((encuentra("<") == 0) &&
      (encuentra(">") == 0) &&
      (encuentra("<=") == 0) &&
      (encuentra(">=") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    if (encuentra("<=")) {
      pos_linea = pos_linea + 2;
      nivel9eval(1, info);
    } else if (encuentra(">=")) {
      pos_linea = pos_linea + 2;
      nivel9eval(2, info);
    } else if (encuentra("<")) {
      pos_linea++;
      nivel9eval(3, info);
    } else if (encuentra(">")) {
      pos_linea++;
      nivel9eval(4, info);
    } else
      return 0;
  }
}

nivel9eval(k, info)
  int k, info[];
{
  int info2[1];
  int izq, der;
  char *tipo;

  izq = ultimo_nodo;
  if (nivel10(info2))
    carga_valor(info2);
  der = ultimo_nodo;
  if (haz_compatible(&izq, info, &der, info2)) {
    if(k == 4 || k == 1)
      crea_nodo(N_MAYORPF, izq, der, *((char *) info[0]) == FLOAT);
    else
      crea_nodo(N_MAYORPF, der, izq, *((char *) info[0]) == FLOAT);
    if(k == 1 || k == 2)
      crea_nodo(N_NOT, ultimo_nodo, 0, 0);
    info[0] = t_int;
    return;
  }
  tipo = info[0];
  if (*tipo == UINT) {
    nivel9op(izq, k);
    info[0] = t_int;
    return;
  }
  tipo = info2[0];
  if (*tipo == UINT) {
    nivel9op(izq, k);
    info[0] = t_int;
    return;
  }
  if(k == 4) {
    if((oper[izq] == N_CONST) && (oper[der] == N_CONST))
      crea_nodo(N_CONST, 0, 0, esp[izq] > esp[der]);
    else
      crea_nodo(N_MAYOR, izq, der, 0);
  }
  else if(k == 3) {
    if((oper[izq] == N_CONST) && (oper[der] == N_CONST))
      crea_nodo(N_CONST, 0, 0, esp[izq] < esp[der]);
    else
      crea_nodo(N_MAYOR, der, izq, 0);
  }
  else if(k == 1) {
    if((oper[izq] == N_CONST) && (oper[der] == N_CONST))
      crea_nodo(N_CONST, 0, 0, esp[izq] <= esp[der]);
    else {
      crea_nodo(N_MAYOR, izq, der, 0);
      crea_nodo(N_NOT, ultimo_nodo, 0);
    }
  } else {
    if((oper[izq] == N_CONST) && (oper[der] == N_CONST))
      crea_nodo(N_CONST, 0, 0, esp[izq] >= esp[der]);
    else {
      crea_nodo(N_MAYOR, der, izq, 0);
      crea_nodo(N_NOT, ultimo_nodo, 0);
    }
  }
  info[0] = t_int;
}

nivel9op(izq, k)
  int izq, k;
{
  if(k == 4) crea_nodo(N_SMAYOR, izq, ultimo_nodo, 0);
  else if(k == 3) crea_nodo(N_SMAYOR, ultimo_nodo, izq, 0);
  else if(k == 1) {
    crea_nodo(N_SMAYOR, izq, ultimo_nodo, 0);
    crea_nodo(N_NOT, ultimo_nodo, 0);
  } else {
    crea_nodo(N_SMAYOR, ultimo_nodo, izq, 0);
    crea_nodo(N_NOT, ultimo_nodo, 0);
  }
}

nivel10(info)
  int info[];
{
  int k, info2[1];
  int izq;

  k = nivel11(info);
  if ((encuentra(">>") == 0) &&
      (encuentra("<<") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra(">>")) {
      pos_linea += 2;
      checa_entero(info[0]);
      if (nivel11(info2))
        carga_valor(info2);
      checa_entero(info2[0]);
      if((oper[izq] == N_CONST) && (oper[ultimo_nodo] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] >> esp[ultimo_nodo]);
      else
        crea_nodo(N_CD, izq, ultimo_nodo, 0);
    } else if (encuentra("<<")) {
      pos_linea += 2;
      checa_entero(info[0]);
      if (nivel11(info2))
        carga_valor(info2);
      checa_entero(info2[0]);
      if((oper[izq] == N_CONST) && (oper[ultimo_nodo] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] << esp[ultimo_nodo]);
      else
        crea_nodo(N_CI, izq, ultimo_nodo, 0);
    } else
      return 0;
  }
}

nivel11(info)
  int info[];
{
  int k, info2[1];
  int izq, der, tam;
  char *tipo;

  k = nivel12(info);
  if ((encuentra("+") == 0) &&
      (encuentra("-") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra("+")) {
      pos_linea++;
      if (nivel12(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if(haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_SUMAPF, izq, der, *((char *) info[0]) == FLOAT);
      } else {
        if(k = dobla(info[0], der)) {
          if(k == 2 && multi == 4) {
            crea_nodo(N_IXP, der, izq, 0);
            continue;
          } else if(k == 2 && multi != 1) {
            crea_nodo(N_CONST, 0, 0, multi);
            crea_nodo(N_MUL, der, ultimo_nodo, 0);
            der = ultimo_nodo;
          }
        } else {
          info[0] = info2[0];
          if(k = dobla(info[0], izq)) {
            if(k == 2 && multi == 4) {
              crea_nodo(N_IXP, izq, der, 0);
              continue;
            } else if(k == 2 && multi != 1) {
              crea_nodo(N_CONST, 0, 0, multi);
              crea_nodo(N_MUL, izq, ultimo_nodo, 0);
              izq = ultimo_nodo;
            }
          }
        }
        if((oper[izq] == N_CONST) && (oper[der] == N_CONST))
          crea_nodo(N_CONST, 0, 0, esp[izq] + esp[der]);
        else if(oper[der] == N_CONST)
          crea_nodo(N_CSUMA, izq, 0, esp[der]);
        else if(oper[izq] == N_CONST)
          crea_nodo(N_CSUMA, der, 0, esp[izq]);
        else
          crea_nodo(N_SUMA, izq, der, 0);
      }
    } else if (encuentra("-")) {
      pos_linea++;
      if (nivel12(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if(haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_RESTAPF, izq, der, *((char *) info[0]) == FLOAT);
      } else {
        tipo = info[0];
        if(*tipo == APUNTADOR || *tipo == MATRIZ) {
          tipo = info2[0];
          if(*tipo == APUNTADOR || *tipo == MATRIZ) {
            crea_nodo(N_RESTA, izq, der, 0);
            if(*tipo == APUNTADOR)
              tam = tam_tipo(tipo + 1);
            else
              tam = tam_tipo(tipo + 5);
            if(tam == 4)
              crea_nodo(N_CUENTA, ultimo_nodo, 0, 0);
            else if(tam != 1) {
              izq = ultimo_nodo;
              crea_nodo(N_CONST, 0, 0, tam);
              crea_nodo(N_DIV, izq, ultimo_nodo, 0);
            }
            info[0] = t_int;
            continue;
          }
        }
        if(k = dobla(info[0], der)) {
          if(k == 2 && multi == 4) {
            crea_nodo(N_NEG, der, 0, 0);
            crea_nodo(N_IXP, ultimo_nodo, izq, 0);
            continue;
          } else if(k == 2 && multi != 1) {
            crea_nodo(N_CONST, 0, 0, multi);
            crea_nodo(N_MUL, der, ultimo_nodo, 0);
            der = ultimo_nodo;
          }
        }
        if((oper[izq] == N_CONST) && (oper[der] == N_CONST))
          crea_nodo(N_CONST, 0, 0, esp[izq] - esp[der]);
        else if(oper[der] == N_CONST)
          crea_nodo(N_CSUMA, izq, 0, -esp[der]);
        else
          crea_nodo(N_RESTA, izq, der, 0);
      }
    } else
      return 0;
  }
}

nivel12(info)
  int info[];
{
  int k, info2[1];
  int izq, der;

  k = nivel13(info);
  if ((encuentra("*") == 0) &&
      (encuentra("/") == 0) &&
      (encuentra("%") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra("*")) {
      pos_linea++;
      if (nivel13(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if(haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_MULPF, izq, der, *((char *) info[0]) == FLOAT);
      } else {
        checa_entero(info[0]);
        checa_entero(info2[0]);
        if((oper[izq] == N_CONST) && (oper[der] == N_CONST))
          crea_nodo(N_CONST, 0, 0, esp[izq] * esp[der]);
        else
          crea_nodo(N_MUL, izq, der, 0);
      }
    } else if (encuentra("/")) {
      pos_linea++;
      if (nivel13(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if(haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_DIVPF, izq, der, *((char *) info[0]) == FLOAT);
      } else {
        checa_entero(info[0]);
        checa_entero(info2[0]);
        if((oper[izq] == N_CONST) && (oper[der] == N_CONST))
          crea_nodo(N_CONST, 0, 0, esp[izq] / esp[der]);
        else
          crea_nodo(N_DIV, izq, der, 0);
      }
    } else if (encuentra("%")) {
      pos_linea++;
      checa_entero(info[0]);
      if (nivel13(info2))
        carga_valor(info2);
      checa_entero(info2[0]);
      der = ultimo_nodo;
      if((oper[izq] == N_CONST) && (oper[der] == N_CONST))
        crea_nodo(N_CONST, 0, 0, esp[izq] % esp[der]);
      else
        crea_nodo(N_MOD, izq, der, 0);
    } else
      return 0;
  }
}

nivel13(info)
  int info[];
{
  int k, p;
  char *tipo, *tipo2;

  if (match("++")) {
    if (nivel13(info) == 0)
      req_valorl();
    checa_entero_o_apuntador(info[0]);
    nivel13inc(info);
    return 0;
  } else if (match("--")) {
    if (nivel13(info) == 0)
      req_valorl();
    checa_entero_o_apuntador(info[0]);
    nivel13dec(info);
    return 0;
  } else if (match("-")) {
    if(nivel13(info))
      carga_valor(info);
    checa_numerico(info[0]);
    tipo = info[0];
    if(oper[ultimo_nodo] == N_CONST)
      esp[ultimo_nodo] = -esp[ultimo_nodo];
    else if(*tipo == DOUBLE || *tipo == FLOAT) {
      k = ultimo_nodo;
      if (*tipo == DOUBLE)
        crea_nodo(N_CEROPF, 0, 0, 0);
      else
        crea_nodo(N_CEROF, 0, 0, 0);
      crea_nodo(N_RESTAPF, ultimo_nodo, k, 0);
    } else
      crea_nodo(N_NEG, ultimo_nodo, 0, 0);
    return 0;
  } else if (match("~")) {
    if(nivel13(info))
      carga_valor(info);
    checa_entero(info[0]);
    if(oper[ultimo_nodo] == N_CONST)
      esp[ultimo_nodo] = ~esp[ultimo_nodo];
    else
      crea_nodo(N_COM, ultimo_nodo, 0, 0);
    return 0;
  } else if (match("!")) {
    if(nivel13(info))
      carga_valor(info);
    checa_numerico(info[0]);
    tipo = info[0];
    if(oper[ultimo_nodo] == N_CONST)
      esp[ultimo_nodo] = !esp[ultimo_nodo];
    else if(*tipo == DOUBLE || *tipo == FLOAT)
      compara_cero(tipo);
    else
      crea_nodo(N_NOT, ultimo_nodo, 0, 0);
    info[0] = t_int;
    return 0;
  } else if (match("*")) {
    return nivel13ap(info);
  } else if (match("&")) {
    return nivel13dir(info);
  } else if (amatch("sizeof", 6)) {
    if(match("(")) {
      if(p_tipo_1(NO)) {
        p_tipo_2(NULL);
        pide(")");
        crea_nodo(N_CONST, 0, 0, tam_tipo(tipo_proc));
        info[0] = t_int;
        return 0;
      } else
        k = primaria(info, SI);
    } else
      k = nivel13(info);
    crea_nodo(N_CONST, 0, 0, tam_tipo(info[0]));
    info[0] = t_int;
    return 0;
  } else if (match("(")) {
    if(p_tipo_1(NO)) {
      p_tipo_2(NULL);
      pide(")");
      tipo = tipo_proc;
      if(nivel13(info))
        carga_valor(info);
      tipo2 = info[0];
      info[0] = tipo;
      k = ultimo_nodo;
      convierte_tipo(&k, tipo2, tipo);
      return 0;
    } else k = primaria(info, SI);
  } else k = primaria(info, NO);
  if (match("++")) {
    if (k == 0)
      req_valorl();
    checa_entero_o_apuntador(info[0]);
    nivel13pinc(info);
    return 0;
  }
  if (match("--")) {
    if (k == 0)
      req_valorl();
    checa_entero_o_apuntador(info[0]);
    nivel13pdec(info);
    return 0;
  }
  return k;
}

nivel13ap(info)
  int info[];
{
  int k;
  char *ap;

  k = nivel13(info);
  if (k)
    carga_valor(info);
  ap = info[0];
  if(*ap == APUNTADOR) ap++;
  else if(*ap == MATRIZ) ap += 5;
  else error("No es un apuntador o matriz");
  info[0] = ap;
  if(*ap == FUNCION || *ap == MATRIZ || *ap == STRUCT)
    return 0;
  else
    return 1;
}

nivel13dir(info)
  int info[];
{
  char *nuevo_tipo, *tipo;

  if(nivel13(info) == 0) {
    tipo = info[0];
    if(*tipo != STRUCT)
      error("Dirección ilegal");
  }
  nuevo_tipo = sig_tipo;
  guarda_tipo(APUNTADOR);
  copia_tipo(info[0]);
  info[0] = nuevo_tipo;
  return 0;
}

nivel13inc(info)
  int info[];
{
  char *tipo;
  int inc;

  tipo = info[0];
  if(*tipo == APUNTADOR) {
    inc = tam_tipo(tipo + 1);
    tipo = t_int;
  } else
    inc = 1;
  crea_nodo(N_INC, ultimo_nodo, inc, *tipo);
}

nivel13dec(info)
  int info[];
{
  nivel13inc(info);
  nodo_der[ultimo_nodo] = -nodo_der[ultimo_nodo];
}

nivel13pinc(info)
  int info[];
{
  nivel13inc(info);
  oper[ultimo_nodo] = N_PINC;
}

nivel13pdec(info)
  int info[];
{
  nivel13dec(info);
  oper[ultimo_nodo] = N_PINC;
}

primaria(info, sin_parentesis)
  int info[], sin_parentesis;
{
  char *ap, nombre[TAM_NOMBRE];
  int k, etiq, izq, der, tam, punto;
  char *tipo;
  int info2[1];

  if (sin_parentesis || match("(")) {
    k = nivel0(info);
    pide(")");
  } else if (nombre_legal(nombre)) {
    if ((ap = busca_loc(nombre))
     || (ap = busca_glb(nombre))) {
      if (ap[IDENT] == TYPEDEF
       || ap[IDENT] == ETIQUETA)
        error("No es una variable o función");
      if (ap[IDENT] != FUNCION) {
        if (ap[CLASE] == AUTO)
          dir_var_loc(ap);
        else
          dir_var_glb(ap);
      } else
        dir_func(ap);
      info[0] = tipo = lee_entero(ap + TIPO);
      if (*tipo == MATRIZ || *tipo == FUNCION || *tipo == STRUCT)
        k = 0;
      else
        k = 1;
    } else if ((ap = busca_enum(nombre)) != NULL) {
      crea_nodo(N_CONST, 0, 0, lee_entero(ap + ENUM_VALOR));
      info[0] = t_int;
      k = 0;
    } else {
      dir_func(nueva_glb(nombre, FUNCION, STATIC, t_func, FUNC_REF));
      info[0] = t_func;
      k = 0;
    }
  } else if (constante(info)) {
    k = 0;
  } else {
    error("Expresión inválida");
    crea_nodo(N_CONST, 0, 0, 0);
    basura();
    info[0] = t_int;
    k = 0;
  }
  tipo = info[0];
  while (1) {
    if (match("[")) {
      if (*tipo != APUNTADOR && *tipo != MATRIZ) {
        error("No se puede usar subscripto");
        basura();
        pide("]");
        return 0;
      }
      if (k)
        carga_valor(info);
      izq = ultimo_nodo;
      if (nivel1(info2))
        carga_valor(info2);
      pide("]");
      if (*tipo == APUNTADOR)
        tam = tam_tipo(++tipo);
      else
        tam = tam_tipo(tipo += 5);
      if (tam == 8) {
        if (oper[ultimo_nodo] == N_CONST)
          crea_nodo(N_LDNLP, izq, 0, esp[ultimo_nodo] * 2);
        else
          crea_nodo(N_IXF, ultimo_nodo, izq, 0);
      } else if (tam == 4) {
        if (oper[ultimo_nodo] == N_CONST)
          crea_nodo(N_LDNLP, izq, 0, esp[ultimo_nodo]);
        else
          crea_nodo(N_IXP, ultimo_nodo, izq, 0);
      } else if (tam == 1) {
        if (oper[ultimo_nodo] == N_CONST)
          crea_nodo(N_CSUMA, izq, 0, esp[ultimo_nodo]);
        else
          crea_nodo(N_SUMA, ultimo_nodo, izq, 0);
      } else {
        if (oper[ultimo_nodo] == N_CONST)
          crea_nodo(N_CSUMA, izq, 0, esp[ultimo_nodo] * tam);
        else {
          der = ultimo_nodo;
          crea_nodo(N_CONST, 0, 0, tam);
          crea_nodo(N_MUL, der, ultimo_nodo, 0);
          crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
        }
      }
      info[0] = tipo;
      if(*tipo == MATRIZ || *tipo == STRUCT)
        k = 0;
      else
        k = 1;
    } else if (match("(")) {
      if (k)
        carga_valor(info);
      if (*tipo != FUNCION)
        error("El tipo no es de función");
      else
        ++tipo;
      if (oper[ultimo_nodo] == N_APFUNC)
        llama_funcion(esp[ultimo_nodo], tipo);
      else
        llama_funcion(0, tipo);
      info[0] = tipo;
      k = 0;
    } else {
      if(match(".")) punto = 1;
      else if(match("->")) punto = 2;
      else punto = 0;
      if(punto) {
        if(nombre_legal(nombre) == 0)
          error("Nombre ilegal para el miembro");
        if(punto == 2) {
          if (k)
            carga_valor(info);
          if(*tipo == APUNTADOR) tipo++;
          else if(*tipo == MATRIZ) tipo += 5;
          else error("No es un apuntador o matriz");
          info[0] = tipo;
        }
        if(*tipo != STRUCT) {
          error("No es una estructura o unión");
          continue;
        }
        ap = lee_entero(tipo + 1);
        if(lee_entero(ap + EST_TAM) == 0) {
          error("Estructura o unión incompleta");
          continue;
        }
        ap = lee_entero(ap + EST_LISTA);
        while(ap != NULL) {
          if(astreq(nombre, ap + MIE_NOMBRE, MAX_NOMBRE)) {
            info[0] = tipo = lee_entero(ap + MIE_TIPO);
            if(*tipo == FUNCION || *tipo == MATRIZ || *tipo == STRUCT)
              k = 0;
            else
              k = 1;
            crea_nodo(N_CSUMA, ultimo_nodo, 0, lee_entero(ap + MIE_POSICION));
            break;
          }
          ap = lee_entero(ap + MIE_SIG);
        }
        if(ap == NULL)
          error("Miembro indefinido");
      } else
        break;
    }
  }
  return k;
}

req_valorl()
{
  error("Debe ser un valor-l");
}

/*
** Compila una llamada a una función
**
** Invocada por "primaria", esta función llamará a la función
** nombrada o a una función indirecta.
*/
llama_funcion(ap, tipo_funcion)
  char *ap, *tipo_funcion;
{
  int info[1];
  int anterior, primero;
  int izq, tam;
  char *tipo;

  anterior = primero = 0;
  espacios();                /* Ya ha sido tomado el parentesis inicial */
  if (ap == 0)
    izq = ultimo_nodo;       /* Llamada indirecta */
  if (*tipo_funcion == STRUCT) {
    tam = (tam_tipo(tipo_funcion) + 3) / 4;
    crea_nodo(N_RESULTA, ultimo_nodo, tam, 0);
    primero = anterior = ultimo_nodo;
  }
  while (car_act != ')') {
    if (fin_sentencia())
      break;
    tam = 0;
    if (nivel1(info))
      carga_valor(info);     /* Obtiene un argumento */
    else {
      tipo = info[0];
      if(*tipo == STRUCT)
        tam = (tam_tipo(tipo) + 3) / 4;
    }
    tipo = info[0];
    if(*tipo == DOUBLE)
      crea_nodo(N_PARF, ultimo_nodo, tam, 0);
    else if(*tipo == FLOAT) {
      crea_nodo(N_CONVFD, ultimo_nodo, 0, 0);
      crea_nodo(N_PARF, ultimo_nodo, tam, 0);
    } else
      crea_nodo(N_PAR, ultimo_nodo, tam, 0);
    if (primero == 0)
      primero = ultimo_nodo;
    if (anterior != 0)
      esp[anterior] = ultimo_nodo;
    anterior = ultimo_nodo;
    if (match(",") == 0)
      break;
    espacios();
  }
  pide(")");
  if (ap == 0)
    crea_nodo(N_FUNCI, primero, 0, izq);
  else
    crea_nodo(N_FUNC, primero, 0, ap);
}

/*
** Carga el valor de una dirección de memoria.
*/
carga_valor(info)
  int info[];
{
  char *tipo;
  tipo = info[0];
  if (*tipo == CHAR) {
    crea_nodo(N_CBYTE, ultimo_nodo, 0, 0);
  } else if (*tipo == SHORT) {
    crea_nodo(N_CSHORT, ultimo_nodo, 0, 0);
  } else if (*tipo == USHORT) {
    crea_nodo(N_CUSHORT, ultimo_nodo, 0, 0);
  } else if (*tipo == FLOAT)
    crea_nodo(N_CFLOAT, ultimo_nodo, 0, 0);
  else if (*tipo == DOUBLE)
    crea_nodo(N_CDOUBLE, ultimo_nodo, 0, 0);
  else if (*tipo == VOID)
    error("Tiene tipo void");
  else
    crea_nodo(N_CPAL, ultimo_nodo, 0, 0);
}

/*
** Carga la dirección de una variable local
*/
dir_var_loc(var)
  char *var;
{
  crea_nodo(N_LDLP, 0, 0, ((var[POSICION] & 255) +
                          ((var[POSICION + 1] & 255) << 8) +
                          ((var[POSICION + 2] & 255) << 16) +
                          ((var[POSICION + 3] & 255) << 24)));
}

/*
** Carga la dir. de una variable global.
*/
dir_var_glb(var)
  char *var;
{
  int j;

  j = (var[POSICION] & 255) +
     ((var[POSICION + 1] & 255) << 8) +
     ((var[POSICION + 2] & 255) << 16) +
     ((var[POSICION + 3] & 255) << 24);
  enlace();
  crea_nodo(N_LDNLP, ultimo_nodo, 0, j);
}

/*
** Carga la dir. de una función.
*/
dir_func(ap)
  char *ap;
{
  crea_nodo(N_APFUNC, 0, 0, ap);
}

/*
** Carga la dir. de comienzo de las variables estáticas.
*/
enlace()
{
  crea_nodo(N_LDL, 0, 0, 1);
}

/*
** Checa si es necesario doblar para suma o resta con apuntadores.
*/
dobla(tipo, nodo)
  char *tipo;
  int nodo;
{
  int cuanto;
  if((*tipo != APUNTADOR) && (*tipo != MATRIZ)) return 0; /* no es necesario */
  if(*tipo == APUNTADOR)
    cuanto = tam_tipo(tipo + 1);
  else
    cuanto = tam_tipo(tipo + 5);
  if(oper[nodo] == N_CONST) {
    esp[nodo] = esp[nodo] * cuanto;     /* es una constante */
    return 1;
  }
  multi = cuanto;
  return 2;    /* optimar segun sea suma o resta */
}

/*
** Prueba si la expresión es cero y salta.
*/
prueba(etiq, parentesis)
  int etiq, parentesis;
{
  int info[1];
  int origen;
  int etiq2;
  char *tipo;

  origen = ultimo_nodo;
  usa_expr = SI;
  if (parentesis) {
    pide("(");
    compara_no_cero(almacena_expresion(SI));
    pide(")");
  } else
    compara_no_cero(almacena_expresion(SI));
  if(oper[ultimo_nodo] == N_CONST) {
    if(esp[ultimo_nodo] == 0)
      salto(etiq);
  } else {
    if(oper[ultimo_nodo] == N_NOT)
      if(oper[nodo_izq[ultimo_nodo]] == N_NOT)
        ultimo_nodo = nodo_izq[nodo_izq[ultimo_nodo]];
    if((oper[ultimo_nodo] == N_ANDB) ||
       (oper[ultimo_nodo] == N_ORB)) {
      etiq_and = etiq;
      etiq_or = nueva_etiq;
      evalua_arbol(SI);
      salta_si_falso(etiq);
      emite_etiq(etiq_or);
      dos_puntos();
      emite_nueva_linea();
    } else {
      evalua_arbol(SI);
      salta_si_falso(etiq);
    }
  }
  ultimo_nodo = origen;
}

constante(info)
  int info[];
{
  int val[1], queonda;

  if (queonda = numero_real(val)) {
    if (queonda == 2)
      crea_nodo(N_CEROPF, 0, 0, 0);
    else
      crea_nodo(N_NUMPF, 0, 0, val[0]);
    info[0] = t_double;
  } else if (numero(val)) {
    crea_nodo(N_CONST, 0, 0, val[0]);
    info[0] = t_int;
  } else if (cad_caracteres(val)) {
    crea_nodo(N_CONST, 0, 0, val[0]);
    info[0] = t_int;
  } else if (cad_literal(val)) {
    crea_nodo(N_LIT, 0, 0, val[0]);
    info[0] = t_achar;
  } else
    return 0;
  return 1;
}

numero_real(val)
  int val[];
{
  double num, escala;
  char *comienzo, *codigo;
  int k, menos;

  comienzo = codigo = linea + pos_linea;
  k = menos = 1;
  while (k) {
    k = 0;
    if (*codigo == '+') {
      ++codigo;
      k = 1;
    }
    if (*codigo == '-') {
      ++codigo;
      k = 1;
      menos = -menos;
    }
  }
  while (isdigit(*codigo))
    ++codigo;
  if (*codigo != '.' && toupper(*codigo) != 'E')
    return 0;
  num = 0;
  if (*codigo == '.') {
    ++codigo;
    while (isdigit(*codigo))
      ++codigo;
    pos_linea = codigo - linea;
    while (*--codigo != '.')
      num = (num + (*codigo - '0')) / 10;
  } else
    pos_linea = codigo - linea;
  escala = 1;
  while (--codigo >= comienzo) {
    num += escala * (*codigo - '0');
    escala *= 10;
  }
  if (toupper(car_act) == 'E') {
    int neg, exp;

    obt_car();
    if (numero(&exp) == 0) {
      error("Exponente incorrecto");
      exp = 0;
    }
    if (exp < 0) {
      neg = 1;
      exp = -exp;
    } else
      neg = 0;
    k = 32;
    escala = 1;
    while (k) {
      escala *= escala;
      if (k & exp)
        escala *= 10;
      k >>= 1;
    }
    if (neg) num /= escala;
    else     num *= escala;
  }
  if (menos < 0)
    num = -num;
  if (num == 0)
    return 2;
  for (k = 0; k < const_definidas; ++k) {
    if(constantes[k].valor == num)
      break;
  }
  val[0] = k;
  if(k == const_definidas)
    if(const_definidas == MAX_CONST)
      error("Demasiadas constantes de punto flotante");
    else
      constantes[const_definidas++].valor = num;
  return 1;
}

numero(val)
  int val[];
{
  int k, menos;
  char c;

  k = menos = 1;
  while (k) {
    k = 0;
    if (match("+"))
      k = 1;
    if (match("-")) {
      menos = -menos;
      k = 1;
    }
  }
  if (isdigit(car_act) == 0)
    return 0;
  if(car_act == '0') {
    while(car_act == '0') obt_car();
    if(toupper(car_act) == 'X') {
      obt_car();
      while(isxdigit(car_act)) {
        c = toupper(obt_car()) - '0';
        if(c > 9) c = c - 7;
        k = (k << 4) | c;
      }
    } else {
      while ((car_act >= '0') && (car_act <= '7'))
        k = k * 8 + (obt_car() - '0');
    }
  } else {
    while (isdigit(car_act))
      k = k * 10 + (obt_car() - '0');
  }
  if(toupper(car_act) == 'L')
    obt_car();
  if (menos < 0)
    k = -k;
  val[0] = k;
  return 1;
}

cad_caracteres(val)
  int val[];
{
  int k;

  k = 0;
  if (match("'") == 0)
    return 0;
  while (car_act != 39)
    k = (k & 255) * 256 + (caracter_literal() & 255);
  ++pos_linea;
  val[0] = k;
  return 1;
}

cad_literal(val)
  int val[];
{
  char c;

  if (match("\"") == 0)
    return 0;
  val[0] = ap_lit;
  while (car_act != '"') {
    if (car_act == 0)
      break;
    if (ap_lit >= MAX_LITS) {
      error("Espacio de almacenamiento de cadenas agotado");
    while (match("\"") == 0)
      if (obt_car() == 0)
        break;
      return 1;
    }
    lits[ap_lit++] = caracter_literal();
  }
  obt_car();
  lits[ap_lit++] = 0;
  return 1;
}

caracter_literal()
{
  int i, oct;

  if ((car_act != 92) || (prox_car() == 0))
    return obt_car();
  obt_car();
  if (car_act == 'n') {
    ++pos_linea;
    return 10;
  }
  if (car_act == 't') {
    ++pos_linea;
    return 9;
  }
  if (car_act == 'b') {
    ++pos_linea;
    return 8;
  }
  if (car_act == 'f') {
    ++pos_linea;
    return 12;
  }
  if (car_act == 'r') {
    ++pos_linea;
    return 13;
  }
  if (car_act == 'x') {
    ++pos_linea;
    while(isxdigit(car_act)) {
      i = toupper(obt_car()) - '0';
      if(i > 9) i = i - 7;
      oct = (oct << 4) | i;
    }
    return oct;
  }
  i = 3;
  oct = 0;
  while ((i-- > 0) && (car_act >= '0') && (car_act <= '7'))
    oct = (oct << 3) + obt_car() - '0';
  if (i == 2)
    return obt_car();
  else
    return oct;
}

/*
** Funciones de conversión y chequeo de tipos.
*/

checa_entero(tipo)
  char *tipo;
{
  if (*tipo != CHAR && *tipo != SHORT && *tipo != INT
   && *tipo != USHORT && *tipo != UINT)
    error("No es un tipo entero");
}

checa_numerico(tipo)
  char *tipo;
{
  if (*tipo != CHAR && *tipo != SHORT && *tipo != INT
   && *tipo != USHORT && *tipo != UINT && *tipo != DOUBLE
   && *tipo != FLOAT && *tipo != APUNTADOR)
    error("No es un tipo númerico");
}

checa_entero_o_apuntador(tipo)
  char *tipo;
{
  if (*tipo != CHAR && *tipo != SHORT && *tipo != INT
   && *tipo != USHORT && *tipo != UINT && *tipo != APUNTADOR)
    error("No es un tipo entero");
}

compara_no_cero(tipo)
  char *tipo;
{
  int izq;

  if (*tipo != DOUBLE && *tipo != FLOAT)
    return;
  izq = ultimo_nodo;
  if (*tipo == DOUBLE)
    crea_nodo(N_CEROPF, 0, 0, 0);
  else
    crea_nodo(N_CEROF, 0, 0, 0);
  crea_nodo(N_IGUALPF, izq, ultimo_nodo, 0);
  crea_nodo(N_NOT, ultimo_nodo, 0, 0);
}

compara_cero(tipo)
  char *tipo;
{
  int izq;

  izq = ultimo_nodo;
  if (*tipo == DOUBLE)
    crea_nodo(N_CEROPF, 0, 0, 0);
  else
    crea_nodo(N_CEROF, 0, 0, 0);
  crea_nodo(N_IGUALPF, izq, ultimo_nodo, 0);
}

convierte_tipo(nodo, tipo_original, nuevo_tipo)
  int *nodo;
  char *tipo_original, *nuevo_tipo;
{
  if(*tipo_original == STRUCT && *nuevo_tipo != STRUCT)
    error("No se puede convertir de estructura");
  else if(*tipo_original != STRUCT && *nuevo_tipo == STRUCT)
    error("No se puede convertir a estructura");
  else if(*tipo_original == VOID)
    error("No se puede convertir de void");
  else if(*tipo_original == APUNTADOR &&
         (*nuevo_tipo == DOUBLE || *nuevo_tipo == FLOAT))
    error("No se puede convertir un apuntador a real");
  else if(*nuevo_tipo == APUNTADOR &&
         (*tipo_original == DOUBLE || *tipo_original == FLOAT))
    error("No se puede convertir un real a apuntador");
  else {
    if(*tipo_original == DOUBLE && *nuevo_tipo == DOUBLE)
      return;
    if(*tipo_original == FLOAT && *nuevo_tipo == FLOAT)
      return;
    if(*tipo_original == DOUBLE && *nuevo_tipo == FLOAT) {
      crea_nodo(N_CONVDF, *nodo, 0, 0);
      *nodo = ultimo_nodo;
    } else if(*tipo_original == FLOAT && *nuevo_tipo == DOUBLE) {
      crea_nodo(N_CONVFD, *nodo, 0, 0);
      *nodo = ultimo_nodo;
    } else if(*tipo_original == DOUBLE || *tipo_original == FLOAT) {
      crea_nodo(N_PFENT, *nodo, 0, 0);
      *nodo = ultimo_nodo;
    } else if(*nuevo_tipo == FLOAT) {
      crea_nodo(N_ENTF, *nodo, 0, 0);
      *nodo = ultimo_nodo;
    } else if(*nuevo_tipo == DOUBLE) {
      crea_nodo(N_ENTPF, *nodo, 0, 0);
      *nodo = ultimo_nodo;
    }
  }
}

haz_compatible(nodo_izq, info_izq, nodo_der, info_der)
  int *nodo_izq, *nodo_der;
  int info_izq[], info_der[];
{
  char *tipo_izq, *tipo_der;

  tipo_izq = info_izq[0];
  tipo_der = info_der[0];
  if(*tipo_izq == STRUCT || *tipo_der == STRUCT)
    error("No se pueden hacer operaciones con estructuras");
  if(*tipo_izq == FLOAT && *tipo_der == FLOAT)
    return 1;
  if(*tipo_izq == DOUBLE && *tipo_der == DOUBLE)
    return 1;
  if((*tipo_izq == FLOAT && *tipo_der == DOUBLE)
  || (*tipo_izq == DOUBLE && *tipo_der == FLOAT)) {
    if(*tipo_izq == FLOAT) {
      crea_nodo(N_CONVFD, *nodo_izq, 0, 0);
      *nodo_izq = ultimo_nodo;
    } else {
      crea_nodo(N_CONVFD, *nodo_der, 0, 0);
      *nodo_der = ultimo_nodo;
    }
    info_izq[0] = t_double;
    return 1;
  }
  if(*tipo_izq == FLOAT || *tipo_der == FLOAT) {
    if(*tipo_izq == FLOAT) {
      crea_nodo(N_ENTF, *nodo_der, 0, 0);
      *nodo_der = ultimo_nodo;
    } else {
      crea_nodo(N_ENTF, *nodo_izq, 0, 0);
      *nodo_izq = ultimo_nodo;
    }
    info_izq[0] = t_float;
    return 1;
  }
  if(*tipo_izq == DOUBLE || *tipo_der == DOUBLE) {
    if(*tipo_izq == DOUBLE) {
      crea_nodo(N_ENTPF, *nodo_der, 0, 0);
      *nodo_der = ultimo_nodo;
    } else {
      crea_nodo(N_ENTPF, *nodo_izq, 0, 0);
      *nodo_izq = ultimo_nodo;
    }
    info_izq[0] = t_double;
    return 1;
  }
  if((*tipo_izq == UINT || *tipo_der == UINT)
  && (*tipo_izq != APUNTADOR && *tipo_der != APUNTADOR)
  && (*tipo_izq != MATRIZ && *tipo_der != MATRIZ))
    info_izq[0] = info_der[0] = t_uint;
  return 0;
}

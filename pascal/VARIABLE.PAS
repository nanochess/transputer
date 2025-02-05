{ Programa: Compilador de Pascal para el Inmos T805 }
{                                                   }
{ Módulo  : Declaraciones de Variables              }
{                                                   }
{ (c) Copyright 1993-1994 Oscar Toledo G.           }
{          Todos los derechos reservados.           }

PROGRAM compilador_de_pascal;

{ Genera el mejor codigo que he podido lograr, casi imita el  }
{ codigo hecho a mano. sin embargo por el «casí» se ve que es }
{ un compilador.                                              }

{ Forma de compilador:                                       }
{ * Análisis léxico simple                                   }
{ * Análisis sintáctico recursivo descendente                }
{ * Generación de arboles sintácticos (sólo en expresiones)  }
{ * Optimación de arboles                                    }
{ * Optimación de mirilla (optimación simple) (Peephole)     }
{ * Sin recuperación de errores                              }

{ Es necesario recordar que no tuve ningun compilador para el T805 en }
{ que basarme, por lo que seguramente no tengo conceptos viciados en  }
{ mi compilador. (es una de las ventajas de no tener información)     }

{ El listado esta profusamente documentado, y espero que no sea }
{ dificil de entender. }

{ Notas de última hora: }
{ 1o. Esta implementado el proceso de tipos FILE, pero no existe ninguna }
{     función interconstruida para su uso, y por ende resultan inutiles }
{     en este momento. }
{ 2o. Esta implementada la sintaxis para el paso de procedimientos y }
{     funciones como parametros, pero no existe la implementación en el }
{     generador de codigo, las expresiones, y las sentencias. }
{ 3o. PACKED no tiene ningun efecto. }

{ MINI-PASCAL = Compilador de un subconjunto de Pascal }

{ Cambios: }

{ 21-sep-93  Empiezo a convertir mi compilador de Mini-Pascal, de BASIC a }
{            mini-Pascal, en este momento es un compilador de Mini-Pascal }
{            escrito en BASIC interpretado, que genera codigo ensamblador }
{            para el T805, se crea el archivo PASCAL.PAS }
{  1-oct-93  Se termina la conversión, ahora es un compilador de Mini-Pascal }
{            escrito en Mini-Pascal compilado por el T805, que genera codigo }
{            para el T805 }
{  9-oct-93  Se mejora la legibilidad de los procedimientos }
{ 13-oct-93  Empiezo a implementar TYPE, RECORD, punto flotante, }
{            enumeradores, subrangos, matrices, etc. }
{ 22-oct-93  Se termina la implementación }
{ 25-oct-93  Corregi algunos detalles en el compilador }
{ 26-oct-93  Se agregan las funciones trigonometricas SIN y COS, soporte }
{            para RECORD variable, XOR, SHL, y SHR, así cómo operaciones }
{            de bits }
{ 27-oct-93  Soporte de constantes reales en notación cientifica, corregi }
{            un error en los operadores de comparación reales. }
{ 28-oct-93  Se le agregan las funciones ARCTAN, LN, y EXP }
{ 30-oct-93  Corrigo varios errores }
{  1-nov-93  Optimación del codigo generado para las comparaciones, y }
{            reducción en el número de ajw generados. }
{  3-nov-93  Se mejora la generación de codigo para que genere cargas de }
{            punto flotante indexadas, genera fpldzerosn para cargas de 0.0 }
{            y se optima la construcción CASE }
{  4-nov-93  Optimé mi compilador para que use fpldnladdsn y fpldnlmulsn }
{            mejore el CASE }
{  5-nov-93  Se hacen varias mejoras, se optimiza el < y el >= para no }
{            generar rev innecesarios, se corrige un error en la generación }
{            de codigo }
{  8-nov-93  Se rehacen las funciones ARCTAN, LN, y EXP, ahora generan }
{            resultados extremadamente precisos. }
{ 10-nov-93  Se hace que optime más el codigo generado }
{ 11-nov-93  Se le agregan 8 optimizaciones, se divide el compilador }
{            PASCAL.PAS en varios archivos. }

{            VARIABLE.PAS  Declaraciones de variables }
{            ERRORES .PAS  Procesador de errores }
{            ANALEXIC.PAS  Analizador léxico }
{            GENECODI.PAS  Generador de codigo }
{            EXPRESIO.PAS  Analizador de expresiones }
{            SENTENCI.PAS  Analizador de sentencias }
{            DECLARAC.PAS  Analizador de declaraciones }
{            PRINCIPA.PAS  Módulo principal }

{ 12-nov-93  Documente un poco el compilador }
{ 16-nov-93  Se hacen los apuntadores }
{ 27-nov-93  Se mejora el codigo en copia de variables de punto flotante }
{            y carga a cero }
{  9-dic-93  Se corrige un error en el acceso a parametros no locales, }
{            y se hace soporte para MAXINT }
{ 14-dic-93  Se optiman los ldnlp seguidos por un ldnl o stnl, o los }
{            ldnlp seguidos }
{ 15-dic-93  Se optiman los ajw seguidos, y las asignaciones de }
{            resultados de funciones }
{ 16-dic-93  Se optimá la carga de enlace estático, en funciones }
{            que sólo retornan un valor, esto elimina las }
{            instrucciones innecesarias que acompañan cuálquier }
{            función, carga de enlace }
{ 21-dic-93  Se hace la fácilidad para GOTO inter-procedimientos de }
{            distintos niveles }
{ 22-dic-93  Se hace la sentencia WITH. (fue un poco díficil) }
{  8-feb-94  Se hace MEM, CALL, _FIN_PAS_, y se corrige ASM }
{ 17-feb-94  Se mejora la legibilidad del acceso a los arboles, }
{            se sustituye arbol[nodo*5+3] -> arbol[nodo,ESP], }
{            se convierte _FIN_PAS_ en _FINPROG, y se hace una }
{            mejora que acelera la compilación. }
{ 18-feb-94  Se mejora la optimación para ldnlp seguidos por stnl, }
{            así cómo accesos a matrices de caracteres en multiplos }
{            constantes de 4, también se mejora el acceso a cadenas se }
{            elimina el salto, y las cadenas se ponen al final de la }
{            función. (esto reduce por mucho los programas) }
{ 19-feb-94  Se hace comparación de cadenas repetidas para eliminarlas, }
{            se corrige un problema en goto inter-procedimientos. }
{ 20-feb-94  Se agregan los tipos WORD, y BYTE, se agrega espacio en el }
{            analizador léxico para STRING, y se dan los primeros pasos }
{            para poner SET, FILE, y STRING }
{ 23-feb-94  Se hace que las funciones puedan retornar resultados de tipo }
{            arbitrario, y se hace que CALL sea una función intercons. y }
{            no una palabra reservada. }
{ 25-feb-94  Se termina la implementación de los conjuntos (SET), se mejora }
{            el WITH para soportar la coma, se hacen algunos mejoramientos }
{            estéticos, se agrega el procedimiento interconstruido RANDOMIZE }
{            las funciones interconstruidas PI y RANDOM. }
{  1-mar-94  Se corrige un defecto en el retorno de conjuntos como }
{            resultados, se corrige un defecto en el manejo del espacio de }
{            trabajo que hacia que fallaran ciertas clases de programas, y }
{            se mejora el codigo generado por la construcción FOR }
{  2-mar-94  Se optiman las llamadas a funciones/procedimientos para que }
{            no se cargue el enlace o la tabla de constantes en cada }
{            llamada a función/procedimiento. }
{  3-mar-94  Se termina la implementación de STRING, y sus funciones }
{            asociadas, pos, concat, copy, insert, delete, length, excepto }
{            str }
{ 19-mar-94  Se quitan las comparaciones con cadenas en la sintaxis, y se }
{            substituyen por comparaciones con constantes, se reescribe }
{            casí por completo el generador de codigo para que sólo use }
{            comparaciones con constantes en vez de comparaciones con }
{            cadenas, esto ahorra memoria, además acelera el compilador }
{            varias veces. }
{ 26-mar-94  Se hace soporte de cadenas con " y también ', se hace que }
{            indique si puede ocurrir un goto, sí no da error. }
{  9-abr-94  Se corrige un error en la compilación de expresiones usando }
{            el operador de negación con reales, y un posible error en }
{            compilación de procedimientos que hacia que no cargara el }
{            enlace estático y la tabla de constantes. }
{ 27-abr-94  Se hace que el compilador pase parametros en los registros del }
{            procesador, y se hace que accese la tabla de constantes a traves }
{            del enlace estático. }
{ 28-abr-94  Se hace que genere mejor codigo para el acceso a matrices }
{            multidimensionales, esto y las modificaciones de ayer ahorran }
{            muchos ciclos en los programas compilados. }
{ 29-abr-94  Se crea el procedimiento optimación para que optime el arbol, }
{            y se hace que se genere mejor codigo para el acceso a }
{            variables de byte. }
{ 30-abr-94  Se hace soporte para nombres de hasta 255 caracteres (antes }
{            sólo 10 caracteres), y se mejora la optimación }
{  1-may-94  Se corrige un error en la generación de codigo para }
{            comparaciones de STRING's, y se mejora la optimación para que }
{            evalue expresiones constantes }
{  6-may-94  Se corrige un error en la optimación, se hace mejor optimación, }
{            se corrige un error en la generación de codigo para read, y se }
{            corrige un error en la declaración de apuntadores. }
{  7-may-94  Se elimina la palabra reservada MEM para acceder a la memoria, }
{            ya que se puede hacer lo mismo con un apuntador a CHAR, ejemplo: }
{}
{               a = RECORD CASE BOOLEAN OF }
{                    TRUE: (dir1:^CHAR);   }
{                    FALSE: (dir2:INTEGER) }
{                   END;                   }
{}
{ 26-may-94  Se corrige un error molesto en la función de procesamiento de }
{            nombres en expresiones, que hacia que las funciones tuvieran }
{            prioridad sobre las variables del mismo nombre. }
{ 31-may-94  Se corrige un error en el paso de parametros en llamadas a }
{            procedimientos o funciones, hacia que se pasaran valores }
{            incorrectos en los parametros }
{  1-jun-94  Se corrige un error en el procesamiento de readln y randomize }
{            que hacia que se perdiera el resultado de la función en que }
{            estuvieran, también se corrige un error que hacia que las }
{            funciones que retornaban cadenas o conjuntos no funcionaran }
{            correctamente. }
{  2-jul-94  Se corrigen algunos mensajes de error vagos. }
{ 29-ago-94  Se corrige un error en el manejo de variables de tipo CHAR. }
{ 29-mar-95  Se añade opción para uso de evaluación en corto circuito de }
{            expresiones boleanas, esto puede acelerar los programas por }
{            mucho. (Nota: el codigo no esta terminado y no hace nada) }
{ 31-mar-95  Hice el codigo para evaluación en corto circuito de expresiones }
{            boleanas. }
{  3-abr-95  Se agrega optimación del uso de los registros de punto }
{            flotante, se ahorran fprev's y accesos a memoria. }
{ 25-oct-95  Se corrige un defecto en la generación de codigo para nodos }
{            NRCARGA con un descendiente NINDEXAPAL, ahora substituye }
{            NINDEXAPAL con NRCARGAI cómo es correcto. }

CONST
 palres  =  42;                          { Número de pals. reservadas   }
 tam     = 850;                          { Tamaño de la tabla de idents }
 bpalabra=   4;                          { Bytes por palabra            }
 maxcad  = 550;                          { Número max. cadenas por prog.}
 maxcadl =6000;                          { Tamaño almacenamiento cadenas}
 maxcon  = 300;                          { Número max. cons. reales     }
 maxniv  =  32;                          { Máximo nivel de anidamiento  }
 maxlit  =5000;                          { Tamaño almac. literal        }
 maxarb  = 250;                          { Tamaño máximo del arbol      }

TYPE
 vareal=                                 { Esta estructura es           }
  RECORD                                 { dependiente de la máquina,   }
   CASE INTEGER OF                       { se usa para convertir un     }
   0:(valor:REAL);                       { valor REAL a un valor        }
   1:(bytes:ARRAY[1..bpalabra] OF CHAR); { INTEGER, o en bytes          }
   2:(valorint:INTEGER)                  { separados.                   }
  END;
 narbol=(                                { Posibles tipos de nodo       }
  NNULO,NISUMA,NIRESTA,NIMUL,NIDIV,NIIGUAL,NINOIGUAL,NIMENOR,NIMAYOR,
  NIMENORIGUAL,NIMAYORIGUAL,NIMOD,NSHL,NSHR,NAND,NOR,NXOR,
  NINUM,NICARGA,NIFRANJA,NICARGACHAR,NNEG,NNOT,NNOTB,
  NRSUMA,NRRESTA,NRMUL,NRDIV,NRIGUAL,NRNOIGUAL,NRMENOR,NRMAYOR,
  NRMENORIGUAL,NRMAYORIGUAL,NRCARGA,NRCARGAI,NRNUM,NRCERO,
  NINTAREAL,NPOR2,NDIV2,NAIGUAL,NANOIGUAL,NAMENOR,NAMAYOR,NAMENORIGUAL,
  NAMAYORIGUAL,NCAD,NCIGUAL,NCNOIGUAL,NCMENOR,NCMAYOR,NCMENORIGUAL,
  NCMAYORIGUAL,NCLIMPIA,NCLIMPIAA,NCCARGA,NCDIR,NARRAYACAD,NASIGNA,
  NMULRAPIDA,NINDEXAPAL,NINDEXABYTE,NCARGAFOR,NCARGAWITH,NFINPROG,
  NDIRVAR,NFUNCION,NNIL,NSLIMPIA,NSGENBIT,NSGENBITS,NSCARGA,NSUNION,
  NSDIFER,NSINTER,NSCOMPA,NSALGO,NSDIR,NEOLN,NEOF,NRANDOM,NROUND,NTRUNC,
  NCALL,NABS,NABSR,NSQR,NSQRR,NSQRT,NARCTAN,NCOS,NSIN,NEXP,NLN,NCONCAT,
  NPOS,NINSERT,NDELETE,NCOPY,NIIGUALESP
  );
 farbol=                                 { Franjas por nodo del arbol   }
  RECORD
   nom:narbol;
   izq:INTEGER;                          { Descendiente izq. del nodo   }
   der:INTEGER;                          { Descendiente der. del nodo   }
   esp:INTEGER;                          { Información específica       }
   reg:INTEGER;                          { Regs. 32 bits req.           }
   regf:INTEGER                          { Regs. de punto flotante req. }
  END;
 comlex=(                                { Posibles componentes léxicos }
  CAND,CARRAY,CASM,CBEGIN,CCASE,CCONST,CDIV,CDO,CDOWNTO,CELSE,CEND,
  CFILE,CFOR,CFORWARD,CFUNCTION,CGOTO,CIF,CIN,CLABEL,CMOD,CNIL,CNOT,
  COF,COR,CPACKED,CPROCEDURE,CPROGRAM,CRECORD,CREPEAT,CSET,CSHL,CSHR,CSTRING,
  CTHEN,CTO,CTYPE,CUNTIL,CVAR,CWHILE,CWITH,CXOR,C_FINPROG,
  CIDENT,CNUM,CNUMREAL,CCAD,CPARENI,CPAREND,CAPUNTA,CMAS,CMENOS,CPOR,CDIVIDE,
  CLLAVEI,CLLAVED,CPUNTO,CPUNTOYCOMA,CPUNTOPUNTO,CCOMA,CDOSPUNTOS,CASIGNA,
  CFINDEARCHIVO,CIGUAL,CNOIGUAL,CMENOR,CMAYOR,CMENORIGUAL,CMAYORIGUAL,
  CERROR);
 tabla =ARRAY[0..tam] OF INTEGER;        { Tabla                        }
 almcad=                                 { Indica etiq y pos de cadenas }
  RECORD
   etiqueta:INTEGER;
   posicion:INTEGER
  END;

VAR
 profact:INTEGER;                        { Profundidad de anidamiento   }
 apret:INTEGER;                          { Apuntador relativo al espacio}
                                         { de trabajo.                  }

                                         { USO DEL ANALIZADOR LÉXICO:   }
 linea:STRING;                           { Linea actual                 }
 linact:INTEGER;                         { Número de linea actual       }
 caract:CHAR;                            { Caracter actual en la linea  }
 aplin:INTEGER;                          { Apuntador al caracter        }
 tamlin:INTEGER;                         { Tamaño de la linea           }
 pal:ARRAY[1..palres] OF STRING[9];      { Diccionario de pals. res.    }

                                         { VALORES RETORNADOS POR EL    }
                                         { ANALIZADOR LÉXICO:           }
 lexema:comlex;                          { Lexema encontrado            }
 numero:INTEGER;                         { Número entero, lexema=NUM    }
 numreal,usoesp:vareal;                  { Número real,   lexema=NUMREAL}
 cadena,funcion:STRING;                  { Cadena.        lexema=CAD    }

                                         { VARIABLES VARIAS USADAS POR  }
                                         { EL COMPILADOR:               }
 proxeti:INTEGER;                        { Proxima etiqueta disponible  }
 especial:INTEGER;
 sevalegoto:BOOLEAN;                     { Indica si se valen gotos     }

 possim,ultsim,numpar,numvar,tpaso:INTEGER;
 pila:INTEGER;                           { Pila relativa                }
 posproc:INTEGER;
 proced:BOOLEAN;
 tipoi:INTEGER;                          { Contiene el tipo actual      }
 resfun:INTEGER;
 nodoact:INTEGER;                        { Ap. al nodo actual del arbol }
 apuarb,apulit:INTEGER;
 dentrotype:BOOLEAN;                     { Indica si se esta en el modo }
                                         { de definición de tipos       }

                                         { Variables para WITH          }
 twith:INTEGER;                          { tipo que sigue               }
 pwith:INTEGER;                          { posición del desplazamiento  }

                                         { OPTIMIZACIÓN ESPECIAL:       }
 oajw:INTEGER;                           { Optimiza ajw, resultados de  }
 aajw,afps,astl,aenl:BOOLEAN;            { funciones y carga de enlace  }
 afps2,astl2:INTEGER;                    { Direccion del resultado      }

 optcc:BOOLEAN;                          { Indica optimación corto-circuito }
 etiqand:INTEGER;                        { Etiqueta de salida para AND  }
 etiqor:INTEGER;                         { Etiqueta de salida para OR   }
 escontrol:BOOLEAN;                      { Indica IF, WHILE o REPEAT    }

 pparinf:ARRAY[0..maxniv] OF INTEGER;    { Posición de parametros de    }
                                         { procedimientos de nivel      }
                                         { inferior, usada para acceder }
                                         { a parametros de funciones de }
                                         { nivel inferior.              }

 literal:ARRAY[0..maxlit] OF CHAR;       { Almacenamiento literal de    }
                                         { nombres, identificadores,    }
                                         { tipos, etc.                  }

 clase:ARRAY[0..tam] OF CHAR;            { Clase de nombre              }
 sim,prof,val,num,paso,tipo,min,max,prox,sub:tabla;

 arbol:ARRAY[0..maxarb] OF farbol;       { Arbol sintáctico de exprs.   }

 constantes:ARRAY[0..maxcon] OF vareal;  { Constantes reales encontradas}
 apucons:INTEGER;                        { Última constante encontrada  }

 cadact:INTEGER;                         { Última cadena en la tabla    }
 cadlit:INTEGER;                         { Posición literal             }
 proxcad:INTEGER;                        { Proxima cadena a sacar       }
 tabcad:ARRAY[1..maxcad] OF almcad;      { Etiquetas cadenas literales  }
 litcad:ARRAY[0..maxcadl] OF CHAR;       { Buffer de cadenas literales  }


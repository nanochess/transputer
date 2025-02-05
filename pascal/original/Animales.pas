PROGRAM animales (input, output);

{                   --------------------------------                   }
{                    A N I M A L E S. Ver. 23-04-96                    }
{                   --------------------------------                   }
{               (c) Copyright 1996 Oscar Toledo Gutiérrez              }
{                   --------------------------------                   }
{                                                                      }
{ Este  programa  asigna  dinámicamente  la  memoria  para el árbol de }
{ conocimiento,  así  que  no  tiene  más  limite que la memoria de la }
{ computadora.                                                         }
{                                                                      }
{ Sería  buena  idea hacer una rutina que salve el contenido del árbol }
{ en disco, y que al comienzo del programa lo restaure a la memoria.   }

CONST
  ERROR = 0;
  SI    = 1;
  NO    = 2;

TYPE

{ Nodo del árbol de conocimiento, cuando opera el programa,  cada nodo }
{ puede tener ninguno  o  dos descendientes,  los  nodos que no tienen }
{ ningún  descendiente  son  respuestas,  los  nodos  que  tienen  dos }
{ descendientes son preguntas,  el  descendiente izquierdo es para una }
{ respuesta "SI" del usuario,  el  descendiente  derecho  es  para una }
{ respuesta "NO" del usuario.                                          }

  animal = RECORD
             pregunta : STRING;    { Pregunta o respuesta              }
             izquierdo : ^animal;  { Descendiente izquierdo            }
             derecho : ^animal     { Descendiente derecho              }
           END;

VAR

  ya_jugo : BOOLEAN;         { Esta variable indica si se ha comenzado }
                             { algún juego, esto permite mostrar dos   }
                             { mensajes distintos a la salida del      }
                             { programa.                               }

  raiz : ^animal;            { Nodo inicial del árbol de conocimiento  }

  numero_animales : INTEGER; { Número de animales que conoce           }

{                    -------------------------------                   }
{                     Crea la base de datos inicial                    }
{                    -------------------------------                   }
{                                                                      }
{  Creamos un nodo con la pregunta '¿Vive en el agua?', si el usuario  }
{  responde "SI", entonces el árbol conduce a la respuesta 'Ballena',  }
{  de lo contrario conduce a la respuesta 'León'.                      }

PROCEDURE crea_base_de_datos;
VAR
  nodo : ^animal;
BEGIN
  new(raiz);                              { Crea el árbol inicial      }
  raiz^.pregunta := 'Vive en el agua';    { Primera pregunta           }
  new(nodo);                              { Crea el descendiente izq.  }
  raiz^.izquierdo := nodo;
  nodo^.pregunta := 'ballena';            { Respuesta para un "SI"     }
  nodo^.izquierdo := NIL;
  nodo^.derecho := NIL;
  new(nodo);                              { Crea el descendiente der.  }
  raiz^.derecho := nodo;
  nodo^.pregunta := 'león';               { Respuesta para un "NO"     }
  nodo^.izquierdo := NIL;
  nodo^.derecho := NIL;
  numero_animales := 2                    { Conoce 2 animales ahora    }
END;

{                            --------------                            }
{                             Presentación                             }
{                            --------------                            }

PROCEDURE presentacion;
BEGIN
  writeln;
  writeln('A N I M A L E S. Ver. 23-04-96');
  writeln('(c) Copyright 1996 Oscar Toledo G.');
  writeln
END;

{                      ---------------------------                     }
{                       Lee una línea del teclado                      }
{                      ---------------------------                     }

PROCEDURE leer(VAR linea : STRING);
VAR
  letra : CHAR;
BEGIN
  write(' ? ');
  linea := '';                         { Empieza con una línea vacia   }
  WHILE NOT eoln DO                    { Mientras no se pulsa "Entrar" }
  BEGIN
    read(letra);                       { Lee una letra del teclado     }
    write(letra);                      { Eco a la pantalla             }
    IF ord(letra) = 8 THEN             { ¿ Pulso "Retroceso" ?         }
      linea := copy(linea, 1, length(linea) - 1)
    ELSE
      linea := linea + letra;          { Agrega la nueva letra         }
  END;
  readln;                              { Absorbe "Entrar"              }
  writeln
END;

{                --------------------------------------                }
{                 Convierte de minúsculas a mayúsculas                 }
{                --------------------------------------                }

FUNCTION mayuscula(caracter : CHAR) : CHAR;
BEGIN
  IF (caracter >= 'a') AND (caracter <= 'z') THEN
    mayuscula := chr(ord(caracter) - ord(' '))
  ELSE
    mayuscula := caracter
END;

{                --------------------------------------                }
{                 Convierte de mayúsculas a minúsculas                 }
{                --------------------------------------                }

FUNCTION minuscula(caracter : CHAR) : CHAR;
BEGIN
  IF (caracter >= 'A') AND (caracter <= 'Z') THEN
    minuscula := chr(ord(caracter) + ord(' '))
  ELSE
    minuscula := caracter
END;

{                   --------------------------------                   }
{                    Checa la respuesta del usuario                    }
{                   --------------------------------                   }
{                                                                      }
{ Acepta respuestas "SI", "NO", "S" y "N".                             }

FUNCTION checa_respuesta(VAR entrada : STRING) : INTEGER;
VAR
  a : INTEGER;
BEGIN
  FOR a := 1 TO length(entrada) DO            { Convierte a mayúsculas }
    entrada[a] := mayuscula(entrada[a]);
  IF (entrada = 'SI') OR (entrada = 'S') THEN
    checa_respuesta := SI
  ELSE IF (entrada = 'NO') OR (entrada = 'N') THEN
    checa_respuesta := NO
  ELSE
  BEGIN
    writeln('Por favor responda solo SI ó NO.');
    checa_respuesta := ERROR
  END
END;

{                 -------------------------------------                }
{                  Pregunta al usuario si quiere jugar                 }
{                 -------------------------------------                }

FUNCTION quiere_jugar : BOOLEAN;
VAR
  pregunta : STRING;
  error : BOOLEAN;
BEGIN
  error := TRUE;
  WHILE error DO         { Repite hasta obtener una respuesta correcta }
  BEGIN
    write('¿ Quiere jugar');
    leer(pregunta);
    IF checa_respuesta(pregunta) = SI THEN
    BEGIN
      error := FALSE;
      quiere_jugar := TRUE
    END
    ELSE IF checa_respuesta(pregunta) = NO THEN
    BEGIN
      error := FALSE;
      quiere_jugar := FALSE
    END
  END
END;

{              -------------------------------------------             }
{               Escribe una cadena de texto en minúsculas              }
{              -------------------------------------------             }

PROCEDURE escribe(VAR texto : STRING);
VAR
  a : INTEGER;
BEGIN
  FOR a := 1 TO length(texto) DO
    write(minuscula(texto[a]))
END;

{          ---------------------------------------------------         }
{           Escribe una cadena de texto con mayúscula inicial          }
{          ---------------------------------------------------         }

PROCEDURE escribe_con_mayuscula(VAR texto : STRING);
VAR
  a : INTEGER;
BEGIN
  write(mayuscula(texto[1]));
  FOR a := 2 TO length(texto) DO
    write(minuscula(texto[a]))
END;

{        ------------------------------------------------------        }
{         Escribe una cadena de texto con el adjetivo correcto         }
{        ------------------------------------------------------        }

PROCEDURE adjetivo(VAR animal : STRING);
BEGIN
  IF mayuscula(animal[length(animal)]) = 'A' THEN
    write('una ')
  ELSE
    write('un ');
  escribe(animal)
END;

{                     -------------------------                        }
{                      Aprende un animal nuevo                         }
{                     -------------------------                        }
{                                                                      }
{ Pide  el  nombre  del  animal  nuevo,  también  una  pregunta que lo }
{ diferencie  del  último  animal  posible,  y  lo  agrega en el lugar }
{ correcto del árbol.                                                  }

PROCEDURE aprende_un_animal(nodo_actual : ^animal);
VAR
  respuesta : STRING;
  nodo_animal1 : ^animal;          { Apuntador al nuevo animal         }
  nodo_animal2 : ^animal;          { Apuntador al animal antiguo       }
  error : BOOLEAN;
BEGIN
  writeln;
  new(nodo_animal1);               { Crea espacio para el nuevo animal }
  write('Me rindo. ¿ Cual es su animal');
  leer(nodo_animal1^.pregunta);    { Obtiene el nombre del animal      }
  nodo_animal1^.izquierdo := NIL;  { Es una respuesta, no tiene...     }
  nodo_animal1^.derecho := NIL;    { descendientes.                    }
  new(nodo_animal2);               { Desplaza el animal anterior       }
  nodo_animal2^.pregunta := nodo_actual^.pregunta;
  nodo_animal2^.izquierdo := NIL;
  nodo_animal2^.derecho := NIL;
  writeln('¿ Introduzca una pregunta que diferencie a ');
  adjetivo(nodo_animal1^.pregunta);
  write(' de ');
  adjetivo(nodo_animal2^.pregunta);
  leer(nodo_actual^.pregunta);
  error := TRUE;
  WHILE error DO
  BEGIN
    write('¿ Para ');
    adjetivo(nodo_animal2^.pregunta);
    write(' la respuesta sería');
    leer(respuesta);
    IF checa_respuesta(respuesta) = SI THEN
    BEGIN
      error := FALSE;
      nodo_actual^.izquierdo := nodo_animal2;
      nodo_actual^.derecho := nodo_animal1
    END
    ELSE IF checa_respuesta(respuesta) = NO THEN
    BEGIN
      error := FALSE;
      nodo_actual^.izquierdo := nodo_animal1;
      nodo_actual^.derecho := nodo_animal2
    END
  END;
  numero_animales := numero_animales + 1;
  writeln;
  writeln('¡ Ahora ya conozco ',numero_animales:0,' animales distintos !');
  writeln
END;

{                       --------------------                           }
{                        Hace las preguntas                            }
{                       --------------------                           }
{                                                                      }
{ Empieza  por  la  raíz  del  árbol,  va haciendo las preguntas, cada }
{ respuesta  lo va llevando por los distintos descendientes del árbol, }
{ hasta  encontrar  una  respuesta,  luego pregunta si la respuesta es }
{ correcta, si no es correcta pide un animal nuevo.                    }

PROCEDURE hace_preguntas;
VAR
  nodo_actual : ^animal;
  pregunta : STRING;
  responde : BOOLEAN;
BEGIN
  writeln;                              { Va pasando por todo el árbol, }
  nodo_actual := raiz;                  { hasta llegar a una respuesta. }
  WHILE (nodo_actual^.izquierdo <> NIL) AND (nodo_actual^.derecho <> NIL) DO
  BEGIN
    write('¿ ');                                     { Hace la pregunta }
    escribe_con_mayuscula(nodo_actual^.pregunta);
    leer(pregunta);
    IF checa_respuesta(pregunta) = SI THEN
      nodo_actual := nodo_actual^.izquierdo
    ELSE IF checa_respuesta(pregunta) = NO THEN
      nodo_actual := nodo_actual^.derecho
  END;
  responde := FALSE;                    { Ahora averigua si adivinó   }
  WHILE NOT responde DO                 { el animal.                  }
  BEGIN
    write('¿ Es ');
    adjetivo(nodo_actual^.pregunta);
    leer(pregunta);
    IF checa_respuesta(pregunta) = SI THEN
    BEGIN
      responde := TRUE;
      writeln;
      write('¡¡¡ Lo adivine !!!, era ');
      adjetivo(nodo_actual^.pregunta);
      writeln;
      writeln
    END
    ELSE IF checa_respuesta(pregunta) = NO THEN
    BEGIN
      responde := TRUE;
      aprende_un_animal(nodo_actual)
    END
  END
END;

{                 --------------------------------                     }
{                  Elimina de la memoria el árbol                      }
{                 --------------------------------                     }
{                                                                      }
{ Usa un procedimiento recursivo para eliminar de la memoria el árbol. }

PROCEDURE limpia_memoria(nodo : ^animal);
BEGIN
  IF nodo <> NIL THEN
  BEGIN
    limpia_memoria(nodo^.izquierdo);
    limpia_memoria(nodo^.derecho);
    dispose(nodo)
  END
END;

{                       --------------------                           }
{                        Final del programa                            }
{                       --------------------                           }
{                                                                      }
{ Muestra  un  mensaje  distinto  dependiendo  de  si el usuario ya ha }
{ jugado. Elimina el arbol de la memoria.                              }

PROCEDURE final;
BEGIN
  writeln;
  IF ya_jugo THEN
    writeln('Gracias por jugar Animales')
  ELSE
    writeln('Intentelo alguna vez');
  writeln;
  limpia_memoria(raiz)
END;

{                    --------------------------                        }
{                     Procedimiento de control                         }
{                    --------------------------                        }

BEGIN
  ya_jugo := FALSE;             { El usuario todavía no ha jugado.     }
  crea_base_de_datos;           { Crea la base de datos inicial.       }
  presentacion;                 { Presentación del programa.           }
  WHILE quiere_jugar DO         { Averigua si el usuario quiere jugar. }
  BEGIN
    hace_preguntas;             { Hace las preguntas.                  }
    ya_jugo := TRUE             { El usuario ya jugo.                  }
  END;
  final                         { Fin del programa.                    }
END.


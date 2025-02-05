{ Programa: Compilador de Pascal para el Inmos T805 }
{                                                   }
{ Módulo  : Mensajes de Errores                     }
{                                                   }
{ (c) Copyright 1993-1994 Oscar Toledo G.           }
{          Todos los derechos reservados.           }

{ Posibles errores léxicos }
PROCEDURE errorlex(error:INTEGER);
VAR contador:INTEGER;
BEGIN
 write(chr(2));
 FOR contador:=1 TO 5+aplin DO
  write(' ');
 writeln('/\');
 write(chr(2));
 CASE error OF
 1: writeln('Cadena demasiado larga');
 2: writeln('Falta el '' de cerrado');
 3: writeln('Número invalido');
 4: writeln('Falta }');
 5: writeln('Falta número hexadecimal');
 6: writeln('Caracter invalido');
 7: writeln('Falta el " de cerrado')
 END;
END;

{ Posibles errores sintácticos y semanticos }
PROCEDURE error(errnum:INTEGER);
VAR
 contador:INTEGER;
BEGIN
 write(chr(2));
 FOR contador:=1 TO 5+aplin DO write(' ');
 writeln('/\');
 write(chr(2));
 CASE errnum OF
   1: writeln('Etiqueta inexistente');
   2: writeln('Falta constante');
   3: writeln('Falta ''=''');
   4: writeln('Falta identificador');
   5: writeln('Falta '':'' o '';''');
   6: writeln('No computable');
   7: writeln('Franja invalida');
   8: writeln('No se puede usar subscripto');
   9: writeln('Falta ''.''');
  10: writeln('Falta '';''');
  11: writeln('Identificador no declarado');
  12: writeln('Identificador ilegal');
  13: writeln('Falta '':=''');
  14: writeln('Tipos incompatibles');
  15: writeln('El mínimo es mayor que el máximo');
  16: writeln('Falta ''THEN''');
  17: writeln('Falta '';'' o ''END''');
  18: writeln('Falta ''DO''');
  19: writeln('Simbolo incorrecto');
  20: writeln('Falta operador relacional');
  21: writeln('Uso de procedimiento en expresión');
  22: writeln('Falta '')''');
  23: writeln('Factor ilegal');
  24: writeln('La variable no es apuntador');
  25: writeln('Falta ''BEGIN''');
  26: writeln('Falta ''OF''');
  27: writeln('Identificador redefinido');
  28: writeln('Falta ''TO'' o ''DOWNTO''');
  29: writeln('Apuntador a un tipo indefinido');
  30: writeln('Numero fuera de rango');
  31: writeln('Falta ''(''');
  33: writeln('Falta ''[''');
  34: writeln('Falta '']''');
  35: writeln('Numero equivocado de parametros');
  36: writeln('Tipo de dato no reconocido');
  37: writeln('Falta ''PROGRAM''');
  38: writeln('No es matriz ni variable');
  39: writeln('Falta ''..''');
  40: writeln('Variable invalida');
  41: writeln('Paso invalido de argumento');
  42: writeln('Cadena vacia');
  43: writeln('Operador incorrecto');
  44: writeln('Tabla de identificadores excedida');
  45: writeln('Tabla de constantes excedida');
  46: writeln('Tabla de datos excedida');
  47: writeln('Tabla de cadenas excedida');
  48: writeln('Tabla literal de cadenas excedida');
  49: writeln('Expresión demasiado compleja');
  50: writeln('Anidamiento muy profundo');
  51: BEGIN
       writeln('No se pueden poner etiquetas');
       writeln('dentro de un FOR o CASE')
      END;
  52: writeln('Falta ''('' o '';''');
 255: writeln('Error fatal en el compilador')
 END;
 write(chr(3))    { Indica error en compilación }
END;


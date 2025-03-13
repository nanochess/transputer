/*
** Compilador de C para transputer
**
** (c) Copyright 1996 Oscar Toledo G.
**
** Creaci�n: 26 de junio de 1995.
** Revisi�n: 27 de julio de 1995. Agrego comillas a los nombres, gracias a la
**                                nueva ampliaci�n del compilador.
** Revisi�n: 23 de agosto de 1995. Incluyo el camino al directorio /c/.
** Revisi�n: 22 de noviembre de 1995. Incluyo el camino a la unidad c:
*/

#include "c:/c/CCvars.c"    /* Variables y definiciones.           */
#include "c:/c/CCinter.c"   /* Interfaz con el usuario.            */
#include "c:/c/CCanasin.c"  /* An�lisis sint�ctico de alto nivel.  */
#include "c:/c/CCvarios.c"  /* Funciones de soporte.               */
#include "c:/c/CCexpr.c"    /* An�lisis sint�ctico de expresiones. */
#include "c:/c/CCgencod.c"  /* Generador de codigo.                */

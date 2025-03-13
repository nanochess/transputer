/*
** Proporciona la hora.
**
** (c) Copyright 1995 Oscar Toledo G.
**
** Creación: 1o. de julio de 1995.
** Revisión: 29 de octubre de 1995. Se acentua correctamente "miércoles"
** Revisión: 1o. de enero de 1996. Corrección de un error de cálculo del día.
** Revisión: 31 de mayo de 1996. Soporte para el año 2000.
** Revisión: 8 de marzo de 2025. Correcciones.
*/

main() {
  char oscar[6];
  int mes, fecha, an, siglo, dia;

  hora(oscar);
  an = oscar[0];
  siglo = (an > 99) ? 20 : 19;
  an = an % 100;
  mes = oscar[1];
  fecha = oscar[2];
  dia = que_dia(fecha, mes, an + siglo * 100);
  putchar(0x1b);
  putchar(1);
  putchar(15);
  puts("Hoy es ");
  switch(dia) {
    case 1: puts("lunes");
            break;
    case 2: puts("martes");
            break;
    case 3: puts("miércoles");
            break;
    case 4: puts("jueves");
            break;
    case 5: puts("viernes");
            break;
    case 6: puts("sábado");
            break;
    case 0: puts("domingo");
            break;
  }
  puts(" ");
  if(oscar[2] > 9) putchar((oscar[2] / 10) + '0');
  putchar((oscar[2] % 10) + '0');
  if(oscar[2] == 1) puts("o.");
  puts(" de ");
  switch(oscar[1]) {
    case 1: puts("enero");
            break;
    case 2: puts("febrero");
            break;
    case 3: puts("marzo");
            break;
    case 4: puts("abril");
            break;
    case 5: puts("mayo");
            break;
    case 6: puts("junio");
            break;
    case 7: puts("julio");
            break;
    case 8: puts("agosto");
            break;
    case 9: puts("septiembre");
            break;
    case 10: puts("octubre");
            break;
    case 11: puts("noviembre");
            break;
    case 12: puts("diciembre");
            break;
  }
  puts(" de ");
  decimal(siglo);
  decimal(an);
  puts(".\r\nSon las ");
  if(oscar[3] > 9) putchar((oscar[3] / 10) + '0');
  putchar((oscar[3] % 10) + '0');
  putchar(':');
  decimal(oscar[4]);
  putchar(':');
  decimal(oscar[5]);
  puts(" horas.\r\n");
}

decimal(dato) int dato; {
  putchar((dato / 10) + '0');
  putchar((dato % 10) + '0');
}

que_dia(dia, mes, a)
  int dia, mes, a;
{
  int t, c;
  int datos[12];
  int dia_de_la_semana;

  /* https://stackoverflow.com/questions/9847213/how-do-i-get-the-day-of-week-given-a-date */

  datos[0] = 0;  datos[1] = 31; datos[2] = 59; datos[3] = 90;
  datos[4] = 120; datos[5] = 151; datos[6] = 181; datos[7] = 212;
  datos[8] = 243; datos[9] = 273; datos[10] = 304; datos[11] = 334;

  t = (mes > 2) ? 0 : 1;
  c = a - 1700 - t;
  dia_de_la_semana = 5;  /* 01-ene-1700 = Viernes */   
  /* Suma parcial de días entre la fecha actual y 01-ene-1700 */
  dia_de_la_semana += (c + t) * 365;
  /* Corrección de año bisiesto */
  dia_de_la_semana += c / 4 - c / 100 + (c + 100) / 400;
  /* Suma meses y día */
  dia_de_la_semana += datos[mes - 1] + (dia - 1);
  return dia_de_la_semana % 7;
}

/*
** Reloj, para ejecutarse en multitarea.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Familia Toledo 1996.
**
** Creación: 11 de marzo de 1996.
**
** Defectos: No hay modo de terminarlo.
*/

main() {
  char oscar[6];
  char **apuntador;
  char *video;
  char *donde;

  apuntador = 0x80000110;
  video = *apuntador;
  while (1) {
    hora(oscar);
    donde = video + 72 * 2;
    if (oscar[3] > 9)
      *donde = oscar[3] / 10 + '0';
    else
      *donde = ' ';
    donde += 2;
    *donde = oscar[3] % 10 + '0';
    donde += 2;
    *donde = ':';
    donde += 2;
    *donde = oscar[4] / 10 + '0';
    donde += 2;
    *donde = oscar[4] % 10 + '0';
    donde += 2;
    *donde = ':';
    donde += 2;
    *donde = oscar[5] / 10 + '0';
    donde += 2;
    *donde = oscar[5] % 10 + '0';
    espera_tiempo(15625);
  }
}

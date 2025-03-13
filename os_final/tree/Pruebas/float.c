/*
** Test de números reales
**
** por Oscar Toledo G.
** https://nanochess.org/
**
** Creation date: Mar/08/2025.
*/

double sin();

/*
** Main program
*/
int main()
{
  float a = 0.5;
  double b = 1.0;
  int c;

  a = a + b;
  c = a * 100;
  decimal(c);
  putchar('\r');
  putchar('\n');
  for (c = 0; c < 10; c++) {
    b = sin(c * 3.141592 / 180.0) * 1000;
    decimal((int) b);
    putchar('\r');
    putchar('\n');
  }
}

void decimal(c)
  int c;
{
  if (c >= 10)
    decimal(c / 10);
  putchar(c % 10 + '0');
}

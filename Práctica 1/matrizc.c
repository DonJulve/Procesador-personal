#define filas 1000
#define columnas 1000

float x[filas][columnas];
float y[filas][columnas];

int main()
{
  int i,j;

  for (j = 0; j < columnas; j++)
  {
	for (i = 0; i < filas; i++) 
	    y[i][j] = 2*x[i][j];
  }
  return 0;
}

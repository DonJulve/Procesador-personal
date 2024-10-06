#define filas 1000
#define columnas 1000

float x[filas][columnas];
float y[filas][columnas];

int main(){
    for (int j = 0; j < columnas; j++){
        for (int i = 0; i < filas; i++){
	        if(x[i][j] < 0) break;
            else if ((int)(x[i][j]) % 2 == 0) y[i][j] = x[i][j];
            else y[i][j] = x[i][j] + 1.0;
        }
    }
    return 0;
}

#include "cabecera.h"

#define logBTBsize 10
#define logBHTsize 15
//0 = NTfuerte 
//1 = NTdebil
//2 = Tdebil
//3 = Tfuerte


static unsigned int historia=0; //BHR
static char predictor = 'l';

typedef struct BTBentry
{
    unsigned long long tag;
    unsigned long long direccion;
} BTBentry_t;

BTBentry_t BTB[1 << logBTBsize];
char BHT[1 << logBHTsize];

void ini_saltos()
{
   int i;
   int BHTsize = 1 << logBHTsize;

   for (i=0; i < BHTsize; i++) BHT[i] = 2;
}

/****************************************************************/
/******    Funciones BTB                    *********************/
/****************************************************************/
char leerBTB (unsigned long long pc, unsigned long long *destino)
{
  pc = pc >> 2; //Eliminar los 2 bits del final vacíos.
  unsigned long long tag = pc >> logBTBsize;
  unsigned long long indice = pc & ((1 << logBTBsize) - 1);
	if (tag == BTB[indice].tag){
	  *destino = BTB[indice].direccion;
	  return 1;
	}
	return 0;

}

void actualizarBTB (unsigned long long pc, unsigned long long destino)
{
  pc = pc >> 2; //Eliminar los 2 bits del final vacíos.
  unsigned long long tag = pc >> logBTBsize;
  unsigned long long indice = pc & ((1 << logBTBsize) - 1);
  BTB[indice].tag = tag;
  BTB[indice].direccion = destino;
}


/****************************************************************/
/******    Funciones especificas BHT local  *********************/
/****************************************************************/
char leerBHTl (unsigned long long pc)
{
  pc = pc >> 2; //Eliminar los 2 bits del final vacíos.
  unsigned long long indice = pc & ((1 << logBHTsize) - 1);
  if (BHT[indice] > 1) {
    return 1;
  }
  return 0;
}

void actualizarBHTl (unsigned long long pc, char tomado)
{
  pc = pc >> 2; //Eliminar los 2 bits del final vacíos.
  unsigned long long indice = pc & ((1 << logBHTsize) - 1);
  if(tomado){
    if (BHT[indice] < 3) BHT[indice]++;
  }
  else{
    if(BHT[indice] > 0) BHT[indice]--;
  }
}


/****************************************************************/
/******    Funciones especificas BHT global  ********************/
/****************************************************************/
char leerBHTg (unsigned long long pc)
{
  if (BHT[historia] > 1){
		return 1;
	}
	return 0;
}

void actualizarBHTg (unsigned long long pc, char tomado)
{
  if(tomado){
		if (BHT[historia] < 3) BHT[historia]++;
	}
	else{
		if (BHT[historia] > 0) BHT[historia]--;
	}
	historia = ((historia << 1) + tomado) & ((1 << logBHTsize) - 1);
}

/****************************************************************/
/******    Funciones especificas BHT hibrido  ********************/
/****************************************************************/
char leerBHTh (unsigned long long pc)
{
  pc = pc >> 2; //Eliminar los 2 bits del final vacíos.
  unsigned long long indice = pc & ((1 << logBHTsize) - 1);
  if (BHT[indice ^ historia] > 1) {
    return 1;
  }
  return 0;
}

void actualizarBHTh (unsigned long long pc, char tomado)
{
  pc = pc >> 2; //Eliminar los 2 bits del final vacíos.
  unsigned long long indice = ((pc & ((1 << logBHTsize) - 1)) ^ historia);
  if(tomado){
    if (BHT[indice] < 3) BHT[indice]++;
  }
  else{
    if(BHT[indice] > 0) BHT[indice]--;
  }
  historia = ((historia << 1) + tomado) & ((1 << logBHTsize) - 1);
}

/****************************************************************/
/******    Funciones genericas BHT     **************************/
/****************************************************************/
char leerBHT (unsigned long long pc)
{
  switch (predictor)
  {
      case 'l': return(leerBHTl(pc));
      case 'g': return(leerBHTg(pc));
      case 'h': return(leerBHTh(pc));
      default:  printf("ERROR en la variable predictor\n");
                exit(1);
  }
}
void actualizarBHT (unsigned long long pc, char tomado)
{
  switch (predictor){
      case 'l': actualizarBHTl (pc, tomado);
                  break;
      case 'g': actualizarBHTg (pc, tomado);
                  break;
      case 'h': actualizarBHTh (pc, tomado);
                  break;
      default:  printf("ERROR en la variable predictor\n");
                exit(1);
  }
}

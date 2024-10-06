#include "cabecera.h"

#define logBTBsize 10
#define logBHTsize 15


static unsigned int historia=0;
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
   unsigned long long entry, tag;

   entry = (pc >> 2) & ((1<<logBTBsize)-1);
   tag = pc >> (logBTBsize + 2);
   
   if (BTB[entry].tag == tag) {
       *destino = BTB[entry].direccion;
       return 1;
   }
   else return 0;
}

void actualizarBTB (unsigned long long pc, unsigned long long destino)
{
   unsigned long long entry, tag;

   entry = (pc >> 2) & ((1<<logBTBsize)-1);
   tag = pc >> (logBTBsize + 2);

   BTB[entry].direccion = destino;
   BTB[entry].tag = tag;
}


/****************************************************************/
/******    Funciones especificas BHT local  *********************/
/****************************************************************/
char leerBHTl (unsigned long long pc)
{
   unsigned long long entry;

   entry = (pc >> 2) & ((1<<logBHTsize)-1);

   if (BHT[entry] < 2) return 0;
   else return 1;
}

void actualizarBHTl (unsigned long long pc, char tomado)
{
   unsigned long long entry;

   entry = (pc >> 2) & ((1<<logBHTsize)-1);

   if (tomado && BHT[entry] < 3) BHT[entry]++;
   else if (!tomado && BHT[entry] > 0) BHT[entry]--;
}


/****************************************************************/
/******    Funciones especificas BHT global  ********************/
/****************************************************************/
char leerBHTg (unsigned long long pc)
{
   unsigned long long entry;

   entry = historia & ((1<<logBHTsize)-1);

   if (BHT[entry] < 2) return 0;
   else return 1;
}

void actualizarBHTg (unsigned long long pc, char tomado)
{
   unsigned long long entry;

   entry = historia & ((1<<logBHTsize)-1);

   if (tomado && BHT[entry] < 3) BHT[entry]++;
   else if (!tomado && BHT[entry] > 0) BHT[entry]--;

   historia = historia << 1;
   if (tomado) historia++;
}

/****************************************************************/
/******    Funciones especificas BHT hibrido  ********************/
/****************************************************************/
char leerBHTh (unsigned long long pc){
   unsigned long long entry;

   entry = ((pc >> 2) ^ historia) & ((1<<logBHTsize)-1);

   if (BHT[entry] < 2) return 0;
   else return 1;
}

void actualizarBHTh (unsigned long long pc, char tomado)
{
   unsigned long long entry;

   entry = ((pc >> 2) ^ historia) & ((1<<logBHTsize)-1);

   if (tomado && BHT[entry] < 3) BHT[entry]++;
   else if (!tomado && BHT[entry] > 0) BHT[entry]--;

   historia = historia << 1;
   if (tomado) historia++;
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

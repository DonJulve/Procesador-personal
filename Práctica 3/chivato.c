//#define STEP
#define NOSTEP

/* para eliminar chivato definir NOSTEP */
/* para activar chivato definir STEP */
/* no pueden estar definidos los dos al mismo tiempo */

#include "cabecera.h"
#include "cpu.h"

#define LINE_WIDTH 70

extern IREG etapa_Bin, etapa_Pin, etapa_Din;
extern char carga_B, carga_P, carga_D;

extern unsigned long instrucciones;

/*extern void decodifica_ins(), imprime_ins();*/
extern void escribirRob();

FILE *fpch, *fpchin;

void verir(), printir();

char *textco[15] = { "NOP  ", "LOAD ", "STORE", "ARITM", "BRCON", "BRINC", "FLOAT", "OTROS",
"LD2  ", "LD2pr", "LD2po", "LDpre", "LDpos", "STpre", "STpos" };

void chivato(IREG sacaISSUE, unsigned long instrucciones)
{
	unsigned long aux;
	int longitud = 0;
	char linea[40];
	static unsigned long int salta = 0, saltai;
	static char termina = 0, busca = 0, quebusco[20];

#ifdef NOSTEP
	return;
#endif

	if (termina) return;
	if (salta) {salta--; return; }
	if (saltai > instrucciones) return;
	if (busca) {
		if (strcmp(textco[etapa_Bin.co], quebusco) != 0) return;
	}
	busca = 0;

        aux = tiempo;
	longitud = 6; /* "TIME: " -> 6 caracteres */
	while (aux > 0) { longitud++; aux = aux/10; }
	printf("\033[%dC", (LINE_WIDTH - 1) - longitud);
	printf("%sTIME: %lu%s", AMARN, tiempo, RESET);
	printf("\033[%dD", LINE_WIDTH - 1);
	printf("%s        B   CO    rf0  rf1  rf2  rd0  rd1\n%s", AMAR, AMAR, RESET);

	/* imprime el numero de instrucciones alineado a la derecha */	
	aux = instrucciones;
	longitud = 9; /* "FETCHED: " -> 6 caracteres */
	while (aux > 0) { longitud++; aux = aux/10; }
	printf("\033[%dC", (LINE_WIDTH - 1) - longitud);
	printf("%sFETCHED: %lu%s", AMARN, instrucciones, RESET);
	printf("\033[%dD", LINE_WIDTH - 1);

	if (carga_B) {
		printf("%sFETCH: %s    ", AMARN, RESET);
		printir(etapa_Bin);
	}
	else {
		printf("%sFETCH:%s  x  ", AMARN, AMAR);
		printir(etapa_Bin);
		printf("%s", RESET);
	}
	printf("\n");

	if (carga_P) {
		printf("%sPREDC: %s    ", AMARN, RESET);
		printir(etapa_Pin);
	}
	else {
		printf("%sPREDC:%s  x  ", AMARN, AMAR);
		printir(etapa_Pin);
		printf("%s", RESET);
	}
	printf("\n");
		
	if (carga_D) {
		printf("%sDECOD: %s    ", AMARN, RESET);
		printir(etapa_Din);
	}
	else {
		printf("%sDECOD:%s  x  ", AMARN, AMAR);
		printir(etapa_Din);
		printf("%s", RESET);
	}
	printf("\n");

	imprimirIssue(sacaISSUE);			// Imprime Issue
	escribirRegistros();				// Imprime estado registros
	escribirRob(tiempo);				// Imprime el ROB
											
	printf(	"=====================================================================");	

	while(1)
	{
		if (fgets(linea, 40, fpchin) == NULL) return;
		switch(linea[0]) {
			case '\0': return;
			case 't': termina = 1; return;
			case 'q': exit(0);
			case 's': salta  = atol(&linea[1]); return;
			case 'g': saltai = atol(&linea[1]); return;
			case 'i': verir(atoi(&linea[1]), fpch); break;
			case 'b':
				strcpy(quebusco, &linea[1]); /* busca codigos de operacion */
				quebusco[strlen(quebusco)-1] = '\0';
				busca = 1;
				return;
			default: return;
     	}
	}
}


void inichivato()
{
#ifdef STEP
	fpch = fopen("/dev/tty", "w");
	fpchin = fopen("/dev/tty", "r");
#endif

}


void verir(int ir,FILE *fp)
{
  IREG *p;

  switch(ir)
  {
     case 1: p = &etapa_Bin; break;
     default: p = &etapa_Din; break;
  }
  
  if (!p->cd0) p->rd0 = -1;
  if (!p->cd1) p->rd1 = -1;
  if (!p->cf0) p->rf0 = -1;
  if (!p->cf1) p->rf1 = -1;
  if (!p->cf2) p->rf2 = -1;
  
  fprintf(fp,"%s r%d, r%d, r%d r%d r%d r%d -- %x(%lu) %x(%lu)\n",
	textco[p->co], p->rd0, p->rd1, p->rf0, p->rf1, p->rf2, p->pc, p->pc, p->ea, p->ea);
}



void printir(IREG *ir,FILE *fp)
{
    printf("%5s", textco[ir->co]);

    if (ir->co == NOP) return;
 
    if (ir->cf0) printf(" %3d,", ir->rf0);
    else printf("  --,");

    if (ir->cf1) printf(" %3d,", ir->rf1);
    else printf("  --,");

    if (ir->cf2) printf(" %3d,", ir->rf2);
    else printf("  --,");

    if (ir->cd0) printf(" %3d,", ir->rd0);
    else printf("  --,");

    if (ir->cd2) printf(" %3d,", ir->rd2);
    else printf("  --,");

    printf("     ");
}

void printirold(IREG *ir,FILE *fp)
{
    printf("%5s", textco[ir->co]);

    if (ir->cf0) printf(" %2d,", ir->rf0);
    else printf(" --,");

    if (ir->cf1) printf(" %2d,", ir->rf1);
    else printf(" --,");

    if (ir->cf2) printf(" %2d,", ir->rf2);
    else printf(" --,");

    if (ir->cd0) printf(" %2d,", ir->rd0);
    else printf(" --,");

    if (ir->cd1) printf(" %2d,", ir->rd1);
    else printf(" --,");

    printf("     ");
}

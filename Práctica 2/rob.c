#include "cabecera.h"
#include "cpu.h"

#define MAX_ROB 3

static char *textco[8] = { "NOP  ", "LOAD ", "STORE", "ARITM", "BRCON", "BRINC", "FLOAT", "OTROS"};

#define ROJO    "\033[0;31m"
#define RESET   "\033[0;37m"
#define PURPU   "\033[0;35m"
#define CYAN    "\033[0;36m"
#define NEGRO   "\033[0;30m"
#define AMARN   "\033[1;33m"
#define AMAR    "\033[0;33m"
#define BLANN   "\033[1;37m"


typedef struct ROBentry {
	unsigned long terminada;
	IREG instr;  /* solo necesario para chivato */
	uint32_t rd0, rd1;
} ROBentry_t;

ROBentry_t ROB[MAX_ROB];

int ROBfirst = 0, ROBlast = 0, ROBocu = 0;

int ROBadd(IREG IR)
{
	int i;
	if (ROBocu == MAX_ROB) return -1;
	ROBocu++;
	ROB[ROBlast].rd0 = IR.rd0;
	ROB[ROBlast].rd1 = IR.rd1;
	ROB[ROBlast].instr = IR;
	ROB[ROBlast].terminada = 0;
	i = ROBlast;
	ROBlast++;
	if (ROBlast == MAX_ROB) ROBlast = 0;
	return i;
}


/*
int ROBadd(IREG IR){
	if (ROBocu==MAX_ROB) return -1;
	ROBocu++;
	ROBlast++;
	if (ROBlast==MAX_ROB) ROBlast = 0;
	ROB[ROBlast].rd=IR.rd;
	ROB[ROBlast].instr=IR;
	ROB[ROBlast].terminada = 0;
	return ROBlast;
}
*/


void ROBejecuta(int entrada, unsigned long int ciclo)
{
	ROB[entrada].terminada = ciclo;
}

char ROBjubila(unsigned long int ciclo)
{
	if (ROBocu == 0 || ROB[ROBfirst].terminada > ciclo) return 0;
	ROBocu--;
	ROBfirst++;
	if (ROBfirst == MAX_ROB) ROBfirst = 0;
	return 1;
}	


/**********************************************************/
/**
 * Escribe por pantalla la instucción manteniendo color que recibe
 */
void escribirInstruccion(IREG I)
{
	 printf("%s", textco[I.co]);
	 if (I.co == NOP) printf("\n");
	 else 
	 {
		 printf(" rf0: %2d", I.rf0);
		 printf(" rf1: %2d", I.rf1);
		 printf(" rf2: %2d", I.rf2);
 		 printf(" rd0: %2d", I.rd0);
		 printf(" rd1: %2d\n", I.rd1);
	 }
}


/**********************************************************/
void escribirRob(unsigned long int ciclo)
{
	struct ROBentry var;
	int i, cont;
			  
	printf("%s---------------------------%s\n", AMAR , RESET);
	printf("%sROB: %d\t       %sNOlanzada %sENejecucion %sTerminada%s\n",
			AMARN, ROBocu, AMAR, RESET, ROJO, RESET);
	printf("%sPOS Tiempo INSTRUCCION%s\n", AMAR , RESET);

	// Muestra por pantalla cada campo del ROB.
	if (ROBlast == 0) i = MAX_ROB - 1;
	else i = ROBlast - 1;

	for (cont = 0; cont < ROBocu; cont++) {
		var = ROB[i];
		if (i > 0) i--;
		else i = MAX_ROB-1;

		if (var.terminada >= ciclo)		
			printf(  "%2d: %6lu ", i , var.terminada - ciclo);
		else	printf("%s%2d:        ", ROJO,i); 
		escribirInstruccion(var.instr);
		printf("%s", RESET);
	 } 
}


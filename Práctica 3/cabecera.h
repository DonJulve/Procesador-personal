/*	cabecera.h	*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define max(a,b) ((a > b)? a : b)
#define min(a,b) ((a < b)? a : b)

#define ROJO	"\033[0;31m"
#define RESET	"\033[0;37m"
#define PURPU	"\033[0;35m"
#define CYAN	"\033[0;36m"
#define NEGRO	"\033[0;30m"
#define AMARN	"\033[1;33m"	// amarillo negrita
#define AMAR	"\033[0;33m"
#define BLANN	"\033[1;37m"	// blanco negrita

typedef enum {false, true} boolean;

extern unsigned long tiempo;

extern FILE *fpout;
extern void finerr();

/*********************************************/
/*              IMPORTANTE !!!!              */
/* al modificar esta tabla se debe modificar */ 
/* tambien la de chivato.c                   */
/*********************************************/
#define NOP	0       
#define LOAD	1      
#define STORE	2      
#define ARITM	3     
#define BRCON	4     /*  salto condicional  */
#define BRINC	5     /* salto incondicional */     
#define FLOAT	6
#define OTROS	7

// typedef unsigned int uint32;
#define getI(x)		(((((uint32)(x)) << 18)) >> 31)
#define getOP(x)	(((((uint32)(x)))) >> 30)
#define getOP2(x)	(((((uint32)(x)) <<  7)) >> 29)
#define getOP3(x)	(((((uint32)(x)) <<  7)) >> 26)
#define getRD(x)	(((((uint32)(x)) <<  2)) >> 27)
#define getRS1(x)	(((((uint32)(x)) << 13)) >> 27)
#define getRS2(x)	(((((uint32)(x)) << 27)) >> 27)
#define getCOND(x)	(((((uint32)(x)) <<  3)) >> 28)


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


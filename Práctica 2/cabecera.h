/*	cabecera.h	*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define max(a,b) ((a > b)? a : b)
#define min(a,b) ((a < b)? a : b)

typedef enum {false, true} boolean;

extern unsigned long tiempo;

extern FILE *fpout;
extern void finerr();


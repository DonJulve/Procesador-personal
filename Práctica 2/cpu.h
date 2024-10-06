/*	cpu.h	*/
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

/* Tabla de instrucciones */
#define NOP	                0       
#define LOAD	            1      
#define STORE	            2      
#define ARITM	            3     
#define BRCON	            4     /*  salto condicional  */
#define BRINC	            5     /* salto incondicional */     
#define FLOAT	            6
#define OTROS	            7
// PAIR y PRE/POST indexado
#define LOAD_PAIR	        8      
#define LOAD_PAIR_PRE_IDX	9      
#define LOAD_PAIR_POST_IDX	10     
#define LOAD_PRE_IDX	    11     
#define LOAD_POST_IDX	    12     
#define STORE_PRE_IDX	    13     
#define STORE_POST_IDX	    14     
#define NO_USADO UINT32_MAX

typedef struct {
	uint64_t ea, pc;
	uint64_t iw;
    uint32_t co; 
    uint32_t rd0; // Registro destino
    uint32_t rf0, rf1, rf2; // Registro fuentes
    char cd0; // Flag registro usado
    char cf0, cf1, cf2; // Flag registro usado
    char taken;
    // Etapa Pre-Decodificación
    uint32_t rd1, rd2; // Registros destinos para Pre-Decode
    char cd1, cd2; // Flag registro usado
} IREG;

void sim(IREG instr);

void reloj();

void inicpu();

void fincpu();

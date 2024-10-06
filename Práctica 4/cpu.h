/*	cpu.h	*/
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#ifndef CPU_H
#define CPU_H

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


#define REG_ISA 32

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
    unsigned short ROBentry;  
    char mispred;
} IREG;

extern IREG inula;

void sim(IREG instr);

void reloj();

void inicpu();

void fincpu();

extern void ROBinit();
extern int ROBhay();
extern int ROBvacio();
extern int ROBadd(IREG IR, int rd0, int old_rd0);
extern void ROBejecuta(int entrada, unsigned long int ciclo);
extern char ROBjubila(unsigned long int ciclo);
extern void escribirInstruccionIW(IREG I);

extern int IWhay();
extern int IWadd();
extern IREG IWsaca();
extern void imprimirIssue(IREG sacaISSUE);

extern int asigna_RF();
extern void libera_RF();
extern void produce_reg();
extern void pendiente_reg();
extern char Rdisponible();
extern void escribirRegistros();
extern char RFhay(char cd0);

extern void ini_reg();
extern void finiw();
extern void inichivato();
extern void chivato(IREG instr, unsigned long instrucciones);
extern IREG get_instr();

extern char leerBTB (unsigned long long pc, unsigned long long *destino);
extern void actualizarBTB (unsigned long long pc, unsigned long long destino);
extern char leerBHT (unsigned long long pc);
extern void actualizarBHT (unsigned long long pc, char tomado);
#endif

#include "cabecera.h"
#include "cpu.h"

#define MAX_ROB 10

static char *textco[8] = { "NOP  ", "LOAD ", "STORE", "ARITM", "BRCON", "BRINC", "FLOAT", "OTROS"};
int rob_latenciasWR[8] = {1, 2, 2, 1, 1, 1, 5, 1};

#define ROJO    "\033[0;31m"
#define RESET   "\033[0;37m"
#define PURPU   "\033[0;35m"
#define CYAN    "\033[0;36m"
#define NEGRO   "\033[0;30m"
#define AMARN   "\033[1;33m"
#define AMAR    "\033[0;33m"
#define BLANN   "\033[1;37m"


typedef struct ROBentry {
    IREG instr;  /* solo necesario para chivato */
    int rd0; // Registros destinos logicos
    int old_rd0; // Anterior version de los registros destinos
    unsigned long terminada; // Ciclo a partir del cual la instruccion puede ser jubilada
    char estado; // 0 == no lanzada, 1 == ha salido de IW
} ROBentry_t;

ROBentry_t ROB[MAX_ROB];

int ROBin = 0, ROBout = 0, ROBocu = 0;
static IREG jubilada; // Variable necesaria solo para el chivato

void ROBinit()
{
    jubilada.co = NOP;
}

/**********************************************************/
int ROBhay()
{
    /* TODO: sustituir MAX_ROB por parámetro tamaño ROB */
    if (ROBocu == MAX_ROB) return 0;
    return 1;
}   

/**********************************************************/
int ROBvacio()
{
    if (ROBocu == 0) return 1;
    return 0;
}   

int ROBadd(IREG IR, int rd0, int old_rd0)
{
    int i = ROBin;
    if (ROBocu == MAX_ROB) 
    {
        printf ("ERROR en funcion ROBadd *****************************\n");
        exit(1);
    }
    ROBocu++;

    ROB[i].rd0 = rd0;
    ROB[i].old_rd0 = old_rd0;
    ROB[i].instr = IR;
    ROB[i].terminada = 0;
    ROB[i].estado = 0;

    ROBin++;
    if (ROBin == MAX_ROB) ROBin = 0;
    return i;
}

void ROBejecuta(int entrada, unsigned long int ciclo)
{
    ROB[entrada].terminada = ciclo;
    ROB[entrada].estado = 1;
}

char ROBjubila(unsigned long int ciclo)
{
    jubilada.co = NOP; // Solo para el chivato
    if (ROBocu == 0 
        || ROB[ROBout].terminada > ciclo
        || ROB[ROBout].estado == 0) return 0;

    jubilada = ROB[ROBout].instr; // Solo para el chivato

    // Liberamos registros
    if (ROB[ROBout].instr.cd0) libera_RF(ROB[ROBout].old_rd0);

    ROBocu--;
    ROBout++;

    if (ROBout == MAX_ROB) ROBout = 0;
    return 1;
}    

/**********************************************************/
/**
 * Escribe por pantalla la instucción manteniendo color que recibe
 */
void escribirInstruccion(unsigned long int tiempo_ter, IREG I)
{
        if (!I.cd0) I.rd0 = -1;
        if (!I.cf0) I.rf0 = -1;
        if (!I.cf1) I.rf1 = -1;
        if (!I.cf2) I.rf2 = -1;
        
        printf("%s", textco[I.co]);
        
        if (I.co == NOP) printf("\n");
        else 
        {
             printf(" rf0: %2d", I.rf0);
             printf(" rf1: %2d", I.rf1);
             printf(" rf2: %2d", I.rf2);
             printf(" rd0: %2d", I.rd0);
             printf(" termina: %lu\n", tiempo_ter);
        }
}

/**********************************************************/
void escribirRob(unsigned long int ciclo)
{
    ROBentry_t var;
    unsigned long int ciclo_issue;
    int i, cont;
    char hay_wb;

    printf("%s---------------------------------------%s\n", AMAR , RESET);
    printf("%sROB: %2d   %sNOlanzada %sENejecucion %sTerminada%s\n",
                AMARN, ROBocu, AMAR, RESET, ROJO, RESET);
    printf("%sPOS  Etapa  CO    rf0  rf1  rf2  rd0%s\n", AMAR , RESET);

    // Muestra por pantalla cada campo del ROB.
    if (ROBin == 0) i = MAX_ROB - 1;
    else i = ROBin - 1;

    for (cont = 0; cont < ROBocu; cont++) 
    {
        var = ROB[i];
        if (var.instr.cd0) hay_wb = 1;
        else hay_wb = 0;

        if (var.estado == 0) printf("%s%2d:        ", AMAR, i);
        else 
        {
            if (var.terminada > tiempo) 
            {
                ciclo_issue = var.terminada - rob_latenciasWR[var.instr.co] - 2 - hay_wb;
                if (tiempo == ciclo_issue) printf("%2d:     I  ", i); /* ciclo issue */
                else 
                {
                    if (tiempo == ciclo_issue + 1) printf("%2d:     R  ", i); /* ciclo register read */
                    else 
                    {
                        if (((var.terminada  - 1) == tiempo) && hay_wb) printf("%2d:     W  ", i); /* ciclo write back */
                        else printf(  "%2d:     E  ", i);
                    }
                }
            }
            else printf("%s%2d:        ", ROJO, i);
        }
                                                                            
        escribirInstruccion(var.terminada, var.instr);
        printf("%s", RESET);

        if (i > 0) i--;
        else i = MAX_ROB - 1;
    }

    if (jubilada.co != NOP)    
    {
        printf("%s        C  ", ROJO); 
        escribirInstruccion(tiempo, jubilada);
        printf("%s", RESET);
    }
}


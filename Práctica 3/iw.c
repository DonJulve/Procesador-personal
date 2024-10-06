#include "cabecera.h"
#include "registros.h"
#include "cpu.h"

/* Esta implementacion de IW mantiene siempre la instruccion mas vieja
   en su entrada cero y no guarda huecos.
   Si tiene n instrucciones, las almacena en las posiciones 0..n-1
   en orden de programa.
   Cuando saca una instruccion, siempre compacta.
*/

#define MAX_IW 6

extern int latenciasWR[8];

typedef struct IWentry {
    IREG instr;
    int f0, f1, f2;
} IWentry_t;

IWentry_t IW[MAX_IW];

int IWocu = 0;
char *iw_textco[8] = { "NOP  ", "LOAD ", "STORE", "ARITM", "BRCON", "BRINC", "FLOAT", "OTROS"};

static unsigned long cSinLanzar = 0, cLanzarI = 0, cIWvacia = 0;
//Mis contadores añadidos
static unsigned long Fuente_no_disponible = 0, Float_ocupado = 0, BR_no_disponible = 0;

/*******************************************************************/
int IWhay()
{
    /* TODO: sustituir MAX_IW por parámetro tamaño IW */
    if (IWocu == MAX_IW) return 0;
    return 1;
}

/*******************************************************************/
int IWadd(IREG instr)
{
    if (IWocu == MAX_IW) {
        printf("ERROR en funcion IWadd *********************************\n");
        exit(1);
    }

    IW[IWocu].instr = instr;
    IW[IWocu].f0 = instr.rf0;
    IW[IWocu].f1 = instr.rf1;
    IW[IWocu].f2 = instr.rf2;

    // Indicamos con -1 a los registros que no se utilizan en la instruccion
    if (!instr.cf0) IW[IWocu].f0 = -1;
    if (!instr.cf1) IW[IWocu].f1 = -1;
    if (!instr.cf2) IW[IWocu].f2 = -1;

    IWocu++;
    return IWocu;
}

/*******************************************************************/
IREG IWsaca(int wBR, unsigned int aluf)
/* Devuelve siguiente instruccion seleccionada o inula si no hay ninguna
 * disponible */
/* aluf: booleano, camino float libre */
{
    int aux, valido = 0;
    IREG aux2;
	for(int i = 0; i < IWocu; i++){
        
        if (Rdisponible(IW[i].f0) && Rdisponible(IW[i].f1) && Rdisponible(IW[i].f2)){

            if ((IW[i].instr.co == FLOAT && aluf) || IW[i].instr.co != FLOAT){

                if(IW[i].instr.cd0){
                    aux=1<<(latenciasWR[IW[i].instr.co] + 2);
                    if(!(wBR & aux)) valido = 1;
                    else BR_no_disponible++;
                    
                }
                else valido = 1;
                
                if(valido){
                    cLanzarI++;
                    aux2 = IW[i].instr;
                    for(int aux3 = i; aux3 < IWocu - 1; aux3++) IW[aux3]=IW[aux3 + 1];
                    IWocu--;
                    return aux2;
                }
            }
            else Float_ocupado++;
        }
        else Fuente_no_disponible++;
    }
    if(IWocu == 0) cIWvacia++;
    else cSinLanzar++;
    return inula;
}    

/*******************************************************************/
int print_iwocu()
{
    return IWocu;
}


/**
 * Escribe por pantalla la instucció cambiando color registros no disponibles
 */
/*******************************************************************/
void escribirInstruccionIW(IREG I)
{
    printf("%s", RESET);
    printf("%-5s", iw_textco[I.co]);
    if (I.co == NOP) printf("\n");
    else 
    {
        if (!Rdisponible(I.rf0)) printf("%s", PURPU);
        printf("  %2d", I.rf0);
        printf("%s", RESET);

        if (!Rdisponible(I.rf1)) printf("%s", PURPU);
        printf("  %2d", I.rf1);
        printf("%s", RESET);

        if (!Rdisponible(I.rf2)) printf("%s", PURPU);
        printf("  %2d", I.rf2);
        printf("%s", RESET);

        printf("  %2d", I.rd0);
    }
    printf("\n");
}



/**
 * Imprime la ISSUE.
 */
/*******************************************************************/
void imprimirIssue(IREG sacaISSUE)
{
    int i;
    char preparada;
          
    //printf("--------------------------------------\n");
    printf("%sISSUE: %2d instrucciones\n", AMARN, IWocu);
    printf("%sPOS     V   CO    rf0  rf1  rf2  rd0\n%s", AMAR, RESET);

    for (i = IWocu-1; i >= 0; i--) 
    {
        preparada = (Rdisponible(IW[i].f0) &&
                     Rdisponible(IW[i].f1) &&
                     Rdisponible(IW[i].f2));
        printf("%2d:     %d  ",    i, preparada);
        escribirInstruccionIW(IW[i].instr);
    }    
    printf("%sISSUED:    %s", ROJO, RESET);
    escribirInstruccionIW(sacaISSUE);
}

void finiw()
{
    //printf("Ciclos sin lanzar I: %lu\n", cSinLanzar);
    printf("Etapa I, ciclos: con issue %lu, sin issue %lu, IW vacia %lu\n", cLanzarI, cSinLanzar, cIWvacia);
    printf("Motivos por los que no se pudo lanzar instrucciones: %lu veces por registro fuente no estaba disponible,\n", Fuente_no_disponible);
    printf("%lu veces porque el camino float estaba ocupado, %lu veces porque el BR estaba ocupado\n", Float_ocupado, BR_no_disponible);
}

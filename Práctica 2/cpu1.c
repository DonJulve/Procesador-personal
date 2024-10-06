/*    cpu.c    */

// tres caminos
// detecta y para en riesgos estructurales
// latencia de memoria 3 ciclos

#include "cpu.h"
#include "rob.h"
#define max_reg 64

extern void chivato();
extern void inichivato();
extern char get_instr();
extern void etapa_PreDecode();

int latenciasWR[8] = {1, 4, 4, 1, 1, 1, 5, 1};
/* NOP LOAD STORE ARITM BRCON BRINC FLOAT OTROS */

unsigned long int disp_reg[max_reg];
/* disp_reg: es el ultimo ciclo de ejecucion de la 
   productora, ciclo en el que una consumidora en 
   Decode podra avanzar */

IREG inula = {0, 0, 0, NOP, NO_USADO, NO_USADO, NO_USADO, NO_USADO, 0, 0, 0, 0, 0, NO_USADO, NO_USADO, 0, 0};
IREG etapa_Bin, etapa_Pin, etapa_Din, etapa_Ain, etapa_Min, etapa_Ein;
IREG etapa_Bout, etapa_Pout, etapa_Dout, etapa_Aout, etapa_Mout, etapa_Eout;

char carga_B = 1, carga_D = 1, carga_A = 1, carga_M = 1, carga_E = 1, carga_P = 1;

unsigned long int tiempo = 0;
unsigned long int instrucciones = 0;

static unsigned long ccpu = 0, craw = 0, csalto = 0, cpredec = 0;
static unsigned long cfloat = 0, cwBR = 0, cWAW = 0, cROB = 0;
static unsigned long loads = 0, stores = 0, saltos = 0, floats = 0;

static char tomado;
static FILE * fpout;


void sim(IREG instr)
{
    static int ciclos_parada_AF=0;
    static int wBR=0;
    int aux = 0, auxrob = 0, ciclo_commit = 0;        

    /* cargamos la instruccion en etapa_Bin  */
    etapa_Bin = instr;
    instrucciones++;

    if (etapa_Bin.co == LOAD) loads++;
    if (etapa_Bin.co == STORE) stores++;
    if (etapa_Bin.co == FLOAT) floats++;
    if (etapa_Bin.co == BRCON || etapa_Bin.co == BRINC) saltos++;

    do
    {
        /* etapa Decode, implementa SCOREBOARD */
        /************************************************************************************/
        /* deteccion de riesgos y lectura en BR, 
        si no hay problemas la instruccion pasa a A */
        /* Tres caminos: 
        I: nunca para, alu de un ciclo
        M: nunca para, @ y M de 3 ciclos. Segmentada
        F: 5 ciclos no segmentada */

        /* tras este desplazamiento el bit de menor peso de wBR representa ciclo actual */
        wBR=wBR>>1;  

        // Si el contador AF es distinto de 0 la unidad F está ocupada
        if (ciclos_parada_AF>0) ciclos_parada_AF--;

        carga_D = 1;

        /* riesgos RAW: verifica si los reg. fuentes estan preparados */
        // Trabajo por hacer

        if ((etapa_Din.cf0 && disp_reg[etapa_Din.rf0] > tiempo)||
            (etapa_Din.cf1 && disp_reg[etapa_Din.rf1] > tiempo)||
            (etapa_Din.cf2 && disp_reg[etapa_Din.rf2] > tiempo)) { 
                craw++;
                carga_D = 0;
        } 
	    else {
            /* RIESGO ESTRUCTURAL EN FLOAT */
            if (etapa_Din.co == FLOAT && ciclos_parada_AF>0) {
                cfloat++;
                carga_D = 0;
            }
            else {
                /* RIESGO ESTRUCTURAL EN BR */
                if (etapa_Din.cd0) {
                    aux=1<<(latenciasWR[etapa_Din.co]+1);
                    if(wBR & aux) {
                        cwBR++;
                        carga_D = 0;
                    } 
		            else {
                        // deteccion y parada RIESGO WAW
                        if (disp_reg[etapa_Din.rd0] > tiempo) {
                            cWAW++;
                            carga_D = 0;
                        }
                    }
                }
            }
        }

        if(carga_D) {              
            /* lanzo la inst. por el camino correspondiente */
            if (etapa_Din.co == FLOAT) ciclos_parada_AF=5; /* para una latencia de 5 */
            
            /* si escribe un registro ocupo wBR */
            if (etapa_Din.cd0) {
                 //Representa la ocupación en el banco de registros
                 wBR=wBR | aux;
                 disp_reg[etapa_Din.rd0] = tiempo + latenciasWR[etapa_Din.co];
            }
        }   

        /* etapa Pre-decodificacion */
	/* desdobla las instrucciones ld/st dobles y con pre/pos incremento/decremento */
	/* ademas, si detecta salto tomado para a la etapa B mediante la variable tomado */

        carga_P = carga_D;
        tomado = 0;
        if (carga_P) {
            if ((etapa_Pin.co == BRCON && etapa_Pin.taken == 1) || (etapa_Pin.co == BRINC)) {
                tomado = 1;
            }
            etapa_PreDecode();
	    if (!carga_P) cpredec++;
        }
        
        /* etapa Busqueda: si no hay problemas la instruccion pasa a D */
        /* nada que simular en esta etapa */    

        if ( tomado ) {
             etapa_Bout = inula;
             carga_B = 0; 
            /* Solo contamos si no hemos parado la carga por algo previamente
               para cuadrar los ciclos de parada */
             if (carga_P) csalto++;
        } 
	else {
            etapa_Bout = etapa_Bin;
            carga_B = carga_P;
        }
        
        if (carga_B) ccpu++;

        reloj();

    } while(!carga_B);
}

void reloj()
{
    if (carga_P) etapa_Pin = etapa_Bout;
    if (carga_D) etapa_Din = etapa_Pout;
    if (carga_A) etapa_Ain = etapa_Dout;
    if (carga_M) etapa_Min = etapa_Aout;
    if (carga_E) etapa_Ein = etapa_Mout;
    tiempo++;
    chivato();
}

void inicpu()
{
    int i;
    fpout = fopen("/dev/tty","w");

    etapa_Ein = inula;
    etapa_Eout = inula;
    etapa_Min = inula;
    etapa_Mout = inula;
    etapa_Ain = inula;
    etapa_Aout = inula;
    etapa_Din = inula;
    etapa_Dout = inula;
    etapa_Bout = inula;
    inichivato();
}

void fincpu()
{
    unsigned long todocpu;

    fprintf(fpout, "CPI: %lu inst. %lu ciclos %2.2f ciclos/inst.\n",
        instrucciones, tiempo, tiempo/(float)instrucciones);

    todocpu =  ccpu + craw + cfloat + cwBR + cROB + csalto + cpredec;
    fprintf(fpout, "ciclos CPU: %lu cpu %lu PREDEC %lu RAW %lu WAW %lu FLOAT %lu wBR %lu ROB %lu Saltos\t(Total: %lu)\n",
        ccpu, cpredec, craw, cWAW, cfloat, cwBR, cROB, csalto, todocpu);

    fprintf(fpout, "%lu loads %lu stores %lu floats %lu saltos\n", loads, stores, floats, saltos);
}

/*	cpu.c	*/

/* trata correctamente rd en STORE como fuente */

#include "cpu.h"

extern void chivato();
extern void inichivato();
extern char get_instr();
extern void etapa_PreDecode();

IREG inula = {0, 0, 0, NOP, NO_USADO, NO_USADO, NO_USADO, NO_USADO, 0, 0, 0, 0, 0, NO_USADO, NO_USADO, 0, 0};
IREG etapa_Bin, etapa_Pin, etapa_Din, etapa_Ain, etapa_Min, etapa_Ein;
IREG etapa_Bout, etapa_Pout, etapa_Dout, etapa_Aout, etapa_Mout, etapa_Eout;

char carga_B = 1, carga_D = 1, carga_A = 1, carga_M = 1, carga_E = 1, carga_P = 1;

unsigned long int tiempo = 0;
unsigned long int instrucciones = 0;

unsigned long int cortos1=0,cortos2=0,cortos3=0,banco=0,total_cortos=0;

static unsigned long ccpu = 0, craw = 0, csalto = 0, cpredec = 0;
static unsigned long cfloat = 0;
static unsigned long loads = 0, stores = 0, saltos = 0, floats = 0;

static char tomado;
static FILE * fpout;


void sim(IREG instr)
{
    /* cargamos la instruccion en etapa_Bin  */
    etapa_Bin = instr;
    instrucciones++;
    
    if (etapa_Bin.co == LOAD) loads++;
    if (etapa_Bin.co == STORE) stores++;
    if (etapa_Bin.co == FLOAT) floats++;
    if (etapa_Bin.co == BRCON || etapa_Bin.co == BRINC) saltos++;

    do
    {
        /* etapa Escritura en BR */
        /* nada que simular en esta etapa */	
        etapa_Eout = etapa_Ein;
        carga_E = 1;

        /* etapa Memoria: si no hay problemas la instruccion pasa a E */
        /* nada que simular en esta etapa */	
        etapa_Mout = etapa_Min;
        carga_M = 1;

        /* etapa Alu: si no hay problemas la instruccion pasa a M */
	/* nada que simular en esta etapa */
        etapa_Aout = etapa_Ain;
        carga_A = 1;

	/* etapa Decode: si no hay problemas la instruccion pasa a A */
	/* a partir de pre-decode solo puede haber un rd valido, el rd0 */
        if (etapa_Ain.co == LOAD && 
                  ((etapa_Din.cf0 && etapa_Din.rf0 == etapa_Ain.rd0)
                || (etapa_Din.cf1 && etapa_Din.rf1 == etapa_Ain.rd0)
                || (etapa_Din.cf2 && etapa_Din.rf2 == etapa_Ain.rd0)))
        {
           etapa_Dout = inula;
           craw++;
           carga_D = 0;
        }
        else {                     
            etapa_Dout = etapa_Din;
            carga_D = 1;
            
            //Registro 1
            if (etapa_Din.cf0){
                if(etapa_Din.rf0 == etapa_Ain.rd0 && etapa_Ain.cd0){
                    cortos1++;
                }
                else if(etapa_Din.rf0 == etapa_Min.rd0 && etapa_Min.cd0){
                    cortos2++;
                }
                else if(etapa_Din.rf0 == etapa_Ein.rd0 && etapa_Ein.cd0){
                    cortos3++;
                }
                else{
                    banco++;
                }
            }

            //Registro 2
            if (etapa_Din.cf1){
                if(etapa_Din.rf1 == etapa_Ain.rd0 && etapa_Ain.cd0){
                    cortos1++;
                }
                else if(etapa_Din.rf1 == etapa_Min.rd0 && etapa_Min.cd0){
                    cortos2++;
                }
                else if(etapa_Din.rf1 == etapa_Ein.rd0 && etapa_Ein.cd0){
                    cortos3++;
                }
                else{
                    banco++;
                }
            }

            //Registro 3
            if (etapa_Din.cf2){
                if(etapa_Din.rf2 == etapa_Ain.rd0 && etapa_Ain.cd0){
                    cortos1++;
                }
                else if(etapa_Din.rf2 == etapa_Min.rd0 && etapa_Min.cd0){
                    cortos2++;
                }
                else if(etapa_Din.rf2 == etapa_Ein.rd0 && etapa_Ein.cd0){
                    cortos3++;
                }
                else{
                    banco++;
                }
            }
            
        }

        
            
        

        /* etapa Pre-decodificacion */
	/* desdobla las instrucciones ld/st dobles y con pre/pos incremento/decremento */
	/* ademas, si detecta salto tomado para a la etapa B mediante la variable tomado */

        carga_P = carga_D;
        tomado = 0;
        if (carga_P)
        {
            if ((etapa_Pin.co == BRCON && etapa_Pin.taken == 1) 
                || (etapa_Pin.co == BRINC))
            {
                tomado = 1;
            }

            etapa_PreDecode();
	    if (!carga_P) cpredec++;
        }
        
        /* etapa Busqueda: si no hay problemas la instruccion pasa a D */
        /* nada que simular en esta etapa */	

        if ( tomado ) 
        {
             etapa_Bout = inula;
             carga_B = 0; 
            /* Solo contamos si no hemos parado la carga por algo previamente
               para cuadrar los ciclos de parada */
             if (carga_P) csalto++;
        } else 
        {            
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
    //chivato();
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
    //inichivato();
}

void fincpu()
{
	unsigned long todocpu;
  total_cortos = cortos1 + cortos2 + cortos3 + banco;

    fprintf(fpout, "CPI: %lu inst. %lu ciclos %2.2f ciclos/inst.\n",
	    instrucciones, tiempo, tiempo/(float)instrucciones);

    todocpu =  ccpu + craw + cfloat + csalto + cpredec;
    fprintf(fpout, "Ciclos CPU: %lu cpu %lu PREDEC %lu RAW %lu FLOAT %lu SALTOS\t(Total: %lu)\n",
	    ccpu, cpredec, craw, cfloat, csalto, todocpu);

    fprintf(fpout, "%lu c1 %lu c2 %lu c3 %lu banco de registros\n", cortos1, 
        cortos2, cortos3, banco);
    
    fprintf(fpout, "%.2lf c1 %.2lf c2 %.2lf c3 %.2lf banco de registros\n", ((double)cortos1/total_cortos)*100, 
            ((double)cortos2/total_cortos)*100, ((double)cortos3/total_cortos)*100, ((double)banco/total_cortos)*100);

    fprintf(fpout, "%lu loads %lu stores %lu floats %lu saltos\n", loads, 
        stores, floats, saltos);
}

/*	cpu.c	*/

/* trata correctamente rd en STORE como fuente */

#include "cpu.h"

extern void chivato();
extern void inichivato();
extern char get_instr();
extern void etapa_PreDecode();

IREG inula = {0, 0, 0, NOP, NO_USADO, NO_USADO, NO_USADO, NO_USADO, 0, 0, 0, 0, 0, NO_USADO, NO_USADO, 0, 0};
IREG etapa_Bin, etapa_Pin, etapa_Din, etapa_Ain, etapa_Min, etapa_Ein, etapa_Floin, etapa_Memin;
IREG etapa_Bout, etapa_Pout, etapa_Dout, etapa_Aout, etapa_Mout, etapa_Eout, etapa_Floout, etapa_Memout;

char carga_B = 1, carga_D = 1, carga_A = 1, carga_M = 1, carga_E = 1, carga_P = 1 , carga_Flo = 1, carga_Mem = 1;

unsigned long int tiempo = 0;
unsigned long int instrucciones = 0;

unsigned long int cortos1=0,cortos2=0,cortos3=0,banco=0;

static unsigned long ccpu = 0, craw = 0, csalto = 0, cpredec = 0;
static unsigned long cfloat = 0, cBR = 0;
static unsigned long loads = 0, stores = 0, saltos = 0, floats = 0;

static unsigned char scoreboard[5] = {'0', '0', '0', '0', '0'};

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

        //Camino ALU
        etapa_Aout = etapa_Ain;
        carga_A = 1;


        //Camino Float
        if ((scoreboard[0] != 'F') && (scoreboard[1] != 'F') && (scoreboard[2] != 'F') && (scoreboard[3] != 'F')){
			etapa_Floout = etapa_Floin;
			carga_Flo = 1;
		}
		else { // Esta ocupada
			etapa_Floout = inula;
			carga_Flo = 0;
		}

        //Camino Memoria
        etapa_Memout = etapa_Memin;
		carga_Mem = 1;


	/* etapa Decode: si no hay problemas la instruccion pasa a A */
	/* a partir de pre-decode solo puede haber un rd valido, el rd0 */
        if (etapa_Memin.co == LOAD && 
                  ((etapa_Din.cf0 && etapa_Din.rf0 == etapa_Memin.rd0)
                || (etapa_Din.cf1 && etapa_Din.rf1 == etapa_Memin.rd0)
                || (etapa_Din.cf2 && etapa_Din.rf2 == etapa_Memin.rd0)))
        {
           etapa_Dout = inula;
           craw++;
           carga_D = 0;
        }
        else if (etapa_Din.co == FLOAT && (scoreboard[0] == 'F' || scoreboard[1] == 'F' || scoreboard[2] == 'F' || scoreboard[3] == 'F')) { 
			etapa_Dout = inula;
	        cfloat++;
	        carga_D = 0;
		}
        else if((etapa_Din.co == ARITM && etapa_Din.cd0) && (etapa_Memin.co == LOAD || scoreboard[3] == 'F')
	            || (etapa_Din.co == LOAD && scoreboard[2]=='F')){

	        etapa_Dout = inula;
	        cBR++;
	   	 	carga_D = 0;
		}
        else {                     
            etapa_Dout = etapa_Din;
            carga_D = carga_A;
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
    for (int i = 4; i > 0; i--){
		scoreboard[i] = scoreboard[i-1];
	}
	scoreboard[0] = '0';

    if (carga_P) etapa_Pin = etapa_Bout;
    if (carga_D) etapa_Din = etapa_Pout;

    if (carga_A && etapa_Dout.co != STORE && etapa_Dout.co != LOAD && etapa_Dout.co != FLOAT){
    	etapa_Ain = etapa_Dout; 
        scoreboard[0] = 'A';
    }
    else etapa_Ain = inula;

    if (carga_Flo && etapa_Dout.co == FLOAT) {
    	scoreboard[0] = 'F';
    	etapa_Floin = etapa_Dout;
    }
    else etapa_Floin = inula;

    if (carga_Mem && etapa_Dout.co == LOAD){
    	scoreboard[0] = 'L';
    	etapa_Memin = etapa_Dout;
    }
    else if (carga_Mem && etapa_Dout.co == STORE){
    	scoreboard[0] = 'S';
    	etapa_Memin = etapa_Dout;
    }
    else etapa_Memin = inula;

    if (carga_M) etapa_Min = etapa_Memout;

    if (carga_E){
    	if (scoreboard[4] == 'A'){ //Escribe en BR de ALU
            etapa_Ein = etapa_Aout;
        } 
        else if(scoreboard[4] == 'L'){ //Escribe en BR de MEM
            etapa_Ein = etapa_Memout;
        }
        else if(scoreboard[4] == 'F'){ //Escribe en BR de FLOAT
            etapa_Ein = etapa_Floout;
        } 
        else {
            etapa_Ein = inula;
        }
    }
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

    etapa_Floin = inula;
    etapa_Floout = inula;
    etapa_Memin = inula;
    etapa_Memout = inula;
    //inichivato();
}

void fincpu()
{
	unsigned long todocpu;

    fprintf(fpout, "CPI: %lu inst. %lu ciclos %2.2f ciclos/inst.\n",
	    instrucciones, tiempo, tiempo/(float)instrucciones);

    todocpu =  ccpu + craw + cfloat + csalto + cpredec + cBR;
    fprintf(fpout, "Ciclos CPU: %lu cpu %lu PREDEC %lu RAW %lu BR %lu FLOAT %lu SALTOS\t(Total: %lu)\n",
	    ccpu, cpredec, craw, cBR, cfloat, csalto, todocpu);

    fprintf(fpout, "%lu loads %lu stores %lu floats %lu saltos\n", loads, 
        stores, floats, saltos);
}

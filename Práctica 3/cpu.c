/*    cpu.c    */

#include "cpu.h"
#include "rob.h"

extern void etapa_PreDecode();

int latenciasWR[8] = {1, 4, 4, 1, 1, 1, 5, 1};
/* NOP LOAD STORE ARITM BRCON BRINC FLOAT OTROS */

int TRen[REG_ISA];
/* Table de renombre */

IREG inula = {0, 0, 0, NOP, NO_USADO, NO_USADO, NO_USADO, NO_USADO, 0, 0, 0, 0, 0, NO_USADO, NO_USADO, 0, 0};
IREG etapa_Bin, etapa_Pin, etapa_Din, etapa_Ain, etapa_Min, etapa_Ein;
IREG etapa_Bout, etapa_Pout, etapa_Dout, etapa_Aout, etapa_Mout, etapa_Eout;

char carga_B = 1, carga_D = 1, carga_A = 1, carga_M = 1, carga_E = 1, carga_P = 1, carga_AF = 1;

unsigned long int tiempo = 0;
unsigned long int instrucciones = 0;

static unsigned long ccpu = 0, craw = 0, csalto = 0, cpredec = 0;
static unsigned long cfloat = 0, cwBR = 0, cWAW = 0, cROB = 0;
static unsigned long cIW = 0, creg = 0, cbloqueoD = 0, cconD = 0, cconD_NOP = 0;
static unsigned long cSinLanzarI = 0;
static unsigned long loads = 0, stores = 0, saltos = 0, floats = 0;
static unsigned long jubiladas = 0;

static FILE * fpout;


void sim(IREG instr)
{
    static int ciclos_parada_AF=0;
    static int wBR=0;
    char hay_salto = 0;
    char tomado;

    /* cargamos la instruccion en etapa_Bin  */
    etapa_Bin = instr;
    instrucciones++;

    if (etapa_Bin.co == LOAD) loads++;
    if (etapa_Bin.co == STORE) stores++;
    if (etapa_Bin.co == FLOAT) floats++;
    if (etapa_Bin.co == BRCON || etapa_Bin.co == BRINC) saltos++;
    
    do
    {
        int aux = 0, hay_wb = 0, auxrob = 0, ciclo_commit = 0;        
        /* etapa consolidacion */
        /**********************************************************************/
        if (ROBjubila(tiempo)) jubiladas++;
                
        /* etapa Escritura en BR */
        /* nada que simular en esta etapa */    

        wBR=wBR>>1;   
        /* tras este desplazamiento el bit de menor peso de wBR representa ciclo actual */
        
        /* etapa Alu */
        /**********************************************************************/
        /* Tres caminos:
            I: nunca para, alu de un ciclo
            M: nunca para, @ y M de 3 ciclos, totalmente segmentada
            F: 5 ciclos no segmentada */
        /* nada que simular en esta etapa */
        
        /* etapa READ: lectura del banco de registros */
        /**********************************************************************/

        /* etapa ISSUE: deteccion de riesgos RAW, estructurales */
        /**********************************************************************/

        /* actualizar contador parada en FLOAT */    
        if (ciclos_parada_AF > 0) ciclos_parada_AF--;
        if (ciclos_parada_AF == 0) carga_AF = 1;
        else carga_AF = 0;

        instr = IWsaca(wBR, carga_AF); // Sacamos instruccion de la ventana

        if (instr.co != NOP)
        {
            if (instr.co == FLOAT) ciclos_parada_AF = latenciasWR[FLOAT];

            // ¿Escribimos en BR?
            if (instr.cd0) hay_wb = 1;
            else hay_wb = 0;

            // Calculo del ciclo commit (Latencia + etapa WB + etapa R + etapa C)
            ciclo_commit = tiempo + latenciasWR[instr.co] + 2 + hay_wb;
            ROBejecuta(instr.ROBentry, ciclo_commit);

            if (instr.cd0) 
            {
                // Ocupamos el puerto de escritura del banco de registros
                aux = 1 << (latenciasWR[instr.co] + 2);
                wBR = wBR | aux;

                // Marcamos cuando estará disponible el regisrot usando
                // cortocircuito
                produce_reg(instr.rd0, tiempo + latenciasWR[instr.co]);
            }
        } else cSinLanzarI++;
        
        /* etapa Decode: Renombre e insercion en ROB e IW */ 
        /**********************************************************************/
        carga_D = 0;
        if (etapa_Din.co == NOP)
        {
            carga_D = 1;
	    cconD_NOP++;
        } else
        {
            if (ROBhay() && IWhay() && RFhay(etapa_Din.cd0)) 
            // Hay recursos disponibles en ROB, ventana de instrucciones y banco de
            // registros fisicos
            {
                int old_rd0 = -1;
                
                // Renombramos registros fuente
                if (etapa_Din.cf0) etapa_Din.rf0 = TRen[etapa_Din.rf0];
                if (etapa_Din.cf1) etapa_Din.rf1 = TRen[etapa_Din.rf1];
                if (etapa_Din.cf2) etapa_Din.rf2 = TRen[etapa_Din.rf2];

                if (!etapa_Din.cd0) etapa_Din.rd0 = -1; // La instrucción no tiene regstro destino (rd0)
                else
                {
                    // Se hace renombre para el registro rd0
                    old_rd0 = TRen[etapa_Din.rd0];
                    TRen[etapa_Din.rd0] = asigna_RF();
                    etapa_Din.rd0 = TRen[etapa_Din.rd0];
                }

                // Anyadimos al ROB
                etapa_Din.ROBentry = ROBadd(etapa_Din, etapa_Din.rd0, old_rd0);

                // Anyadimos a la ventana de instrucciones
                IWadd(etapa_Din);

                // Marcamos los registros destino como pendientes de 
                // obtener su valor
                if (etapa_Din.cd0) pendiente_reg(etapa_Din.rd0);

                carga_D = 1;
		cconD++;
            } else 
            {
                // No se ha podido continuar
                cbloqueoD++;
                // Indicamos por que no se ha podido continuar la ejecucion
                // Un mismo ciclo puede generar dos riesgos diferentes.
                // TODO: modificar para que solo pueda generar un riesgo de forma
                // correcta.
                if (!ROBhay()) cROB++;
                if (!IWhay()) cIW++;
                if (!RFhay(etapa_Din.cd0)) creg++;
            }
        }
        carga_P = carga_D;
        if (carga_P) {
		etapa_PreDecode();
		if (!carga_P) cpredec++;
	}
               
        /* etapa Busqueda: si no hay problemas la instruccion pasa a D */
        /* nada que simular en esta etapa */ 
        
        /* Para esta practica el simulador tiene predictor de salto ideal,
           nunca falla */      
        etapa_Bout = etapa_Bin;
        carga_B = carga_P;

        if (carga_B) ccpu++;    

        // Llamamos al chivato
        chivato(instr, instrucciones);
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
    ini_reg();

    for (i = 0; i < REG_ISA; i++) TRen[i] = i;
}

void fincpu()
{
    fprintf(fpout, "CPI: %lu inst. %lu jubiladas %lu ciclos %2.2f ciclos/inst.\n",
        instrucciones, jubiladas, tiempo, tiempo/(float)instrucciones);

    fprintf(fpout, "ciclos frontend: %lu fetch %lu PREDEC \n", ccpu, cpredec);
    fprintf(fpout, "Etapa D, ciclos: con decode %lu, %lu NOPs, sin decode %lu\n", cconD, cconD_NOP, cbloqueoD);
    fprintf(fpout, "\t%lu IWllena, %lu ROBlleno, %lu NoRegistros\n", cIW, cROB, creg);

    finiw();

    fprintf(fpout, "%lu loads %lu stores %lu floats %lu saltos\n", loads, stores, floats, saltos);
}

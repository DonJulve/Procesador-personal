#define NOSTEP

/* para eliminar chivato definir NOSTEP */
/* para activar chivato definir STEP */
/* no pueden estar definidos los dos al mismo tiempo */

#include "cabecera.h"
#include "cpu.h"

extern IREG etapa_Bin, etapa_Din;

extern unsigned long instrucciones;

/*extern void decodifica_ins(), imprime_ins();*/
extern void escribirRob();

FILE *fpch, *fpchin;

void verir(), printir();

char *textco[8] = { "NOP  ", "LOAD ", "STORE", "ARITM", "BRCON", "BRINC", "FLOAT", "OTROS"};


void chivato()
{
    char dosreg = ' ';
    char linea[40];
    static unsigned long int salta = 0, saltai;
    static char termina = 0, busca = 0, quebusco[20];

#ifdef NOSTEP
    return;
#endif

    if (termina) return;
    if (salta) {salta--; return; }
    if (saltai > instrucciones) return;
    if (busca) {
        if (strcmp(textco[etapa_Bin.co], quebusco) != 0) return;
    }
    busca = 0;

    fprintf(fpch, "ciclo %lu \tinstruccion %lu\n", tiempo, instrucciones);
    printir(&etapa_Bin, fpch);
    printir(&etapa_Din, fpch);
    fprintf(fpch,"\n");

    escribirRob(tiempo);

/******************************************/

    while(1)
    {
        if (fgets(linea, 40, fpchin) == NULL) return;
        switch(linea[0]) {
            case '\0': return;
            case 't': termina = 1; return;
            case 'q': exit(0);
            case 's': salta  = atol(&linea[1]); return;
            case 'g': saltai = atol(&linea[1]); return;
            case 'i': verir(atoi(&linea[1]), fpch); break;
            case 'b':
                strcpy(quebusco, &linea[1]); /* busca codigos de operacion */
                quebusco[strlen(quebusco)-1] = '\0';
                busca = 1;
                return;
            default: return;
         }
    }
}


void inichivato()
{
#ifdef STEP
    fpch = fopen("/dev/tty", "w");
    fpchin = fopen("/dev/tty", "r");
#endif

}


void verir(int ir,FILE *fp)
{
  IREG *p;

  switch(ir)
  {
     case 1: p = &etapa_Bin; break;
     default: p = &etapa_Din; break;
  }
  fprintf(fp,"%s r%d, r%d r%d r%d r%d -- %x(%lu) %x(%lu)\n",
    textco[p->co], p->rd0, p->rf0, p->rf1, p->rf2, p->pc, p->pc, p->ea, p->ea);
}

void printir(IREG *ir,FILE *fp)
{
    fprintf(fp, "%5s", textco[ir->co]);

    if (ir->cf0) fprintf(fp, " %2d,", ir->rf0);
    else fprintf(fp," --,");

    if (ir->cf1) fprintf(fp, " %2d,", ir->rf1);
    else fprintf(fp," --,");

    if (ir->cf2) fprintf(fp, " %2d,", ir->rf2);
    else fprintf(fp," --,");

    if (ir->cd0) fprintf(fp, " %2d,", ir->rd0);
    else fprintf(fp," --,");

    fprintf(fp,"     ");
}

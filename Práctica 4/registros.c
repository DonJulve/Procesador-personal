#include "cabecera.h"
#include "registros.h"
#include "cpu.h"

extern unsigned long tiempo;

ReadyReg_t disp_reg[max_reg];    /* ciclo a partir del cual una inst. dependiente
                                   podrá hacer issue */
int Reg_usados;

int asigna_RF()
{
    int i;

    if (Reg_usados == max_reg) return -1;

    for (i = 0; i < max_reg; i++) 
    {
        if (disp_reg[i].usado == 0) 
        {
            // Devolvemos el primer registro no utilizado
            disp_reg[i].usado = 1;
            disp_reg[i].pendiente_issue = 1;
            disp_reg[i].ciclo = 0;
            Reg_usados++;
            return i;
        }
    }

    printf("ERROR en funcion asigna_RF *********************************\n");
    exit(1);
}

char RFhay(char cd0)
{
    // Devuelve 1 si hay suficientes registros fisicos para satisfacer el 
    // renombre de registros
    if (!cd0) return 1;
    if (cd0 && Reg_usados < max_reg) return 1;
    return 0;
}

void libera_RF(int reg)
{
    // El registro vuelve a estar disponible para su uso
    disp_reg[reg].usado = 0;
    disp_reg[reg].pendiente_issue = 0;
    disp_reg[reg].ciclo = 0;
    Reg_usados--;
}

void produce_reg(int reg, unsigned long ciclo)
{
    // Indicamos el ciclo en el cual el registro sera producido
    disp_reg[reg].ciclo = ciclo;
    disp_reg[reg].pendiente_issue = 0;
}

void pendiente_reg(int reg)
{
    disp_reg[reg].pendiente_issue = 1;
}

char Rdisponible(int i)
{
    return (i<0 
            || (disp_reg[i].ciclo <= tiempo && !disp_reg[i].pendiente_issue));
}

void ini_reg()
{
    int i;

    for (i = 0; i < max_reg; i++) 
    {
        if (i >= 0 && i < REG_ISA) disp_reg[i].usado = 1;
        else disp_reg[i].usado = 0;
        disp_reg[i].ciclo = 0;
        disp_reg[i].pendiente_issue = 0;
    }
    Reg_usados = REG_ISA;
}

/**
 * Escribe el valor entero destacandolo en caso de que valor sea mayor que 0.
 */
void pintar(int num, char usado, int valor, char pend_issue)
{
    if (!usado) printf("%s%2d->%3d  %s", AMAR, num, valor, RESET);
    else 
    {
        if (valor > 0 || pend_issue)
        {
            printf("%s%2d->%3d  %s", ROJO, num, valor, RESET);
        } else printf("%2d->%3d  ", num, valor);
    }
}


/**
 * Realiza la escritura de todos los registros.
 */
void escribirRegistros()
{
    int i, j, queda = 0;
    printf("%s Registros\n", RESET);

    for (j = 0; j< max_reg; j += 8) 
    {
        for (i = j; i < j + 8 && i < max_reg; i++) 
        {
            queda = disp_reg[i].ciclo - tiempo;
            if (queda < 0) queda = 0;
            pintar(i, disp_reg[i].usado, queda, disp_reg[i].pendiente_issue);
        }
        printf("\n");
    }

}


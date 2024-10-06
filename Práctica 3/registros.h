#define max_reg 96

typedef struct ReadyReg {
	unsigned long ciclo;   /* ciclo de ultima actualizacion de cada reg */
					/* si es menos que tiempo, esta pendiente de escribir */
	char usado;
	char pendiente_issue;
} ReadyReg_t;

extern int asigna_RF();

extern void libera_RF();

extern void produce_reg();

extern void pendiente_reg();

extern char Rdisponible();

extern void ini_reg();

//extern void pintar();

extern void escribirRegistros();


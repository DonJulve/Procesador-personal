#include <stdio.h>
#define SETS 64  /* no se pueden cambiar sin cambiar codigo */
#define ASOC 2

typedef struct CACHEblock {
    unsigned long long direccion;
    char valido;
    char sucio;         /* en valido y sucio solo se usa un bit */
} CACHEblock_t;

typedef struct CACHEset { 
    struct CACHEblock bloques[ASOC];
    char LRU;        /* en LRU solo se usa un bit para ASOC=2 */
} CACHEset_t;

struct CACHEset Cache_datos[SETS];

static unsigned long lecturas=0, escrituras=0, fallos=0, sucios=0, expulsados=0;

char leer(unsigned long long dir){
	// Para quitar los 6 bits de menos peso que no se usan para indexar
	dir = dir >> 6;
	int n = dir & 0x3F;
	// Se mira en los dos bloques del conjunto (por asociatividad 2) de la cache
	lecturas++;
    for (int i = 0; i < ASOC; i++) {
        if (Cache_datos[n].bloques[i].direccion == dir && Cache_datos[n].bloques[i].valido == 1) {
            // Si el bloque esta en memoria cache, se actualiza el valor de LRU
            Cache_datos[n].LRU = (i + 1) % ASOC;
            return 1;
        }
    }
	// No está en la cache, se suma el numero de fallos
	fallos++;
	if (Cache_datos[n].bloques[Cache_datos[n].LRU].valido == 1){
		if(Cache_datos[n].bloques[Cache_datos[n].LRU].sucio == 1){
			sucios++;
		}
		expulsados++;
	}
	Cache_datos[n].bloques[Cache_datos[n].LRU].direccion = dir;
	Cache_datos[n].bloques[Cache_datos[n].LRU].valido = 1;
	// Se actualiza el valor de sucio
	Cache_datos[n].bloques[Cache_datos[n].LRU].sucio = 0;
	Cache_datos[n].LRU = (Cache_datos[n].LRU + 1) % 2;
	// Como no esta, devuelve un falso
	return 0;
}


char escribir(unsigned long long dir){
	// Devuelve booleano acierto/fallo

	// Para quitar los 6 bits de menos peso que no se usan para indexar
	dir = dir >> 6;
	int n = dir & 0x3F;

	// Se mira en los dos bloques del conjunto (por asociatividad 2) de la cache
	escrituras++;
    for (int i = 0; i < ASOC; i++) {
        if (Cache_datos[n].bloques[i].direccion == dir && Cache_datos[n].bloques[i].valido == 1) {
            // Si el bloque esta en memoria cache, se actualiza el valor de LRU
            Cache_datos[n].LRU = (i + 1) % ASOC;
            Cache_datos[n].bloques[i].sucio = 1;
            return 1;
        }
    }
	// Si no
	
	fallos++;
	if(Cache_datos[n].bloques[Cache_datos[n].LRU].valido == 1){
		if(Cache_datos[n].bloques[Cache_datos[n].LRU].sucio == 1){
			sucios++;
		}
		expulsados++;
	}
	Cache_datos[n].bloques[Cache_datos[n].LRU].direccion = dir;
	Cache_datos[n].bloques[Cache_datos[n].LRU].valido = 1;
	Cache_datos[n].bloques[Cache_datos[n].LRU].sucio = 1;
	Cache_datos[n].LRU = (Cache_datos[n].LRU + 1) % 2;
	return 0;
}

/* llamar a esta funcion desde inicpu() */
void inicache(){
	int i;

	for(i=0;i<SETS;i++){
		Cache_datos[i].bloques[0].valido=0;
		Cache_datos[i].bloques[1].valido=0;
		Cache_datos[i].LRU=0;
	}
}

void print_cache()
{
    printf("CACHE: %lu lect %lu escr %lu acc %lu fallos (%2.1f%%) %lu expul %lu sucios \n",
    lecturas, escrituras, lecturas+escrituras, fallos, 100*(float)fallos/(lecturas+escrituras), expulsados, sucios);
}

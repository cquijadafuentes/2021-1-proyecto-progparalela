#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>
#include "k2tree_operations_parallel.h"

#define KK  K*K

MREP * k2tree_intersection_parallel_main(MREP * repA, MREP * repB, uint parLev);
uint k2tree_intersection_parallel_sup(MREP * repA, MREP * repB, uint infLen, int** resSup, misBits** resInf, uint actPos, ulong pA, ulong pB);
uint k2tree_intersection_parallel_inf(MREP * repA, MREP * repB, misBits* res, ulong* posA, ulong* posB, uint relLev);
ulong posNodoHijo(MREP* rep, ulong pos);

int main(int argc, char * argv[]){
	if(argc < 5){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <LEVEL> <GRAPH>\n", argv[0]);
		return(-1);
	}

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);
	uint parLev = (uint) atoi(argv[3]);
	// Variables para el control de tiempo y conteo de posibles errores
	struct timeval t_ini;
	struct timeval t_fin;
	double milisecs = 0.0;
	
	gettimeofday(&t_ini, NULL);
	MREP * result = k2tree_intersection_parallel_main(repA, repB, parLev);
	gettimeofday(&t_fin, NULL);
	milisecs = ((double)(t_fin.tv_sec*1000 + (double)t_fin.tv_usec/1000) - 
		    		(double)(t_ini.tv_sec*1000 + (double)t_ini.tv_usec/1000));

	printf("%lf ms.\t", milisecs);

	if(result == NULL){
		printf("Fallo en la operación.\n");
		return -1;
	}

	destroyRepresentation(repA);
	destroyRepresentation(repB);
	saveRepresentation(result, argv[4]);
	destroyRepresentation(result);
	return 0;
}

void printBitmap(MREP* X){
	for(int i=0; i<X->btl_len; i+=KK){
		for(int j=0; j<KK; j++){
			if(isBitSet(X->btl, i+j)){
				printf("1");
			}else{
				printf("0");
			}
		}
		printf(" ");
	}
}

void printArray(int** arr, int n){
	for(int i=0; i<n; i++){
		printf("%d  ", *(arr[0]));
	}
	printf("\n");
}


MREP * k2tree_intersection_parallel_main(MREP * repA, MREP * repB, uint parLev){
	// Algoritmo que retorna el resultado de la operación de Intersección de manera paralela para k=2
	// recibiendo como parámetros las dos representaciones de k2tree a operar.
/*
	La estrategia corresponde a calcular de manera paralela el resultado hasta cierto nivel de la manera
	tradicional con ciertas consideraciones. No se exploran los árboles buscando la posición inicial de cada nivel

	En esta función se hace la preparación de las estructuras para alojar el resultado y luego su unificación.
	Estos resultados, para ser calculados en paralelo, se guardan en estructuras de punteros.

	Se recibe un parámetro que indica hasta qué nivel se realizan llamadas en paralelo. A partir de ese nivel 
	no se crean más hebras y el resultado se calcula como se hace en la propuesta secuencial oringinal.
*/

	printf("BitmapA: \n");
	printBitmap(repA);
	printf("\n");

	printf("BitmapB: \n");
	printBitmap(repB);
	printf("\n");


/*
	1- Calcular dimensiones de arreglo para resultado de los niveles superiores.
*/
	printf("En la función de preparación de resultados\n");
	uint maxBitsSup = 0;
	uint numThreads = 1;
	for(int i=0; i<parLev; i++){
		numThreads *= KK;
		maxBitsSup += numThreads;
	}
	printf("numThreads: %u\n", numThreads);
	printf("maxBitsSup: %u\n", maxBitsSup);
/*
	2 - Estructura para solución en nodos superiores
*/
	int** resSup = (int**) malloc(sizeof(int*)*maxBitsSup);
	for(int i=0; i<maxBitsSup; i++){
		resSup[i] = (int*) malloc(sizeof(int));
		*(resSup[i]) = 0;
	}

/*
	3 - Estructura para solución en nodos inferiores (sólo reserva de memoria)
*/

	misBits** resInf = (misBits**) malloc(sizeof(misBits*));
	if(resInf == NULL){
		printf("Error en la reserva de memoria (Interseccion paralela - Estructura misBits).\n");
		return NULL;
	}
	for(int i=0; i<numThreads; i++){
		resInf[i] = (misBits *) malloc(sizeof(misBits));
		if(resInf[i] == NULL){
			printf("Error en la reserva de memoria (Interseccion paralela - Estructura misBits).\n");
			return NULL;
		}
	}

/*
	4 - Llamada a solución para nodos superiores (la misma luego llama solución a niveles inferiores)
*/

	k2tree_intersection_parallel_sup(repA, repB, maxBitsSup, resSup, resInf, 0u, 0u, 0u);

/*
	5 - Unificación de los resultados en una estructura
*/
	printf("Uniendo los resultados.\n");
	printf("Resultado superior:\n");
	for(int i=0; i<maxBitsSup; i+=KK){
		for(int j=0; j<KK; j++){
			printf("%u", *(resSup[i+j]));
		}
		printf(" ");
	}
	printf("\n");
	return NULL;
}


uint k2tree_intersection_parallel_sup(MREP * repA, MREP * repB, uint infLen, int** resSup, misBits** resInf, uint actPos, ulong pA, ulong pB){
	// Calcula el resultado para el nodo que comienza en <actPos> de tamaño k*k
	// Si en ambos casos bits son 1, se llama a la función para el cálculo del resultad según
	// si aún están en el rango de nodos superiores o son inferiores. Si son nodos inferiores 
	// se realizan los cálculos de posiciones en el subárbol correspondiente y se llama la función
	// tradicional usando la estructura <resInf>
printf("par_sup -> pA: %lu - pB: %lu - actPos: %u - infLen: %u\n", pA, pB, actPos, infLen);
/*
	1 - Si <actPos> corresponde a un nodo inferior
	Calcular posiciones iniciales de los dos subárboles y estimación de largo por nivel para el resultado.
*/
	if(actPos > infLen){
		uint pResInf = ((actPos - infLen) / KK) - 1;
		printf("inf con resInf_%u\n", pResInf);
		return k2tree_intersection_parallel_inf(repA, repB, resInf[pResInf], NULL, NULL, 0u);
	}

/*
	2 - Si <actPos> corresponde a un nodo superior
	Verificar que el resultado podría exisitr (ambos bits son 1) y si es así hacer llamada para calcular hijos
*/
	int unos = 0;
	for(int i=0; i<KK; i++){
		if(isBitSet(repA->btl, pA+i) && isBitSet(repB->btl, pB+i)){
			uint pAhijo = posNodoHijo(repA, pA+i);
			uint pBhijo = posNodoHijo(repB, pB+i);
			uint actPosHijo = (actPos + i + 1) * KK;
			*(resSup[actPos+i]) = k2tree_intersection_parallel_sup(repA, repB, infLen, resSup, resInf, actPosHijo, pAhijo, pBhijo);
			unos += *(resSup[actPos]);
		}
	}
	if(unos > 0){
		return 1u;
	}
	return 0u;
}


uint k2tree_intersection_parallel_inf(MREP * repA, MREP * repB, misBits* res, ulong* posA, ulong* posB, uint relLev){
printf("par_inf -> pA: %lu - pB: %lu - relLev: %u\n", posA[relLev], posB[relLev], relLev);
	return 1;
}


ulong posNodoHijo(MREP* rep, ulong pos){
	if(rep == NULL || pos > rep->bt_len){
		return 0;
	}
//	printf("ranknodohijo: %u bits en la pos: %lu\n", rank(rep->btl, pos-1), pos);
	ulong posHijo = rank(rep->btl, pos-1) * (K*K) + (K*K);
	if(posHijo > rep->btl_len){
		return rep->btl_len;
	}
	return posHijo;
}


















MREP * k2tree_intersection_parallel(MREP * repA, MREP * repB){


	uint maximalLevel = repA->maxLevel;	
	ulong numNodos = repA->numberOfNodes;
	
	/*
		1 - Generar las posiciones para cada sub-árbol de las entradas
	*/

	// Filas = una por cada subarbol al primer nivel
	// Columnas = si el subarbol es 0, el puntero es NULL
	// 			  sino el puntero tiene (maxLevel-1) elementos
	ulong ** pRepA_parallel = posByLevel_parallel(repA);
	ulong ** pRepB_parallel = posByLevel_parallel(repB);

	if(pRepA_parallel == NULL){
		printf("Error! en cálculo de pRepA_parallel\n");
		return NULL;
	}

	if(pRepB_parallel == NULL){
		printf("Error! en cálculo de pRepB_parallel\n");
		// Liberando memoria de reservas anteriores
		for(int i=0; i<KK; i++){
			if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
		}
		return NULL;
	}

	/*
		2 - Calcular los bits que podrían usar en cada nivel (por debajo del máximo)
	*/

	ulong** minBitsPar = minsResultadoParalelo(repA, repB, pRepA_parallel, pRepB_parallel);
	if(minBitsPar == NULL){
		printf("Error! al generar minBitsPar\n");
		// Liberando memoria de reservas anteriores
		for(int i=0; i<KK; i++){
			if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
			if(pRepB_parallel[i] != NULL) free(pRepB_parallel[i]);
		}
		return NULL;
	}

	misBits** R = (misBits**) malloc(sizeof(misBits*) * KK);
	if(R == NULL){
		printf("Error! en la reserva de memoria de misBits\n");
		// Liberando memoria de reservas anteriores
		for(int i=0; i<KK; i++){
			// Liberando memoria de reservas anteriores
			if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
			if(pRepB_parallel[i] != NULL) free(pRepB_parallel[i]);
			if(minBitsPar[i] != NULL) free(minBitsPar[i]);
		}
		return NULL;
	}

	for(int i=0; i<KK; i++){
		if(minBitsPar[i] != NULL){
			R[i] = nuevoBitMap(maximalLevel+1, minBitsPar[i]);	
			if(R[i] == NULL){
				printf("Error! en reserva de misBits para resultado parcial R[%d].\n", i);
				// Liberando memoria de reservas anteriores
				for(int i=0; i<KK; i++){
					if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
					if(pRepB_parallel[i] != NULL) free(pRepB_parallel[i]);
					if(minBitsPar[i] != NULL) free(minBitsPar[i]);
					if(R[i] != NULL) destruirBitMap(R[i]);
				}
				return  NULL;
			}
		}
	}

	/*
		3 - Ejecutar la operación en un for paralelo por cada sub-árbol
			usando su respectiva sub-estructura y arreglo de posiciones
			para cada caso
	*/
	
	//	******************* IMPLEMENTACIÓN FOR PARALELO *******************
	#pragma omp parallel for
	for(int i=0; i<KK; i++){
//		printf("id_thread: %d\n", omp_get_thread_num());
		if(pRepA_parallel[i] != NULL && pRepB_parallel[i] != NULL){
			uint resInter = intersectionOperation(1u, repA, repB, pRepA_parallel[i], pRepB_parallel[i], R[i]);
			setBit(R[i], 0, resInter);
		}
	}

	/*
		4 -	Unificar la estructura de los resultados en el resultado
	*/
	// La copia del resultado es bit a bit

	ulong * minimosBits = (ulong *) malloc(sizeof(ulong));
	minimosBits[0] = KK;
	for(int i=1; i<=maximalLevel; i++){
		for(int j=0; j<KK; j++){
			if(R[j] != NULL){
				minimosBits[0] += R[j]->n[i];
			}
		}
	}

	
	misBits * Rfinal = nuevoBitMap(1, minimosBits);
	if(Rfinal == NULL){
		printf("Error! en reserva de misBits para resultado final.\n");
		// Liberando memoria de reservas anteriores
		for(int i=0; i<KK; i++){
			if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
			if(pRepB_parallel[i] != NULL) free(pRepB_parallel[i]);
			if(minBitsPar[i] != NULL) free(minBitsPar[i]);
			if(R[i] != NULL) destruirBitMap(R[i]);
		}
		return  NULL;
	}

	for(int j=0; j<KK; j++){
		if(R[j] != NULL){
			for(int l=0; l<R[j]->n[0]; l++){
				setBit(Rfinal, 0u, isBitSeted(R[j], 0, l));
			}
		}else{
			setBit(Rfinal, 0u, 0u);
		}
	}
	for(int i=1; i<=maximalLevel; i++){
		for(int j=0; j<KK; j++){
			if(R[j] != NULL){
				for(int l=0; l<R[j]->n[i]; l++){
					setBit(Rfinal, 0u, isBitSeted(R[j], i, l));
				}
			}
		}
	}

	ulong edgesF = 0;
	for(int i=0; i<KK; i++){
		if(R[i] != NULL){
			edgesF += R[i]->numEdges;
		}
	}

	/*
		5 -	Construir estructura con resultado final
	*/

	// Liberando memoria de reservas anteriores
	for(int i=0; i<KK; i++){
		if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
		if(pRepB_parallel[i] != NULL) free(pRepB_parallel[i]);
		if(minBitsPar[i] != NULL) free(minBitsPar[i]);
		if(R[i] != NULL) destruirBitMap(R[i]);
	}

	return createFromBitmap(Rfinal, maximalLevel, numNodos, edgesF);

}


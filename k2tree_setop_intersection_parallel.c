#include <stdio.h>
#include <string.h>
#include <omp.h>
#include "k2tree_operations_parallel.h"

MREP * k2tree_intersection_parallel(MREP * repA, MREP * repB);

int main(int argc, char * argv[]){
	if(argc < 4){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);
	MREP * result = k2tree_intersection_parallel(repA, repB);

	if(result == NULL){
		printf("Fallo en la operación.\n");
		return -1;
	}

	destroyRepresentation(repA);
	destroyRepresentation(repB);
	saveRepresentation(result, argv[3]);
	destroyRepresentation(result);
	return 0;
}

MREP * k2tree_intersection_parallel(MREP * repA, MREP * repB){
	// Algoritmo que retorna el resultado de la operación de Intersección de manera paralela para k=2
	// recibiendo como parámetros las dos representaciones de k2tree a operar.
	// Calcula y genera los elementos adionales que necesita el algoritmo intersectionOperation.
/*
	La estrategia inicial corresponde a calcular cada sub-árbol que define el nodo raíz de manera paralela
	Esto supone que se tendrán, por cada representación 4 listas de posiciones iniciales
	Y también 4 sub-árboles de resultado 
*/
	uint maximalLevel = repA->maxLevel;	
	ulong numNodos = repA->numberOfNodes;
	int kk = K*K;

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
		for(int i=0; i<kk; i++){
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
		for(int i=0; i<kk; i++){
			if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
			if(pRepB_parallel[i] != NULL) free(pRepB_parallel[i]);
		}
		return NULL;
	}

	misBits** R = (misBits**) malloc(sizeof(misBits*) * kk);
	if(R == NULL){
		printf("Error! en la reserva de memoria de misBits\n");
		// Liberando memoria de reservas anteriores
		for(int i=0; i<kk; i++){
			// Liberando memoria de reservas anteriores
			if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
			if(pRepB_parallel[i] != NULL) free(pRepB_parallel[i]);
			if(minBitsPar[i] != NULL) free(minBitsPar[i]);
		}
		return NULL;
	}

	for(int i=0; i<kk; i++){
		if(minBitsPar[i] != NULL){
			R[i] = nuevoBitMap(maximalLevel+1, minBitsPar[i]);	
			if(R[i] == NULL){
				printf("Error! en reserva de misBits para resultado parcial R[%d].\n", i);
				// Liberando memoria de reservas anteriores
				for(int i=0; i<kk; i++){
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
	for(int i=0; i<kk; i++){
		printf("id_thread: %d\n", omp_get_thread_num());
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
	minimosBits[0] = kk;
	for(int i=1; i<=maximalLevel; i++){
		for(int j=0; j<kk; j++){
			if(R[j] != NULL){
				minimosBits[0] += R[j]->n[i];
			}
		}
	}

	
	misBits * Rfinal = nuevoBitMap(1, minimosBits);
	if(Rfinal == NULL){
		printf("Error! en reserva de misBits para resultado final.\n");
		// Liberando memoria de reservas anteriores
		for(int i=0; i<kk; i++){
			if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
			if(pRepB_parallel[i] != NULL) free(pRepB_parallel[i]);
			if(minBitsPar[i] != NULL) free(minBitsPar[i]);
			if(R[i] != NULL) destruirBitMap(R[i]);
		}
		return  NULL;
	}

	for(int j=0; j<kk; j++){
		if(R[j] != NULL){
			for(int l=0; l<R[j]->n[0]; l++){
				setBit(Rfinal, 0u, isBitSeted(R[j], 0, l));
			}
		}else{
			setBit(Rfinal, 0u, 0u);
		}
	}
	for(int i=1; i<=maximalLevel; i++){
		for(int j=0; j<kk; j++){
			if(R[j] != NULL){
				for(int l=0; l<R[j]->n[i]; l++){
					setBit(Rfinal, 0u, isBitSeted(R[j], i, l));
				}
			}
		}
	}

	ulong edgesF = 0;
	for(int i=0; i<kk; i++){
		if(R[i] != NULL){
			edgesF += R[i]->numEdges;
		}
	}

	/*
		5 -	Construir estructura con resultado final
	*/

	// Liberando memoria de reservas anteriores
	for(int i=0; i<kk; i++){
		if(pRepA_parallel[i] != NULL) free(pRepA_parallel[i]);
		if(pRepB_parallel[i] != NULL) free(pRepB_parallel[i]);
		if(minBitsPar[i] != NULL) free(minBitsPar[i]);
		if(R[i] != NULL) destruirBitMap(R[i]);
	}

	return createFromBitmap(Rfinal, maximalLevel, numNodos, edgesF);

}
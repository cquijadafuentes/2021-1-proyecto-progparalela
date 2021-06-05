#include "k2tree_operations.h"

uint intersectionOperation_parallel(uint l, MREP * A, MREP * B, ulong * pA, ulong * pB, misBits * C){
	// Operación de intersección sobre 2 representaciones de l niveles cada una
	// donde pA y pB son las posiciones por nivel y C es el bitmap para el resultado
	uint writesomething = 0u;
	uint i;
	uint maxH = (A->maxLevel > B->maxLevel) ? A->maxLevel : B->maxLevel;
	ulong t[K*K];

	for(i=0; i<K*K; i++){
		if(l<maxH){
			if(isBitSet(A->btl, pA[l]) && isBitSet(B->btl, pB[l])){
				t[i] = intersectionOperation(l+1, A, B, pA, pB, C);
			}
			else{
				t[i] = 0u;
				if(isBitSet(A->btl, pA[l])){
					SkipNodes(l+1, A, pA, 1u);
				}
				if(isBitSet(B->btl, pB[l])){
					SkipNodes(l+1, B, pB, 1u);
				}
			}
		}
		else{
			t[i] = (isBitSet(A->btl, pA[l]) && isBitSet(B->btl, pB[l]))?1u:0u;
		}
		writesomething = (writesomething==1u || t[i]==1u)?1u:0u;
		pA[l] = pA[l]+1;
		pB[l] = pB[l]+1;
	}

	if(writesomething == 1u || l==0){
		for(i=0; i<K*K; i++){
			setBit(C, l, t[i]);
		}		
	}

	return (writesomething==1u)?1u:0u;
}


MREP * k2tree_intersection_parallel(MREP * repA, MREP * repB){

	printf("En la operación paralela\n");


	// Algoritmo que retorna el resultado de la operación de Intersección
	// recibiendo como parámetros las dos representaciones de k2tree a operar.
	// Calcula y genera los elementos adionales que necesita el algoritmo intersectionOperation.
	uint maximalLevel = repA->maxLevel;	
	ulong * pRepA = posByLevel(repA);
	ulong * pRepB = posByLevel(repB);	
	ulong * minBits = (ulong *) malloc(sizeof(ulong)*(maximalLevel+1));
	
	if(minBits == NULL){
		printf("Se ha generado un problema...\n");
		return NULL;
	}

	// La intersección podría tener a lo más, tantas celdas activas 
	// como las que posee el menor de los dos elementos operados.

	ulong numBitsByLevelA, numBitsByLevelB;
	uint i;

	for(i = 0; i <= maximalLevel; i++){
		if(i==repA->maxLevel){
			numBitsByLevelA = repA->btl_len - pRepA[i];
			numBitsByLevelB = repB->btl_len - pRepB[i];
		}else{
			numBitsByLevelA = pRepA[i+1] - pRepA[i];
			numBitsByLevelB = pRepB[i+1] - pRepB[i];
		}
		minBits[i] = (numBitsByLevelA > numBitsByLevelB)?numBitsByLevelB:numBitsByLevelA;
	}

	misBits * C = nuevoBitMap(maximalLevel+1, minBits);

	if(C == NULL){
		printf("Error en la reserva de memoria.\n");
		return NULL;
	}

	intersectionOperation(0u, repA, repB, pRepA, pRepB, C);
	ulong vinculolos = concatenar(C);	
	ulong numNodos = repA->numberOfNodes;

	return createFromBitmap(C, maximalLevel, numNodos, vinculolos);
}

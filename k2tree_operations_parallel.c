#include "k2tree_operations.h"

ulong ** posByLevel_parallel(MREP * X){
	//	Calcula las posiciones del bitmap para cada sub-árbol
	// Filas = una por cada subarbol al primer nivel
	// Columnas = si el subarbol es 0, el puntero es NULL
	// 			  sino el puntero tiene (maxLevel) elementos
	ulong kk = K*K;
	ulong ** pX = (ulong **) malloc(sizeof(ulong*) * (kk));
	if(pX == NULL){
		printf("Error en al reserva de memoria.\n");
		return NULL;
	}
	for(int j=0; j<kk; j++){
		if(isBitSet(X->btl, j)){
			pX[j] = (ulong *) malloc(sizeof(ulong) * X->maxLevel + 1);
			pX[j][0] = j;
			for(int i=1; i <= X->maxLevel; i++){
				pX[j][i] = rank(X->btl, pX[j][i-1]-1) * (kk) + (kk);
			}
		}else{
			pX[j] = NULL;
		}
	}
	return pX;
}

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


	// Algoritmo que retorna el resultado de la operación de Intersección de manera paralela para k=2
	// recibiendo como parámetros las dos representaciones de k2tree a operar.
	// Calcula y genera los elementos adionales que necesita el algoritmo intersectionOperation.
	uint maximalLevel = repA->maxLevel;	
/*
	La estrategia inicial corresponde a calcular cada sub-árbol que define el nodo raíz de manera paralela
	Esto supone que se tendrán, por cada representación 4 listas de posiciones iniciales
	Y también 4 sub-árboles de resultado 
*/
	ulong * pRepA = posByLevel(repA);
	ulong * pRepB = posByLevel(repB);	
	//<--------------------------------------------
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

//	**************** IMPLEMENTACIÓN PARALELA ****************
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
	}else{
		printf("pRepA_parallel:\n");
		for(int i=0; i<kk; i++){
			if(pRepA_parallel[i] == NULL){
				printf("NULL\n");
			}else{
				for(int j=0; j <= repA->maxLevel; j++){
					printf("%ld ", pRepA_parallel[i][j]);
				}
				printf("\n");
			}
		}
	}
	if(pRepB_parallel == NULL){
		printf("Error! en cálculo de pRepB_parallel\n");
	}else{
		printf("pRepB_parallel:\n");
		for(int i=0; i<kk; i++){
			if(pRepB_parallel[i] == NULL){
				printf("NULL\n");
			}else{
				for(int j=0; j <= repA->maxLevel; j++){
					printf("%ld ", pRepB_parallel[i][j]);
				}
				printf("\n");
			}
		}
	}

	/*
		2 - Generar la estructura para el resultado de la intersección
	*/

	// 	El bitmap para el resultado contendrá el resultado de intersectar
	//	los K*K sub-árboles con los que se trabaja.
	//	En esta primera etapa, se genera espacio suficiente para los sub-árbol completos

	ulong * minimosBits = (ulong *) malloc(sizeof(ulong) * (maximalLevel + 1));
	if(minimosBits == NULL){
		printf("Error! En reserva de memoria para minimosBits.\n");
		return NULL;
	}

	minimosBits[0] = 1;
	for(int i=1; i<=maximalLevel; i++){
		minimosBits[i] = minimosBits[i-1] * kk;
	}

	
	printf("minimosBits: \n");
	for(int i=0; i<=maximalLevel; i++){
		printf("%ld \n", minimosBits[i]);
	}
	printf("\n");


	misBits** R = (misBits**) malloc(sizeof(misBits*) * kk);
	if(R == NULL){
		printf("Error! en la reserva de memoria de misBits\n");
	}

	for(int i=0; i<kk; i++){
		R[i] = nuevoBitMap(maximalLevel+1, minimosBits);
	}

	/*
		3 - Ejecutar la operación en un for paralelo por cada sub-árbol
			usando su respectiva sub-estructura y arreglo de posiciones
			para cada caso
	*/
	uint resInter;
	for(int i=0; i<kk; i++){
		if(pRepA_parallel[i] != NULL && pRepB_parallel[i] != NULL){
			resInter = intersectionOperation(1u, repA, repB, pRepA_parallel[i], pRepB_parallel[i], R[i]);
		}else{
			resInter = 0u;
		}
		setBit(R[i], 0, resInter);
	}


	/*
		4 -	Unificar la estructura de los resultados en el resultado
	*/

	for(int i=0; i<kk; i++){
		
	}



//	************** FIN IMPLEMENTACIÓN PARALELA **************

	return createFromBitmap(C, maximalLevel, numNodos, vinculolos);
}

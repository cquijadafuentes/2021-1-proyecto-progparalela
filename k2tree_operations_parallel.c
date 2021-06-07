#include "k2tree_operations.h"

ulong bitsUsados(MREP * X, ulong** pX, int subarbol, int nivel, int kk){
	int actual = pX[subarbol][nivel];
	int sa = subarbol+1;
	int n = nivel;
	if(sa == kk){
		sa = 0;
		n++;
	}
	while(pX[sa] == NULL && n <= X->maxLevel){
		// Este ciclo itera hasta dar con el siguiente bloque válido
		// definidos por sa (subarbol) y n (nivel)
		sa++;
		if(sa == kk){
			sa = 0;
			n++;
		}
	}
	if(n > X->maxLevel){
		return X->btl_len - actual;
	}
	return pX[sa][n];
}

ulong ** minsResultadoParalelo(MREP * A, MREP * B, ulong ** pA, ulong ** pB){
	ulong kk = K*K;
	if(A == NULL || pA == NULL || B == NULL || pB == NULL){
		printf("Error! argumentos nulos en funcion minsResultadoParalelo\n");
		return NULL;
	}

	ulong ** minimos = (ulong **) malloc(sizeof(ulong*) * kk);
	if(minimos == NULL){
		printf("Error! en reserva de memoria de minimos (generaMinimosPosiciones)\n");
		return NULL;
	}
	for(int i=0; i<kk; i++){
		if(pA[i] == NULL || pB[i] == NULL){
			// No hay intersección entre estos sub-árboles
			minimos[i] = NULL;
		}else{
			minimos[i] = (ulong *) malloc(sizeof(ulong) * (A->maxLevel + 1));
			if(minimos[i] == NULL){
				printf("Error! en reserva de memoria de minimos en func minsResultadoParalelo\n");
				return NULL;
			}
		}
	}

	ulong bitsMaxsNivel = 1;
	ulong sumAB;
	for(int j=0; j<= A->maxLevel; j++){
		for(int i=0; i<kk; i++){
			if(minimos[i] != NULL){
				sumAB = bitsUsados(A, pA, i, j, kk) + bitsUsados(B, pB, i, j, kk);
				if(sumAB < bitsMaxsNivel){
					minimos[i][j] = sumAB;
				}else{
					minimos[i][j] = bitsMaxsNivel;
				}
			}
		}
		bitsMaxsNivel *= kk;
	}

	return minimos;
}

ulong ** posByLevel_parallel(MREP * X){
	//	Calcula las posiciones del bitmap para cada sub-árbol
	// Filas = una por cada subarbol al primer nivel
	// Columnas = si el subarbol es 0, el puntero es NULL
	// 			  sino el puntero tiene (maxLevel) elementos
	ulong kk = K*K;
	ulong ** pX = (ulong **) malloc(sizeof(ulong*) * (kk));
	if(pX == NULL){
		printf("Error en al reserva de memoria en posByLevel_parallel.\n");
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
	
	//	******************* IMPLEMENTAR AQUÍ FOR PARALELO *******************

	for(int i=0; i<kk; i++){
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

//	************** FIN IMPLEMENTACIÓN PARALELA **************

	//return createFromBitmap(C, maximalLevel, numNodos, vinculolos);
}



MREP * k2tree_intersection_estrategia_parallel(MREP * repA, MREP * repB){
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
	
	for(int i=0; i<kk; i++){
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

//	************** FIN IMPLEMENTACIÓN PARALELA **************

	//return createFromBitmap(C, maximalLevel, numNodos, vinculolos);
}

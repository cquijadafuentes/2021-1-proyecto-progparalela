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
		printf("\n");
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
		printf("\n");
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
		if(R[i] == NULL){
			printf("Error! en reserva de misBits para resultado parcial R[%d].\n", i);
			return  NULL;
		}
	}

	free(minimosBits);

	/*						INICIO PRUEBA
		PRUEBA PARA GENERAR MINIMOSBITS DEL RESULTADO PARALELO
	*/

	ulong** minBitsPar = minsResultadoParalelo(repA, repB, pRepA_parallel, pRepB_parallel);
	if(minBitsPar == NULL){
		printf("Error! al generar minBitsPar\n");
	}else{
		printf("minBitsPar:\n");
		for(int i=0; i<kk; i++){
			if(minBitsPar[i] == NULL){
				printf("NULL\n");
			}else{
				for(int j=0; j <= repA->maxLevel; j++){
					printf("%ld ", minBitsPar[i][j]);
				}
				printf("\n");
			}
		}
		printf("\n");
	}
		

	/*						 FIN PRUEBA
		PRUEBA PARA GENERAR POSICIONES DE SUB-ÁRBOLES Y MINIMOSBITS
		EN UNA SOLA FUNCIÓN
	*/

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


	printf("Comparando cantidad bits estimados máximos vs bits del resultado:\n");
	for(int i=0; i<kk; i++){
		if(minBitsPar[i]!=NULL){
			for(int j=0; j<=maximalLevel; j++){
				if(minBitsPar[i][j] < R[i]->n[j]){
					printf("!!! minimos bits < resultado: subarbol: %d - nivel %d (%ld < %ld)\n", i, j, minBitsPar[i][j], R[i]->n[j]);
				}
			}
		}
	}
	printf("\n");

	/*
		4 -	Unificar la estructura de los resultados en el resultado
	*/
	// La copia del resultado es bit a bit

	minimosBits = (ulong *) malloc(sizeof(ulong));
	minimosBits[0] = 0;
	for(int i=0; i<=maximalLevel; i++){
		for(int j=0; j<kk; j++){
			if(R[j] != NULL){
				minimosBits[0] += R[j]->n[i];
			}
		}
	}

	printf("minimosBits Resultado final: %ld\n", minimosBits[0]);

	misBits * Rfinal = nuevoBitMap(1, minimosBits);
	if(Rfinal == NULL){
		printf("Error! en reserva de misBits para resultado final.\n");
		return  NULL;
	}

	for(int i=0; i<=maximalLevel; i++){
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

	printf("Comparando bitmaps...\n");
	if(C->cant != Rfinal->cant){
		printf("Diferencia en cantidad de elementos\n");
	}else{
		printf("Misma cantidad de elementos (bits)\n");
	}
	for(int i=0; i<C->cant && i<Rfinal->cant; i++){
		if(isBitSeted(C, 0u, i) != isBitSeted(Rfinal, 0u, i)){
			printf("Diferencia en el bit %d\n", i);
		}
	}
	if(edgesF != vinculolos){
		printf("Diferente cantidad de edges.\n");
	}else{
		printf("Misma cantidad de edges!\n");
	}


	/*
		5 -	Construir estructura con resultado final
	*/

	return createFromBitmap(Rfinal, maximalLevel, numNodos, edgesF);

//	************** FIN IMPLEMENTACIÓN PARALELA **************

	//return createFromBitmap(C, maximalLevel, numNodos, vinculolos);
}

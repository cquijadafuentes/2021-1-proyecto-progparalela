#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>
#include "k2tree_operations_parallel.h"

#define KK  K*K

MREP * k2tree_intersection_parallel_main(MREP * repA, MREP * repB, uint parLev);
uint k2tree_intersection_parallel_sup(MREP * repA, MREP * repB, uint infLen, int** resSup, misBits** resInf, uint actPos, ulong pA, ulong pB, uint level);
uint k2tree_intersection_parallel_inf(MREP * repA, MREP * repB, misBits* res, ulong* posA, ulong* posB, uint level, uint maximalLevel);
ulong posNodoHijo(MREP* rep, ulong pos);

int main(int argc, char * argv[]){
	if(argc < 5){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <LEVEL> <GRAPH>\n", argv[0]);
		return(-1);
	}

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);
	uint parLev = (uint) atoi(argv[3]);
	if(parLev == 0){
		parLev = 1;
	}

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
	printf("--------------------------------------------\n");
	printf("BitmapA: \n");
	printBitmap(repA);
	printf("\n");
	printf("MaxLevel: %d - btl_len: %u - bt_len: %u \n", repA->maxLevel, repA->btl_len, repA->bt_len);
	printf("--------------------------------------------\n");
	printf("BitmapB: \n");
	printBitmap(repB);
	printf("\n");
	printf("MaxLevel: %d - btl_len: %u - bt_len: %u \n", repB->maxLevel, repB->btl_len, repB->bt_len);
	printf("--------------------------------------------\n");

	uint kk = KK;
	
/*
	1- Calcular dimensiones de arreglo para resultado de los niveles superiores.
*/
	printf("En la función de preparación de resultados\n");
	uint maxBitsSup = 0;
	uint numThreads = 1;
	for(int i=1; i<=parLev; i++){
		numThreads *= kk;
		maxBitsSup += numThreads;
	}
	//numThreads *= kk;
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

	misBits** resInf = (misBits**) malloc(sizeof(misBits*)*numThreads);
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
		resInf[i]->cant = 0;
		resInf[i]->niveles = 0;
		resInf[i]->tam = 0;
		resInf[i]->numEdges = 0;
	}

/*
	4 - Llamada a solución para nodos superiores (la misma luego llama solución a niveles inferiores)
*/

	k2tree_intersection_parallel_sup(repA, repB, maxBitsSup, resSup, resInf, 0u, 0u, 0u, 0u);

	// RESULTADOS DE LA OPERACIÓN
	printf("Uniendo los resultados.\n");
	printf("Resultado superior:\n");
	for(int i=0; i<maxBitsSup; i+=kk){
		for(int j=0; j<kk; j++){
			printf("%u", *(resSup[i+j]));
		}
		printf(" ");
	}
	printf("\n");
	printf("Resultado inferior:\n");
	printf("-  cant  niveles  tam  numEdges\n");
	for(int j=0; j<numThreads; j++){
		if(resInf[j]->cant > 0){
			printf("%u: %lu - %u - %lu - %lu - bits_x_niveles: ", j, resInf[j]->cant, resInf[j]->niveles, resInf[j]->tam, resInf[j]->numEdges);
			for(int i=0; i<resInf[j]->niveles; i++){
				printf("%lu  ", resInf[j]->n[i]);
			}
			printf("\n");
		}
	}
	
	for(int i=parLev; i<=repA->maxLevel; i++){
		for(int j=0; j<numThreads; j++){
			if(resInf[j]->cant > 0){
				for(int k=0; k<resInf[j]->n[i-parLev]; k+=kk){
					for(int m=0; m<kk; m++){
						printf("%u", isBitSeted(resInf[j], i-parLev, m+k));
					}
					printf(" ");
				}
			}
		}
	}
	printf("\n");

/*
	5 - Unificación de los resultados en una estructura
*/

	// 5.a - Conteo de bits necesarios resultado superior
	uint bloquesActivos = 0;
	uint maxbloquesActivos = maxBitsSup / kk;
	printf("maxbloquesActivos: %u\n", maxbloquesActivos);

	uint* posbloquesActivos = (uint *) malloc(sizeof(unsigned int) * (maxbloquesActivos));
	for(int i=0; i<maxBitsSup; i+=kk){
		uint bitsNodo = 0;
		for(int j=0; j<kk; j++){
			bitsNodo += *(resSup[i+j]);
		}
		if(bitsNodo > 0){
			posbloquesActivos[bloquesActivos] = i;
			bloquesActivos++;
		}
	}

	uint bitsSuperior = bloquesActivos * kk;
	
	// 5.b - Conteo de bits necesarios resultado inferior
	uint misBitsActivos = 0;
	uint* posMisBitsActivos = (uint *) malloc(sizeof(uint) * numThreads);
	uint bitsInferior = 0;
	uint numEdgesRes = 0;
	for(int j=0; j<numThreads; j++){
		if(resInf[j]->cant > 0){
			posMisBitsActivos[misBitsActivos] = j;
			misBitsActivos++;
			bitsInferior += resInf[j]->cant;
			numEdgesRes += resInf[j]->numEdges;
		}
	}

	// 5.c - Crear estructura definitiva y unificar los resultados
	// 		Crear estructura con 1 nivel para unir todos los resultados
	ulong* totalBits = (ulong*) malloc(sizeof(unsigned long));
	*totalBits = bitsSuperior + bitsInferior;
	misBits* C = nuevoBitMap(1, totalBits);
	//		Pasar los resultados de niveles superiores
	for(int i=0; i<bloquesActivos; i++){
		for(int j=0; j<kk; j++){
			setBit(C, 0u, *(resSup[posbloquesActivos[i]+j]));
		}
	}
	//		Pasar los resultados de niveles inferiores
	for(int i=parLev; i<=repA->maxLevel; i++){
		for(int j=0; j<misBitsActivos; j++){
			for(int k=0; k<resInf[posMisBitsActivos[j]]->n[i-parLev]; k+=kk){
				for(int m=0; m<kk; m++){
					setBit(C, 0u, isBitSeted(resInf[posMisBitsActivos[j]], i-parLev, m+k));
				}
			}
		}
	}
	//		Construir estructura final
	MREP* res = createFromBitmap(C, repA->maxLevel, repA->numberOfNodes, numEdgesRes);

/*
	6 - Liberar memoria de estructuras temporales
*/

	for(int i=0; i<C->cant; i+=kk){
		for(int j=0; j<kk; j++){
			printf("%u", isBitSeted(C,0u,i+j));
		}
		printf("  ");
	}
	printf("\n");
	printf("numEdgesRes: %u\n", numEdgesRes);

	return res;
}


ulong* posicionesRamaYEstimaRes(MREP* A, MREP* B, ulong* posA, ulong* posB, ulong pA, ulong pB, uint levels){
	// Esta función calcula las posiciones de los resultados y hace la estimación
	// del espacio que podría requerir el resultado de la operación
	uint hijosAntA = 1;
	uint hijosAntB = 1;
	ulong* bistResInterseccion = (ulong*) malloc(sizeof(unsigned long) * levels);
	if(bistResInterseccion == NULL){
		return NULL;
	}
	for(int i=0; i<levels; i++){
		posA[i] = pA;
		posB[i] = pB;
		// Estimación de bits basado en hijos anteriores
		if(hijosAntA < hijosAntB){
			bistResInterseccion[i] = hijosAntA * KK;
		}else{
			bistResInterseccion[i] = hijosAntB * KK;
		}
		// Cantidad de hijos en cada rama
		hijosAntA = rank(A->btl, pA-1 + (hijosAntA*KK)) - rank(A->btl, pA-1);
		hijosAntB = rank(B->btl, pB-1 + (hijosAntB*KK)) - rank(B->btl, pB-1);
		// Posición de inicio para cada nivel en la rama
		pA = posNodoHijo(A, pA);
		pB = posNodoHijo(B, pB);
	}

	for(int i=0; i<levels; i++){
		printf("pA: %lu - pB: %lu - bitsResultado: %lu\n", posA[i], posB[i], bistResInterseccion[i]);		
	}
	
	printf("Levels: %d\n", levels);
	return bistResInterseccion;
}

uint nuevoBitMap_Parallel(misBits* bitses, uint levels, ulong * cants){
	if(bitses == NULL){
		printf("Error en la reserva de memoria (Estructura misBits).\n");
		return 0u;
	}
	bitses->pos = (ulong*) malloc(sizeof(ulong)*levels);
	if(bitses->pos == NULL){
		printf("Error en la reserva de memoria (Posiciones misBits.\n");
		return 0u;
	}
	bitses->n = (ulong *) malloc(sizeof(ulong)*levels);
	if(bitses->n == NULL){
		printf("Error en la reserva de memoria (Niveles misBits).\n");
		return 0u;
	}
	bitses->niveles = levels;
	bitses->tam = 0;
	bitses->cant = 0;
	bitses->numEdges = 0;

	uint i;
	for(i=0; i< levels; i++){
		bitses->pos[i] = bitses->tam;
		bitses->n[i] = 0;
		bitses->tam += cants[i];
	}

	bitses->bitsm = (unsigned int *) malloc(sizeof(unsigned int) *(bitses->tam/W+1));
	if(bitses->bitsm == NULL){
		printf("Error en la reserva de memoria (bitses de misBits).\n");
		return 0u;
	}

	for(i=0; i<bitses->tam/W+1; i++){
		bitses->bitsm[i]=0;
	}
	return 1u;
}

uint k2tree_intersection_parallel_sup(MREP * repA, MREP * repB, uint infLen, int** resSup, misBits** resInf, uint actPos, ulong pA, ulong pB, uint level){
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
	if(actPos >= infLen){
		printf("kk:%d\n", KK);
		uint aaux = KK;
		uint pResInf = (actPos - infLen) / aaux;
		printf("----pResInf: %u\n", pResInf);
		int cantLevels = repA->maxLevel - level + 1;
		ulong* posA = (ulong*) malloc(sizeof(unsigned long) * cantLevels);
		if(posA == NULL){
			printf("Error! en reserva de memoria en k2tree_intersection_parallel_sup");
		}
		ulong* posB = (ulong*) malloc(sizeof(unsigned long) * cantLevels);
		if(posB == NULL){
			free(posA);
			printf("Error! en reserva de memoria en k2tree_intersection_parallel_sup");
		}
		ulong* bitsRes = posicionesRamaYEstimaRes(repA, repB, posA, posB, pA, pB, cantLevels);
		if(nuevoBitMap_Parallel(resInf[pResInf], cantLevels, bitsRes) == 0u){
			printf("Error! En la construcción de misbitses\n");
			return 0;
		}
		return k2tree_intersection_parallel_inf(repA, repB, resInf[pResInf], posA, posB, 0u, cantLevels-1);
	}

/*
	2 - Si <actPos> corresponde a un nodo superior
	Verificar que el resultado podría exisitr (ambos bits son 1) y si es así hacer llamada para calcular hijos
*/
	int unos = 0;
	for(int i=0; i<KK; i++){
		printf("i: %d - bits - A: %u - B: %u\n", i, isBitSet(repA->btl, pA+i), isBitSet(repB->btl, pB+i));
		if(isBitSet(repA->btl, pA+i) && isBitSet(repB->btl, pB+i)){
			uint pAhijo = posNodoHijo(repA, pA+i);
			uint pBhijo = posNodoHijo(repB, pB+i);
			uint actPosHijo = (actPos + i + 1) * KK;
			*(resSup[actPos+i]) = k2tree_intersection_parallel_sup(repA, repB, infLen, resSup, resInf, actPosHijo, pAhijo, pBhijo, level+1);
			unos += *(resSup[actPos+i]);
		}
	}
	if(unos > 0){
		return 1u;
	}
	return 0u;
}


uint k2tree_intersection_parallel_inf(MREP * A, MREP * B, misBits* C, ulong* pA, ulong* pB, uint level, uint maximalLevel){
	// Operación de intersección sobre 2 representaciones de l niveles cada una
	// donde pA y pB son las posiciones por nivel y C es el bitmap para el resultado
	uint writesomething = 0u;
	uint i;
	ulong t[KK];

	for(i=0; i<KK; i++){
		if(level<maximalLevel){
			if(isBitSet(A->btl, pA[level]) && isBitSet(B->btl, pB[level])){
				t[i] = k2tree_intersection_parallel_inf(A, B, C, pA, pB, level+1, maximalLevel);
			}
			else{
				t[i] = 0u;
				if(isBitSet(A->btl, pA[level])){
					SkipNodes(level+1, A, pA, 1u);
				}
				if(isBitSet(B->btl, pB[level])){
					SkipNodes(level+1, B, pB, 1u);
				}
			}
		}
		else{
			t[i] = (isBitSet(A->btl, pA[level]) && isBitSet(B->btl, pB[level]))?1u:0u;
		}
		writesomething = (writesomething==1u || t[i]==1u)?1u:0u;
		pA[level] = pA[level]+1;
		pB[level] = pB[level]+1;
	}

	if(writesomething == 1u || level==0){
		for(i=0; i<KK; i++){
			setBit(C, level, t[i]);
		}		
		printf("++++ ");
	}

	return (writesomething==1u)?1u:0u;
}


ulong posNodoHijo(MREP* rep, ulong pos){
	if(rep == NULL || pos > rep->bt_len){
		return 0;
	}
//	printf("ranknodohijo: %u bits en la pos: %lu\n", rank(rep->btl, pos-1), pos);
	ulong posHijo = rank(rep->btl, pos-1) * (KK) + (KK);
	if(posHijo > rep->btl_len){
		return rep->btl_len;
	}
	return posHijo;
}



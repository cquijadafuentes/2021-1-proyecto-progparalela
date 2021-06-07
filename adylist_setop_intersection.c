#include <stdio.h>
#include <string.h>
#include "kTree.h"
#include "basic.h"
#include "adylist_operations.h"

int main(int argc, char* argv[]){

	if(argc<4){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH>\n",argv[0]);
		return(-1);
	}

	// Cargando Lista de Adyacencia A
	ALREP * A = loadAdyacencyList(argv[1]);

	// Cargando Lista de Adyacencia B
	ALREP * B = loadAdyacencyList(argv[2]);

	// Operación de Intersección entre listas de adyacencia
	ALREP * listadyResult = adylistIntersectionOperation(A, B);
	if(listadyResult == NULL){
		printf("Error en la operación...\n");
		return 0;
	}

	destroyAdyacencyList(A);
	destroyAdyacencyList(B);
	saveAdyacencyList(listadyResult, argv[3]);
	destroyAdyacencyList(listadyResult);

	return 0;
}


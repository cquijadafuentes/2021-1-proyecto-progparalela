#include <stdio.h>
#include <string.h>
#include "k2tree_operations.h"

int main(int argc, char * argv[]){
	if(argc < 4){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}

/*
	setbuf(stdout, NULL);
*/

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);
	MREP * result = k2tree_intersection(repA, repB);

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
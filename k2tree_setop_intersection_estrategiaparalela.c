#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "k2tree_operations_parallel.h"

int main(int argc, char * argv[]){
	if(argc < 4){
		fprintf(stderr,"USAGE: %s <GRAPH> <GRAPH> <GRAPH>\n", argv[0]);
		return(-1);
	}

	MREP * repA = loadRepresentation(argv[1]);
	MREP * repB = loadRepresentation(argv[2]);

	// Variables para el control de tiempo y conteo de posibles errores
	struct timeval t_ini;
	struct timeval t_fin;
	double milisecs = 0.0;
	
	gettimeofday(&t_ini, NULL);
	MREP * result = k2tree_intersection_estrategia_parallel(repA, repB);
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
	saveRepresentation(result, argv[3]);
	destroyRepresentation(result);
	return 0;
}
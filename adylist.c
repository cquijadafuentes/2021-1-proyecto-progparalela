#include <math.h>
#include "misBits.h"
#include "kTree.h"
#include "adylist.h"

ALREP * loadAdyacencyList(char * basename){
	// Apertura del archivo
	char *filename = (char *) malloc(sizeof(char)*(strlen(basename)+8));
	strcpy(filename,basename);
	strcat(filename,".rbfull");
	FILE * ft = fopen(filename,"r");
	// Declaración de estructura
	ALREP * list;
	list = (ALREP *) malloc(sizeof(struct adyList));
	// Lectura desde el archivo
	fread(&list->numNodes,sizeof(uint),1,ft);
	fread(&list->numEdges,sizeof(ulong),1,ft);
	// Reserva de la lista de adyacencia
	list->listady = (int*)malloc(sizeof(int)*(list->numNodes+list->numEdges));
	fread(list->listady,sizeof(int), list->numNodes+list->numEdges,ft);
	fclose(ft);
	free(filename);
	return list;
}

MREP * ktreeFromList(ALREP * list){
	uint nodes; 
	ulong edges;

	nodes = list->numNodes;

	uint max_level = floor(log(nodes)/log(K));
	if(floor(log(nodes)/log(K))==(log(nodes)/log(K))) {
		max_level=max_level-1;
	}
	
	edges = list->numEdges;
	if(edges == 0){
		return NULL;
	}

	uint nodes_read=0;

	uint *xedges = (uint *)malloc(sizeof(uint)*edges);
	uint *yedges = (uint *)malloc(sizeof(uint)*edges);
	if(xedges == NULL || yedges == NULL){
		printf("Error en la reserva de memoria.\n");
		return NULL;
	}
	uint cedg = 0;

	ulong i;
	for(i=0;i<nodes+edges;i++) {
		int k;
		k = list->listady[i];
		if(k<0) {
			nodes_read++;
		}
		else {
			k--;
			xedges[cedg]=nodes_read-1;
			yedges[cedg]=k;
			cedg++;
		}
	}

	MREP * rep;
	rep = compactCreateKTree(xedges, yedges, nodes,edges,max_level);
	free(xedges);
	free(yedges);
	return rep;
}

ALREP * listFromKtree(MREP * rep){
	ALREP * list;
	list = (ALREP *) malloc(sizeof(struct adyList));
	list->numNodes = rep->numberOfNodes;
	list->numEdges = rep->numberOfEdges;
	list->listady = (int *) compactFullDecompression(rep);
	return list;
}

void destroyAdyacencyList(ALREP * list){
	if(list->listady!=NULL){
		free(list->listady);
	}
	free(list);
}

void saveAdyacencyList(ALREP * list, char * basename){
	char *filename = (char *)malloc(sizeof(char)*(strlen(basename)+8));
	strcpy(filename,basename);
	strcat(filename,".rbfull");
	FILE *fr = fopen(filename,"w");
	fwrite(&list->numNodes,sizeof(uint),1,fr);
	fwrite(&list->numEdges,sizeof(ulong),1,fr);

	fwrite(list->listady,sizeof(int),list->numNodes+list->numEdges,fr);
	fclose(fr);
	free(filename);
}
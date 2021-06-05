#include <stdio.h>
#include <string.h>
#include <math.h>
#include "basic.h"
#include "kTree.h"

typedef struct adyList{
	uint numNodes;
    int* listady;
	ulong numEdges;    
}ALREP;

ALREP * loadAdyacencyList(char * basename);
MREP * ktreeFromList(ALREP * list);
ALREP * listFromKtree(MREP * rep);
void destroyAdyacencyList(ALREP * list);
void saveAdyacencyList(ALREP * list, char * basename);
#include <stdio.h>
#include <string.h>
#include "Queue.h"
#include "misBits.h"
#include "kTree.h"

/*
	Implementado con la última versión del BitMap que desplaza para concatenar.
*/

void SkipNodes(uint l, MREP * X, ulong * pX, ulong s);
void fillin(int nlevel, misBits * C);
int preFillin(int nlevel, misBits * C, ulong * limits, uint numNodes, uint maxLevel);
void copy(uint l, MREP * rep, ulong * pX, uint s, misBits * C);
void limitsCalculator(int cuadrante, ulong * limitsOriginal, ulong * limitsToReturn);
uint complementOperation(uint l, MREP * rep, ulong * pA, misBits * C, ulong* limits);
uint differenceOperation(uint l, MREP * repA, MREP * repB, ulong * pA, ulong * pB, misBits * C);
uint intersectionOperation(uint l, MREP * A, MREP * B, ulong * pA, ulong * pB, misBits * C);
uint symmetricDifferenceOperation(uint l, MREP * repA, MREP * repB, ulong * pA, ulong * pB, misBits * C);
ulong unionOperation(MREP * repA, MREP * repB, misBits * C);
ulong * posByLevel(MREP * X);
ulong posLastLevel(misBits * bitsMaped);
MREP * createFromBitmap(misBits * C, int maximalLevel, ulong numNodos, ulong numEdges);
MREP * emptyKtree(uint nodes);
MREP * k2tree_union(MREP * repA, MREP * repB);
MREP * k2tree_symmetricdifference(MREP * repA, MREP * repB);
MREP * k2tree_intersection(MREP * repA, MREP * repB);
MREP * k2tree_difference(MREP * repA, MREP * repB);
MREP * k2tree_complement(MREP * repA);
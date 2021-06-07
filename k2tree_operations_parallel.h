#include "k2tree_operations.h"

ulong bitsUsados(MREP * X, ulong** pX, int subarbol, int nivel, int kk);
ulong ** minsResultadoParalelo(MREP * A, MREP * B, ulong ** pA, ulong ** pB);
ulong ** posByLevel_parallel(MREP * X);
//uint intersectionOperation_parallel(uint l, MREP * A, MREP * B, ulong * pA, ulong * pB, misBits * C);
MREP * k2tree_intersection_estrategia_parallel(MREP * repA, MREP * repB);

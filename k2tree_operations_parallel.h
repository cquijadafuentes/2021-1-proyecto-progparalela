#include "k2tree_operations.h"

ulong ** posByLevel_parallel(MREP * X);
uint intersectionOperation_parallel(uint l, MREP * A, MREP * B, ulong * pA, ulong * pB, misBits * C);
MREP * k2tree_intersection_parallel(MREP * repA, MREP * repB);
MREP * k2tree_intersection_estrategia_parallel(MREP * repA, MREP * repB);
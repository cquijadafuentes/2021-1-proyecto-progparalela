#!/bin/bash
#

echo 'Calculando intersección normal...'
../k2tree_setop_intersection $1 $2 inter_normal
echo 'Calculando intersección con estragia sin paralelizar...'
../k2tree_setop_intersection_estrategiaparalela $1 $2 $3 inter_paralela_fake
echo 'Calculando intersección en paralelo...'
../k2tree_setop_intersection_parallel $1 $2 $3 inter_paralela_real
echo 'Comparando archivos de la entrada'
cmp $1.kt $2.kt
echo 'Comparando archivos resultado'
cmp inter_normal.kt inter_paralela_real.kt
cmp inter_paralela_real.kt inter_paralela_fake.kt
rm inter_normal.kt inter_paralela_real.kt inter_paralela_fake.kt

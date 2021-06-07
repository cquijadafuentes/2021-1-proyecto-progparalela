#!/bin/bash
#

echo 'Calculando intersección normal...'
../k2tree_setop_intersection $1 $2 inter_normal
echo 'Calculando intersección paralela...'
../k2tree_setop_intersection_parallel $1 $2 inter_paralela
echo 'Comparando archivos de la entrada'
cmp $1.kt $2.kt
echo 'Comparando archivos resultado'
cmp inter_normal.kt inter_paralela.kt
rm inter_normal.kt inter_paralela.kt

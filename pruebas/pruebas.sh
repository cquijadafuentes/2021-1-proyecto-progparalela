#!/bin/bash
#

echo '---------------------------------------------'
echo '-------------Intersección normal-------------'
echo '---------------------------------------------'
../k2tree_setop_intersection $1 $2 inter_normal
echo '---------------------------------------------'
echo '------------Intersección paralela------------'
echo '---------------------------------------------'
../k2tree_setop_intersection_parallel $1 $2 inter_paralela
echo '---------------------------------------------'
echo '-------------Comparando archivos-------------'
echo '---------------------------------------------'
cmp $1.kt $2.kt
echo '---------------------------------------------'
echo '------------Comparando resultados------------'
echo '---------------------------------------------'
cmp inter_normal.kt inter_paralela.kt
rm inter_*

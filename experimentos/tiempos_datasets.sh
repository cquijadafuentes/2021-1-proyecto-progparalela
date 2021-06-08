#!/bin/bash

# Configuración inicial
DATA=/home/alumnos/caquijada/prog_parlela_proy_sem/datasets
DIRS="barabasi random smallworld snaps"
SUFFIXES="00 01 02 03 04 05 06 07 08 09 10 11"
CODE=/home/alumnos/caquijada/prog_parlela_proy_sem/codes

FILE=${1}

echo "Operación tradicional:" > ${FILE}
for d in $DIRS; do
    echo "dataset: ${d}" >> ${FILE}
    for suf1 in $SUFFIXES; do
        for suf2 in $SUFFIXES; do
            ${CODE}/k2tree_setop_intersection ${DATA}/${d}/${d}${suf1} ${DATA}/${d}/${d}${suf2} borrar >> ${FILE}
        done
        echo " " >> ${FILE}    
    done
    echo " " >> ${FILE}
done

echo "Operación estrategia sin paralelizar:" > ${FILE}
for d in $DIRS; do
    echo "dataset: ${d}" >> ${FILE}
    for suf1 in $SUFFIXES; do
        for suf2 in $SUFFIXES; do
            ${CODE}/k2tree_setop_intersection_estrategiaparalela ${DATA}/${d}/${d}${suf1} ${DATA}/${d}/${d}${suf2} borrar >> ${FILE}
        done
        echo " " >> ${FILE}    
    done
    echo " " >> ${FILE}
done

echo "Operación en paralelo:" > ${FILE}
for d in $DIRS; do
    echo "dataset: ${d}" >> ${FILE}
    for suf1 in $SUFFIXES; do
        for suf2 in $SUFFIXES; do
            ${CODE}/k2tree_setop_intersection_parallel ${DATA}/${d}/${d}${suf1} ${DATA}/${d}/${d}${suf2} borrar >> ${FILE}
        done
        echo " " >> ${FILE}    
    done
    echo " " >> ${FILE}
done

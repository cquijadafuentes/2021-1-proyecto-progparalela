#!/bin/bash

# Configuración inicial
BASE=/home/alumnos/caquijada/prog_parlela_proy_sem/datasets/snaps_esc
DIRS="1k 10k 100k 1m 10m 100m"
PREFIXES="snap"
SUFFIXES="00. 01. 02. 03. 04. 05. 06. 07. 08. 09. 10. 11."
PATH=$PATH:/home/alumnos/caquijada/prog_parlela_proy_sem/codes

FILE=$1

cd $BASE
echo "Operación tradicional:" > ${FILE}
for d in $DIRS; do
    cd ${BASE}/${d}
    echo "n: ${d}" >> ${FILE}
    for suf1 in $SUFFIXES; do
        for suf2 in $SUFFIXES; do
            ./k2tree_setop_intersection ${PREFIXES}${suf1}${d} ${PREFIXES}${suf2}${d} borrar >> ${FILE}
        done    
        echo " " >> ${FILE}
    done
done

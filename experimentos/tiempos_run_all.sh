#!/bin/bash

PREFIJO=${1}

./tiempos_datasets.sh ${PREFIJO}_datasets.txt
./tiempos_escalabilidad_random.sh ${PREFIJO}_esc_random.txt
./tiempos_escalabilidad_snaps.sh ${PREFIJO}_esc_snaps.txt

#!/bin/bash

echo "Subiendo datos:"
scp ./*.c* rafike:/home/alumnos/caquijada/prog_parlela_proy_sem/codes/
scp ./*.h* rafike:/home/alumnos/caquijada/prog_parlela_proy_sem/codes/
scp ./Makefile rafike:/home/alumnos/caquijada/prog_parlela_proy_sem/codes/
scp ./experimentos/*.sh rafike:/home/alumnos/caquijada/prog_parlela_proy_sem/codes/experimentos/

#include <stdio.h>
#include <stdlib.h>
#include "misBits.h"
#include "basic.h"

/*
	unsigned int * pos; // Posiciones de inicio para cada nivel del bitmap
	unsigned int * n; // Elementos de cada nivel del bitmap
	unsigned int niveles; // Cantidad de niveles del bitmap
	unsigned int tam; // Tamaño del bitmap
	unsigned int cant; // Elementos del bitmap
	unsigned int * bitsm;
*/

misBits* nuevoBitMap(uint levels, ulong * cants){

	misBits* bitses = (misBits *) malloc(sizeof(misBits));
	if(bitses == NULL){
		printf("Error en la reserva de memoria (Estructura misBits).\n");
		return NULL;
	}
	bitses->pos = (ulong*) malloc(sizeof(ulong)*levels);
	if(bitses->pos == NULL){
		printf("Error en la reserva de memoria (Posiciones misBits.\n");
		return NULL;
	}
	bitses->n = (ulong *) malloc(sizeof(ulong)*levels);
	if(bitses->n == NULL){
		printf("Error en la reserva de memoria (Niveles misBits).\n");
		return NULL;
	}
	bitses->niveles = levels;
	bitses->tam = 0;
	bitses->cant = 0;
	bitses->numEdges = 0;

	uint i;
	for(i=0; i< levels; i++){
		bitses->pos[i] = bitses->tam;
		bitses->n[i] = 0;
		bitses->tam += cants[i];
	}

	bitses->bitsm = (unsigned int *) malloc(sizeof(unsigned int) *(bitses->tam/W+1));
	if(bitses->bitsm == NULL){
		printf("Error en la reserva de memoria (bitses de misBits).\n");
		return NULL;
	}

	for(i=0; i<bitses->tam/W+1; i++){
		bitses->bitsm[i]=0;
	}

	return bitses;
}

void setBit(misBits* bitses, uint level, uint cont){
	if(bitses->tam == bitses->cant || level >= bitses->niveles){
		//Verifica que no esté lleno el bitmap 
		// o que no se haya enviado como parámetro un nivel inexistente
		return;
	}
	ulong posicion = bitses->pos[level] + bitses->n[level];
	if( (level < bitses->niveles-1 && posicion >= bitses->pos[level+1])
		|| (level == bitses->niveles-1 && posicion >= bitses->tam) ){
		//Verifica que los segmentos no sobrepasen sus límites 
		// (que no escriban en el siguiente nivel)
		return;
	}
	if(cont==1){		
		bitses->bitsm[posicion/W] = bitses->bitsm[posicion/W] | (1u << (posicion % W));	
		if(level == bitses->niveles-1){
			bitses->numEdges++;
		}
	}
	bitses->n[level]++;
	bitses->cant++;
}

uint isBitSeted(misBits* bitses, uint level, ulong i){
	if(level >= bitses->niveles){
		return 0u;
	}
	ulong posicion = bitses->pos[level] + i;
	if( (level < bitses->niveles-1 && posicion >= bitses->pos[level+1])
		|| (level == bitses->niveles-1 && posicion >= bitses->tam) ){
		//Verifica que los segmentos no sobrepasen sus límites 
		// (que no escriban en el siguiente nivel)
		return 0u;
	}
	if((1u << (posicion % W)) & bitses->bitsm[posicion/W]){
		return 1u;
	}
	return 0u;
}

void destruirBitMap(misBits* bitses){
	//free(bitses->bitsm);
	// bitsm se deja disponible para MREP * que se retorna de las operaciones
	free(bitses->pos);
	free(bitses->n);
	free(bitses);
	bitses=NULL;
}

ulong concatenar(misBits* bitses){
	ulong i, j, posicion;
	posicion = bitses->pos[0] + bitses->n[0];
	for(i=1; i < bitses->niveles; i++){
		for(j=0; j < bitses->n[i]; j++){
			if(isBitSeted(bitses, i, j)){
				bitses->bitsm[posicion/W] = bitses->bitsm[posicion/W] | (1u << (posicion % W));
			}else{
				bitses->bitsm[posicion/W] = bitses->bitsm[posicion/W] & ~(1u << (posicion % W));
			}
			posicion++;
		}
	}

	while((posicion-1)/W == posicion/W){
		bitses->bitsm[posicion/W] = bitses->bitsm[posicion/W] & ~(1u << (posicion % W));
		posicion++;
	}
	
	// Asegura que los bits finales de la representación sean 0.
	for(i=posicion/W; i<bitses->tam/W+1 && i<posicion/W+8; i++){
		bitses->bitsm[i] = 0;
	}

	bitses->pos = (ulong*) realloc(bitses->pos, sizeof(ulong));
	bitses->n = (ulong*) realloc(bitses->n, sizeof(ulong));
	bitses->pos[0]=0;
	bitses->n[0] = posicion;
	bitses->niveles = 1;
	bitses->bitsm = (uint *) realloc(bitses->bitsm, sizeof(uint) *(posicion/W+1));
	return bitses->numEdges;
}
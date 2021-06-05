#include <stdio.h>
#include "basic.h"

/*
**	Esta estructura permite crear un bitmap segmentado que puede ser completado
**  de manera independiente según nivel/segmento.
**  La concatenación reúne todos segmentos de bits en uno solo segmento, transformandolo a un nivel.
**  Esto evita crear un bitmap por nivel y posteriormente otro
**  capaz de contener los bits de todos los niveles. (Lo que duplicaría la información.)
*/

typedef struct MISBITS{
	ulong * pos; // Posiciones de inicio para cada nivel del bitmap
	ulong * n; // Número de elementos de cada nivel del bitmap
	uint niveles; // Cantidad de niveles del bitmap
	ulong tam; // Tamaño del bitmap (Capacidad)
	ulong cant; // Número de elementos totales del bitmap
	ulong numEdges; // Número de 1s al último nivel del bitmap
	uint * bitsm;
} misBits;

misBits* nuevoBitMap(uint levels, ulong * cants); 
//Crea una nueva instancia de misBits con <levels> niveles y <cants> cantidad de elementos por nivel

void setBit(misBits* bitses, uint level, uint cont); 
//Inserta el bit <cont> al final de la cadena según el nivel <level> en <bitses>.

uint isBitSeted(misBits* bitses, uint level, ulong i); 
//Retorna el contenido del i-ésimo bit (0 o 1) en el nivel <level>.

void destruirBitMap(misBits* bitses); 
//Destruye la estructura.

ulong concatenar(misBits* bitses); 
// Concatena los bits de todos los niveles en un bitmap de un nivel y lo retorna.
// Modifica los valores de level y tam.
// Libera pos y n
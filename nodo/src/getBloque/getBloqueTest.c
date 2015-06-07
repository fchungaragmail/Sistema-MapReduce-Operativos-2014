/*
 * getBloqueTest.c
 *
 *  Created on: 5/6/2015
 *      Author: daniel
 */
#include "getBloque.c"
#include <stdlib.h>
#include <stdio.h>
/*
 * 1:nombre del archivo de espacio de  datos 2Gb
 * 2:numero de bloque a leer
 */
int main(int argc, char **argv) {
	char* bloque = getBloque(argv[1], atoi(argv[2]));
	printf("%s\n", bloque);
}



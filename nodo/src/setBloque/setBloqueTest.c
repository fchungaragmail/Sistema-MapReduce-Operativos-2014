#include "setBloque.c"
#include <stdlib.h>
#include <string.h>
#define TAMANIO_BLOQUE 20971520 // 20mb
#include <stdio.h>
/*
 * 1:ruta del archivo de espacio de  datos 2Gb
 * 2:numero de bloque a escribir
 * 3:datos a escribir
 */
int main(int argc, char **argv) {
	char *datos = malloc(TAMANIO_BLOQUE);
	printf("%s\n",argv[3]);
	setBloque(argv[1], atoi(argv[2]), strcpy(datos, argv[3]));
}

/*
 * getBloque.c
 *
 *  Created on: 3/6/2015
 *      Author: daniel
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>


#define TAMANIO_BLOQUE 20971520 // 20mb
//Retorna el contenido(20Mb) del numeroBloque del espacioDatos

char* getBloque(char* espacioDatos,int numeroBloque) {

	//abro el espacio de datos para lectura
	int archivo = open(espacioDatos,O_RDONLY);

	//calculo offset
	long int offset = numeroBloque * TAMANIO_BLOQUE;

	//leo el contenido del bloque
	lseek(archivo, offset, SEEK_SET);
	char *contenido = malloc(TAMANIO_BLOQUE);
	read(archivo, contenido, TAMANIO_BLOQUE);

	return contenido;
}


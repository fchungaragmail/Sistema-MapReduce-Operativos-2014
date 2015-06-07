/*
 * escribirArchivo.c
 *
 *  Created on: 5/6/2015
 *      Author: daniel
 */
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
/*
 * 1:rutaArchivo
 * 2:palabra1
 * 3:palabra2
 * 4:palabra3
 * */
int main(int argc, char **argv) {
	int archivo = open(argv[1],O_RDWR);
	char *registros = malloc(100);
	sprintf(registros, "%s\n%s\n%s\n",argv[2],argv[3],argv[4]);
	write(archivo, registros, 100);
}


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
//recibe 1=nombre de archivo a mapear, 2=numero de bloque, 3=cantidad de bytes del bloque a mostrar

int main(int argc, char **argv) {

	//MAPEO CON ESCRITURA DE ARCHIVO
	//mapear 20mb en memoria
	int archivo = open(argv[1],O_CREAT|O_RDWR);
	long int numeroBloque = atol(argv[2]) * TAMANIO_BLOQUE;
	//parametros mmap(): 1=null, 2=tamaño a mapear, 3=operaciones permitidas sobre el mapeo, 4=si el mapeo es privado o compartido, 4=archivo, 5=desplazamiento desde el inicio del archivo
	void *bloque= mmap(NULL,  TAMANIO_BLOQUE,  PROT_READ, MAP_PRIVATE,  archivo,  numeroBloque);
	close(archivo);//mmap ya tiene la referencia
	//escribir sobre sobre un buffer la cantidad de bytes especificados
	int i;
	for (i = 0; i < atoi(argv[3]); ++i) {
		putchar(((char*)bloque)[i]);
	}
	puts("");

	//se va liberar memoria
	munmap(bloque, TAMANIO_BLOQUE);


	return 0;
}


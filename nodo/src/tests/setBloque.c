/*
 * mapeo.c
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
//recibe 1=nombre de archivo a mapear, 2=numero de bloque, 3=mensaje a escribir
int main(int argc, char **argv) {

	//MAPEO CON ESCRITURA DE ARCHIVO
	//mapear 20mb en memoria
	int archivo = open(argv[1],O_CREAT|O_RDWR);
	long int numeroBloque = atol(argv[2]) * TAMANIO_BLOQUE;
	//parametros mmap(): 1=null, 2=tamaño a mapear, 3=operaciones permitidas sobre el mapeo, 4=si el mapeo es privado o compartido, 4=archivo, 5=desplazamiento desde el inicio del archivo
	void *bloque= mmap(NULL,  TAMANIO_BLOQUE,  PROT_WRITE, MAP_SHARED,  archivo,  numeroBloque);
	//escribir sobre el mapeo el mensaje
	memcpy(bloque, argv[3], strlen(argv[3]));
	//se va actualizar el archivo
	munmap(bloque, TAMANIO_BLOQUE);

	//muestro la parte del bloque que se escribio
	char *msj = malloc(100);
	lseek(archivo, numeroBloque, SEEK_SET);
	read(archivo, msj, 100);
	printf("%s\n", msj);

	close(archivo);//mmap ya tiene la referencia


	return 0;
}

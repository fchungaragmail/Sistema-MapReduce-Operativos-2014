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
//Escribe en el numeroBloque del espacioDatos los datos enviados(deben ser de 20Mb)
int setBloque(char* espacioDatos,int numeroBloque, char* datos){

	//MAPEO CON ESCRITURA DE ARCHIVO
	int archivo = open(espacioDatos,O_RDWR);
	long int offset = numeroBloque * TAMANIO_BLOQUE;

	//parametros mmap(): 1=null, 2=tamaño a mapear, 3=operaciones permitidas sobre el mapeo, 4=si el mapeo es privado o compartido, 4=archivo, 5=desplazamiento desde el inicio del archivo
	void *bloque= mmap(NULL,  TAMANIO_BLOQUE,  PROT_WRITE, MAP_SHARED,  archivo,  offset);
	close(archivo);//mmap ya tiene una copia del fd

	//escribir sobre el mapeo el mensaje
	memcpy(bloque, datos, TAMANIO_BLOQUE);

	//se va actualizar el archivo
	munmap(bloque, TAMANIO_BLOQUE);


	/*
	//muestro la parte del bloque que se escribio
	archivo = open(espacioDatos, O_RDWR);
	char *msj = malloc(100);
	lseek(archivo, offset, SEEK_SET);
	read(archivo, msj, 100);
	printf("%s\n", msj);
	close(archivo);
	*/

	return 0;
}

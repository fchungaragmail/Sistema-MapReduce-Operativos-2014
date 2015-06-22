/*
 * aparearArchivos_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */


#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>

#define TAMANIO_BLOQUE 20971520


int aparearArchivos(t_list* listaArchivos) {

	t_list* elemento;
	int apareoArchivo;
	char* buffer;

	buffer = malloc(TAMANIO_BLOQUE);

	apareoArchivo = open("apareo.txt", O_APPEND);

	while (!list_is_empty(listaArchivos)) {
		int i = 0;
		elemento = list_get(listaArchivos, i);

		while(fgets(buffer, TAMANIO_BLOQUE, elemento->head->data /*->nombreArchivo*/) != NULL) {
			write(apareoArchivo, buffer, TAMANIO_BLOQUE);
		}

		i++;
	}

	close(apareoArchivo);



	return apareoArchivo;

}

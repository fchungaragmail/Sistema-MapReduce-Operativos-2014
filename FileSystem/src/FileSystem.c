/*
 ============================================================================
 Name        : FileSystem.c
 Author      : The ByteLess
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "estructuras.h"
#include "consola.h"
#include <commons/collections/list.h>
#include <commons/config.h>

void initFileSystem();

t_list* listaArchivos;
int PUERTO_LISTEN;
char** LISTA_NODOS;

int main(void) {
	pthread_t tConsola;

	initFileSystem();
	pthread_create(&tConsola, NULL, execConsola, NULL);

	pthread_join(tConsola,NULL);
	return EXIT_SUCCESS;
}

void initFileSystem()
{
	t_config* archivoConfig = config_create("./FileSistem.config");
	PUERTO_LISTEN = config_get_int_value(archivoConfig, "PUERTO_LISTEN");
	LISTA_NODOS = config_get_array_value(archivoConfig, "LISTA_NODOS");
	listaArchivos = list_create();
}

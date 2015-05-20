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
#include "consola.h"
#include "recursosCompartidos.h"

void initFileSystem();
void exitFileSystem();


int main(void) {
	pthread_t tConsola;

	initFileSystem();
	pthread_create(&tConsola, NULL, execConsola, NULL);

	pthread_join(tConsola,NULL);

	exitFileSystem();
	return EXIT_SUCCESS;
}

void initFileSystem()
{
	t_config* archivoConfig = config_create("./FileSystem.config");
	PUERTO_LISTEN = config_get_int_value(archivoConfig, "PUERTO_LISTEN");
	LISTA_NODOS = config_get_int_value(archivoConfig, "LISTA_NODOS");
	config_destroy(archivoConfig);
	listaArchivos = list_create();
	log = log_create("./FileSystem.log","FileSystem", true, LOG_LEVEL_TRACE);
}

void exitFileSystem()
{
	list_destroy(listaArchivos);
	log_destroy(log);
}

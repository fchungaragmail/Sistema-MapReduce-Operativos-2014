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
#include "conexiones.h"
#include "recursosCompartidos.h"

void initFileSystem();
void exitFileSystem();


int main(void) {
	pthread_t tConsola, tEscucharConexioes;

	initFileSystem();

	//Hasta que no tenga los nodos suficientes no sigue
	pthread_create(&tEscucharConexioes, NULL, escucharConexiones, NULL);
	pthread_join(tEscucharConexioes, NULL);

	pthread_create(&tConsola, NULL, execConsola, NULL);
	pthread_join(tConsola,NULL);

	exitFileSystem();
	return EXIT_SUCCESS;
}

void initFileSystem()
{

	log = log_create("./FileSystem.log","FileSystem", true, LOG_LEVEL_TRACE);
	t_config* archivoConfig = config_create("./FileSystem.config");
	PUERTO_LISTEN = config_get_int_value(archivoConfig, "PUERTO_LISTEN");
	char* tmp = config_get_string_value(archivoConfig, "IP_LISTEN");
	strcpy(IP_LISTEN,tmp);
	free(tmp);
	LISTA_NODOS = config_get_int_value(archivoConfig, "LISTA_NODOS");
	config_destroy(archivoConfig);

	listaArchivos = list_create();

	initConexiones();
}

void exitFileSystem()
{
	list_destroy(listaArchivos);
	log_destroy(log);
}

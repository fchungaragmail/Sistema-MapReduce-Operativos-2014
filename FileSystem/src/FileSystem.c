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
	pthread_t tConsola, tEscucharConexioesInicial, tEscucharConexioes,
			  tConectarMaRTA;

	initFileSystem();

	//Hasta que no tenga los nodos suficientes no sigue
	pthread_create(&tEscucharConexioesInicial, NULL, escucharConexiones, LISTA_NODOS);
	pthread_join(tEscucharConexioesInicial, NULL);


	pthread_create(&tConectarMaRTA, NULL, conectarMaRTA, NULL);
	pthread_create(&tEscucharConexioes, NULL, escucharConexiones, -1);
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

	PUERTO_MARTA = config_get_int_value(archivoConfig, "PUERTO_MARTA");
	tmp = config_get_string_value(archivoConfig, "IP_MARTA");
	strcpy(IP_MARTA,tmp);
	free(tmp);

	LISTA_NODOS = config_get_int_value(archivoConfig, "LISTA_NODOS");

	//config_destroy(archivoConfig);

	listaArchivos = list_create();


	initConexiones();
}

void exitFileSystem()
{
	list_destroy(listaArchivos);
	log_destroy(log);
}

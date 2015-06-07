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
#include "consola.h"
#include "conexiones.h"
#include "recursosCompartidos.h"

void initFileSystem();
void exitFileSystem();


int main(void) {
	pthread_t tConsola, tEscucharConexiones,
			  tConectarMaRTA, tLeerEntradas, tPrueba;

	initFileSystem();

	pthread_create(&tConsola, NULL, execConsola, NULL);

	pthread_create(&tPrueba, NULL, probarConexiones, NULL);

	pthread_create(&tEscucharConexiones, NULL, escucharConexiones, NULL);
	pthread_create(&tLeerEntradas, NULL, leerEntradas, NULL);

	pthread_join(tConsola,NULL);

	return 0;
}

void initFileSystem()
{
	log = log_create("./FileSystem.log","FileSystem", true, LOG_LEVEL_TRACE);


	t_config* archivoConfig = config_create("./FileSystem.config");

	PUERTO_LISTEN = config_get_int_value(archivoConfig, "PUERTO_LISTEN");
	char* tmp = config_get_string_value(archivoConfig, "IP_LISTEN");
	strcpy(IP_LISTEN,tmp);
	free(tmp);

	PUERTO_MARTA = config_get_int_value(archivoConfig, "PUERTO_MARTA");

	LISTA_NODOS = config_get_int_value(archivoConfig, "LISTA_NODOS");

	//config_destroy(archivoConfig);

	listaArchivos = list_create();

	listaDirs = list_create();
	t_reg_directorio* raiz = malloc(sizeof(t_reg_directorio));
	strcpy(raiz->directorio, "");
	raiz->padre = -1;
	list_add(listaDirs, raiz);

	initConsola();
	initConexiones();
}

void exitFileSystem()
{
	list_destroy(listaArchivos);
	log_destroy(log);
	//cerrarConexiones();
	exit(0);
}

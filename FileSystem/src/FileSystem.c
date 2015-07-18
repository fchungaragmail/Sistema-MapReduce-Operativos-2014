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
#include "persistencia.h"
#include "recursosCompartidos.h"

void initFileSystem();
void exitFileSystem();
FILE* logFile;
pthread_mutex_t mLogFile;

int main(void) {
	pthread_t tConsola, tEscucharConexiones,
			  tConectarMaRTA, tLeerEntradas, tPrueba, tPersistencia;

	initFileSystem();

	pthread_create(&tPersistencia, NULL, persistirEstructuras, NULL);

	pthread_create(&tConsola, NULL, execConsola, NULL);
	pthread_create(&tEscucharConexiones, NULL, escucharConexiones, NULL);
	pthread_create(&tLeerEntradas, NULL, leerEntradas, NULL);

	pthread_join(tConsola,NULL);

	return 0;
}

void initFileSystem()
{
	logFile = log_create("./FileSystem.log","FileSystem", true, LOG_LEVEL_TRACE);
	pthread_mutex_init(&mLogFile, NULL);

	t_config* archivoConfig = config_create("./FileSystem.config");

	PUERTO_LISTEN = config_get_int_value(archivoConfig, "PUERTO_LISTEN");
	char* tmp = config_get_string_value(archivoConfig, "IP_LISTEN");
	strcpy(IP_LISTEN,tmp);
	free(tmp);

	LISTA_NODOS = config_get_int_value(archivoConfig, "LISTA_NODOS");

	//config_destroy(archivoConfig);

	listaArchivos = list_create();
	pthread_mutex_init(&mListaArchivos, NULL);

	listaDirs = list_create();
	pthread_mutex_init(&mListaDirs, NULL);
	t_reg_directorio* raiz = malloc(sizeof(t_reg_directorio));
	strcpy(raiz->directorio, "\0");
	raiz->padre = -1;
	list_add(listaDirs, raiz);

	initConsola();
	initComandos();
	initConexiones();
	leerPersistencia();
}

void exitFileSystem()
{
	list_destroy(listaArchivos);
	log_destroy(logFile);
	cerrarConexiones();
	exit(0);
}

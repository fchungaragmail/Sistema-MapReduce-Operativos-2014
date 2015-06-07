/*
 * recursosCompartidos.c
 *
 *  Created on: 13/5/2015
 *      Author: utnso
 */

#include "recursosCompartidos.h"

t_list* listaArchivos;
t_list* listaDirs;
int PUERTO_LISTEN = -1;
char IP_LISTEN[16] = "127.0.0.1";
int PUERTO_MARTA = -1;
int LISTA_NODOS;
FILE* log;

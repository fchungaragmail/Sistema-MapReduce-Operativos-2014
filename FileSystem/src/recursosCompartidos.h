/*
 * recursosCompartidos.h
 *
 *  Created on: 13/5/2015
 *      Author: utnso
 */

#ifndef RECURSOSCOMPARTIDOS_H_
#define RECURSOSCOMPARTIDOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include "estructuras.h"

#define TAMANIO_BLOQUE 20971520

extern t_list* listaArchivos;
extern t_list* listaDirs;
extern int PUERTO_LISTEN;
extern char IP_LISTEN[16];
extern int LISTA_NODOS;
extern int PUERTO_MARTA;
extern FILE* log;
extern t_list* conexiones;
extern int nodosOnline;
extern pthread_mutex_t mListaArchivos;

struct _conexion {
	int sockfd;
	pthread_mutex_t mSocket;
	char nombre[22];
	int estado;
	bool estadoBloques[52]; //en uso? Todo por default False
	pthread_mutex_t mEstadoBloques;
};
typedef struct _conexion Conexion_t;


#endif /* RECURSOSCOMPARTIDOS_H_ */

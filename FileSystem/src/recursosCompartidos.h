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


extern t_list* listaArchivos;
extern t_list* listaDirs;
extern int PUERTO_LISTEN;
extern char IP_LISTEN[16];
extern int LISTA_NODOS;
extern int PUERTO_MARTA;
extern FILE* log;
extern t_list* conexiones;
extern int nodosOnline;

struct _conexion {
	int sockfd;
	char nombre[20];
	int estado;
};
typedef struct _conexion Conexion_t;


#endif /* RECURSOSCOMPARTIDOS_H_ */

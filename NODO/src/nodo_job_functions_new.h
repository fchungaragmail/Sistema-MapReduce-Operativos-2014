/*
 * nodo_job_functions_new.h
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#ifndef NODO_JOB_FUNCTIONS_NEW_H_
#define NODO_JOB_FUNCTIONS_NEW_H_


#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include "nodo_fs_functions_new.h"

typedef struct {
	int nodo;
	char* archivo;
} t_cont;


/*Ejecuta el script sobre el contenido del numeroBloque del espacioDatos, el resultado se almacena en archivoTemporal1
 * Luego ejecuta sort sobre archivoTemporal1 y lo almacenta en archivoTemporal2
 * */
int mapping(char *script, int numeroBloque, char *archivoTemporal1, char* archivoTemporal2);
int reduce(char *script, t_list* listArchivos, char *archivoTemporal);


#endif /* NODO_JOB_FUNCTIONS_NEW_H_ */

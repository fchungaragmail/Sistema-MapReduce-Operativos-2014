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
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "nodo_new.h"

typedef struct {
	int nodo;
	char* archivo;
} t_cont;



int mapping(char *script, int numeroBloque, char *archivoTemporal1, char* archivoTemporal2);
int reduce(char *script, char *archivosParaReduce, char *archivoTemporalFinal, char* ipNodoFallido);


#endif /* NODO_JOB_FUNCTIONS_NEW_H_ */

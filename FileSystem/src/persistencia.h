/*
 * persistencia.h
 *
 *  Created on: 9/7/2015
 *      Author: utnso
 */

#ifndef PERSISTENCIA_H_
#define PERSISTENCIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "recursosCompartidos.h"

#define SECCION_LISTA_ARCHIVOS "#LISTA_ARCHIVOS"
#define SECCION_LISTA_DIRS "#LISTA_DIRS"
#define SECCION_LISTA_CONEXIONES "#LISTA_CONEXIONES"
#define MAX_BUFF_SIZE 1024

int persistirEstructuras();
int leerPersistencia();

#endif /* PERSISTENCIA_H_ */

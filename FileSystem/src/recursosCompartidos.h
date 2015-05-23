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
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include "estructuras.h"


extern t_list* listaArchivos;
extern int PUERTO_LISTEN;
extern char IP_LISTEN[16];
extern int LISTA_NODOS;
extern int PUERTO_MARTA;
extern char IP_MARTA[16];
extern FILE* log;


#endif /* RECURSOSCOMPARTIDOS_H_ */

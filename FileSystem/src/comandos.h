/*
 * comandos.h
 *
 *  Created on: 14/5/2015
 *      Author: utnso
 */

#ifndef COMANDOS_H_
#define COMANDOS_H_

#include <pthread.h>
#include <commons/string.h>
#include "recursosCompartidos.h"

void procesarComando(char** comando, void(*doComando)(void*));



#endif /* COMANDOS_H_ */

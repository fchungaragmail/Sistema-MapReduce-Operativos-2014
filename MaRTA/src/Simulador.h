/*
 * Simulador.h
 *
 *  Created on: 6/6/2015
 *      Author: utnso
 */

#ifndef SIMULADOR_H_
#define SIMULADOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Utilities.h"
#include <commons/collections/dictionary.h>

//Simulador FS
Message* simulacion_FS_DataFullResponse();

//Simulador Job
Message *simulacion_Job_newFileToProcess();
Message *simulacion_Job_mapResponse();
Message *simulacion_Job_reduceResponse();

//Ambos
Message *simulacion_NewConnection(int sckt);
Message *simular();
#endif /* SIMULADOR_H_ */

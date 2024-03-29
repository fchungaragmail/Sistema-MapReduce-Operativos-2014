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
Message *simulacion_Job_newFileToProcess(char *cmd);
Message *simulacion_Job_mapResponse(int x);
Message *simulacion_Job_reduceResponse(char *tipo);
Message *simulacion_Job_reduceResponse_Fallo(char *tipo,char *ipFallo);

//Ambos
Message *simulacion_NewConnection(int sckt);
Message *simular();
#endif /* SIMULADOR_H_ */

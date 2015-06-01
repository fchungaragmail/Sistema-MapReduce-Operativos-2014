/*
 * variablesGlobales.h
 *
 *  Created on: 30/5/2015
 *      Author: utnso
 */

#ifndef VARIABLESGLOBALES_H_
#define VARIABLESGLOBALES_H_

#include <semaphore.h>
#include <commons/collections/dictionary.h>

extern t_dictionary *filesToProcess;
extern t_dictionary *filesStates;
extern t_dictionary *nodosData;

extern sem_t semFilesToProcess;
extern sem_t semFilesToProcessPerJob;
extern sem_t semFilesStates;
extern sem_t semNodosData;
extern sem_t semNodoState;

extern int MaxPedidosEnRed;
extern int pedidosEnRed;

//***************************************

extern sem_t leerConexiones;
extern sem_t semPrueba;

#endif /* VARIABLESGLOBALES_H_ */

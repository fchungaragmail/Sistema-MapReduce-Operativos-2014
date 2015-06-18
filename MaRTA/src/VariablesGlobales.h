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
extern t_dictionary *fullDataTables;

extern sem_t semFilesToProcess;
extern sem_t semFilesToProcessPerJob;
extern sem_t semFilesStates;
extern sem_t semNodosData;
extern sem_t semNodoState;
extern sem_t semFullDataTables;

#endif /* VARIABLESGLOBALES_H_ */

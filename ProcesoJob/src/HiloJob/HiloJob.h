/*
 * HiloJob.h
 *
 *  Created on: 6/7/2015
 *      Author: utnso
 */

#ifndef SRC_HILOJOB_HILOJOB_H_
#define SRC_HILOJOB_HILOJOB_H_

#include "../Definiciones.h"

pthread_t* CrearHiloJob(HiloJobInfo*, TipoHilo);
void* hiloJobHandler(void*);

#endif /* SRC_HILOJOB_HILOJOB_H_ */

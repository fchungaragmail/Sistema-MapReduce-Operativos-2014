/*
 * Utils.h
 *
 *  Created on: 5/7/2015
 *      Author: utnso
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include "Definiciones.h"

void FreeStringArray(char***);
void FreeMensaje(mensaje_t*);
mensaje_t* CreateMensaje(char*, char*);
void FreeHiloJobInfo(HiloJobInfo*);

#endif /* SRC_UTILS_H_ */

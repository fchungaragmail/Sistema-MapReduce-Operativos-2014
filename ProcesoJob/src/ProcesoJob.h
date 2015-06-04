/*
 * ProcesoJob.h
 *
 *  Created on: 17/5/2015
 *      Author: utnso
 */

#ifndef SRC_PROCESOJOB_H_
#define SRC_PROCESOJOB_H_

#include "Definiciones.h"


void IniciarConexionMarta();
void HacerPedidoMarta();
void IniciarConfiguracion();
void Terminar(int);


void* pedidosMartaHandler(void*);

#endif /* SRC_PROCESOJOB_H_ */

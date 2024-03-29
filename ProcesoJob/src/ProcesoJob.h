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
void LeerArchivo(char*, char**, int*);
void ReportarResultadoHilo(HiloJobInfo*, EstadoHilo);

/*
 *
 * PLANIFICACION DE HILOS
 *
 */
void PlanificarHilosMapper(mensaje_t*);
void PlanificarHilosReduce(mensaje_t*, int, char*, SubTipoHilo);
/*
 * HANDLERS
 */
void* pedidosMartaHandler(void*);

#endif /* SRC_PROCESOJOB_H_ */

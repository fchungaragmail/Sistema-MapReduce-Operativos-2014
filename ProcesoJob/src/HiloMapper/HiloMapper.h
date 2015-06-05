
#ifndef SRC_HILOMAPPER_HILOMAPPER_H_
#define SRC_HILOMAPPER_HILOMAPPER_H_

#include "../Definiciones.h"

#define NODO_ERROR_CONEXION -1
#define NODO_TERMINA_OK 1


pthread_t* CrearHiloMapper(HiloJob*);
void* hiloMapperHandler(void*);
void reportarResultadoHilo(HiloJob*, EstadoHilo);

#endif /* SRC_HILOMAPPER_HILOMAPPER_H_ */

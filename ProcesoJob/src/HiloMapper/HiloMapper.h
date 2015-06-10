
#ifndef SRC_HILOMAPPER_HILOMAPPER_H_
#define SRC_HILOMAPPER_HILOMAPPER_H_

#include "../Definiciones.h"

pthread_t* CrearHiloMapper(HiloJob*);
void* hiloMapperHandler(void*);

#endif /* SRC_HILOMAPPER_HILOMAPPER_H_ */


#ifndef SRC_HILOREDUCER_HILOREDUCER_H_
#define SRC_HILOREDUCER_HILOREDUCER_H_

#include "../Definiciones.h"

//TO DEFINE

pthread_t* CrearHiloReducer(HiloJob*);
void* hiloReducerHandler(void*);

#endif /* SRC_HILOREDUCER_HILOREDUCER_H_ */

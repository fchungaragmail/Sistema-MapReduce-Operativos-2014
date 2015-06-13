/*
 * estructuras.h
 *
 *  Created on: 2/5/2015
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

typedef enum {DISPONIBLE, NO_DISPONIBLE} t_estado;

typedef struct {
	char nodo[22];
	int16_t bloque;
} t_ubicacion_bloque;

typedef struct {
	char nombre[50];
	int64_t tamanio;
	int16_t dirPadre;
	t_estado estado;
	t_list* bloques; //A su vez, cada bloque tiene una lista de t_ubicacion_bloque
	pthread_mutex_t mBloques;
} t_reg_archivo;

typedef struct {
	char directorio[256];
	int padre;
} t_reg_directorio;

#endif /* ESTRUCTURAS_H_ */

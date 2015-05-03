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
	char nodo[5];
	unsigned int bloque;
} t_ubicacion;

typedef struct {
	unsigned long long int tamanio;
	unsigned short int dirPadre;
	t_estado estado;
	t_ubicacion** bloques;
} t_reg_archivo;

typedef struct {
	char directorio[256];
	int padre;
} t_reg_directorio;

#endif /* ESTRUCTURAS_H_ */

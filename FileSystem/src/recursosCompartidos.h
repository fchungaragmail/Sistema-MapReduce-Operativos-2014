/*
 * recursosCompartidos.h
 *
 *  Created on: 13/5/2015
 *      Author: utnso
 */

#ifndef RECURSOSCOMPARTIDOS_H_
#define RECURSOSCOMPARTIDOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <protocolo.h>
#include "estructuras.h"

#define TAMANIO_BLOQUE 20971520
#define MARTA "MaRTA"
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

extern t_list* listaArchivos;
extern pthread_mutex_t mListaArchivos;
extern t_list* listaDirs;
extern pthread_mutex_t mListaDirs;
extern int PUERTO_LISTEN;
extern char IP_LISTEN[16];
extern int LISTA_NODOS;
extern int PUERTO_MARTA;
extern FILE* logFile;
extern t_list* conexiones;
extern pthread_mutex_t mConexiones;
extern int nodosOnline;

struct _conexion {
	int sockfd;
	pthread_mutex_t mSocket;
	char nombre[22];
	int estado;
	int totalBloques;
	bool* estadoBloques; //en uso? Todos por default False
	pthread_mutex_t mEstadoBloques;
	sem_t respuestasP;//Semaforo para procesar respuestas
	sem_t respuestasR;//Semaforo para recibir respuestas
	int32_t respuestaSize;
	void* respuestaBuffer;
};
typedef struct _conexion Conexion_t;

typedef struct {
	Conexion_t* nodo;
	int16_t bloque;
} t_ubicacion_bloque;

typedef struct {
	mensaje_t* mensaje;
	Conexion_t* conexion;
} argumentos_t;


#endif /* RECURSOSCOMPARTIDOS_H_ */

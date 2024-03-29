/*
 * Definiciones.h
 *
 *  Created on: 18/5/2015
 *      Author: utnso
 */

#ifndef SRC_DEFINICIONES_H_
#define SRC_DEFINICIONES_H_

#include <netinet/in.h>
#include <pthread.h>
#include "Protocolo/protocolo.h"

#define EXIT_ERROR -1
#define EXIT_OK 0
#define FALSE 0
#define TRUE 1

//FLAG PARA BUILDEAR PARA TESTS, CON MENSAJES DE JOB Y NODO Y SLEEPS PARA SIMULACION
//#define BUILD_CON_MOCK_NODO
//#define BUILD_CON_MOCK_MARTA

#define MENSAJE_COMANDO 0
#define MENSAJE_COMANDO_NOMBREARCHIVO 1

typedef enum {
	ESTADO_HILO_NUEVO,
	ESTADO_HILO_FINALIZO_OK,
	ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION,
	ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO,
} EstadoHilo;

typedef enum {
	TIPO_HILO_MAPPER, TIPO_HILO_REDUCE
} TipoHilo;

typedef enum {
	SUBTIPO_MAPPER, SUBTIPO_REDUCE_SIN_COMBINER, SUBTIPO_REDUCE_CON_COMBINER_1, SUBTIPO_REDUCE_CON_COMBINER_2, SUBTIPO_REDUCE_CON_COMBINER_FINAL
} SubTipoHilo;


typedef struct {
	short int sin_family;
	unsigned short int sin_port;
	struct in_addr sin_addr;
	unsigned char sin_zero[8];
} Sockaddr_in;

typedef struct {
	Sockaddr_in direccionNodo;
	pthread_t* threadhilo;
	int socketFd;
	char* nombreArchivo;
	char* archivoOriginal;
	char* parametros;
	char* parametrosError;
	int nroBloque;
	TipoHilo tipoHilo;
	SubTipoHilo subTipoHilo;
} HiloJobInfo;

#endif /* SRC_DEFINICIONES_H_ */

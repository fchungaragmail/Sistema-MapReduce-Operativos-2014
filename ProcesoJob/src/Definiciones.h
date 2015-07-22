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
#define FALLO_EN_NODO_JOB_MUERTO

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
	char* parametros;
	char* parametrosError;
	int nroBloque;
	TipoHilo tipoHilo;
} HiloJobInfo;

#endif /* SRC_DEFINICIONES_H_ */

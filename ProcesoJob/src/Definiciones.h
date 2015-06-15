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
#include "Protocolo/protocolo.h" //TODO

#define EXIT_ERROR -1
#define EXIT_OK 1
#define FALSE 0
#define TRUE 1

#define BUILD_CON_MOCK 0

#define MENSAJE_COMANDO 0
#define MENSAJE_COMANDO_NOMBREARCHIVO 1


typedef enum {
	ESTADO_HILO_NUEVO,
	ESTADO_HILO_FINALIZO_OK,
	ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION,
	ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO,
} EstadoHilo;


typedef struct {
	short int sin_family;
	unsigned short int sin_port;
	struct in_addr sin_addr;
	unsigned char sin_zero[8];
} Sockaddr_in ;

typedef struct {
	Sockaddr_in direccionNodo;
	pthread_t* threadhilo;
	int socketFd;
	char* nombreArchivo;
	int nroBloque;

} HiloJob ;

#endif /* SRC_DEFINICIONES_H_ */

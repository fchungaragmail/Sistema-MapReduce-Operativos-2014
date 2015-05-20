/*
 * conexiones.h
 *
 *  Created on: 19/5/2015
 *      Author: utnso
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include "recursosCompartidos.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NODOS_MAX 20

struct _sockaddr_in {
	short int sin_family;  // familia de direcciones, AF_INET
	unsigned short int sin_port;    // Número de puerto
	struct in_addr sin_addr;    // Dirección de Internet
	unsigned char sin_zero[8]; // Relleno para preservar el tamaño original de struct sockaddr
};
typedef struct _sockaddr_in Sockaddr_in;

struct _conexion {
	int sockfd;
	Sockaddr_in sockaddr_in;
};
typedef struct _conexion Conexion;

int initConexiones();
void escucharConexiones();

#endif /* CONEXIONES_H_ */

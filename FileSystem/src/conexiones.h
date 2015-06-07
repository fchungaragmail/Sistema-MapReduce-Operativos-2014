/*
 * conexiones.h
 *
 *  Created on: 19/5/2015
 *      Author: utnso
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include "recursosCompartidos.h"
#include <protocolo.h>
#include "comandos.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define NODOS_MAX 20

int initConexiones();
void escucharConexiones(int nodosMax); //Si le mandas -1 sigue buscando
void leerEntradas();
void cerrarConexiones();
void probarConexiones();

extern int nodosOnline;

struct _sockaddr_in {
	short int sin_family;  // familia de direcciones, AF_INET
	unsigned short int sin_port;    // Número de puerto
	struct in_addr sin_addr;    // Dirección de Internet
	unsigned char sin_zero[8]; // Relleno para preservar el tamaño original de struct sockaddr
};
typedef struct _sockaddr_in Sockaddr_in;



#endif /* CONEXIONES_H_ */

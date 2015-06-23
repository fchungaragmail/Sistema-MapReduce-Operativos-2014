/*
 * sockets.h
 *
 *  Created on: 27/05/2014
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdint.h>

#define SOCKET_ERROR -1
#define CONECCIONES_PENDIENTES 1024//PARA MANEJO DE LISTEN
struct t_conection{
	char ip[15];
	int32_t puerto;
}t_conection;

int32_t new_connection(char *ip, int puerto);
int32_t aceptar_cliente (int descriptor);
int32_t abrir_servidor (int puerto);

#endif /* SOCKETS_H_ */

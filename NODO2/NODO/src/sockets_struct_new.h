/*
 * sockets_struct_new.h
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#ifndef SOCKETS_STRUCT_NEW_H_
#define SOCKETS_STRUCT_NEW_H_


#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "protocolo_new.h"
#include <commons/config.h>
#include <commons/string.h>


struct _sockaddr_in {
	short int sin_family;  // familia de direcciones, AF_INET
	unsigned short int sin_port;    // Número de puerto
	struct in_addr sin_addr;    // Dirección de Internet
	unsigned char sin_zero[8]; // Relleno para preservar el tamaño original de struct sockaddr
};
typedef struct _sockaddr_in Sockaddr_in;



#endif /* SOCKETS_STRUCT_NEW_H_ */

/*
 * sockets_struct.h
 *
 *  Created on: 30/5/2015
 *      Author: utnso
 */

#ifndef SOCKETS_STRUCT_H_
#define SOCKETS_STRUCT_H_

#define PORT 8000
#define MESSAGE_LENGTH 256
#define BACKLOG 5
#define MESSAGE_PROTOCOL_LENGTH 2


struct _sockaddr_in {
	short int sin_family;  // familia de direcciones, AF_INET
	unsigned short int sin_port;    // Número de puerto
	struct in_addr sin_addr;    // Dirección de Internet
	unsigned char sin_zero[8]; // Relleno para preservar el tamaño original de struct sockaddr
};
typedef struct _sockaddr_in Sockaddr_in;


struct fs_info {

};





#endif /* SOCKETS_STRUCT_H_ */

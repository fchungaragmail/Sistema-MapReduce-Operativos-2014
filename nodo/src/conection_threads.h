/*
 * conection_threads.h
 *
 *  Created on: 30/5/2015
 *      Author: utnso
 */

/* #ifndef CONECTION_THREADS_H_
#define CONECTION_THREADS_H_


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "sockets_struct.h"



int initServer() {

	int sockFD;
	Sockaddr_in my_sock;

	my_sock.sin_family = AF_INET;
	my_sock.sin_port = htons(PORT);
	memset(&my_sock.sin_zero, 0, 8);
	inet_aton("127.0.0.1", &my_sock.sin_addr);

	sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFD == -1) {
		printf("Error al crear socket\n");
		exit(-1);
	} else {
		printf("Socket creado correctamente...\n");
	}

	if(bind(sockFD, (struct sockaddr*) &my_sock, sizeof(my_sock)) == -1) {
		printf("Fallo al hacer el bind...\n");
		exit(-1);
	} else {
		printf("Bind realizado con exito...\n");
	}

	if(listen(sockFD, 5) == -1) {
		printf("Fallo en listen\n");
	} else {
		printf("Socket escuchando conexiones...\n");
	}


	return sockFD;

} */





//#endif /* CONECTION_THREADS_H_ */

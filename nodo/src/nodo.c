/*
 * nodo.c
 *
 *  Created on: 17/5/2015
 *      Author: utnso
 */

#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define PORT 7500
#define MSJ_LENGTH 256

void* conectionHandler(void*);

int main() {

	int conectionListener;
	int comunicationSocket;
	int *auxSocket;
	socklen_t length;
	struct sockaddr_in sock_in, sock_out;


	sock_in.sin_family = AF_INET;
	sock_in.sin_port = htons(PORT);
	sock_in.sin_addr.s_addr = htonl(INADDR_ANY);

	conectionListener = socket(AF_INET, SOCK_STREAM, 0);

	if (conectionListener == -1) {
		printf("Fallo en creación de socket \n");
		return -1;
	} else {
		printf("Socket creado con exito...\n");
	}


	if (bind(conectionListener, (struct sockaddr*) &sock_in, sizeof(struct sockaddr)) < 0) {
		printf("No se pudo hacer Bind...\n");
		return -1;
	} else {
		printf("Bind exitoso...\n");
	}


	//escucho conecciones entrantes, hasta 2 a la vez, verifico que sea menor o igual a 2 la cant de conecciones
	if (listen(conectionListener, 2) == -1) {
		printf("Sobrecarga del servidor \n");
		return -1;
	} else {
		printf("Escuchando conecciones\n");
	}

	length = sizeof(struct sockaddr);

	//acepto conecciones
	comunicationSocket = accept(conectionListener, (struct sockaddr*) &sock_out, &length);
	if (comunicationSocket != 1) {
		printf("No se puede aceptar conección\n");
	}

	while (comunicationSocket) {

		printf("Conección aceptada\n");

		pthread_t conectionThread;
		auxSocket = malloc(sizeof(comunicationSocket));
		*auxSocket = comunicationSocket;

		pthread_create(&conectionThread, NULL, conectionHandler, (void*) &auxSocket );

		pthread_join(conectionThread, NULL);

	}

	return 0;

}


void* conectionHandler(void* sockAux) {

	int sock = * (int*) sockAux;
	char* message;

	message = "Hello client, I'm your server";

	write(sock, message, sizeof(message));

	close(sock);

}


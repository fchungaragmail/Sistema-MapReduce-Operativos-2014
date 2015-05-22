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

#define PORT 7500
#define MSJ_LENGTH 256


int main() {

	int conectionListener;   //crear hilos, uno escucha al filesystem, el resto esperan map o reduce
	int comunicationSocket;
	char buffer[140];
	struct sockaddr_in sock_in, sock_out;

	sock_in.sin_family = AF_INET;
	sock_in.sin_port = htons(PORT);
	sock_in.sin_addr.s_addr = htonl(INADDR_ANY);

	conectionListener = socket(AF_INET, SOCK_STREAM, 0);

	if (conectionListener == -1) {
		printf("Fallo en creación de socket \n");
		return -1;
	}

	memset(&conectionListener, '0', sizeof(conectionListener));  //limpio el contenido
	memset(buffer, '0', sizeof(buffer));

	bind(conectionListener, (struct sockaddr*) &sock_in, sizeof(sock_in));


	//escucho conecciones entrantes, hasta 2 a la vez, verifico que sea menor o igual a 2 la cant de conecciones
	if (listen(conectionListener, 2) == -1) {
		printf("Sobrecarga del servidor \n");
		return -1;
	}


	//acepto conecciones
	while(1) {

	comunicationSocket = accept(conectionListener, (struct sockaddr*) &sock_out, NULL);

	strcpy(buffer, "Hola, bienvenido al servidor que sirve");
	send(comunicationSocket, buffer, sizeof(buffer), 0);
	close(comunicationSocket);

	}








	return 0;
}


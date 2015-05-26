/*
 * nodoCliente.c
 *
 *  Created on: 21/5/2015
 *      Author: utnso
 */

#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>


#define PORT 9000
#define MESSAGE_LENGTH 256


int main() {

	int socketFD;
	struct sockaddr_in my_addr;
	char buffer[MESSAGE_LENGTH];


//Obtengo FD del socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD == -1) {
		printf("Fallo en crear el socket\n");
	} else {
		printf("Socket creado correctamente\n");
	}


	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);


//Solicito conexión
	if (connect(socketFD, (struct sockaddr*) &my_addr, sizeof(my_addr)) == -1) {
		printf("Fallo en establecer conexión\n");
	} else {
		printf("Conexión establecida...\n");
	}


//Espero por recibir mensaje
	recv(socketFD, buffer, MESSAGE_LENGTH, 0);

	printf("%s", buffer);

	close(socketFD);





	return 0;
}

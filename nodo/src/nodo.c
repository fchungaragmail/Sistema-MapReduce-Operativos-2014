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


#define PORT 8000
#define BACKLOG 5
#define MESSAGE_LENGTH 256


int main() {

	int sockListener;
	int sockAccept;
	socklen_t size;
	struct sockaddr_in my_sock;
	struct sockaddr_in client_sock;
	pthread_t listener_thread;  //este hilo se va a encargar de escuchar
	pthread_t comunication_thread; //este hilo acepta y se comunica
	char buffer[MESSAGE_LENGTH];

//Establezo datos de estructura de my_sock
	my_sock.sin_family = AF_INET;
	inet_aton("127.0.0.1", &my_sock.sin_addr);
	my_sock.sin_port = htons(PORT);
	bzero(&(my_sock.sin_zero), 8);

//Creo socket que va a escuchar en la IP establecida anteriormente, en este caso en cualquiera
	sockListener = socket(AF_INET, SOCK_STREAM, 0);
	if (sockListener == -1) {
		printf("Fallo al crear el socket\n");
		return -1;
	} else {
		printf("Socket creado con exito\n");
	}

//Enlazo el socket al puerto para que escuche por ese puerto en la IP que haya quedado definida
	if (bind(sockListener, (struct sockaddr*) &my_sock, sizeof(my_sock)) < 0) {
		printf("Fallo al hacer el bind\n");
		close(sockListener);
		return -1;
	} else {
		printf("Bind hecho exitosamente\n");
	}

//Pongo a escuchar al socket, hasta un maximo de 10 msj
	if (listen(sockListener, 2) == -1) {
		printf("Sobrecarga de servidor\n");
		close(sockListener);
		return -1;
	} else {
		printf("Escuchando conexiones...\n");
	}


	size = sizeof(struct sockaddr_in);

//Acepto conexiones
	sockAccept = accept(sockListener, (struct sockaddr*) &client_sock, &size);
	if (sockAccept != 1) {
		printf("No se pudo aceptar conexión\n");
		close(sockListener);
		close(sockAccept);
		return -1;
	}

//Obtengo la IP del cliente que se conectó
	printf("Se obtuvo una conexión desde: %s\n", inet_ntoa(client_sock.sin_addr));

//Envio mensaje al cliente
	strcpy(buffer, "Bienvenido al servidor que sirve\n");
	send(sockAccept, &buffer, MESSAGE_LENGTH, 0);

	close(sockAccept);









	return 0;
}


/*
 * interfaz-NodoFS.c
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#include "interfaz-NodoFS.h"


mensaje_t* recibir(int socket);

mensaje_t* recibir(int socket){
	mensaje_t* mensaje = malloc(sizeof(mensaje_t));

	recv(socket, mensaje->comandoSize, sizeof(int),0);
	mensaje->comando = malloc(mensaje->comandoSize);
	recv(socket, mensaje->comando, mensaje->comandoSize,0);

	recv(socket, mensaje->dataSize, sizeof(long),0);
	mensaje->data = malloc(mensaje->dataSize);
	recv(socket, mensaje->data, mensaje->dataSize,0);

	return mensaje;
}

void enviar(int socket, mensaje_t* mensaje)
{
	send(socket, mensaje->comandoSize, sizeof(int), 0);
	send(socket, mensaje->comando, mensaje->comandoSize, 0);
	send(socket, mensaje->dataSize, sizeof(long), 0);
	send(socket, mensaje->data, mensaje->dataSize, 0);
}

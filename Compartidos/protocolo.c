/*
 * interfaz-NodoFS.c
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#include <protocolo.h>


int recibir(int socket, mensaje_t* mensaje);
void enviar(int socket, mensaje_t* mensaje);

int recibir(int socket, mensaje_t* mensaje){

	int primerRecv = 0;

	primerRecv = recv(socket, &(mensaje->comandoSize), sizeof(int16_t),0);
	if (primerRecv == 0) return DESCONECTADO;

	mensaje->comando = malloc(mensaje->comandoSize);
	if (mensaje->comandoSize != 0)
		recv(socket, mensaje->comando, mensaje->comandoSize,0);

	recv(socket, &(mensaje->dataSize), sizeof(int32_t),0);
	mensaje->data = malloc(mensaje->dataSize);
	if (mensaje->dataSize != 0)
		recv(socket, mensaje->data, mensaje->dataSize,0);

	return CONECTADO;
}

void enviar(int socket, mensaje_t* mensaje)
{
	send(socket, &(mensaje->comandoSize), sizeof(int16_t), 0);
	send(socket, mensaje->comando, mensaje->comandoSize, 0);
	send(socket, &(mensaje->dataSize), sizeof(int32_t), 0);
	send(socket, mensaje->data, mensaje->dataSize, 0);
}

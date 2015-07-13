/*
 * protocolo.c
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#include "protocolo.h"


int recibir(int socket, mensaje_t* mensaje);
void enviar(int socket, mensaje_t* mensaje);

int recibir(int socket, mensaje_t* mensaje){

	int estado = 0;

	uint16_t net_comando;
	uint32_t net_data;

	estado = recv(socket, &net_comando, 0/*sizeof(uint16_t)*/,0);
	if (estado == 0) return DESCONECTADO;

	mensaje->comandoSize = ntohs(net_comando);

	mensaje->comando = malloc(mensaje->comandoSize);
	if (mensaje->comandoSize != 0)
		recv(socket, mensaje->comando, mensaje->comandoSize,0);

	mensaje->dataSize = 0;
	recv(socket, &net_data, sizeof(uint32_t),0);

	mensaje->dataSize = ntohl(net_data);

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

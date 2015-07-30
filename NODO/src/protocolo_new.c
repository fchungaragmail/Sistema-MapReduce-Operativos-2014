/*
 * protocolo_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#include "protocolo_new.h"



int recibir(int socket, mensaje_t* mensaje){

	int primerRecv = 0;

	primerRecv = recv(socket, &(mensaje->comandoSize), sizeof(int16_t),MSG_WAITALL);
	if (primerRecv == 0) return DESCONECTADO;

	mensaje->comando = malloc(mensaje->comandoSize);
	if (mensaje->comandoSize != 0)
		recv(socket, mensaje->comando, mensaje->comandoSize,MSG_WAITALL);

	recv(socket, &(mensaje->dataSize), sizeof(int32_t),MSG_WAITALL);
	mensaje->data = malloc(mensaje->dataSize);
	/*
	if (mensaje->dataSize != 0)
		recv(socket, mensaje->data, mensaje->dataSize,0);
	*/
	int recibido = 0;
	int tempRecibido = 0;
	if (mensaje->dataSize != 0){
		while(!(recibido == mensaje->dataSize)){
		   tempRecibido = recv(socket, mensaje->data + recibido, mensaje->dataSize - recibido, MSG_WAITALL);
		   if ( tempRecibido <= 0){
			   return DESCONECTADO;
		   }
		   recibido +=tempRecibido;
		}
		//printf("data recibido %d\n", recibido);
		//printf("datos: %s\n", mensaje->data);

	}
	return CONECTADO;
}



int enviar(int socket, mensaje_t* mensaje)
{
	send(socket, &(mensaje->comandoSize), sizeof(int16_t), 0);
	send(socket, mensaje->comando, mensaje->comandoSize, 0);
	send(socket, &(mensaje->dataSize), sizeof(int32_t), 0);

	//send(socket, mensaje->data, mensaje->dataSize, 0);
	int enviado = 0;
	int tempEnviado = 0;
	while(!(enviado == mensaje->dataSize)){
		tempEnviado= send(socket, mensaje->data + enviado , mensaje->dataSize - enviado, 0 );
		if (tempEnviado <= 0) {
			return DESCONECTADO;
		}
		enviado += tempEnviado;
	}
	return CONECTADO;

}

/*
 * protocolo_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#include "protocolo_new.h"



int recibir(int socket, mensaje_t* mensaje){

	int primerRecv = 0;

	primerRecv = recv(socket, &(mensaje->comandoSize), sizeof(int16_t),0);
	if (primerRecv == 0) return DESCONECTADO;

	mensaje->comando = malloc(mensaje->comandoSize);
	if (mensaje->comandoSize != 0)
		recv(socket, mensaje->comando, mensaje->comandoSize,0);

	recv(socket, &(mensaje->dataSize), sizeof(int32_t),0);
	mensaje->data = malloc(mensaje->dataSize);
	/*
	if (mensaje->dataSize != 0)
		recv(socket, mensaje->data, mensaje->dataSize,0);
	*/
	int recibido = 0;
	if (mensaje->dataSize != 0){
		while(!(recibido == mensaje->dataSize)){
		   recibido += recv(socket, mensaje->data + recibido, mensaje->dataSize - recibido, 0);
		   if ( recibido == -1){
			   return DESCONECTADO;
		   }
		}
		//printf("data recibido %d\n", recibido);
		//printf("datos: %s\n", mensaje->data);

	}
	return CONECTADO;
}



void enviar(int socket, mensaje_t* mensaje)
{
	send(socket, &(mensaje->comandoSize), sizeof(int16_t), 0);
	send(socket, mensaje->comando, mensaje->comandoSize, 0);
	send(socket, &(mensaje->dataSize), sizeof(int32_t), 0);

	//send(socket, mensaje->data, mensaje->dataSize, 0);
	int enviado = 0;
	while(!(enviado == mensaje->dataSize)){
		enviado += send(socket, mensaje->data + enviado , mensaje->dataSize - enviado, 0 );
	}

}

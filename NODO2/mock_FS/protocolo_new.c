/*
 * protocolo_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#include "protocolo_new.h"
#include "socketsFunciones/LecturaYEscritura.h"


int recibir(int socket, mensaje_t* mensaje){
	int recibido = 0;
	int primerRecv = 0;

	primerRecv = recv(socket, &(mensaje->comandoSize), sizeof(int16_t),0);
	if (primerRecv == 0) return DESCONECTADO;

	mensaje->comando = malloc(mensaje->comandoSize);
	if (mensaje->comandoSize != 0)
		recibido = recv(socket, mensaje->comando, mensaje->comandoSize,0);

	recv(socket, &(mensaje->dataSize), sizeof(int32_t),0);
	mensaje->data = malloc(mensaje->dataSize);

	recibido = 0;
	if (mensaje->dataSize != 0){
		while(!(recibido == mensaje->dataSize)){
		   recibido += recv(socket, mensaje->data + recibido, mensaje->dataSize - recibido, 0);
		}
		printf("data recibido %d\n", recibido);
		//printf("datos: %s\n", mensaje->data);

	}

	return CONECTADO;

}


#include <stdio.h>
#include <string.h>

void enviar(int socket, mensaje_t* mensaje)
{

	send(socket, &(mensaje->comandoSize), sizeof(int16_t), 0);
	send(socket, mensaje->comando, mensaje->comandoSize, 0);
	send(socket, &(mensaje->dataSize), sizeof(int32_t), 0);
	//send(socket, mensaje->data, mensaje->dataSize, 0);
	//para enviar grades volumenes de datos los envio de a 1024 bytes

	/*
	int enviado = 0;
	while(!(enviado == mensaje->dataSize)){
		enviado += send(socket, mensaje->data + enviado , 1024, 0 );
		//printf("enviado %d\n", enviado);
	}
	*/

	int enviado = 0;
	while(!(enviado == mensaje->dataSize)){
		enviado += send(socket, mensaje->data + enviado , mensaje->dataSize - enviado, 0 );
	}
	printf("data enviado %d\n", enviado);


	//printf("enviado %d == 20971520\n", enviado);
	//printf("datos: %s\n", mensaje->data);

}

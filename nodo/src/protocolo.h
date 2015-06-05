/*
 * protocolo.h
 *
 *  Created on: 12/5/2015
 *      Author: utnso
 */

#ifndef COMPARTIDOS_PROTOCOLO_H_
#define COMPARTIDOS_PROTOCOLO_H_

//El FileSystem u otro proceso Nodo podrán solicitar las siguientes operaciones:

//- getBloque(numero) ​
//:   Devolverá   el   contenido   del   bloque   solicitado   almacenado   en   el
//Espacio de Datos.

//- setBloque(numero,   [datos]) ​
//:   Grabará   los   datos   enviados   en   el   bloque   solicitado   del
//Espacio de Datos

// getFileContent(nombre) ​
//:   Devolverá   el   contenido   del   archivo   de   Espacio   Temporal
//solicitado.

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>


typedef struct mensaje
{
	int16_t comandoSize;
	char* comando;
	int32_t dataSize; 	//Pongo int32_t asi entra el valor correspondiente a 20mb
	char* data;
} mensaje_t;


#define CONECTADO 0
#define DESCONECTADO -1

//Es necesario que el malloc para mensaje ya este hecho
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



#endif /* COMPARTIDOS_PROTOCOLO_H_ */

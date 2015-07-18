/*
 * protocolo_new.h
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#ifndef PROTOCOLO_NEW_H_
#define PROTOCOLO_NEW_H_

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
//	char id[2];
	int16_t comandoSize;
	char* comando;
	int32_t dataSize; 	//Pongo int32_t asi entra el valor correspondiente a 20mb
	char* data;
} mensaje_t;


#define CONECTADO 0
#define DESCONECTADO -1

//Es necesario que el malloc para mensaje ya este hecho
int recibir(int socket, mensaje_t* mensaje);
int enviar(int socket, mensaje_t* mensaje);



#endif /* PROTOCOLO_NEW_H_ */
